//
// Created by yaochen on 23/6/17.
//

#ifndef _ACC_INSTANCE_H_
#define _ACC_INSTANCE_H_

#include "conv_acc_break.h"
#include "conv_acc_break_noact.h"
#include "max_pool_acc.h"
#include "config.h"

conv_acc<data_type, data_type_w, data_type_o, 64, 3, 24, 24> convAcc2;//{0<Tm<=M;0<Tn<=N;0<Tr<=R;0<Tc<=C;}

void conv_layer_new(
        int N, //input feature number
        int K, //input kernel size
        int M, // output feature number
        int R, // output Row
        int C, // output column
        int S, // stride size
        int P, // padding size
        data_type *in_data, // in_data[N][(R-1)*S + K][(C-1)*S + K] --> [N][(R-1)*S + K - 2*P][(C-1)*S + K - 2*P]
        data_type_w *layer_weights, //w[M][N][K][K]
        data_type_w *layer_bias, // b[M]
        data_type_o *out_data,
        int weight_offset,
        int bias_offset,
	    int in_offset,
	    int out_offset) {

    convAcc2.conv_layer_acc(N, K, M, R, C, S, P, in_data, layer_weights, layer_bias, out_data, weight_offset, bias_offset, in_offset, out_offset);

};

conv_acc_noact<data_type, data_type_w, data_type_o, 32, 3, 32, 32> convAcc1;

void conv_layer_new_noact(
	    int N,
	    int K,
        int M, // output feature number
        int R, // output Row
        int C, // output column
        int S, // stride size
        int P, // padding size
        data_type *in_data, // in_data[N][(R-1)*S + K][(C-1)*S + K] --> [N][(R-1)*S + K - 2*P][(C-1)*S + K - 2*P]
        data_type_w *layer_weights, //w[M][N][K][K]
        data_type_w *layer_bias, // b[M]
        data_type_o *out_data,
        int weight_offset,
        int bias_offset,
	    int in_offset,
	    int out_offset) {

    convAcc1.conv_layer_acc_noact(N, K, M, R, C, S, P, in_data, layer_weights, layer_bias, out_data, weight_offset, bias_offset, in_offset, out_offset);

};


max_pool_acc<data_type, data_type_w, data_type_o, 64, 12, 12> maxPoolAcc1;

void max_pool_layer_new(
		int R_in,
        int C_in,
        int N,
        int K,
	    int R,
	    int C,
	    int S,
	    int P,
	    data_type *in_data,
	    data_type_o *out_data){

    maxPoolAcc1.max_pool_layer_acc(R_in, C_in, N, K, R, C, S, P, in_data, out_data);

};

#endif //_ACC_INSTANCE_H_