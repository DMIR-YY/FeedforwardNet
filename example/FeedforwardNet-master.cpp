// FeedforwardNet-master.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "stdafx.h"
//LeNet-5 image recognition with std image
//eliminating OpenCV lib dependency
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#include "stb_image/stb_image_resize.h"
#include "stb_image/stb_image_write.h"
#include "../tiny_dnn/tiny_dnn.h"
#include "../tiny_dnn/hls_lib/static_vector.h"
#include "../tiny_dnn/util/image.h"
#include "../ff_lib/convolution.h"
#include "../ff_lib/average_pooling.h"
using namespace tiny_dnn;
using namespace tiny_dnn::activation;
using namespace std;
int nn_in_data_size_conv[3] = { 32,14,5 };
int nn_out_data_size_conv[3] = { 28,10,1 };
int nn_in_number_conv[4] = { 1,6,16,120 };
int nn_channel_size_conv = 5;
int nn_channel_number_conv[3] = { 6,16,120 };
bool has_connection_table[3] = {false,true,false};
int in_number_conv = 0;//�ڼ�����������
int weight_bias_count_1 = 0;//ǰn���Ȩ��ƫ����
int weight_bias_count_2 = 0;//ǰn-1���Ȩ��ƫ����
int weight_count = 0;
int convolution_layer_count = 0;

int nn_in_data_size_pooling[2] = { 28,10 };
int nn_out_data_size_pooling[2] = { 14,5 };
int nn_in_number_pooling[2] = { 6,16 };
int nn_channel_number_pooling[2] = { 6,16 };
int nn_channel_size_pooling = 2;
int in_number_pooling = 0;//�ڼ����ػ������
//int weight_bias_count_1_pooling = 0;//ǰn��ػ����Ȩ��ƫ����
//int weight_bias_count_2_pooling = 0;//ǰn-1��ػ����Ȩ��ƫ����
typedef s_vector<float, 16> hls_vec;
typedef s_vector<hls_vec, 16> hls_tensor;

// rescale output to 0-100
template <typename Activation>
double rescale(double x) {
	Activation a;
	return 100.0 * (x - a.scale().first) / (a.scale().second - a.scale().first);
}


void convert_image(const std::string& imagefilename,
	double minv,
	double maxv,
	int w,
	int h,
	vec_t& data) {
	// load
	int input_w, input_h, comp;
	stbi_uc* input_pixels = stbi_load(imagefilename.c_str(), &input_w, &input_h, &comp, 1);
	//cout<<data.size()<<endl;
	//cout << input_pixels << endl;
	if (!input_pixels) {
		// cannot open, or it's not an image
		cout << "stbi_load failed";
		return;
	}

	// resize
	std::vector<uint8_t> resized(w * h);
	uint8_t* resized_pixels = &(resized[0]);
	int input_stride_in_bytes = input_w;
	if (!stbir_resize_uint8(input_pixels, input_w, input_h, input_stride_in_bytes, resized_pixels, w, h, w, 1)) {
		cout << "stbir_resize_uint8 failed";
		stbi_image_free(input_pixels);
		return;
	}
	stbi_image_free(input_pixels);

	// mnist dataset is "white on black", so negate required
	std::transform(resized.begin(), resized.end(), std::back_inserter(data),
		[=](uint8_t c) { return (255 - c) * (maxv - minv) / 255.0 + minv; });
	cout << data.size() << endl;

}

bool save_image(const std::string& imagefilename,
	const image<>& img
)
{
	// no scaling, save at original size
	int stride_bytes = img.width();
	int ret = stbi_write_png(
		imagefilename.c_str(),
		img.width(),
		img.height(),
		1,
		&(img.at(0, 0)),
		stride_bytes);
	return (ret != 0);
}


