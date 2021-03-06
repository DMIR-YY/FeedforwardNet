// caffe_converter.cpp : ¶šÒå¿ØÖÆÌšÓŠÓÃ³ÌÐòµÄÈë¿Úµã¡£

#include <iostream>
#include <vector>
#include <fstream>
#include <memory>
#include <ctime>
#include <limits>
#include <math.h>
#include "caffe.pb.h"
#include "layer_factory_impl.h"
//#ifdef _MSC_VER
//#define _NOMINMAX
//#include <io.h>
//#include <fcntl.h>
//#define CNN_OPEN_BINARY(filename) open(filename, _O_RDONLY|_O_BINARY)
//#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define CNN_OPEN_BINARY(filename) open(filename, O_RDONLY)
#define CNN_OPEN_TXT(filename) open(filename, O_RDONLY)
//#endif
using namespace std;
using namespace caffe;
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/repeated_field.h>

void load(const caffe::LayerParameter& src,int num_input,int num_output,int kernel_size) {
    int src_idx = 0;
    ofstream out;
    out.open("net_weights.txt",ios::app);
    out<<"weights: "<<endl;
    //load weight
    for (int o = 0; o < num_output; o++) {
        for (int i = 0; i < num_input; i++) {
            for (int x = 0; x < kernel_size * kernel_size; x++) {
                out<<src.blobs(0).data(src_idx++)<<" ";
            }
        }
    }
    out<<""<<endl;
        if(src.convolution_param().bias_term()==false){
            
        }else{
            out<<"bias: "<<endl;
            //load bias
            for (int o = 0; o < num_output; o++) {
                out<<src.blobs(1).data(o)<<" ";
            }
            out<<""<<endl;
        }
    out.close();
}

void reload_weight_from_caffe_net(const caffe::NetParameter& layer,int input_param[])
{
    caffe_layer_vector src_net(layer);
    int num_layers = src_net.size();
    int num_input=input_param[0];
    int input_size=input_param[1];
    int num_output=0;
    for (int i = 0; i < src_net.size(); i++) {
        int pad=0;
        int kernel_size=0;
        int stride=1;
    // name，type，kernel size，pad，stride
        if(src_net[i].type()=="Convolution"||src_net[i].type()=="ConvolutionRistretto"){//get conv_layers' kernel_size,num_output
            ConvolutionParameter conv_param = src_net[i].convolution_param();
            num_output=conv_param.num_output();
            if (conv_param.pad_size()>0){
                pad=conv_param.pad(0);
            }
            //google::protobuf::RepeatedField<string> repeated_field;
            //repeated_field.size();
            kernel_size=conv_param.kernel_size(0);
            if (conv_param.stride_size()>0){
                stride=conv_param.stride(0);
            }
            input_size = (input_size + 2 * pad - kernel_size) / stride + 1;
            num_input=num_input/conv_param.group();
        }else if(src_net[i].type()=="InnerProduct"){//get fc_layers' kernel_size,num_output
            InnerProductParameter inner_product_param = src_net[i].inner_product_param();
            kernel_size=input_size;
            num_output=inner_product_param.num_output();
            input_size=1;
        }else if(src_net[i].type()=="Pooling"){//get pooling_layers' kernel_size,num_output
            PoolingParameter pooling_param = src_net[i].pooling_param();
            pad=pooling_param.pad();
            kernel_size=pooling_param.kernel_size();
            stride=pooling_param.stride();
            input_size = static_cast<int>(ceil(static_cast<float>(input_size + 2 * pad - kernel_size) / stride)) + 1;
        }

        if(src_net[i].type()=="Convolution"||src_net[i].type()=="InnerProduct"){
            load(src_net[i],num_input,num_output,kernel_size);
        }
        if(src_net[i].type()=="Convolution"||src_net[i].type()=="InnerProduct"||src_net[i].type()=="Pooling"){
            num_input=num_output;//set each layer's num_input equals to the last layer's num_output
        }
    }
}

