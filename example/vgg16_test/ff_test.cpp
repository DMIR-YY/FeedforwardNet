
#include <iostream>
#include <cstring>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <time.h>
#include <malloc.h>
#include "inference_net/config.h"
#include "inference_net/construct_net.h"
#include "inference_net/image_converter.h"
#include "inference_net/weight_bias_one_dim.h"
#include "inference_net/softmax_one_dim.h"
#include "inference_net/predict_one_dim.h"
#include "inference_net/accuracy_one_dim.h"
#include "inference_net/pow_function.h"
#include "inference_net/resize_image.h"


using namespace std;


const unsigned char * loadfile(const std::string &file, int &size) {

   std::ifstream fs(file.c_str(), std::ios::binary);
   fs.seekg(0, std::ios::end);
   size = fs.tellg();
   char * data = new char[size];
   fs.seekg(0);
   fs.read(data, sizeof(char) * size);
   fs.close();
   return (unsigned char *)data;
}



int main() {

   cout<< "Calculating memory space ... ... ... ..." << endl;

   // data size calculation
   unsigned int in_data_mem_size = (3 * 224 * 224) * sizeof(data_type);
   unsigned int conv_weight_size = (1728 + 36864 + 73728 + 147456 + 294912 + 589824 + 589824 + 1179648 + 2359296 + 2359296 + 2359296 + 2359296 + 2359296) * sizeof(data_type_w);
   unsigned int conv_bias_size = (64 + 64 + 128 + 128 + 256 + 256 + 256 + 512 + 512 + 512 + 512 + 512 + 512) * sizeof(data_type_w);
   unsigned int fc_weight_size = (102760448 + 16777216 + 4096000) * sizeof(data_type_w);
   unsigned int fc_bias_size = (4096 + 4096 + 1000) * sizeof(data_type_w);
   unsigned int fc_16_out_size = (1000) * sizeof(data_type_o);
   unsigned int out_size_0_1 = (1605632) * sizeof(data_type_o);
   unsigned int out_size_0_2 = (1605632) * sizeof(data_type_o);
   unsigned int out_size_1_1 = (1605632) * sizeof(data_type_o);
   unsigned int out_size_1_2 = (1605632) * sizeof(data_type_o);

   // assign memory space to different ports
#if _KERNEL_DEBUG_
   data_type   *in_data_mem_port = (data_type*)malloc(in_data_mem_size);
   if (in_data_mem_port == NULL) {
      printf("False memory allocation of in_data_mem_port\n");
   }
   else {
      printf("in data memory location= 0x%x \n", in_data_mem_port);
   }
#endif
   data_type_w   *conv_weight_mem_port = (data_type_w*)malloc(conv_weight_size);
   if (conv_weight_mem_port == NULL) {
      printf("False memory allocation of conv_weight_mem_port\n");
   }
   else {
      printf("conv weight memory location= 0x%x \n", conv_weight_mem_port);
   }
   data_type_w   *conv_bias_mem_port = (data_type_w*)malloc(conv_bias_size);
   if (conv_bias_mem_port == NULL) {
      printf("False memory allocation of conv_bias_mem_port\n");
   }
   else {
      printf("conv bias memory location= 0x%x \n", conv_bias_mem_port);
   }
   data_type_w   *fc_weight_mem_port = (data_type_w*)malloc(fc_weight_size);
   if (fc_weight_mem_port == NULL) {
      printf("False memory allocation of fc_weight_mem_port\n");
   }
   else {
      printf("fc_weight_mem_port memory location= 0x%x \n", fc_weight_mem_port);
   }
   data_type_w   *fc_bias_mem_port = (data_type_w*)malloc(fc_bias_size);
   if (fc_bias_mem_port == NULL) {
      printf("False memory allocation of fc_bias_mem_port\n");
   }
   else {
      printf("fc_bias_mem_port memory location= 0x%x \n", fc_bias_mem_port);
   }
   data_type_o   *fc_16_out_mem_int = (data_type_o*)malloc(fc_16_out_size);
   if (fc_16_out_mem_int == NULL) {
      printf("False memory allocation of fc_out_mem_int\n");
   }
   else {
      printf("fc_out_mem_int memory location= 0x%x \n", fc_16_out_mem_int);
   }
   data_type_o *temp_out_0_1 = (data_type_o *)malloc(out_size_0_1);
   if (temp_out_0_1 == NULL) {
      printf("False memory allocation of temp_out_0_1\n");
   }
   else {
      printf("temp_out_0_1 memory location= 0x%x \n", temp_out_0_1);
   }
   data_type_o *temp_out_0_2 = (data_type_o *)malloc(out_size_0_2);
   if (temp_out_0_2 == NULL) {
      printf("False memory allocation of temp_out_0_2\n");
   }
   else {
      printf("temp_out_0_2 memory location= 0x%x \n", temp_out_0_2);
   }
   data_type_o *temp_out_1_1 = (data_type_o *)malloc(out_size_1_1);
   if (temp_out_1_1 == NULL) {
      printf("False memory allocation of temp_out_1_1\n");
   }
   else {
      printf("temp_out_1_1 memory location= 0x%x \n", temp_out_1_1);
   }
   data_type_o *temp_out_1_2 = (data_type_o *)malloc(out_size_1_2);
   if (temp_out_1_2 == NULL) {
      printf("False memory allocation of temp_out_1_2\n");
   }
   else {
      printf("temp_out_1_2 memory location= 0x%x \n", temp_out_1_2);
   }
#if _KERNEL_DEBUG_
   cout << "FC mem init\n";
   memset(fc_16_out_mem_int, 0, fc_16_out_size);
   memset(temp_out_0_1, 0, out_size_0_1);
   memset(temp_out_0_2, 0, out_size_0_2);
   memset(temp_out_1_1, 0, out_size_1_1);
   memset(temp_out_1_2, 0, out_size_1_2);
#endif

   //net weight src *****************************
#if _HLS_MODE_
   const char* weight_src = "net_weights.txt";
#else
   const char* weight_src = "net_inputs/net_weights.txt";
#endif
   //load mean file *****************************
#if _HLS_MODE_
   ifstream ifs1("net_mean.txt");
#else
   ifstream ifs1("net_inputs/net_mean.txt");
#endif



   float channel_mean[3] = { 0 };
   string str1;
   string y1 = "[";
   string y2 = "]";
   if (!ifs1) {
      cout << "mean data file not found !" << endl;
      getchar();
   }
   int index = 0;
   while (ifs1 >> str1) {
      int p1 = str1.find(y1, 0);
      if (p1 >= 0) {
         str1.erase(p1, y1.length());
      }

      int p2 = str1.find(y2, 0);
      if (p2 >= 0) {
         str1.erase(p2, y2.length());
      }

      float f = atof(str1.c_str());
      channel_mean[index] = f;
      index++;
   }

   ifs1.close();

   //load val (image_name,image_class) set file *****************************
#if _HLS_MODE_
   ifstream ifs("val.txt");
#else
   ifstream ifs("net_inputs/val.txt");
#endif
   string val_name[10];
   float val_class[10];
   string str;
   if (!ifs) {
      cout << "val data file not found !" << endl;
      getchar();
   }
   int num = 0;
   while (ifs >> str&&num<20) {
      if (num % 2 == 1) {
         val_class[num / 2] = int(atof(str.c_str()));
      }
      else {
         val_name[num / 2] = str;
      }

      num++;
   }

   ifs.close();

   //load val image file *****************************
#if _KERNEL_DEBUG_
#if _HLS_MODE_
   string image_dir = "ILSVRC2012_val_00000003.JPEG";
#else
   string image_dir = "./net_inputs/test_imgs/ILSVRC2012_val_00000003.JPEG";
#endif
   float in_data_3D_channel_swap[3][375][500] = { 0 };
   float in_data_3D[3][224][224] = { 0 };
   int crop_w = 224;
   int crop_h = 224;
   int w;
   int h;
   int channels;
   int size;
   const unsigned char * data = loadfile(image_dir, size);
   const unsigned char * image_orig = stbi_load_from_memory(data, size, &w, &h, &channels, 3);
   for (int i = 0; i < 3; i++) {
      for (int j = i; j < w*h*3; j += 3) {
         in_data_3D_channel_swap[2 - i][j / (w * 3)][(j % (w * 3) - i) / 3] = (float)image_orig[j]; //range:0--255
      }

   }
   for (int i = 0; i < 3; i++) {
      for (int j = 0; j < h; j++) {
         for (int k = 0; k < w; k++) {
            in_data_3D_channel_swap[i][j][k] /= 255;// range:0--1
         }

      }

   }
   resize_image(in_data_3D_channel_swap, h, w, in_data_3D);
   for (int i = 0; i < 3; i++) {
      for (int j = 0; j < crop_h; j++) {
         for (int k = 0; k < crop_w; k++) {
            in_data_3D[i][j][k] = in_data_3D[i][j][k] * 255 - channel_mean[i];
         }

      }

   }
   cout << "Writing data to input data memory space ... ... ..." << endl;
   cout << endl;
   cout << endl;
   int in_data_size=0;
   for (int i = 0; i < 3; i += 2) {
      for (int j = 0; j < crop_h; j++) {
         for (int k = 0; k < crop_w; k++) {
            if(i+0<3){
               temp_out_0_1[in_data_size] = (data_type)in_data_3D[i+0][j][k];
            }if(i+1<3){
               temp_out_0_2[in_data_size] = (data_type)in_data_3D[i+1][j][k];
            }
            in_data_size++;
         }

      }

   }
   cout << "Finished writing data to input data memory space ... ..." << endl;

#endif

   char tan_h = 't';
   char relu = 'r';
   char none = 'i';
   int in_number_conv = 0;
   int in_number_fc = 0;
   int in_number_pooling = 0;

   // Prepare weights and bias for conv layer 1
   float *conv_1_weight2D = (float*)malloc(1728 * sizeof(float));
   memset(conv_1_weight2D, 0, 1728 * sizeof(float));
   load_weight_conv(
weight_src, 
conv_1_weight2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   int conv_weight_num=0;
   cout << "Loading conv weight 1 to memory space, starting at: " <<conv_weight_num << '\n';
   for (int i = 0; i < 1728; i++) {
      conv_weight_mem_port[conv_weight_num] = (data_type_w)conv_1_weight2D[i];
      conv_weight_num++;
   }
   free(conv_1_weight2D);

   float *conv_1_bias2D = (float*)malloc(64 * sizeof(float));
   memset(conv_1_bias2D, 0, 64 * sizeof(float));
   load_bias_conv(
weight_src, 
conv_1_bias2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   int conv_bias_num=0;
   in_number_conv++;
   cout << "Loading conv bias 1 to memory space, starting at: " <<conv_bias_num << '\n';
   for (int i = 0; i < 64; i++) {
      conv_bias_mem_port[conv_bias_num] = (data_type_w)conv_1_bias2D[i];
      conv_bias_num++;
   }
   free(conv_1_bias2D);

   // Prepare weights and bias for conv layer 2
   float *conv_2_weight2D = (float*)malloc(36864 * sizeof(float));
   memset(conv_2_weight2D, 0, 36864 * sizeof(float));
   load_weight_conv(
weight_src, 
conv_2_weight2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   cout << "Loading conv weight 2 to memory space, starting at: " <<conv_weight_num << '\n';
   for (int i = 0; i < 36864; i++) {
      conv_weight_mem_port[conv_weight_num] = (data_type_w)conv_2_weight2D[i];
      conv_weight_num++;
   }
   free(conv_2_weight2D);

   float *conv_2_bias2D = (float*)malloc(64 * sizeof(float));
   memset(conv_2_bias2D, 0, 64 * sizeof(float));
   load_bias_conv(
weight_src, 
conv_2_bias2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   in_number_conv++;
   cout << "Loading conv bias 2 to memory space, starting at: " <<conv_bias_num << '\n';
   for (int i = 0; i < 64; i++) {
      conv_bias_mem_port[conv_bias_num] = (data_type_w)conv_2_bias2D[i];
      conv_bias_num++;
   }
   free(conv_2_bias2D);

   // Prepare weights and bias for conv layer 3
   float *conv_3_weight2D = (float*)malloc(73728 * sizeof(float));
   memset(conv_3_weight2D, 0, 73728 * sizeof(float));
   load_weight_conv(
weight_src, 
conv_3_weight2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   cout << "Loading conv weight 3 to memory space, starting at: " <<conv_weight_num << '\n';
   for (int i = 0; i < 73728; i++) {
      conv_weight_mem_port[conv_weight_num] = (data_type_w)conv_3_weight2D[i];
      conv_weight_num++;
   }
   free(conv_3_weight2D);

   float *conv_3_bias2D = (float*)malloc(128 * sizeof(float));
   memset(conv_3_bias2D, 0, 128 * sizeof(float));
   load_bias_conv(
weight_src, 
conv_3_bias2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   in_number_conv++;
   cout << "Loading conv bias 3 to memory space, starting at: " <<conv_bias_num << '\n';
   for (int i = 0; i < 128; i++) {
      conv_bias_mem_port[conv_bias_num] = (data_type_w)conv_3_bias2D[i];
      conv_bias_num++;
   }
   free(conv_3_bias2D);

   // Prepare weights and bias for conv layer 4
   float *conv_4_weight2D = (float*)malloc(147456 * sizeof(float));
   memset(conv_4_weight2D, 0, 147456 * sizeof(float));
   load_weight_conv(
weight_src, 
conv_4_weight2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   cout << "Loading conv weight 4 to memory space, starting at: " <<conv_weight_num << '\n';
   for (int i = 0; i < 147456; i++) {
      conv_weight_mem_port[conv_weight_num] = (data_type_w)conv_4_weight2D[i];
      conv_weight_num++;
   }
   free(conv_4_weight2D);

   float *conv_4_bias2D = (float*)malloc(128 * sizeof(float));
   memset(conv_4_bias2D, 0, 128 * sizeof(float));
   load_bias_conv(
weight_src, 
conv_4_bias2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   in_number_conv++;
   cout << "Loading conv bias 4 to memory space, starting at: " <<conv_bias_num << '\n';
   for (int i = 0; i < 128; i++) {
      conv_bias_mem_port[conv_bias_num] = (data_type_w)conv_4_bias2D[i];
      conv_bias_num++;
   }
   free(conv_4_bias2D);

   // Prepare weights and bias for conv layer 5
   float *conv_5_weight2D = (float*)malloc(294912 * sizeof(float));
   memset(conv_5_weight2D, 0, 294912 * sizeof(float));
   load_weight_conv(
weight_src, 
conv_5_weight2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   cout << "Loading conv weight 5 to memory space, starting at: " <<conv_weight_num << '\n';
   for (int i = 0; i < 294912; i++) {
      conv_weight_mem_port[conv_weight_num] = (data_type_w)conv_5_weight2D[i];
      conv_weight_num++;
   }
   free(conv_5_weight2D);

   float *conv_5_bias2D = (float*)malloc(256 * sizeof(float));
   memset(conv_5_bias2D, 0, 256 * sizeof(float));
   load_bias_conv(
weight_src, 
conv_5_bias2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   in_number_conv++;
   cout << "Loading conv bias 5 to memory space, starting at: " <<conv_bias_num << '\n';
   for (int i = 0; i < 256; i++) {
      conv_bias_mem_port[conv_bias_num] = (data_type_w)conv_5_bias2D[i];
      conv_bias_num++;
   }
   free(conv_5_bias2D);

   // Prepare weights and bias for conv layer 6
   float *conv_6_weight2D = (float*)malloc(589824 * sizeof(float));
   memset(conv_6_weight2D, 0, 589824 * sizeof(float));
   load_weight_conv(
weight_src, 
conv_6_weight2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   cout << "Loading conv weight 6 to memory space, starting at: " <<conv_weight_num << '\n';
   for (int i = 0; i < 589824; i++) {
      conv_weight_mem_port[conv_weight_num] = (data_type_w)conv_6_weight2D[i];
      conv_weight_num++;
   }
   free(conv_6_weight2D);

   float *conv_6_bias2D = (float*)malloc(256 * sizeof(float));
   memset(conv_6_bias2D, 0, 256 * sizeof(float));
   load_bias_conv(
weight_src, 
conv_6_bias2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   in_number_conv++;
   cout << "Loading conv bias 6 to memory space, starting at: " <<conv_bias_num << '\n';
   for (int i = 0; i < 256; i++) {
      conv_bias_mem_port[conv_bias_num] = (data_type_w)conv_6_bias2D[i];
      conv_bias_num++;
   }
   free(conv_6_bias2D);

   // Prepare weights and bias for conv layer 7
   float *conv_7_weight2D = (float*)malloc(589824 * sizeof(float));
   memset(conv_7_weight2D, 0, 589824 * sizeof(float));
   load_weight_conv(
weight_src, 
conv_7_weight2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   cout << "Loading conv weight 7 to memory space, starting at: " <<conv_weight_num << '\n';
   for (int i = 0; i < 589824; i++) {
      conv_weight_mem_port[conv_weight_num] = (data_type_w)conv_7_weight2D[i];
      conv_weight_num++;
   }
   free(conv_7_weight2D);

   float *conv_7_bias2D = (float*)malloc(256 * sizeof(float));
   memset(conv_7_bias2D, 0, 256 * sizeof(float));
   load_bias_conv(
weight_src, 
conv_7_bias2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   in_number_conv++;
   cout << "Loading conv bias 7 to memory space, starting at: " <<conv_bias_num << '\n';
   for (int i = 0; i < 256; i++) {
      conv_bias_mem_port[conv_bias_num] = (data_type_w)conv_7_bias2D[i];
      conv_bias_num++;
   }
   free(conv_7_bias2D);

   // Prepare weights and bias for conv layer 8
   float *conv_8_weight2D = (float*)malloc(1179648 * sizeof(float));
   memset(conv_8_weight2D, 0, 1179648 * sizeof(float));
   load_weight_conv(
weight_src, 
conv_8_weight2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   cout << "Loading conv weight 8 to memory space, starting at: " <<conv_weight_num << '\n';
   for (int i = 0; i < 1179648; i++) {
      conv_weight_mem_port[conv_weight_num] = (data_type_w)conv_8_weight2D[i];
      conv_weight_num++;
   }
   free(conv_8_weight2D);

   float *conv_8_bias2D = (float*)malloc(512 * sizeof(float));
   memset(conv_8_bias2D, 0, 512 * sizeof(float));
   load_bias_conv(
weight_src, 
conv_8_bias2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   in_number_conv++;
   cout << "Loading conv bias 8 to memory space, starting at: " <<conv_bias_num << '\n';
   for (int i = 0; i < 512; i++) {
      conv_bias_mem_port[conv_bias_num] = (data_type_w)conv_8_bias2D[i];
      conv_bias_num++;
   }
   free(conv_8_bias2D);

   // Prepare weights and bias for conv layer 9
   float *conv_9_weight2D = (float*)malloc(2359296 * sizeof(float));
   memset(conv_9_weight2D, 0, 2359296 * sizeof(float));
   load_weight_conv(
weight_src, 
conv_9_weight2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   cout << "Loading conv weight 9 to memory space, starting at: " <<conv_weight_num << '\n';
   for (int i = 0; i < 2359296; i++) {
      conv_weight_mem_port[conv_weight_num] = (data_type_w)conv_9_weight2D[i];
      conv_weight_num++;
   }
   free(conv_9_weight2D);

   float *conv_9_bias2D = (float*)malloc(512 * sizeof(float));
   memset(conv_9_bias2D, 0, 512 * sizeof(float));
   load_bias_conv(
weight_src, 
conv_9_bias2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   in_number_conv++;
   cout << "Loading conv bias 9 to memory space, starting at: " <<conv_bias_num << '\n';
   for (int i = 0; i < 512; i++) {
      conv_bias_mem_port[conv_bias_num] = (data_type_w)conv_9_bias2D[i];
      conv_bias_num++;
   }
   free(conv_9_bias2D);

   // Prepare weights and bias for conv layer 10
   float *conv_10_weight2D = (float*)malloc(2359296 * sizeof(float));
   memset(conv_10_weight2D, 0, 2359296 * sizeof(float));
   load_weight_conv(
weight_src, 
conv_10_weight2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   cout << "Loading conv weight 10 to memory space, starting at: " <<conv_weight_num << '\n';
   for (int i = 0; i < 2359296; i++) {
      conv_weight_mem_port[conv_weight_num] = (data_type_w)conv_10_weight2D[i];
      conv_weight_num++;
   }
   free(conv_10_weight2D);

   float *conv_10_bias2D = (float*)malloc(512 * sizeof(float));
   memset(conv_10_bias2D, 0, 512 * sizeof(float));
   load_bias_conv(
weight_src, 
conv_10_bias2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   in_number_conv++;
   cout << "Loading conv bias 10 to memory space, starting at: " <<conv_bias_num << '\n';
   for (int i = 0; i < 512; i++) {
      conv_bias_mem_port[conv_bias_num] = (data_type_w)conv_10_bias2D[i];
      conv_bias_num++;
   }
   free(conv_10_bias2D);

   // Prepare weights and bias for conv layer 11
   float *conv_11_weight2D = (float*)malloc(2359296 * sizeof(float));
   memset(conv_11_weight2D, 0, 2359296 * sizeof(float));
   load_weight_conv(
weight_src, 
conv_11_weight2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   cout << "Loading conv weight 11 to memory space, starting at: " <<conv_weight_num << '\n';
   for (int i = 0; i < 2359296; i++) {
      conv_weight_mem_port[conv_weight_num] = (data_type_w)conv_11_weight2D[i];
      conv_weight_num++;
   }
   free(conv_11_weight2D);

   float *conv_11_bias2D = (float*)malloc(512 * sizeof(float));
   memset(conv_11_bias2D, 0, 512 * sizeof(float));
   load_bias_conv(
weight_src, 
conv_11_bias2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   in_number_conv++;
   cout << "Loading conv bias 11 to memory space, starting at: " <<conv_bias_num << '\n';
   for (int i = 0; i < 512; i++) {
      conv_bias_mem_port[conv_bias_num] = (data_type_w)conv_11_bias2D[i];
      conv_bias_num++;
   }
   free(conv_11_bias2D);

   // Prepare weights and bias for conv layer 12
   float *conv_12_weight2D = (float*)malloc(2359296 * sizeof(float));
   memset(conv_12_weight2D, 0, 2359296 * sizeof(float));
   load_weight_conv(
weight_src, 
conv_12_weight2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   cout << "Loading conv weight 12 to memory space, starting at: " <<conv_weight_num << '\n';
   for (int i = 0; i < 2359296; i++) {
      conv_weight_mem_port[conv_weight_num] = (data_type_w)conv_12_weight2D[i];
      conv_weight_num++;
   }
   free(conv_12_weight2D);

   float *conv_12_bias2D = (float*)malloc(512 * sizeof(float));
   memset(conv_12_bias2D, 0, 512 * sizeof(float));
   load_bias_conv(
weight_src, 
conv_12_bias2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   in_number_conv++;
   cout << "Loading conv bias 12 to memory space, starting at: " <<conv_bias_num << '\n';
   for (int i = 0; i < 512; i++) {
      conv_bias_mem_port[conv_bias_num] = (data_type_w)conv_12_bias2D[i];
      conv_bias_num++;
   }
   free(conv_12_bias2D);

   // Prepare weights and bias for conv layer 13
   float *conv_13_weight2D = (float*)malloc(2359296 * sizeof(float));
   memset(conv_13_weight2D, 0, 2359296 * sizeof(float));
   load_weight_conv(
weight_src, 
conv_13_weight2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   cout << "Loading conv weight 13 to memory space, starting at: " <<conv_weight_num << '\n';
   for (int i = 0; i < 2359296; i++) {
      conv_weight_mem_port[conv_weight_num] = (data_type_w)conv_13_weight2D[i];
      conv_weight_num++;
   }
   free(conv_13_weight2D);

   float *conv_13_bias2D = (float*)malloc(512 * sizeof(float));
   memset(conv_13_bias2D, 0, 512 * sizeof(float));
   load_bias_conv(
weight_src, 
conv_13_bias2D,
 weight_bias_record,
 nn_channel_size_conv, 
 nn_in_number_conv,
 nn_out_number_conv,
 in_number_conv);
   in_number_conv++;
   cout << "Loading conv bias 13 to memory space, starting at: " <<conv_bias_num << '\n';
   for (int i = 0; i < 512; i++) {
      conv_bias_mem_port[conv_bias_num] = (data_type_w)conv_13_bias2D[i];
      conv_bias_num++;
   }
   free(conv_13_bias2D);

   cout<<"Finished loading conv weight into memory! Total: " <<conv_weight_num  << "... ... ..."<<endl;
   cout<<"Finished loading conv bias into memory! Total: " <<conv_bias_num  << "... ... ..."<<endl;

   // Prepare weights and bias for fc layer 1
   float *fc_1_weight2D = (float*)malloc(102760448 * sizeof(float));
   memset(fc_1_weight2D, 0, 102760448 * sizeof(float));
   load_weight_fc(
weight_src, 
fc_1_weight2D,
 weight_bias_record,
 nn_channel_size_fc, 
 nn_in_number_fc,
 nn_out_number_fc,
 in_number_fc);
   int fc_weight_num=0;
   cout << "Loading fc weight 1 to memory space, starting at: " <<fc_weight_num << '\n';
   for (int i = 0; i < 102760448; i++) {
      fc_weight_mem_port[fc_weight_num] = (data_type_w)fc_1_weight2D[i];
      fc_weight_num++;
   }
   free(fc_1_weight2D);

   float *fc_1_bias2D = (float*)malloc(4096 * sizeof(float));
   memset(fc_1_bias2D, 0, 4096 * sizeof(float));
   load_bias_fc(
weight_src, 
fc_1_bias2D,
 weight_bias_record,
 nn_channel_size_fc, 
 nn_in_number_fc,
 nn_out_number_fc,
 in_number_fc);
   int fc_bias_num=0;
   in_number_fc++;
   cout << "Loading fc bias 1 to memory space, starting at: " <<fc_bias_num << '\n';
   for (int i = 0; i < 4096; i++) {
      fc_bias_mem_port[fc_bias_num] = (data_type_w)fc_1_bias2D[i];
      fc_bias_num++;
   }
   free(fc_1_bias2D);

   // Prepare weights and bias for fc layer 2
   float *fc_2_weight2D = (float*)malloc(16777216 * sizeof(float));
   memset(fc_2_weight2D, 0, 16777216 * sizeof(float));
   load_weight_fc(
weight_src, 
fc_2_weight2D,
 weight_bias_record,
 nn_channel_size_fc, 
 nn_in_number_fc,
 nn_out_number_fc,
 in_number_fc);
   cout << "Loading fc weight 2 to memory space, starting at: " <<fc_weight_num << '\n';
   for (int i = 0; i < 16777216; i++) {
      fc_weight_mem_port[fc_weight_num] = (data_type_w)fc_2_weight2D[i];
      fc_weight_num++;
   }
   free(fc_2_weight2D);

   float *fc_2_bias2D = (float*)malloc(4096 * sizeof(float));
   memset(fc_2_bias2D, 0, 4096 * sizeof(float));
   load_bias_fc(
weight_src, 
fc_2_bias2D,
 weight_bias_record,
 nn_channel_size_fc, 
 nn_in_number_fc,
 nn_out_number_fc,
 in_number_fc);
   in_number_fc++;
   cout << "Loading fc bias 2 to memory space, starting at: " <<fc_bias_num << '\n';
   for (int i = 0; i < 4096; i++) {
      fc_bias_mem_port[fc_bias_num] = (data_type_w)fc_2_bias2D[i];
      fc_bias_num++;
   }
   free(fc_2_bias2D);

   // Prepare weights and bias for fc layer 3
   float *fc_3_weight2D = (float*)malloc(4096000 * sizeof(float));
   memset(fc_3_weight2D, 0, 4096000 * sizeof(float));
   load_weight_fc(
weight_src, 
fc_3_weight2D,
 weight_bias_record,
 nn_channel_size_fc, 
 nn_in_number_fc,
 nn_out_number_fc,
 in_number_fc);
   cout << "Loading fc weight 3 to memory space, starting at: " <<fc_weight_num << '\n';
   for (int i = 0; i < 4096000; i++) {
      fc_weight_mem_port[fc_weight_num] = (data_type_w)fc_3_weight2D[i];
      fc_weight_num++;
   }
   free(fc_3_weight2D);

   float *fc_3_bias2D = (float*)malloc(1000 * sizeof(float));
   memset(fc_3_bias2D, 0, 1000 * sizeof(float));
   load_bias_fc(
weight_src, 
fc_3_bias2D,
 weight_bias_record,
 nn_channel_size_fc, 
 nn_in_number_fc,
 nn_out_number_fc,
 in_number_fc);
   in_number_fc++;
   cout << "Loading fc bias 3 to memory space, starting at: " <<fc_bias_num << '\n';
   for (int i = 0; i < 1000; i++) {
      fc_bias_mem_port[fc_bias_num] = (data_type_w)fc_3_bias2D[i];
      fc_bias_num++;
   }
   free(fc_3_bias2D);

   cout<<"Finished loading fc weight into memory! Total: " <<fc_weight_num  << "... ... ..."<<endl;
   cout<<"Finished loading fc bias into memory! Total: " <<fc_bias_num  << "... ... ..."<<endl;

#if _KERNEL_DEBUG_
   float fc_16_out[1000] = { 0 };
   clock_t start, finish, inf_start, inf_finish;
   double totaltime, inf_time;
   start = clock();
#endif

   //Inference network process
   inference_net(
   //activation function
   relu,
#if _KERNEL_DEBUG_
   //input pic data
   in_data_mem_port, 
#endif
   //layer weights and bias inputs
   conv_weight_mem_port,
   conv_bias_mem_port,
   fc_weight_mem_port,
   fc_bias_mem_port,
#if _KERNEL_DEBUG_
   //output fc data
   fc_16_out_mem_int,
   temp_out_0_1,
   temp_out_0_2,
   temp_out_1_1,
   temp_out_1_2);

   finish = clock();
   totaltime = (double)(finish - start) / CLOCKS_PER_SEC;
   cout <<"predicted time is: " << totaltime << " s" << endl;
   for (int i = 0; i < 1000; i++) {
      fc_16_out[i]=(float)(fc_16_out_mem_int[i]);
   }
   softmax(fc_16_out, 1000);
   predict(fc_16_out, 1000);
#endif

   return 0;

}