//void construct_net(network<sequential>& nn) {
void construct_net(network<sequential>& nn) {
	// connection table [Y.Lecun, 1998 Table.1]
#define O true
#define X false
	static const bool tbl[] = {
		O, X, X, X, O, O, O, X, X, O, O, O, O, X, O, O,
		O, O, X, X, X, O, O, O, X, X, O, O, O, O, X, O,
		O, O, O, X, X, X, O, O, O, X, X, O, X, O, O, O,
		X, O, O, O, X, X, O, O, O, O, X, X, O, X, O, O,
		X, X, O, O, O, X, X, O, O, O, O, X, O, O, X, O,
		X, X, X, O, O, O, X, X, O, O, O, O, X, O, O, O
	};
#undef O
#undef X

	// construct nets
	nn << convolutional_layer<tan_h>(32, 32, 5, 1, 6)  // C1, 1@32x32-in, 6@28x28-out
		<< average_pooling_layer<tan_h>(28, 28, 6, 2)   // S2, 6@28x28-in, 6@14x14-out
		<< convolutional_layer<tan_h>(14, 14, 5, 6, 16,
			connection_table(tbl, 6, 16))              // C3, 6@14x14-in, 16@10x10-out
		<< average_pooling_layer<tan_h>(10, 10, 16, 2)  // S4, 16@10x10-in, 16@5x5-out
		<< convolutional_layer<tan_h>(5, 5, 5, 16, 120) // C5, 16@5x5-in, 120@1x1-out
		<< fully_connected_layer<tan_h>(120, 10);       // F6, 120-in, 10-out

														//cout << "ά����Ȩ��&ƫ��" << nn[1]->weights().size() << endl;//2
														//for (int j = 0; j < nn[1]->weights().size(); j++) {
														//	cout << "Ȩ��������&ƫ����" << (*nn[1]->weights()[j]).size() << endl;//150��Ȩ��������5��5��6;6��ƫ�ø���
														//	for (int k = 0; k < (*nn[1]->weights()[j]).size(); k++) {
														//		freopen("weight.txt", "a+", stdout);
														//			cout << (*nn[1]->weights()[j])[k] << " ";//���Ȩ��map����&ƫ��
														//		//	//n++;
														//		}
														//		cout << endl;
														//	cout << endl;
														//}

														//int n = 0;
														//for (int i = 0; i < nn.depth(); i++) { 
														//	cout << "#layer:" << i << "\n"; 
														//	cout << "layer type:" << nn[i]->layer_type() << "\n"; 
														//	/*for (int j = 0; j < nn[i]->weights().size();j++) {
														//		for (int k = 0;k < (*nn[i]->weights()[j]).size(); k++) {
														//			cout << "input:" << (*nn[i]->weights()[j])[k] << "\n";
														//		}
														//	}*/
														//	cout << "input:" << nn[i]->in_data_size() << "(" << nn[i]->in_data_shape() << ")\n";
														//	cout << "output:" << nn[i]->out_data_size() << "(" << nn[i]->out_data_shape() << ")\n";
														//	cout << "ά����Ȩ��&ƫ��" << nn[i]->weights().size() << endl;//2
														//	for (int j = 0; j < nn[i]->weights().size(); j++) {
														//		cout << "Ȩ��������&ƫ����" << (*nn[i]->weights()[j]).size() << endl;//150��Ȩ��������5��5��6;6��ƫ�ø���
														//		for (int k = 0; k < (*nn[i]->weights()[j]).size(); k++) {
														//			cout << (*nn[i]->weights()[j])[k] << " ";//���Ȩ��map����&ƫ��
														//			n++;
														//		}
														//		cout <<"Ȩ��ƫ������Ϊ��"<<n<< endl;
														//	}
														//}
}

void nn_load_weight(const std::string& dic, network<sequential>& nn) {

	ifstream weights(dic);
	nn.load(weights);
}