void get_config_params_from_caffe_net(const caffe::NetParameter& layer,int input_param[],int net_has_eltwise_layer[])
{
    caffe_layer_vector src_net(layer);
    int num_input=input_param[0];
    int input_size=input_param[1];
    bool has_fc_layer=false;
    bool has_lrn_layer=false;
    bool has_batch_norm_layer=false;
    bool has_scale_layer=false;
    bool has_eltwise_layer=false;
    bool has_match_conv_layer=false;
    bool has_concat_layer=false;
    bool has_expand3x3_layer=false;
    bool is_match_conv_layer_next=false;
    int in_num_match_conv_layer=0;
    int in_size_match_conv_layer=0;
    bool is_match_conv_layer=false;
    bool is_expand3x3_layer_next=false;
    int in_num_expand3x3_layer=0;
    int in_size_expand3x3_layer=0;
    bool is_expand3x3_layer=false;
    bool has_input_layer=false;
    bool is_single_line_1=true;
    bool is_single_line_2=true;
    
    //conv_layer config params
    vector<int> nn_in_data_size_conv;
    vector<int> nn_channel_size_conv;
    vector<int> nn_padding_conv;
    vector<int> nn_stride_conv;
    vector<int> nn_in_number_conv;
    vector<int> nn_out_number_conv;
    vector<int> nn_group_conv;
    vector<int> nn_bias_conv;
    
    //pool_layer config params
    vector<int> nn_in_data_size_pooling;
    vector<int> nn_channel_size_pooling;
    vector<int> nn_padding_pooling;
    vector<int> nn_stride_pooling;
    vector<int> nn_in_number_pooling;
    
    //fc_layer config params
    vector<int> nn_in_data_size_fc;
    vector<int> nn_in_number_fc;
    vector<int> nn_out_number_fc;
    vector<int> nn_channel_size_fc;

    //lrn_layer config params
    vector<int> nn_local_size_lrn;
    vector<float> nn_alpha_lrn;
    vector<float> nn_beta_lrn;

    //batch_norm_layer config params
    vector<int> nn_in_number_batch_norm;

    //scale_layer config params
    vector<int> nn_in_number_scale;

    //eltwise_layer config params
    vector<int> nn_in_number_eltwise;
    vector<int> nn_input_size_eltwise;

    //eltwise_layer config params
    vector<int> nn_in_number_concat;
    vector<int> nn_input_size_concat;

    int num_output=0;
    ofstream out;
    out.open("net_config_params.txt");
    ofstream out_bottom;
    ofstream out_match_conv;
    ofstream out_eltwise_bottom;
    ofstream out_expand3x3;
    ofstream out_concat_bottom;
    out<<"Network Structure: ";
    //judge if the net is single line structure
    for (int i = 0; i < src_net.size(); i++) {
        if(src_net[0].type()=="Input")
            has_input_layer=true;
        if(src_net[i].type()=="Eltwise"){
            is_single_line_1=false;
            break;
        }
        if(src_net[i].type()=="Concat"){
            is_single_line_2=false;
            break;
        }
    }
    //save split_layer & match_conv_layer's serial number into file
    for (int i = 0; i < src_net.size(); i++) {
        cout << "Layer " << i << ":" << src_net[i].name() << "\t" << src_net[i].type()<<endl;
        int bottom_count=0;
        if(!is_single_line_1){
            for(int j = i;j < src_net.size()-1; j++){
                if(src_net[j].type()!="Input"){
                    if(src_net[j].bottom(0)==src_net[i].name()){
                        bottom_count++;
                        if(src_net[j+1].bottom(0)==src_net[j].name()&&src_net[j+1].type()=="Eltwise"){
                            out_match_conv.open("match_conv_layer_serial.txt",ios::app);
                            out_match_conv<<j<<" ";
                            out_match_conv.close();
                        }
                    }
                }
            }
        }
        if(!is_single_line_2){
            for(int j = i;j < src_net.size()-2; j++){
                if(src_net[j].type()!="Input"){
                    if(src_net[j].bottom(0)==src_net[i].name()){
                        bottom_count++;
                        if(src_net[j+2].type()=="Concat"){
                            if(src_net[j+2].bottom(1)==src_net[j].name()){
                                out_expand3x3.open("expand3x3_layer_serial.txt",ios::app);
                                out_expand3x3<<j<<" ";
                                out_expand3x3.close();
                            }
                        }
                    }
                }
            }
        }
        if((!is_single_line_1&&bottom_count==2)||(bottom_count==4&&src_net[i].type()=="BatchNorm")){
            out_bottom.open("split_layer_serial.txt",ios::app);
            out_bottom<<i<<" ";
            out_bottom.close();
        }if(!is_single_line_2&&bottom_count==3&&src_net[i].type()=="Convolution"){
            out_bottom.open("split_layer_serial.txt",ios::app);
            out_bottom<<i<<" ";
            out_bottom.close();
        }
    }
    //save match_conv_layer's bottom layer's serial number for judgement latter
    std::vector<float> match_conv_layer_serial;
    std::vector<float> in_match_conv_layer;
    if(!is_single_line_1){
        ifstream ifs1("match_conv_layer_serial.txt");
        string str1;
        while (ifs1 >> str1){
            float f = atof(str1.c_str());
            match_conv_layer_serial.push_back(f);
        }
        std::vector<int> split_layer_serial;
        ifstream ifs2("split_layer_serial.txt");
        string str2;
        while (ifs2 >> str2){
            float f = atof(str2.c_str());
            split_layer_serial.push_back(f);
        }
        for(int i=0;i<match_conv_layer_serial.size();i++){
            for(int j=0;j<split_layer_serial.size()-1;j++){
                if(split_layer_serial[j]<match_conv_layer_serial[i]&&split_layer_serial[j+1]>match_conv_layer_serial[i]){
                    in_match_conv_layer.push_back(split_layer_serial[j]);
                    break;
                }
            }
        }
    }
    std::vector<float> expand3x3_layer_serial;
    std::vector<float> in_expand3x3_layer;
    if(!is_single_line_2){
        ifstream ifs1("expand3x3_layer_serial.txt");
        string str1;
        while (ifs1 >> str1){
            float f = atof(str1.c_str());
            expand3x3_layer_serial.push_back(f);
        }
        std::vector<int> split_layer_serial;
        ifstream ifs2("split_layer_serial.txt");
        string str2;
        while (ifs2 >> str2){
            float f = atof(str2.c_str());
            split_layer_serial.push_back(f);
        }
        for(int j=0;j<split_layer_serial.size();j++){
            in_expand3x3_layer.push_back(split_layer_serial[j]);
        }
    }
    //save net necessary config information into file
    for (int i = 0; i < src_net.size(); i++) {
        int pad=0;
        int kernel_size=0;
        int stride=1;
        bool tag=false;
        if(src_net[i].type()=="Pooling"){
            if(src_net[i].pooling_param().global_pooling()){
                out<<"Global";
            }
            if(src_net[i].pooling_param().pool()==0){
                out<<"Max"<<src_net[i].type();
            }else if(src_net[i].pooling_param().pool()==1){
                out<<"Ave"<<src_net[i].type();
            }
        }else{
            out<<src_net[i].type();
        }
        if(src_net[i].type()=="Convolution"){
            if(!is_single_line_1){
                for(int j=0;j<match_conv_layer_serial.size();j++){
                    if(int(match_conv_layer_serial[j])==i){
                        is_match_conv_layer=true;
                    }
                }
            }
            if(!is_single_line_2){
                for(int j=0;j<expand3x3_layer_serial.size();j++){
                    if(int(expand3x3_layer_serial[j])==i){
                        is_expand3x3_layer=true;
                    }
                }
            }
            ConvolutionParameter conv_param = src_net[i].convolution_param();
            if(is_match_conv_layer){
                nn_in_data_size_conv.push_back(in_size_match_conv_layer);
                is_match_conv_layer=false;
            }else if(is_expand3x3_layer){
                nn_in_data_size_conv.push_back(in_size_expand3x3_layer);
                is_expand3x3_layer=false;
            }else{
                nn_in_data_size_conv.push_back(input_size);
            }
            num_output=conv_param.num_output();
            nn_out_number_conv.push_back(num_output);
            kernel_size=conv_param.kernel_size(0);
            nn_channel_size_conv.push_back(kernel_size);
            if (conv_param.pad_size()>0){
                out<<"(padding";
                tag=true;
                pad=conv_param.pad(0);
            }
            nn_padding_conv.push_back(pad);
            if (conv_param.stride_size()>0){
                stride=conv_param.stride(0);
            }
            nn_stride_conv.push_back(stride);
            if(i!=0&&src_net[i-1].type()=="Convolution"&&src_net[i+1].type()=="Eltwise")
                ;
            else
                input_size = (input_size + 2 * pad - kernel_size) / stride + 1;
            nn_group_conv.push_back(conv_param.group());
            if(conv_param.group()>1){
                if(tag){
                    out<<",group";
                }else{
                    out<<"(group";
                    tag=true;
                }
            }
            //num_input=num_input/conv_param.group();
            nn_in_number_conv.push_back(num_input);
            if(!conv_param.bias_term())
                nn_bias_conv.push_back(0);
            else
                nn_bias_conv.push_back(num_output);
            if(!is_single_line_1){
                for(int j=0;j<match_conv_layer_serial.size();j++){
                    if((!has_input_layer)&&int(match_conv_layer_serial[j])==i+1){
                        is_match_conv_layer_next=true;
                    }else if(has_input_layer&&int(match_conv_layer_serial[j])==i+2){
                        is_match_conv_layer_next=true;
                    }
                }
            }
            if(!is_single_line_2){
                for(int j=0;j<expand3x3_layer_serial.size();j++){
                    if((!has_input_layer)&&int(expand3x3_layer_serial[j])==i+1){
                        is_expand3x3_layer_next=true;
                    }else if(has_input_layer&&int(expand3x3_layer_serial[j])==i+2){
                        is_expand3x3_layer_next=true;
                    }
                }
            }
            if(!is_single_line_2){
                for(int j=0;j<in_expand3x3_layer.size();j++){
                    if(int(in_expand3x3_layer[j])==i){
                        has_expand3x3_layer=true;
                        in_num_expand3x3_layer=num_output;
                        in_size_expand3x3_layer=input_size;
                    }
                }
            }
        }else if(src_net[i].type()=="InnerProduct"){
            has_fc_layer=true;
            InnerProductParameter inner_product_param = src_net[i].inner_product_param();
            nn_in_data_size_fc.push_back(input_size);
            nn_in_number_fc.push_back(num_input);
            kernel_size=input_size;
            nn_channel_size_fc.push_back(kernel_size);
            num_output=inner_product_param.num_output();
            nn_out_number_fc.push_back(num_output);
            input_size=1;
        }else if(src_net[i].type()=="Pooling"){
            PoolingParameter pooling_param = src_net[i].pooling_param();
            nn_in_data_size_pooling.push_back(input_size);
            nn_in_number_pooling.push_back(num_input);
            if (pooling_param.pad()>0){
                out<<"(padding";
                tag=true;
            }
            pad=pooling_param.pad();
            nn_padding_pooling.push_back(pad);
            kernel_size=pooling_param.kernel_size();
            nn_channel_size_pooling.push_back(kernel_size);
            stride=pooling_param.stride();
            nn_stride_pooling.push_back(stride);
            if(src_net[i].pooling_param().global_pooling()){
                input_size = 1;
            }else{
                input_size = static_cast<int>(ceil(static_cast<float>(input_size + 2 * pad - kernel_size) / stride)) + 1;
            }
            if(!is_single_line_1){
                for(int j=0;j<in_match_conv_layer.size();j++){
                    if(int(in_match_conv_layer[j])==i){
                        has_match_conv_layer=true;
                        in_num_match_conv_layer=num_input;
                        in_size_match_conv_layer=input_size;
                    }
                }
            }
        }else if(src_net[i].type()=="LRN"){
            has_lrn_layer=true;
            int local_size=src_net[i].lrn_param().local_size();
            nn_local_size_lrn.push_back(local_size);
            float alpha=src_net[i].lrn_param().alpha();
            nn_alpha_lrn.push_back(alpha);
            float beta=src_net[i].lrn_param().beta();
            nn_beta_lrn.push_back(beta);
        }else if(src_net[i].type()=="BatchNorm"){
            has_batch_norm_layer=true;
            nn_in_number_batch_norm.push_back(num_input);
            if(!is_single_line_1){
                for(int j=0;j<in_match_conv_layer.size();j++){
                    if(int(in_match_conv_layer[j])==i){
                        has_match_conv_layer=true;
                        in_num_match_conv_layer=num_input;
                        in_size_match_conv_layer=input_size;
                    }
                }
            }
        }else if(src_net[i].type()=="Scale"){
            has_scale_layer=true;
            nn_in_number_scale.push_back(num_input);
        }else if(src_net[i].type()=="Eltwise"){
            has_eltwise_layer=true;
            net_has_eltwise_layer[0]=1;
            nn_in_number_eltwise.push_back(num_input);
            nn_input_size_eltwise.push_back(input_size);
            for(int j = i;j >= 0; j--){
                if(src_net[i].bottom(0)==src_net[j].name()||src_net[i].bottom(1)==src_net[j].name()){
                    out_eltwise_bottom.open("eltwise_bottom_layer_serial.txt",ios::app);
                    out_eltwise_bottom<<j<<" ";
                    out_eltwise_bottom.close();
                }
            }
            out_eltwise_bottom.open("eltwise_bottom_layer_serial.txt",ios::app);
            out_eltwise_bottom<<i<<" ";
            out_eltwise_bottom.close();
        }else if(src_net[i].type()=="Concat"){
            has_concat_layer=true;
            net_has_eltwise_layer[0]=1;
            nn_in_number_concat.push_back(num_input);
            nn_input_size_concat.push_back(input_size);
            for(int j = i;j >= 0; j--){
                if(src_net[i].bottom(0)==src_net[j].name()||src_net[i].bottom(1)==src_net[j].name()){
                    out_concat_bottom.open("concat_bottom_layer_serial.txt",ios::app);
                    out_concat_bottom<<j<<" ";
                    out_concat_bottom.close();
                }
            }
            out_concat_bottom.open("concat_bottom_layer_serial.txt",ios::app);
            out_concat_bottom<<i<<" ";
            out_concat_bottom.close();
        }
        if(src_net[i].type()=="Convolution"||src_net[i].type()=="InnerProduct"){
            num_input=num_output;//set each layer's num_input equals to the last layer's num_output
            if(has_match_conv_layer&&is_match_conv_layer_next){
                num_input=in_num_match_conv_layer;
                has_match_conv_layer=false;
                is_match_conv_layer_next=false;
            }
            if(has_expand3x3_layer&&is_expand3x3_layer_next){
                num_input=in_num_expand3x3_layer;
                has_expand3x3_layer=false;
                is_expand3x3_layer_next=false;
            }
        }else if(src_net[i].type()=="Concat"){
            num_input=num_input*2;
        }
        if(tag){
            out<<")";
        }
        out<<" ";
    }
    out<<endl;
    out.close();
    vector<string> str_nn_config_params_name_int;
    str_nn_config_params_name_int.push_back("nn_in_data_size_conv: ");
    str_nn_config_params_name_int.push_back("nn_channel_size_conv: ");
    str_nn_config_params_name_int.push_back("nn_padding_conv: ");
    str_nn_config_params_name_int.push_back("nn_stride_conv: ");
    str_nn_config_params_name_int.push_back("nn_in_number_conv: ");
    str_nn_config_params_name_int.push_back("nn_out_number_conv: ");
    str_nn_config_params_name_int.push_back("nn_group_conv: ");
    int count=0;
    for(int i=0;i<nn_bias_conv.size();i++){
          count+=nn_bias_conv[i];
    }
    if(count>0)
        str_nn_config_params_name_int.push_back("nn_bias_conv: ");

    str_nn_config_params_name_int.push_back("nn_in_data_size_pooling: ");
    str_nn_config_params_name_int.push_back("nn_channel_size_pooling: ");
    str_nn_config_params_name_int.push_back("nn_padding_pooling: ");
    str_nn_config_params_name_int.push_back("nn_stride_pooling: ");
    str_nn_config_params_name_int.push_back("nn_in_number_pooling: ");

    if(has_fc_layer==true){
        str_nn_config_params_name_int.push_back("nn_in_data_size_fc: ");
        str_nn_config_params_name_int.push_back("nn_in_number_fc: ");
        str_nn_config_params_name_int.push_back("nn_out_number_fc: ");
        str_nn_config_params_name_int.push_back("nn_channel_size_fc: ");
    }    
    if(has_lrn_layer==true){
        str_nn_config_params_name_int.push_back("nn_local_size_lrn: ");
    }
    if(has_batch_norm_layer==true){
        str_nn_config_params_name_int.push_back("nn_in_number_batch_norm: ");
    }
    if(has_scale_layer==true){
        str_nn_config_params_name_int.push_back("nn_in_number_scale: ");
    }
    if(has_eltwise_layer==true){
        str_nn_config_params_name_int.push_back("nn_in_number_eltwise: ");
        str_nn_config_params_name_int.push_back("nn_input_size_eltwise: ");
    }
    if(has_concat_layer==true){
        str_nn_config_params_name_int.push_back("nn_in_number_concat: ");
        str_nn_config_params_name_int.push_back("nn_input_size_concat: ");
    }

    vector<vector<int>> nn_config_params_int;
    nn_config_params_int.push_back(nn_in_data_size_conv);
    nn_config_params_int.push_back(nn_channel_size_conv);
    nn_config_params_int.push_back(nn_padding_conv);
    nn_config_params_int.push_back(nn_stride_conv);
    nn_config_params_int.push_back(nn_in_number_conv);
    nn_config_params_int.push_back(nn_out_number_conv);
    nn_config_params_int.push_back(nn_group_conv);
    if(count>0)
        nn_config_params_int.push_back(nn_bias_conv);

    nn_config_params_int.push_back(nn_in_data_size_pooling);
    nn_config_params_int.push_back(nn_channel_size_pooling);
    nn_config_params_int.push_back(nn_padding_pooling);
    nn_config_params_int.push_back(nn_stride_pooling);
    nn_config_params_int.push_back(nn_in_number_pooling);

    if(has_fc_layer==true){
        nn_config_params_int.push_back(nn_in_data_size_fc);
        nn_config_params_int.push_back(nn_in_number_fc);
        nn_config_params_int.push_back(nn_out_number_fc);
        nn_config_params_int.push_back(nn_channel_size_fc);
    }
    if(has_lrn_layer){
        nn_config_params_int.push_back(nn_local_size_lrn);
    }
    if(has_batch_norm_layer==true){
        nn_config_params_int.push_back(nn_in_number_batch_norm);
    }
    if(has_scale_layer==true){
        nn_config_params_int.push_back(nn_in_number_scale);
    }
    if(has_eltwise_layer==true){
        nn_config_params_int.push_back(nn_in_number_eltwise);
        nn_config_params_int.push_back(nn_input_size_eltwise);
    }
    if(has_concat_layer==true){
        nn_config_params_int.push_back(nn_in_number_concat);
        nn_config_params_int.push_back(nn_input_size_concat);
    }
    out.open("net_config_params.txt",ios::app);
    for (int i = 0; i < nn_config_params_int.size(); i++) {
        out<<str_nn_config_params_name_int[i];
        for (int j = 0; j < nn_config_params_int[i].size(); j++) {
        out<<nn_config_params_int[i][j]<<" ";
        }
        out<<endl;
    }
    if(has_lrn_layer){
        vector<string> str_nn_config_params_name_float;
        str_nn_config_params_name_float.push_back("nn_alpha_lrn: ");
        str_nn_config_params_name_float.push_back("nn_beta_lrn: ");

        vector<vector<float>> nn_config_params_float;
        nn_config_params_float.push_back(nn_alpha_lrn);
        nn_config_params_float.push_back(nn_beta_lrn);
        for (int i = 0; i < nn_config_params_float.size(); i++) {
        out<<str_nn_config_params_name_float[i];
            for (int j = 0; j < nn_config_params_float[i].size(); j++) {
            out<<nn_config_params_float[i][j]<<" ";
            }
            out<<endl;
        }
    }
    out.close();
}
    
