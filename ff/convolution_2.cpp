// ConsoleApplication3.cpp : �������̨Ӧ�ó������ڵ㡣
//#include "stdafx.h"
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
//#include "hls_video.h"
//#include "hls_opencv.h"

//#include "../tiny_dnn/util/util.h"
//#define _HAS_ITERATOR_DEBUGGING 0
//#define _SECURE_SCL_ 0

using namespace tiny_dnn;
using namespace tiny_dnn::activation;
using namespace std;

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
			connection_table(tbl, 6, 16))              // C3, 6@14x14-in, 16@10x10-in
		<< average_pooling_layer<tan_h>(10, 10, 16, 2)  // S4, 16@10x10-in, 16@5x5-out
		<< convolutional_layer<tan_h>(5, 5, 5, 16, 120) // C5, 16@5x5-in, 120@1x1-out
		<< fully_connected_layer<tan_h>(120, 10);       // F6, 120-in, 10-out

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

std::vector<tensor_t> forward(const std::vector<tensor_t>& in, network<sequential> nn) {
	tensor_t in_data = in[0];//ԭ��������һά��ʾ
	tensor_t in_data2D;//ת���ɶ�ά��ʾ
	fill_tensor(in_data2D, float(0));
	vec_t vec1;//�����������
	vec_t vec2;//�����������
	for (int i = 0; i < in_data[0].size(); i++)
	{
		//if (i==0||i % 32!=0) {�������У���Ϊÿ�еĵ�һ��Ԫ�ض���
		if(i==0||(i-1)/32==i/32){//��ԭ����һά���밴ͼƬ��Сת���ɶ�ά
			vec1.push_back(in_data[0][i]);//��������mapÿ�����ص�����ֵ��������������
			if (i == in_data[0].size() - 1) {//���һ������������
				in_data2D.push_back(vec1);//�����������������ɶ�ά����tensor
				vec1.clear();
			}
		}
		else {
			in_data2D.push_back(vec1);
			vec1.clear();
			vec1.push_back(in_data[0][i]);//����������������һ��Ԫ��
		}
	}
	tensor_t out_data2D;
	vector<tensor_t> out;
	fill_tensor(out_data2D, float(0));
	cout << "����map����" << in_data.size()<<endl;//1
	cout << "����map������" << in_data[0].size() << endl;//1024
	/*for(int i=0;i< in_data.size(); i++) {
		const vec_t& in = in_data[i];
		vec_t& a = out_data[i];*/
		int K = 5;//kernel����
		int m = 32;//ͼƬm��n
		int n = 32;
		ifstream ifs("LeNet-weights");
		string str;
		int count = 0;
		int bias_num=0;
		tensor_t weight2D;//Ȩ��&ƫ�þ���weight2D[0]��Ȩ�أ�weight2D[1]��ƫ��
		vec_t vec3;//Ȩ������
		vec_t vec4;//ƫ������
		while (ifs >> str&&count<156)
		{
			if (count<150) {//ǰ150��������weight
				float f = atof(str.c_str());
				vec3.push_back(f);//����Ȩ��
				cout << vec3[count] << " ";
			}
			else {//��6����bias
				float f = atof(str.c_str());
				vec4.push_back(f);//����ƫ��
				cout << vec4[bias_num] << " ";
				bias_num++;
			}
			count++;
		}
		weight2D.push_back(vec3);
		weight2D.push_back(vec4);
		cout << "Ȩ������Ϊ��" << count << endl;
		ifs.close();
		for (int a = 0; a < (*nn[0]->weights()[1]).size(); a++) {
			for (int i = K / 2; i < m - K / 2; ++i) //��������map
			{
				for (int j = K / 2; j < n - K / 2; ++j)
				{
					float sum = 0; //
					for (int ii = -K / 2; ii <= K / 2; ++ii) //����kernel
					{
						for (int jj = -K / 2; jj <= K / 2; ++jj)
						{
							float data = in_data2D[i + ii][j + jj];
							//float weight = (*nn[0]->weights()[0])[a*K*K+(ii + K / 2) * 5 + (jj + K / 2)];
							float weight = weight2D[0][a*K*K+(ii + K / 2) * 5 + (jj + K / 2)];
							sum += data * weight;// ���mapÿ�����ص����ֵ
						}
					}
					//sum += (*nn[0]->weights()[1])[a];//����ƫ��bias
					sum += weight2D[1][a];
					const float ep = exp(sum);
					const float em = exp(-sum);
					sum= (ep - em) / (ep + em);//tan_h����
					vec2.push_back(sum);//����sum�������������
				}
				out_data2D.push_back(vec2);//��������������������map
				vec2.clear();
			}
			out.push_back(out_data2D);//�������map������ά���
			out_data2D.clear();
		}
		//for (int i = 0; i < out.size(); i++) {//����ȫ�����map
			for (int j = 0; j < out[0].size(); j++) {
				for (int k = 0; k < out[0][j].size(); k++) {
					cout << out[0][j][k]<<" ";
				}
				cout << endl;
			}
			cout << endl;
				//if (!params.tbl.is_connected(o, inc)) continue;
	return out;
}