//std::vector<tensor_t> fprop(int& input_size,
//	int& kernel_size, std::vector<tensor_t>& in, tensor_t& kernel_weights, std::vector<tensor_t>& out, int& in_channel, int& out_channel) {
//	/*for (int i = 0; i < in.size(); i++) {
//	for (int j = 0; j < in[i].size(); j++) {
//	for (int k = 0; k < in[i][j].size(); k++) {
//	cout << in[i][j][k] << " ";
//	}
//	cout << endl;
//	}
//	cout << endl;
//	}*/
//	return in;
//	//return convolution_layer(input_size, kernel_size, in, kernel_weights, out, in_channel, out_channel);
//}
//
//std::vector<vec_t> fprop(int& input_size,
//	int& kernel_size, std::vector<vec_t>& in, tensor_t& kernel_weights, std::vector<vec_t>& out, int& in_channel, int& out_channel) {
//	/*for (int i = 0; i < in.size(); i++) {
//	for (int j = 0; j < in[i].size(); j++) {
//	cout << in[i][j];
//	}
//	cout << endl;
//	}*/
//	return fprop(input_size, kernel_size, std::vector<tensor_t>{ in }, kernel_weights, std::vector<tensor_t>{ out }, in_channel, out_channel)[0];
//}
//
//vec_t fprop(int& input_size,
//	int& kernel_size, vec_t& in, tensor_t& kernel_weights, vec_t& out, int& in_channel, int& out_channel) {
//	//if (in.size() != (size_t)in_data_size())
//	//	data_mismatch(**net_.begin(), in);
//	return fprop(input_size, kernel_size, std::vector<vec_t>{ in }, kernel_weights, std::vector<vec_t>{ out }, in_channel, out_channel)[0];
//}

//vec_t predict(vec_t& in, vec_t& out) {
//	return fprop(in, out);
//}

//void recognize(network<sequential>& nn, vec_t& data, vec_t& res) {
//	res = nn.predict(data);
//	//return res;
//}
//void recognize(int& input_size,
//	int& kernel_size, vec_t& data_in, tensor_t& kernel_weights, vec_t& data_out, int& in_channel, int& out_channel) {
//	fprop(input_size, kernel_size, data_in, kernel_weights, data_out, in_channel, out_channel);
//	//res = nn.predict(data);
//	//return res;
//}

tensor_t in_2_3D(vec_t& data_in) {//��ͼƬ����һ��ά��
	tensor_t data_out;
	data_out.push_back(data_in);
	return data_out;
}

std::vector<tensor_t> in_2_2D_conv(int& input_size, tensor_t in) {//��άinת���ɶ�ά��ʾ
	vec_t in_data;//= in[0];//ԭ��������һά��ʾ
	vec_t vec1;//�����������
	tensor_t vec2;
	std::vector<tensor_t> in_data2D;
	for (int j = 0; j < in.size(); j++) {
		in_data = in[j];
		for (int i = 0; i < in_data.size(); i++)//����һάת��ά
		{
			if (in_data.size() < input_size*input_size) {//�ж������
				//if (i==0||i % 32!=0) {�������У���Ϊÿ�еĵ�һ��Ԫ�ض���
				if (i == 0 || (i - 1) / input_size == i / input_size) {//��ԭ����һά���밴ͼƬ��Сת���ɶ�ά
					vec1.push_back(in_data[i]);//��������mapÿ�����ص�����ֵ��������������

					if (i == in_data.size() - 1) {//���һ������������
						vec2.push_back(vec1);//�����������������ɶ�ά����tensor
						vec1.clear();
					}
					if (i != 0 && i % (input_size*input_size) == 0) {
						in_data2D.push_back(vec2);
						vec2.clear();
					}
				}
				else {
					vec2.push_back(vec1);
					vec1.clear();
					vec1.push_back(in_data[i]);//����������������һ��Ԫ��
					if (i != 0 && i % (input_size*input_size) == 0) {
						in_data2D.push_back(vec2);
						vec2.clear();
					}
				}
			}
			else {//ֻ��һ������
				if (i == 0 || (i - 1) / input_size == i / input_size) {//��ԭ����һά���밴ͼƬ��Сת���ɶ�ά
					vec1.push_back(in_data[i]);//��������mapÿ�����ص�����ֵ��������������
					if (i == in_data.size() - 1) {//���һ������������
						vec2.push_back(vec1);//�����������������ɶ�ά����tensor
						vec1.clear();
						in_data2D.push_back(vec2);
						vec2.clear();
					}
				}
				else {
					vec2.push_back(vec1);
					vec1.clear();
					vec1.push_back(in_data[i]);//����������������һ��Ԫ��
				}
			}
		}
	}
	return in_data2D;
}

