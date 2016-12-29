//
// Created by yaochen on 29/12/16.
//

#ifndef _FC_LAYER_H_
#define _FC_LAYER_H_

//#pragma once
#include "config.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include "data_type.h"
#include "activation_functions.h"


using namespace std;

template <int _INPUT_SIZE_, int _IN_CHANNEL_NUM_, int _OUT_CHANNEL_NUM_>
class fc_layer{

private:
    int fc_layer_num;

public:
    fc_layer():fc_layer_num(0){};

    //convolution kernel function
    void fully_connect(
            tensor_t& in_data,
            tensor_t& kernel_weights,
            tensor_t& out_data) {

        out_data.clear();
        vec_t vec2;//output row vector
        for (int i = 0; i < _INPUT_SIZE_; i++)
        {
#pragma HLS PIPELINE
            for (int j = 0; j < _INPUT_SIZE_; j++)
            {
#pragma HLS UNROLL
                float sum = 0;
                float data = in_data[i][j];
                float weight = kernel_weights[i][j];
                sum += data * weight;
                vec2.push_back(sum);
            }
            out_data.push_back(vec2);
            vec2.clear();
        }
    }

    //tensor to tensor convolutional layer with connection table
    void fully_connected_layer(
            char activation_type,
            tensor_t_3d& in_data3D,
            tensor_t_3d& kernel_weights,
            vec_t& kernel_bias,
            tensor_t_3d& out_data3D ) {


        cout << "starting fully_connect ...." << endl;
        out_data3D.clear();

        tensor_t out_data2D_plus;//每一个滤波器filter中所有卷积核卷积计算结果的累加结果
        float out_data2D_final_f;//每个像素加偏置、激活后的值
        vec_t out_data2D_final_v;//每个像素加偏置、激活后的值组成的行向量
        tensor_t out_data2D_final;//最终的输出矩阵

        for (int b = 0; b < _OUT_CHANNEL_NUM_; b++) {//output kernel loop
            int connection_num = 0;
            for (int a = 0; a < _IN_CHANNEL_NUM_; a++) {//input kernel loop
                tensor_t out_data2D;//每一个卷积计算的结果
                fully_connect(in_data3D[a],
                              kernel_weights[a * _OUT_CHANNEL_NUM_ + b],//weight的存放顺序跟convolution层的不同
                              out_data2D);
                for (int i = 0; i < out_data2D.size(); i++) {
                    vec_t result_1;
                    if (connection_num == 0) {//第一个连接
                        for (int j = 0; j < out_data2D[i].size(); j++) {
                            result_1.push_back(0);//行向量的累加和初始置0
                        }
                    }
                    else if (connection_num != 0) {
                        for (int j = 0; j < out_data2D[i].size(); j++) {
                            result_1.push_back(out_data2D_plus[i][j]);//行向量的累加和
                        }
                    }
                    for (int r = 0; r < result_1.size(); r++) {
                        result_1[r] = result_1[r] + out_data2D[i][r];
                    }//行向量累加

                    out_data2D_plus.push_back(result_1);//把每个行向量累加结果放入out_data2D_plus，每次放入会增加10行
                }
                //将行向量累加放入out_data2D_plus中后，删除前10个行向量
                if (connection_num != 0) {
                    for (int i = 0; i < out_data2D.size(); i++) {
                        out_data2D_plus.erase(0);
                    }
                }
                connection_num++;
            }
            //循环遍历out_data2D_plus矩阵加偏置和激活
            for (int i = 0; i < out_data2D_plus.size(); i++) {
                for (int j = 0; j < out_data2D_plus[i].size(); j++) {
                    out_data2D_final_f = out_data2D_plus[i][j] + kernel_bias[b];

                    out_data2D_final_f = f(activation_type, out_data2D_final_f);//激活函数激活

                    out_data2D_final_v.push_back(out_data2D_final_f);//放入每个像素最终输出值
                }
                out_data2D_final.push_back(out_data2D_final_v);//把每个行向量累加结果放入out_data2D_final
                out_data2D_final_v.clear();
            }
            out_data3D.push_back(out_data2D_final);
            //out_data2D.clear();
            out_data2D_final.clear();
            out_data2D_plus.clear();
        }

        //debugging output
#if _C_DEBUG_MODE_
        cout << "finished fully_connect ...." << endl;
        ofstream out_fc_class;
        out_fc_class.open("out_fc_class.txt", ios::app);
        for (int i = 0; i < out_data3D.size(); i++) {
            for (int j = 0; j < out_data3D[i].size(); j++) {
                for (int k = 0; k < out_data3D[i][j].size(); k++) {
                    out_fc_class << out_data3D[i][j][k] << " ";
                }
                out_fc_class << endl;
            }
            out_fc_class << endl;
        }
        out_fc_class.close();
        cout << endl;
#endif
    }

};


#endif //FFNET_FC_LAYER_H
