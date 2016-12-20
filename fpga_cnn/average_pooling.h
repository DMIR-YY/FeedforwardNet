//average pooling layer with kernel weights and without kernel weights
//TODO: Add average pooling without kernel weights functions.

#ifndef _AVERAGE_POOLING_H_
#define _AVERAGE_POOLING_H_

#pragma once
#include <iostream>
#include <fstream>
#include <algorithm>
#include "data_type.h"
#include "activation_functions.h"

using namespace std;

tensor_t pooling_kernel(int input_size,
						int kernel_size,
                        tensor_t& in_data,
                        float kernel_weights,
                        tensor_t& out_data) {
    out_data.clear();
	std::vector<tensor_t> out_1;
	std::vector<tensor_t> out_2;
	vec_t vec2;//output row vector
	for (int i = 0; i < input_size - kernel_size / 2; i = i + kernel_size) //��������map
	{
		for (int j = 0; j < input_size - kernel_size / 2; j = j + kernel_size)
		{
			float sum = 0;
			for (int ii = 0; ii < kernel_size; ++ii) //����kernel
			{
				for (int jj = 0; jj < kernel_size; ++jj)
				{
					float data = in_data[i + ii][j + jj];
					sum += data;
				}
			}
			sum = (float)(sum / (kernel_size*kernel_size));//���ÿ��pooling�����ڵľ�ֵ
			sum = sum*kernel_weights;//ÿ�������ͬһ��weight
									 //sum += kernel_bias;
			vec2.push_back(sum);//����sum�������������
		}
		out_data.push_back(vec2);//��������������������map
		vec2.clear();
	}
	return out_data;
}

std::vector<tensor_t> pooling_layer(
	string activation_type,
	int& input_size,
	int& kernel_size,
	std::vector<tensor_t>& in_data3D,
	vec_t& kernel_weights,
	vec_t& kernel_bias,
	std::vector<tensor_t>& out_data3D,
	int& in_channel, int& out_channel)
{

	cout << "Starting average_pooling ...." << endl;
	out_data3D.clear();
	tensor_t out_data2D;
	float out_data2D_final_f;
	vec_t out_data2D_final_v;
	tensor_t out_data2D_final;

	for (int a = 0; a < in_channel; a++) {//6��in
		out_data2D = pooling_kernel(input_size, kernel_size, in_data3D[a], kernel_weights[a], out_data2D);
		//ѭ������out_data2D�����ƫ�úͼ���
		for (int i = 0; i < out_data2D.size(); i++) {
			for (int j = 0; j < out_data2D[i].size(); j++) {
				out_data2D_final_f = out_data2D[i][j] + kernel_bias[a];
				//const float ep = exp(out_data2D_final_f);
				//const float em = exp(-out_data2D_final_f);
				//out_data2D_final_f = (ep - em) / (ep + em);//tan_h????

				out_data2D_final_f = f(activation_type, out_data2D_final_f);//���������

				out_data2D_final_v.push_back(out_data2D_final_f);//����ÿ�������������ֵ
			}
			out_data2D_final.push_back(out_data2D_final_v);//��ÿ���������ۼӽ������out_data2D_final
			out_data2D_final_v.clear();
		}
		out_data3D.push_back(out_data2D_final);
		out_data2D.clear();
		out_data2D_final.clear();
	}
	cout << "Finished average_pooling ...." << endl;

    //debugging output
    ofstream out_pool;
    out_pool.open("out_pool.txt", ios::app);
	for (int i = 0; i < out_data3D.size(); i++) {
		for (int j = 0; j < out_data3D[i].size(); j++) {
			for (int k = 0; k < out_data3D[i][j].size(); k++) {
				out_pool << out_data3D[i][j][k] << " ";
			}
			out_pool << endl;
		}
        out_pool << endl;
	}
	out_pool.close();
	cout << endl;
	return out_data3D;
}

#endif