//std::vector<tensor_t> in_2_2D_pool(int& input_size, std::vector<tensor_t> in) {//��άinת���ɶ�ά��ʾ
//	vec_t in_data;//= in[0];//ԭ��������һά��ʾ
//	vec_t vec1;//�����������
//	tensor_t vec2;
//	std::vector<tensor_t> in_data2D;
//	for (int j = 0; j < in.size(); j++) {//6
//		in_data = in[j];
//		for (int i = 0; i < in_data.size(); i++)//����һάת��ά 28*28=784
//		{
//			if (in_data.size() < input_size*input_size) {//�ж������
//														 //if (i==0||i % 32!=0) {�������У���Ϊÿ�еĵ�һ��Ԫ�ض���
//				if (i == 0 || (i - 1) / input_size == i / input_size) {//��ԭ����һά���밴ͼƬ��Сת���ɶ�ά
//					vec1.push_back(in_data[i]);//��������mapÿ�����ص�����ֵ��������������
//					if (i == in_data.size() - 1) {//���һ������������
//						vec2.push_back(vec1);//�����������������ɶ�ά����tensor
//						vec1.clear();
//					}
//					if (vec2.size() == nn_channel_size_pooling) {
//						in_data2D.push_back(vec2);
//						vec2.clear();
//					}
//				}
//				else {
//					vec2.push_back(vec1);
//					vec1.clear();
//					vec1.push_back(in_data[i]);//����������������һ��Ԫ��
//					if (i != 0 && i % (input_size*input_size) == 0) {
//						in_data2D.push_back(vec2);
//						vec2.clear();
//					}
//				}
//			}
//			else {//ֻ��һ������
//				if (i == 0 || (i - 1) / input_size == i / input_size) {//��ԭ����һά���밴ͼƬ��Сת���ɶ�ά
//					vec1.push_back(in_data[i]);//��������mapÿ�����ص�����ֵ��������������
//					if (i == in_data.size() - 1) {//���һ������������
//						vec2.push_back(vec1);//�����������������ɶ�ά����tensor
//						vec1.clear();
//						in_data2D.push_back(vec2);
//						vec2.clear();
//					}
//				}
//				else {
//					vec2.push_back(vec1);
//					vec1.clear();
//					vec1.push_back(in_data[i]);//����������������һ��Ԫ��
//				}
//			}
//		}
//	}
//	return in_data2D;
//}

//void in_2_2D_pool(int& input_size, std::vector<tensor_t> in, tensor_t& in_data2D) {//inת���ɶ�ά��ʾ
//	tensor_t in_data;//= in[0];//ԭ��������һά��ʾ
//	vec_t vec1;//�����������
//	for (int j = 0; j < in.size(); j++) {
//		in_data = in[j];
//		for (int i = 0; i < in_data[0].size(); i++)//����һάת��ά
//		{
//			//if (i==0||i % 32!=0) {�������У���Ϊÿ�еĵ�һ��Ԫ�ض���
//			if (i == 0 || (i - 1) / input_size == i / input_size) {//��ԭ����һά���밴ͼƬ��Сת���ɶ�ά
//				vec1.push_back(in_data[0][i]);//��������mapÿ�����ص�����ֵ��������������
//				if (i == in_data[0].size() - 1) {//���һ������������
//					in_data2D.push_back(vec1);//�����������������ɶ�ά����tensor
//					vec1.clear();
//				}
//			}
//			else {
//				in_data2D.push_back(vec1);
//				vec1.clear();
//				vec1.push_back(in_data[0][i]);//����������������һ��Ԫ��
//			}
//		}
//	}
//}

