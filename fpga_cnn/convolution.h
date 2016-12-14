#ifndef _CONVOLUTION_H_
#define _CONVOLUTION_H_

#include <iostream>
//#include "../tiny_dnn/util/util.h"
#include "average_pooling.h"
#include <fstream>
#include <algorithm>
using namespace tiny_dnn;
using namespace std;

std::vector<tensor_t> in_2_2D_conv(int& input_size, tensor_t in);
void load_weight(std::vector<tensor_t> weight2D, vec_t bias2D);

#define O true
#define X false
static const bool tbl[6][16] = {
	O, X, X, X, O, O, O, X, X, O, O, O, O, X, O, O,
	O, O, X, X, X, O, O, O, X, X, O, O, O, O, X, O,
	O, O, O, X, X, X, O, O, O, X, X, O, X, O, O, O,
	X, O, O, O, X, X, O, O, O, O, X, X, O, X, O, O,
	X, X, O, O, O, X, X, O, O, O, O, X, O, O, X, O,
	X, X, X, O, O, O, X, X, O, O, O, O, X, O, O, O
};
#undef O
#undef X

//convolution kernel
void convolution_kernel(int input_size, 
			    int kernel_size, 
			    tensor_t& in_data, 
			    tensor_t& kernel_weights, 
			    float kernel_bias, 
			    tensor_t& out_data, 
			    int in_channel, 
			    int out_channel) {
	out_data.clear();
	std::vector<tensor_t> out_1;
	std::vector<tensor_t> out_2;
	vec_t vec2;//�����������
	
	//����2��Ȩ�ش��˳��in1��Ӧ��16��kernel��Ȩ�ؾ���in2��Ӧ��16��kernel��Ȩ�ؾ���...
	//for (int b = 0; b < nn_channel_number[in_number]; b++) {//16��kernel
	//for (int a = 0; a < nn_in_number[in_number]; a++) {//6��in
	for (int i = kernel_size / 2; i < input_size - kernel_size / 2; ++i) //��������map
	{
		for (int j = kernel_size / 2; j < input_size - kernel_size / 2; ++j)
		{
			float sum = 0; //
			//if (tbl[a][b]) {//��������ӵ�
			for (int ii = -kernel_size / 2; ii <= kernel_size / 2; ++ii) //����kernel
			{
				for (int jj = -kernel_size / 2; jj <= kernel_size / 2; ++jj)
				{
					//int c = a*input_size;//��a������map��in_data2D�е���ʼ����
					float data = in_data[i + ii][j + jj];//��a������map��Ӧ��kernelÿ�����ص�����
														 //����1
					float weight = kernel_weights[ii + kernel_size / 2][jj + kernel_size / 2];
					//����2
					//float weight = weight2D[0][nn_channel_size*nn_channel_size*nn_in_number[in_number] * b + a*nn_channel_size*nn_channel_size + (ii + nn_channel_size / 2) * nn_channel_size + (jj + nn_channel_size / 2)];
					sum += data * weight;// ���mapÿ�����ص����ֵ
				}
			}
			sum += kernel_bias;
			const float ep = exp(sum);
			const float em = exp(-sum);
			sum = (ep - em) / (ep + em);//tan_h����
			vec2.push_back(sum);//����sum�������������
		}
	}
	out_data.push_back(vec2);//��������������������map
	vec2.clear();
	//return out_data;
}

//tensor to tensor convolutional layer 
void convolution_layer(
	int& input_size,
	int& kernel_size,
	std::vector<tensor_t>& in_data2D,
	std::vector<tensor_t>& kernel_weights,
	vec_t& kernel_bias,
	std::vector<tensor_t>& out_data3D,
	int& in_channel, 
	int& out_channel) {
	/*
	2d convolution function body
	in_data should be 1 32x32 data array
	out_data should be 6 28x28 data array
	please restruct the convolution function body here
	this function will be used in layer_1/layer_3/layer_5 in LeNet-5 model
	*/
	
	cout << "starting convolution ...." << endl;

	tensor_t out_data;
	//out_data.clear();
	//std::vector<tensor_t> in_data2D;
	//in_data2D = in_2_2D_conv(input_size, in_data);//inת���ɶ�ά��ʾ
	//std::vector<tensor_t> out_data3D;

	//load_weight(kernel_weights, kernel_bias);

	for (int b = 0; b < out_channel; b++) {//6��kernel
		for (int a = 0; a < in_channel; a++) {//1��in
			//tensor_t out_data;
			convolution_kernel(input_size, 
			   		      kernel_size, 
					      in_data2D[a], 
					      kernel_weights[b], 
					      kernel_bias[b], 
					      out_data, 
					      in_channel, 
					      out_channel);
			out_data3D.push_back(out_data); ////6*16��
		}
	}

	cout << "finished convolution ...." << endl;

	//if (in_channel>1) {//�����ж������map�ľ���㣬�����ۼӵõ����map
	//	for (int i = 0; i < out_channel; i++) {//ÿ������map��kernel�����Ľ���ۼӵõ�ÿ�����map
	//		vector<float> result(out_1[0][0].size(), 0);//�ۼӺ�
	//		for (int j = 0; j < in_channel; j++) {
	//			//����1
	//			transform(result.begin(), result.end(), out_1[i + j * out_channel][0].begin(), result.begin(), plus<float>());
	//			//����2
	//			//transform(result.begin(), result.end(), out_1[i*6 + j ][0].begin(), result.begin(), plus<float>());
	//			for (int k = 0; k<result.size(); k++)
	//				cout << result[k] << " ";
	//			cout << endl;
	//		}
	//		out_data2D.push_back(result);//��������������������map
	//		out_2.push_back(out_data2D);//16��
	//		out_data2D.clear();
	//	}
	//}
	//else {//��һ�������
	//	out_2 = out_1;
	//}
	for (int i = 0; i < out_data3D.size(); i++) {
		for (int j = 0; j < out_data3D[i].size(); j++) {//�鿴��һ�����map
			for (int k = 0; k < out_data3D[i][j].size(); k++) {
				//freopen("out.txt", "a+", stdout);
				cout << out_data3D[i][j][k] << " ";
			}
			cout << endl;
		}
	}
	cout << endl;
	getchar();
	//return out_data3D;
}

#endif
