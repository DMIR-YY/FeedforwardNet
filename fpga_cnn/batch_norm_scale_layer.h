
#ifndef _BATCH_NORM_SCALE_LAYER_H_
#define _BATCH_NORM_SCALE_LAYER_H_

#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <fstream>
#include <algorithm>
#include <math.h>
//#include "config.h"
#include "pow_function.h"
#include "activation_functions.h"

using namespace std;

template <typename T, int _IN_CHANNEL_NUM_, int _INPUT_SIZE_>
class batch_norm_scale_layer {

private:
	int batch_norm_scale_layer_num;

public:
	batch_norm_scale_layer() :batch_norm_scale_layer_num(0) {};

	/************************************************************************************************/
	void batch_norm_scale_layer_a(
		T *mean,
		T *denominator,
		T *gamma,
		T *beta,
		int bn_offset,
		int scale_offset,
		T *in_data3D,
		T *out_data3D) {
#if _C_DEBUG_MODE_
#if _KERNEL_DEBUG_
        cout << "Starting batch_norm_scale layer ...." << endl;
#endif
#endif
			T gamma_=0;
		    T beta_=0;
		    T mean_=0;
		    T denominator_=0;
		    T x_normed = 0;
			for(int n = 0; n < _IN_CHANNEL_NUM_; n++){
				mean_ = (*(mean + bn_offset + n));
				denominator_ = (*(denominator + bn_offset + n));
				gamma_ = (*(gamma + scale_offset + n));
			    beta_ = (*(beta + scale_offset + n));
				for(int i=0;i<_INPUT_SIZE_;i++){
					for(int j=0;j<_INPUT_SIZE_;j++){
					    x_normed = ((*(in_data3D + n*_INPUT_SIZE_*_INPUT_SIZE_ + i*_INPUT_SIZE_ +j)) - mean_) * denominator_;
					    (*(out_data3D + n*_INPUT_SIZE_*_INPUT_SIZE_ + i*_INPUT_SIZE_ +j)) = relu(gamma_* x_normed + beta_);
				    }
				}
			}
#if _C_DEBUG_MODE_
#if _KERNEL_DEBUG_
        cout << "Finished batch_norm_scale layer ...." << endl;
		ofstream out_batch_norm_scale_a;
		out_batch_norm_scale_a.open("batch_norm_scale_layer_a.txt",ios::app);
		out_batch_norm_scale_a << "output from batch norm scale layer .........................." << endl;
		for(int n = 0; n < _IN_CHANNEL_NUM_; n++){
			for(int i=0;i<_INPUT_SIZE_;i++){
				for(int j=0;j<_INPUT_SIZE_;j++){
				    out_batch_norm_scale_a << *(out_data3D + n*_INPUT_SIZE_*_INPUT_SIZE_ + i*_INPUT_SIZE_ +j) << " ";
				}
				out_batch_norm_scale_a << endl;
			}
			out_batch_norm_scale_a << endl;
		}
		out_batch_norm_scale_a.close();
#endif
#endif
	}
};
#endif