std::vector<tensor_t> load_weight_conv() {
	ifstream ifs(".\\ff\\LeNet-weights");
	string str;
	tensor_t weight_v;//Ȩ�ؾ���
	std::vector<tensor_t> weight2D;
	vec_t vec3;//Ȩ������
	//ǰn-1��Ȩ��&ƫ��+��n��Ȩ����
	int weight_count = 0 + weight_bias_count_1 + nn_channel_size_conv*nn_channel_size_conv*nn_in_number_conv[in_number_conv] * nn_in_number_conv[in_number_conv + 1] ;
	int weight_bias_count_3_conv = 0;
	while (ifs >> str&&weight_bias_count_3_conv<weight_count)
	{
		if (weight_bias_count_3_conv >= weight_bias_count_2) {
		//����n��������weight
					if (weight_bias_count_1 == 0 || (weight_bias_count_1- weight_bias_count_2 - 1) / nn_channel_size_conv == (weight_bias_count_1- weight_bias_count_2) / nn_channel_size_conv) 
					{//��ԭ����һάȨ�ذ�kernel��Сת���ɶ�ά
						float f = atof(str.c_str());
						vec3.push_back(f);//����kernelÿ�����ص�Ȩ��ֵ��������������
						if (weight_bias_count_1 == weight_count - 1) {//���һ������������
							weight_v.push_back(vec3);//�����������������ɶ�ά����tensor
							vec3.clear();
							
						}
						if (weight_v.size() == nn_channel_size_conv) {
							weight2D.push_back(weight_v);
							weight_v.clear();
						}
			}
					else {
						weight_v.push_back(vec3);
						vec3.clear();
						float f = atof(str.c_str());
						vec3.push_back(f);//����������������һ��Ԫ��
					}
					weight_bias_count_1++;
		}
		
		weight_bias_count_3_conv++;
	}
	cout << "Ȩ������Ϊ��" << weight_bias_count_1 << endl;
	ifs.close();
	return weight2D;
}

//std::vector<tensor_t> load_weight_pooling() {
//	ifstream ifs(".\\ff\\LeNet-weights");
//	string str;
//	tensor_t weight_v;//Ȩ�ؾ���
//	std::vector<tensor_t> weight2D;
//	vec_t vec3;//Ȩ������
//	//ǰn-1��Ȩ��&ƫ��+��n��Ȩ����
//	int weight_count = 0 + weight_bias_count_1 + nn_channel_size_pooling*nn_channel_size_pooling*nn_in_number_pooling[in_number_pooling];
//	int weight_bias_count_3_pooling = 0;
//	while (ifs >> str&&weight_bias_count_3_pooling<weight_count)
//	{
//		if (weight_bias_count_3_pooling >= weight_bias_count_2) {
//			//����n��������weight
//			if (weight_bias_count_1 == 0 || (weight_bias_count_1 - weight_bias_count_2 - 1) / nn_channel_size_pooling == (weight_bias_count_1 - weight_bias_count_2) / nn_channel_size_pooling)
//			{//��ԭ����һάȨ�ذ�kernel��Сת���ɶ�ά
//				float f = atof(str.c_str());
//				vec3.push_back(f);//����kernelÿ�����ص�Ȩ��ֵ��������������
//				if (weight_bias_count_1 == weight_count - 1) {//���һ������������
//					weight_v.push_back(vec3);//�����������������ɶ�ά����tensor
//					vec3.clear();
//
//				}
//				if (weight_v.size() == nn_channel_size_pooling) {
//					weight2D.push_back(weight_v);
//					weight_v.clear();
//				}
//			}
//			else {
//				weight_v.push_back(vec3);
//				vec3.clear();
//				float f = atof(str.c_str());
//				vec3.push_back(f);//����������������һ��Ԫ��
//			}
//			weight_bias_count_1++;
//		}
//
//		weight_bias_count_3_pooling++;
//	}
//	cout << "Ȩ������Ϊ��" << weight_bias_count_1 << endl;
//	ifs.close();
//	return weight2D;
//}