void create_net_from_caffe_net(const caffe::NetParameter& layer,int input_param[])
{
        caffe_layer_vector src_net(layer);
        if (layer.input_shape_size() > 0) {
            // input_shape is deprecated in Caffe
            // blob dimensions are ordered by number N x channel K x height H x width W
            input_param[0]  = static_cast<int>(layer.input_shape(0).dim(1));
            input_param[1] = static_cast<int>(layer.input_shape(0).dim(2));
            //int width  = static_cast<int>(layer.input_shape(0).dim(3));
            //cout<<"depth:********************"<<depth<<endl;
        }else if (layer.layer(0).has_input_param()) {
            // blob dimensions are ordered by number N x channel K x height H x width W
            input_param[0] = static_cast<int>(layer.layer(0).input_param().shape(0).dim(1));
            input_param[1] = static_cast<int>(layer.layer(0).input_param().shape(0).dim(2));
            //int kernel_h = static_cast<int>(layer.layer(0).input_param().shape(0).dim(2));
            //int kernel_w = static_cast<int>(layer.layer(0).input_param().shape(0).dim(3));
            //cout<<"width:********************"<<kernel_w<<endl;
            //return input_param;
        }//else if (layer.input_dim_size() > 0){
         //   input_param[0] = static_cast<int>(layer.input_dim(1));
         //   input_param[1] = static_cast<int>(layer.input_dim(2));
         //   cout<<"no input shape: "<<endl;
        //}
}

