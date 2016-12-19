//Fully connected function in NNs

#ifndef _FULLY_CONNECTED_H_
#define _FULLY_CONNECTED_H_

#pragma once

#include <iostream>
#include <fstream>
#include <algorithm>
#include "data_type.h"
#include "activation_functions.h"

using namespace std;

//convolution kernel function
tensor_t fully_connect(int input_size,
	tensor_t& in_data,
	tensor_t& kernel_weights,
	tensor_t& out_data) {
	out_data.clear();
	vec_t vec2;//output row vector
	for (int i = 0; i < input_size; i++)
	{
		for (int j = 0; j < input_size; j++)
		{
			float sum = 0;
			float data = in_data[i][j];
			float weight = kernel_weights[i][j];
			sum += data * weight;
			vec2.push_back(sum);
		}
		out_data.push_back(vec2);
		vec2.clear();
	}
	return out_data;
}

//tensor to tensor convolutional layer with connection table
void fully_connected_layer(
	string activation_type,
	int input_size,
	std::vector<tensor_t>& in_data3D,
	std::vector<tensor_t>& kernel_weights,
	vec_t& kernel_bias,
	std::vector<tensor_t>& out_data3D,
	int in_channel,
	int out_channel
/*const bool* tbl*/)
{
	/*
	2d convolution function body
	in_data should be 1 32x32 data array
	out_data should be 6 28x28 data array
	please restruct the convolution function body here
	this function will be used in layer_1/layer_3/layer_5 in LeNet-5 model
	*/
	cout << "starting fully_connect ...." << endl;
	out_data3D.clear();

	tensor_t out_data2D_plus;//ÿһ���˲���filter�����о���˾�����������ۼӽ��
	float out_data2D_final_f;//ÿ�����ؼ�ƫ�á�������ֵ
	vec_t out_data2D_final_v;//ÿ�����ؼ�ƫ�á�������ֵ��ɵ�������
	tensor_t out_data2D_final;//���յ��������

	for (int b = 0; b < out_channel; b++) {//output kernel loop
		int connection_num = 0;
		for (int a = 0; a < in_channel; a++) {//input kernel loop
			tensor_t out_data2D;//ÿһ���������Ľ��
			out_data2D = fully_connect(input_size,
				in_data3D[a],
				kernel_weights[a*out_channel + b],//weight�Ĵ��˳���convolution��Ĳ�ͬ
				out_data2D);
			for (int i = 0; i < out_data2D.size(); i++) {
				vector<float> result_1;
				if (connection_num == 0) {//��һ������
					for (int j = 0; j < out_data2D[i].size(); j++) {
						result_1.push_back(0);//���������ۼӺͳ�ʼ��0
					}
				}
				else if (connection_num != 0) {
					for (int j = 0; j < out_data2D[i].size(); j++) {
						result_1.push_back(out_data2D_plus[i][j]);//���������ۼӺ�
					}
				}
				transform(result_1.begin(),
					result_1.end(),
					out_data2D[i].begin(),
					result_1.begin(),
					plus<float>());//�������ۼ�
				out_data2D_plus.push_back(result_1);//��ÿ���������ۼӽ������out_data2D_plus��ÿ�η��������10��
			}
			//���������ۼӷ���out_data2D_plus�к�ɾ��ǰ10��������
			if (connection_num != 0) {
				tensor_t::iterator it;
				//vector<string>::iterator subIt = (*it).begin();
				for (int i = 0; i < out_data2D.size(); i++)//���ۼӺ�tensor��ǰ10��ɾ����ʣ�µ�10����Ϊÿ���ۼӵ��м���
				{
					it = out_data2D_plus.begin();
					out_data2D_plus.erase(it);
					//it++;//�������������
				}
			}
			connection_num++;
		}
		//ѭ������out_data2D_plus�����ƫ�úͼ���
		for (int i = 0; i < out_data2D_plus.size(); i++) {
			for (int j = 0; j < out_data2D_plus[i].size(); j++) {
				out_data2D_final_f = out_data2D_plus[i][j] + kernel_bias[b];
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
		//out_data2D.clear();
		out_data2D_final.clear();
		out_data2D_plus.clear();
	}

	//debugging output
	cout << "finished fully_connect ...." << endl;
	FILE *fp = NULL;
	for (int i = 0; i < out_data3D.size(); i++) {
		for (int j = 0; j < out_data3D[i].size(); j++) {
			for (int k = 0; k < out_data3D[i][j].size(); k++) {
				fp = freopen("out_fc.txt", "a+", stdout);
				cout << out_data3D[i][j][k] << " ";
			}
			cout << endl;
		}
		cout << endl;
	}
	fclose(fp);
	cout << endl;
}

#endif