vec_t load_weight_pooling() {
	ifstream ifs(".\\ff\\LeNet-weights");
	string str;
	vec_t weight_v;//Ȩ������
	//ǰn-1��Ȩ��&ƫ��+��n��Ȩ����
	int weight_count = 0 + weight_bias_count_1 + nn_channel_size_pooling*nn_channel_size_pooling*nn_in_number_pooling[in_number_pooling];
	int weight_bias_count_3_pooling = 0;
	while (ifs >> str&&weight_bias_count_3_pooling<weight_count)
	{
		if (weight_bias_count_3_pooling >= weight_bias_count_2) {
			//����n��������weight
				float f = atof(str.c_str());
				weight_v.push_back(f);//����kernelÿ�����ص�Ȩ��ֵ��������������
				weight_bias_count_1++;
				}
		weight_bias_count_3_pooling++;
	}
	cout << "Ȩ������Ϊ��" << weight_bias_count_1 << endl;
	ifs.close();
	return weight_v;
}

vec_t load_bias_conv() {
	ifstream ifs(".\\ff\\LeNet-weights");
	string str;
	vec_t bias2D;
	int weight_count = 0 + weight_bias_count_2 + nn_channel_size_conv*nn_channel_size_conv*nn_in_number_conv[in_number_conv] * nn_in_number_conv[in_number_conv + 1] ;
	int weight_bias_count = 0 + weight_bias_count_2 + nn_channel_size_conv*nn_channel_size_conv*nn_in_number_conv[in_number_conv] * nn_in_number_conv[in_number_conv + 1] + nn_channel_number_conv[in_number_conv];
	int weight_bias_count_3_conv = 0;
	while (ifs >> str&&weight_bias_count_3_conv<weight_bias_count)
	{
		if (weight_bias_count_3_conv>= weight_bias_count_1&&weight_bias_count_1 >= weight_count&&weight_bias_count_1<= weight_bias_count) {
			//����n��������bias
				float f = atof(str.c_str());
				bias2D.push_back(f);//����ƫ��
				weight_bias_count_1++;
		}
		weight_bias_count_3_conv++;
	}
	weight_bias_count_2 = weight_bias_count_1;
	cout << "Ȩ������Ϊ��" << weight_bias_count_1 << endl;
	ifs.close();
	return bias2D;
}

vec_t load_bias_pooling() {
	ifstream ifs(".\\ff\\LeNet-weights");
	string str;
	vec_t bias2D;
	int weight_count = 0 + weight_bias_count_2 + nn_channel_size_pooling*nn_channel_size_pooling*nn_in_number_pooling[in_number_pooling];
	int weight_bias_count = 0 + weight_bias_count_2 + nn_channel_size_pooling*nn_channel_size_pooling*nn_in_number_pooling[in_number_pooling] + nn_channel_number_pooling[in_number_pooling];
	int weight_bias_count_3_pooling = 0;
	while (ifs >> str&&weight_bias_count_3_pooling<weight_bias_count)
	{
		if (weight_bias_count_3_pooling >= weight_bias_count_1&&weight_bias_count_1 >= weight_count&&weight_bias_count_1 <= weight_bias_count) {
			//����n��������bias
			float f = atof(str.c_str());
			bias2D.push_back(f);//����ƫ��
			weight_bias_count_1++;
		}
		weight_bias_count_3_pooling++;
	}
	weight_bias_count_2 = weight_bias_count_1;
	cout << "Ȩ������Ϊ��" << weight_bias_count_1 << endl;
	ifs.close();
	return bias2D;
}

//�����������֣�����
void print_score(vec_t res)
{
	vector<pair<double, int> > scores;

	// sort & print top-3
	for (int i = 0; i < 10; i++)
		scores.emplace_back(rescale<tan_h>(res[i]), i);
	//�����ʴӴ�С����
	sort(scores.begin(), scores.end(), greater<pair<double, int>>());

	for (int i = 0; i < 10; i++)
		cout << scores[i].second << "," << scores[i].first << endl;
	getchar();
}