void read_proto_from_text(const std::string& prototxt,
                                 google::protobuf::Message *message) {
    int fd = CNN_OPEN_TXT(prototxt.c_str());
    if (fd == -1) {
        cout<<"file not found: "<<prototxt<<endl;
    }

    google::protobuf::io::FileInputStream input(fd);
    input.SetCloseOnDelete(true);

    if (!google::protobuf::TextFormat::Parse(&input, message)) {
        cout<<"failed to parse"<<endl;
    }
}

void read_proto_from_binary(const std::string& protobinary,
                                   google::protobuf::Message *message) {
    int fd = CNN_OPEN_BINARY(protobinary.c_str());
    google::protobuf::io::FileInputStream rawstr(fd);
    google::protobuf::io::CodedInputStream codedstr(&rawstr);

    rawstr.SetCloseOnDelete(true);
    codedstr.SetTotalBytesLimit(std::numeric_limits<int>::max(),
                                std::numeric_limits<int>::max() / 2);

    if (!message->ParseFromCodedStream(&codedstr)) {
        cout<<"failed to parse"<<endl;
    }
}

void create_net_from_caffe_prototxt(const std::string& caffeprototxt,int input_param[])
{
    caffe::NetParameter np;
    read_proto_from_text(caffeprototxt, &np);
    cout <<"net_name: "<<np.name()<<endl;
    create_net_from_caffe_net(np,input_param);
}

