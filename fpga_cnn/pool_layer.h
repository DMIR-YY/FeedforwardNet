//
// Created by yaochen on 29/12/16.
//
//TODO: if conv_layer use relu, then max_pooling do not need to use relu again. Need to modify this part of code.

#ifndef _POOLING_LAYER_H_
#define _POOLING_LAYER_H_

#include <iostream>
#include <fstream>
#include "activation_functions.h"
//#include "config.h"

//using namespace std;

template <typename T, int _INPUT_SIZE_, int _POOL_KERNEL_SIZE_, int _POOL_PADDING_, int _POOL_STRIDE_, int _IN_CHANNEL_NUM_>
class pool_layer {

private:
	int pooling_layer_number;

public:
	pool_layer() :pooling_layer_number(0) {};

	/************************************************************************************/
	// pooling kernel function with array input
	void pooling_kernel_a(
		T in_data[_INPUT_SIZE_][_INPUT_SIZE_],
		T kernel_weight,
		T out_data[][(_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1]) {
		if (_POOL_KERNEL_SIZE_ % 2 != 0) {//_POOL_KERNEL_SIZE_ is an odd or even,the loop is different
			for (int i = _POOL_KERNEL_SIZE_ / 2 - _POOL_PADDING_; i < _INPUT_SIZE_ + _POOL_PADDING_ - _POOL_KERNEL_SIZE_ / 2; i += _POOL_STRIDE_) {
				for (int j = _POOL_KERNEL_SIZE_ / 2 - _POOL_PADDING_; j < _INPUT_SIZE_ + _POOL_PADDING_ - _POOL_KERNEL_SIZE_ / 2; j += _POOL_STRIDE_) {
					T sum = 0;
					for (int ii = -_POOL_KERNEL_SIZE_ / 2; ii <= _POOL_KERNEL_SIZE_ / 2; ++ii) {
						for (int jj = -_POOL_KERNEL_SIZE_ / 2; jj <= _POOL_KERNEL_SIZE_ / 2; ++jj) {
							T data = in_data[i + ii][j + jj];
							sum += data;
						}
					}
					sum = (T)(sum / (_POOL_KERNEL_SIZE_ * _POOL_KERNEL_SIZE_));//���ÿ��pooling�����ڵľ�ֵ
					out_data[(i - _POOL_KERNEL_SIZE_ / 2 + _POOL_PADDING_) / _POOL_STRIDE_][(j - _POOL_KERNEL_SIZE_ / 2 + _POOL_PADDING_) / _POOL_STRIDE_] = sum * kernel_weight;//ÿ�������ͬһ��weight
				}
			}
		}
		else {
			for (int i = _POOL_KERNEL_SIZE_ / 2 - _POOL_PADDING_; i <= _INPUT_SIZE_ + _POOL_PADDING_ - _POOL_KERNEL_SIZE_ / 2; i += _POOL_STRIDE_) {
				for (int j = _POOL_KERNEL_SIZE_ / 2 - _POOL_PADDING_; j <= _INPUT_SIZE_ + _POOL_PADDING_ - _POOL_KERNEL_SIZE_ / 2; j += _POOL_STRIDE_) {
					T sum = 0;
					for (int ii = -_POOL_KERNEL_SIZE_ / 2; ii < _POOL_KERNEL_SIZE_ / 2; ++ii) {
						for (int jj = -_POOL_KERNEL_SIZE_ / 2; jj < _POOL_KERNEL_SIZE_ / 2; ++jj) {
							T data = in_data[i + ii][j + jj];
							sum += data;
						}
					}
					sum = (T)(sum / (_POOL_KERNEL_SIZE_ * _POOL_KERNEL_SIZE_));//���ÿ��pooling�����ڵľ�ֵ
					out_data[(i - _POOL_KERNEL_SIZE_ / 2 + _POOL_PADDING_) / _POOL_STRIDE_][(j - _POOL_KERNEL_SIZE_ / 2 + _POOL_PADDING_) / _POOL_STRIDE_] = sum * kernel_weight;//ÿ�������ͬһ��weight
				}
			}
		}

#if _C_DEBUG_MODE_
#if _KERNEL_DEBUG_
//		cout << "pooling kernel a input array...." << endl;
		ofstream pool_kernel_a;
		pool_kernel_a.open("pool_kernel_a.txt", ios::app);
//		pool_kernel_a << "pooling kernel a input data" << endl;
		pool_kernel_a << kernel_weight << endl;
		for (int i = 0; i < _INPUT_SIZE_; i++) {
			for (int j = 0; j < _INPUT_SIZE_; j++) {
				pool_kernel_a << in_data[i][j] << " ";
			}
			pool_kernel_a << endl;
		}
		pool_kernel_a << endl;

//		pool_kernel_a << "pooling kernel a output data" << endl;
		for (int i = 0; i < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; i++) {
			for (int j = 0; j < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; j++) {
				pool_kernel_a << out_data[i][j] << " ";
			}
			pool_kernel_a << endl;
		}
		pool_kernel_a.close();
		cout << endl;
#endif
#endif
	}

	/************************************************************************************/
	//average pooling layer function with array input
	void pooling_layer_a(
		char activation_type,
		T in_data3D[_IN_CHANNEL_NUM_][_INPUT_SIZE_][_INPUT_SIZE_],
		T kernel_weights[],
		T kernel_bias[],
		T out_data3D[_IN_CHANNEL_NUM_][(_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1]
		[(_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1]) {

		cout << "Starting average_pooling ...." << endl;

		for (int a = 0; a < _IN_CHANNEL_NUM_; a++) {//input kernel loop
			pooling_kernel_a(
				in_data3D[a],
				kernel_weights[a],
				out_data3D[a]);
			//ѭ������out_data2D�����ƫ�úͼ���
			for (int i = 0; i < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; i++) {
				for (int j = 0; j < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; j++) {
					out_data3D[a][i][j] = f(activation_type, (out_data3D[a][i][j] + kernel_bias[a]));
				}
			}
		}
		cout << "Finished pooling array layer...." << endl;

		//debugging output
#if _C_DEBUG_MODE_
		ofstream out_pool_a;
		out_pool_a.open("pool_layer_a.txt", ios::app);
		for (int i = 0; i < _IN_CHANNEL_NUM_; i++) {
			for (int j = 0; j < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; j++) {
				for (int k = 0; k < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; k++) {
					out_pool_a << out_data3D[i][j][k] << " ";
				}
				out_pool_a << endl;
			}
			out_pool_a << endl;
		}
		out_pool_a.close();
		cout << endl;
#endif
	}

	/************************************************************************************/
	// pooling kernel function with array input without kernel weights
	void pooling_kernel_a_no_w(
		T in_data[_INPUT_SIZE_][_INPUT_SIZE_],
		T out_data[][(_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1]) {
		//for (int i = 0; i < _INPUT_SIZE_ - _KERNEL_SIZE_ / 2; i = i + _KERNEL_SIZE_) {
		//	for (int j = 0; j < _INPUT_SIZE_ - _KERNEL_SIZE_ / 2; j = j + _KERNEL_SIZE_) {
		//		T sum = 0;
		//		for (int ii = 0; ii < _KERNEL_SIZE_; ++ii) {
		//			for (int jj = 0; jj < _KERNEL_SIZE_; ++jj) {
		//				T data = in_data[i + ii][j + jj];
		//				sum += data;
		//			}
		//		}
		//		sum = (T)(sum / (_KERNEL_SIZE_ * _KERNEL_SIZE_));//���ÿ��pooling�����ڵľ�ֵ
		//		out_data[i / _KERNEL_SIZE_][j / _KERNEL_SIZE_] = sum;//ÿ�����벻��weight
		//	}
		//}
		if (_POOL_KERNEL_SIZE_ % 2 != 0) {//_POOL_KERNEL_SIZE_ is an odd or even,the loop is different
			for (int i = _POOL_KERNEL_SIZE_ / 2 - _POOL_PADDING_; i < _INPUT_SIZE_ + _POOL_PADDING_ - _POOL_KERNEL_SIZE_ / 2; i += _POOL_STRIDE_) {
				for (int j = _POOL_KERNEL_SIZE_ / 2 - _POOL_PADDING_; j < _INPUT_SIZE_ + _POOL_PADDING_ - _POOL_KERNEL_SIZE_ / 2; j += _POOL_STRIDE_) {
					T sum = 0;
					for (int ii = -_POOL_KERNEL_SIZE_ / 2; ii <= _POOL_KERNEL_SIZE_ / 2; ++ii) {
						for (int jj = -_POOL_KERNEL_SIZE_ / 2; jj <= _POOL_KERNEL_SIZE_ / 2; ++jj) {
							T data = in_data[i + ii][j + jj];
							sum += data;
						}
					}
					sum = (T)(sum / (_POOL_KERNEL_SIZE_ * _POOL_KERNEL_SIZE_));//���ÿ��pooling�����ڵľ�ֵ
					out_data[(i - _POOL_KERNEL_SIZE_ / 2 + _POOL_PADDING_) / _POOL_STRIDE_][(j - _POOL_KERNEL_SIZE_ / 2 + _POOL_PADDING_) / _POOL_STRIDE_] = sum;//ÿ�����벻��weight
				}
			}
		}
		else {
			for (int i = _POOL_KERNEL_SIZE_ / 2 - _POOL_PADDING_; i <= _INPUT_SIZE_ + _POOL_PADDING_ - _POOL_KERNEL_SIZE_ / 2; i += _POOL_STRIDE_) {
				for (int j = _POOL_KERNEL_SIZE_ / 2 - _POOL_PADDING_; j <= _INPUT_SIZE_ + _POOL_PADDING_ - _POOL_KERNEL_SIZE_ / 2; j += _POOL_STRIDE_) {
					T sum = 0;
					for (int ii = -_POOL_KERNEL_SIZE_ / 2; ii < _POOL_KERNEL_SIZE_ / 2; ++ii) {
						for (int jj = -_POOL_KERNEL_SIZE_ / 2; jj < _POOL_KERNEL_SIZE_ / 2; ++jj) {
							T data = in_data[i + ii][j + jj];
							sum += data;
						}
					}
					sum = (T)(sum / (_POOL_KERNEL_SIZE_ * _POOL_KERNEL_SIZE_));//���ÿ��pooling�����ڵľ�ֵ
					out_data[(i - _POOL_KERNEL_SIZE_ / 2 + _POOL_PADDING_) / _POOL_STRIDE_][(j - _POOL_KERNEL_SIZE_ / 2 + _POOL_PADDING_) / _POOL_STRIDE_] = sum ;//ÿ�����벻��weight
				}
			}
		}

#if _C_DEBUG_MODE_
//		cout << "pooling kernel a input array...." << endl;
		ofstream pool_kernel_a;
		pool_kernel_a.open("pool_kernel_a.txt", ios::app);
		pool_kernel_a << "pooling kernel a input data" << endl;
		for (int i = 0; i < _INPUT_SIZE_; i++) {
			for (int j = 0; j < _INPUT_SIZE_; j++) {
				pool_kernel_a << in_data[i][j] << " ";
			}
			pool_kernel_a << endl;
		}
		pool_kernel_a << endl;

//		pool_kernel_a << "pooling kernel a output data" << endl;
		for (int i = 0; i < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; i++) {
			for (int j = 0; j < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; j++) {
				pool_kernel_a << out_data[i][j] << " ";
			}
			pool_kernel_a << endl;
		}
		pool_kernel_a.close();
		cout << endl;
#endif

	}

	/************************************************************************************/
	//average pooling layer function with array input without pooling weights
	void pooling_layer_a_no_w(
		char activation_type,
		T in_data3D[_IN_CHANNEL_NUM_][_INPUT_SIZE_][_INPUT_SIZE_],
		T out_data3D[_IN_CHANNEL_NUM_][(_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1]
		[(_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1]) {

		cout << "Starting average_pooling ...." << endl;

		for (int a = 0; a < _IN_CHANNEL_NUM_; a++) {//input kernel loop
			pooling_kernel_a_no_w(
				in_data3D[a],
				out_data3D[a]);
			//ѭ������out_data2D�����ƫ�úͼ���
			for (int i = 0; i < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; i++) {
				for (int j = 0; j < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; j++) {
					out_data3D[a][i][j] = f(activation_type, out_data3D[a][i][j]);
				}
			}
		}
		cout << "Finished pooling array layer...." << endl;

		//debugging output
#if _C_DEBUG_MODE_
		ofstream out_pool_a;
		out_pool_a.open("pool_layer_a.txt", ios::app);
		for (int i = 0; i < _IN_CHANNEL_NUM_; i++) {
			for (int j = 0; j < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; j++) {
				for (int k = 0; k < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; k++) {
					out_pool_a << out_data3D[i][j][k] << " ";
				}
				out_pool_a << endl;
			}
			out_pool_a << endl;
		}
		out_pool_a.close();
		cout << endl;
#endif
	}

	/************************************************************************************/
	// max pooling kernel function with array input without kernel weights
	void max_pooling_kernel_a(
		T in_data[_INPUT_SIZE_][_INPUT_SIZE_],
		T out_data[][(_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1]) {

		if (_POOL_KERNEL_SIZE_ % 2 != 0) {//_POOL_KERNEL_SIZE_ is an odd or even,the loop is different
			for (int i = _POOL_KERNEL_SIZE_ / 2 - _POOL_PADDING_; i < _INPUT_SIZE_ + _POOL_PADDING_ - _POOL_KERNEL_SIZE_ / 2; i += _POOL_STRIDE_) {
				for (int j = _POOL_KERNEL_SIZE_ / 2 - _POOL_PADDING_; j < _INPUT_SIZE_ + _POOL_PADDING_ - _POOL_KERNEL_SIZE_ / 2; j += _POOL_STRIDE_) {
					T max = 0;
					for (int ii = -_POOL_KERNEL_SIZE_ / 2; ii <= _POOL_KERNEL_SIZE_ / 2; ++ii) {
						for (int jj = -_POOL_KERNEL_SIZE_ / 2; jj <= _POOL_KERNEL_SIZE_ / 2; ++jj) {
							if (i + ii >= 0 && i + ii< _INPUT_SIZE_
								&& j + jj >= 0 && j + jj< _INPUT_SIZE_) {//if overlapped
								max = (in_data[i + ii][j + jj] > max ? in_data[i + ii][j + jj] : max);
							}
						}
					}
					out_data[(i - _POOL_KERNEL_SIZE_ / 2 + _POOL_PADDING_) / _POOL_STRIDE_][(j - _POOL_KERNEL_SIZE_ / 2 + _POOL_PADDING_) / _POOL_STRIDE_] = max;
				}
			}
		}
		else {
			for (int i = _POOL_KERNEL_SIZE_ / 2 - _POOL_PADDING_; i <= _INPUT_SIZE_ + _POOL_PADDING_ - _POOL_KERNEL_SIZE_ / 2; i += _POOL_STRIDE_) {
				for (int j = _POOL_KERNEL_SIZE_ / 2 - _POOL_PADDING_; j <= _INPUT_SIZE_ + _POOL_PADDING_ - _POOL_KERNEL_SIZE_ / 2; j += _POOL_STRIDE_) {
					T max = 0;
					for (int ii = -_POOL_KERNEL_SIZE_ / 2; ii < _POOL_KERNEL_SIZE_ / 2; ++ii) {
						for (int jj = -_POOL_KERNEL_SIZE_ / 2; jj < _POOL_KERNEL_SIZE_ / 2; ++jj) {
							if (i + ii >= 0 && i + ii< _INPUT_SIZE_
								&& j + jj >= 0 && j + jj< _INPUT_SIZE_) {//if overlapped
								max = (in_data[i + ii][j + jj] > max ? in_data[i + ii][j + jj] : max);
							}
						}
					}
					out_data[(i - _POOL_KERNEL_SIZE_ / 2 + _POOL_PADDING_) / _POOL_STRIDE_][(j - _POOL_KERNEL_SIZE_ / 2 + _POOL_PADDING_) / _POOL_STRIDE_] = max;
				}
			}
		}

#if _C_DEBUG_MODE_
#if _KERNEL_DEBUG_
//		cout << "pooling kernel a input array...." << endl;
		//ofstream pool_kernel_a;
		//pool_kernel_a.open("pool_kernel_a.txt", ios::app);
		//pool_kernel_a << "pooling kernel a input data" << endl;
		//for (int i = 0; i < _INPUT_SIZE_; i++) {
		//	for (int j = 0; j < _INPUT_SIZE_; j++) {
		//		pool_kernel_a << in_data[i][j] << " ";
		//	}
		//	pool_kernel_a << endl;
		//}
		//pool_kernel_a << endl;

//		pool_kernel_a << "pooling kernel a output data" << endl;
		//for (int i = 0; i < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; i++) {
		//	for (int j = 0; j < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; j++) {
		//		pool_kernel_a << out_data[i][j] << " ";
		//	}
		//	pool_kernel_a << endl;
		//}
		//pool_kernel_a.close();
		//cout << endl;
#endif
#endif
	}

	/************************************************************************************/
	//max pooling layer function with array input without pooling weights
	void max_pooling_layer_a(
		char activation_type,
		T in_data3D[_IN_CHANNEL_NUM_][_INPUT_SIZE_][_INPUT_SIZE_],
		T out_data3D[_IN_CHANNEL_NUM_][(_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1]
		[(_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1]) {

#if _C_DEBUG_MODE_
#if _KERNEL_DEBUG_
		cout << "Starting Max_pooling layer ...." << endl;
#endif
#endif

		for (int a = 0; a < _IN_CHANNEL_NUM_; a++) {//input kernel loop
			max_pooling_kernel_a(
				in_data3D[a],
				out_data3D[a]);
			//ѭ������out_data2D�����ƫ�úͼ���
			for (int i = 0; i < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; i++) {
				for (int j = 0; j < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; j++) {
#if _ACT_RELU_
                    out_data3D[a][i][j] = Relu_64(out_data3D[a][i][j]);
#else
					out_data3D[a][i][j] = f(activation_type, out_data3D[a][i][j]);
#endif
				}
			}
		}

#if _C_DEBUG_MODE_
#if _KERNEL_DEBUG_
		cout << "Finished Max_pooling layer ...." << endl;
        //debugging output
		ofstream out_pool_a;
		out_pool_a.open("pool_layer_a.txt", ios::app);
		out_pool_a << "output from pool layer .........................." << endl;
		for (int i = 0; i < _IN_CHANNEL_NUM_; i++) {
			for (int j = 0; j < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; j++) {
				for (int k = 0; k < (_INPUT_SIZE_ + _POOL_PADDING_ * 2 - _POOL_KERNEL_SIZE_) / _POOL_STRIDE_ + 1; k++) {
					out_pool_a << out_data3D[i][j][k] << " ";
				}
				out_pool_a << endl;
			}
			out_pool_a << endl;
		}
		out_pool_a.close();
		cout << endl;
#endif
#endif
	}
};

#endif //_POOLING_LAYER_H