/*
// save outputs of each layer
for (size_t i = 0; i < nn.depth(); i++) {
auto out_img = nn[i]->output_to_image();
auto filename = "layer_" + std::to_string(i) + ".png";
if (!save_image(filename, out_img)) {
cout << "failed to save " << filename << endl;
}
}
// save filter shape of first convolutional layer
{
auto weight = nn.at<convolutional_layer<tan_h>>(0).weight_to_image();
auto filename = "weights.png";
if (!save_image(filename, weight)) {
cout << "failed to save " << filename << endl;
}
}
*/

int main(int argc, char** argv) {
	float c = 3;

	network<sequential> nn;
	vec_t res;
	vec_t data_in;
	vec_t data_out;
	vec_t data;
	tensor_t in_data;
	tensor_t out_data;

	//convert image to data matrix
	const std::string filename = ".\\ff\\4.bmp";
	convert_image(filename, -1.0, 1.0, 32, 32, data_in);
	//convert_image(filename, -1.0, 1.0, 32, 32, data);

	//construct net and load weights to net
	construct_net(nn);
	/**/
	nn_load_weight("LeNet-weights", nn);

	std::vector<tensor_t> weight2D_conv_1;//�����Ȩ�ؾ���
	vec_t bias2D_conv_1;//ƫ�þ���
	weight2D_conv_1 =load_weight_conv();
	bias2D_conv_1 = load_bias_conv();

	in_data = in_2_3D(data_in);//vector������ͼƬת��tensor
	//out_data = in_data;

	std::vector<tensor_t> in_data2D;
	in_data2D = in_2_2D_conv(nn_in_data_size_conv[0], in_data);//inת���ɶ�ά��ʾ

	std::vector<tensor_t> nn_out_data;

	//convolution_1
	nn_out_data=convolution_layer(nn_in_data_size_conv[0], nn_channel_size_conv, in_data2D, has_connection_table[0], weight2D_conv_1, bias2D_conv_1, nn_out_data, nn_in_number_conv[0], nn_channel_number_conv[0]);
	in_number_conv++;
	
	//std::vector<tensor_t> weight2D_pooling;//�ػ���Ȩ�ؾ���
	vec_t weight_pooling;
	vec_t bias2D_pooling;//ƫ�þ���
	//weight2D_pooling = load_weight_pooling();
	weight_pooling = load_weight_pooling();
	bias2D_pooling = load_bias_pooling();

	//pooling_1
	nn_out_data = pooling_layer(nn_in_data_size_pooling[0], nn_channel_size_pooling, nn_out_data, weight_pooling, bias2D_pooling, nn_out_data, nn_in_number_pooling[0], nn_channel_number_pooling[0]);
	in_number_pooling++;

	std::vector<tensor_t> weight2D_conv_2;//�����Ȩ�ؾ���
	vec_t bias2D_conv_2;//ƫ�þ���
	weight2D_conv_2 = load_weight_conv();
	bias2D_conv_2 = load_bias_conv();

	//ifstream ifs(".\\ff\\in.txt");//in.txt����ŵĵڶ�������������
	//string str;
	//cout << "Դ����ִ�еڶ�������������:" << endl;
	//std::vector<tensor_t> in_data_3;
	//tensor_t in_data_2;//ԭ��������һά��ʾ
	//vec_t vec;//�����������
	//while (ifs >> str)
	//{
	//			float f = atof(str.c_str());
	//			vec.push_back(f);//
	//}
	//in_data_2.push_back(vec);
	//in_data_3.push_back(in_data_2);

	//convolution_2
	//nn_out_data = convolution_layer(nn_in_data_size_conv[1], nn_channel_size_conv, nn_out_data, has_connection_table[1], weight2D_conv_2, bias2D_conv_2, nn_out_data, nn_in_number_conv[1], nn_channel_number_conv[1]);
	//in_number_conv++;

	//print out ranked scores 
	//print_score(res);

	return 0;
}