void reload_weight_from_caffe_protobinary(const std::string& caffebinary,int input_param[])
{
    caffe::NetParameter np;

    read_proto_from_binary(caffebinary, &np);
    reload_weight_from_caffe_net(np,input_param);
}

void compute_mean(const string& mean_file)
{
    caffe::BlobProto blob;
    read_proto_from_binary(mean_file, &blob);

    vector<cv::Mat> channels;
    auto data = blob.mutable_data()->mutable_data();

    for (int i = 0; i < blob.channels(); i++, data += blob.height() * blob.width())
        channels.emplace_back(blob.height(), blob.width(), CV_32FC1, data);

    ofstream out;
    out.open("net_mean.txt");
    cv::Mat mean;
    cv::merge(channels, mean);
    out<<cv::Mat(cv::Size(1, 1), mean.type(), cv::mean(mean))<<" ";
    out.close();
}

void get_config_params_from_caffe_protobinary(const std::string& caffebinary,int input_param[],int net_has_eltwise_layer[])
{
    caffe::NetParameter np;

    read_proto_from_text(caffebinary, &np);
    get_config_params_from_caffe_net(np,input_param,net_has_eltwise_layer);
}

void test_has_mean(const string& model_file,
          const string& trained_file,
          const string& mean_file) {
    int input_param[]={0,0};
    int net_has_eltwise_layer[]={0};
    create_net_from_caffe_prototxt(model_file,input_param);
    get_config_params_from_caffe_protobinary(model_file,input_param,net_has_eltwise_layer);
    if(!net_has_eltwise_layer[0]){
        reload_weight_from_caffe_protobinary(trained_file,input_param);
    }
    compute_mean(mean_file);
}

void test_no_mean(const string& model_file,
          const string& trained_file) {
    int input_param[]={0,0};
    int net_has_eltwise_layer[]={0};
    create_net_from_caffe_prototxt(model_file,input_param);
    reload_weight_from_caffe_protobinary(trained_file,input_param);
    get_config_params_from_caffe_protobinary(model_file,input_param,net_has_eltwise_layer);
}

int main(int argc, char** argv) {
    cout<<"argc:"<<argc<<endl;
    int arg_channel = 1;
    string model_file = argv[arg_channel++];
    string trained_file = argv[arg_channel++];
    cout<<"model_file:"<<model_file<<endl;
    cout<<"trained_file:"<<trained_file<<endl;
    try  
    {  
        if(argc==4){
            string mean_file = argv[arg_channel++];
            cout<<"mean_file:"<<mean_file<<endl;
            test_has_mean(model_file,trained_file,mean_file);
        }else if(argc==3){
            test_no_mean(model_file,trained_file);
        }
    }  
    catch (exception& e)  
    {  
        cout << "Standard exception: " << e.what() << endl;  
    }  
}