std::vector<tensor_t> fprop(const std::vector<tensor_t>& in, network<sequential> nn) {
	for (int i = 0; i < in.size(); i++) {
		for (int j = 0; j < in[i].size(); j++) {
			for (int k = 0; k < in[i][j].size(); k++) {
				cout << in[i][j][k]<<" ";
			}
			cout << endl;
		}
		cout << endl;
	}
	return forward(in,nn);
}

std::vector<vec_t> fprop(const std::vector<vec_t>& in, network<sequential> nn) {
	/*for (int i = 0; i < in.size(); i++) {
		for (int j = 0; j < in[i].size(); j++) {
				cout << in[i][j];
		}
		cout << endl;
	}*/
	return fprop(std::vector<tensor_t>{ in },nn)[0];
}

vec_t fprop(const vec_t& in, network<sequential> nn) {
	//if (in.size() != (size_t)in_data_size())
	//	data_mismatch(**net_.begin(), in);
	return fprop(std::vector<vec_t>{ in },nn)[0];
}

vec_t predict(const vec_t& in, network<sequential> nn) { 
	return fprop(in,nn); 
}

void recognize(network<sequential>& nn, vec_t& data, vec_t& res) {

	// recognize
	res = predict(data,nn);
	//return res;
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
	cnn_size_t in_channels_;   // number of input vectors
	cnn_size_t out_channels_;  // number of output vectors
	//std::vector<vector_type> in_type_;
	//std::vector<vector_type> out_type_;
	float c = 3;

	network<sequential> nn;
	vec_t res;
	vec_t data;

	//convert image to data matrix
	const std::string filename = "4.bmp";
	convert_image(filename, -1.0, 1.0, 32, 32, data);

	//construct net and load weights to net
	construct_net(nn);
	/**/
	nn_load_weight("LeNet-weights", nn);

	//image<unsigned char> img = nn[0]->output_to_image(); // visualize activations of recent input 
	//img.write("layer0.bmp");
	//image<unsigned char> img1 = nn.at< convolutional_layer<tan_h>>(0).weight_to_image();
	//img1.write("kernel0.bmp");

	//cout <<"Դ����ִ��������:"<< endl;
	//vector<tensor_t> source_out=nn[0]->output();//Դ����ִ��������
	//for (int i = 0; i < source_out.size(); i++) {
	//	for (int j = 0; j < source_out[i].size(); j++) {
	//		for (int k = 0; k < source_out[i][j].size(); k++) {
	//			cout << source_out[i][j][k] << " ";
	//		}
	//		cout << endl;
	//	}
	//	cout << endl;
	//}

	//prediction stage    
	recognize(nn, data, res);

	//print out ranked scores 
	print_score(res);
	/**/
	return 0;
}
