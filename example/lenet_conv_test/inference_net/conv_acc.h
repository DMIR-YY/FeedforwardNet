//
// Created by Yao Chen on 27/05/2017
// 

#ifndef _CONV_ACC_H_
#define _CONV_ACC_H_

#include <iostream>
#include <fstream>
#include "activation_functions.h"

#if _C_DEBUG_MODE_
#include <algorithm>
#endif

using namespace std;

template <typename T, typename W, typename G, int Tm, int Tn, int Tr, int Tc>
class conv_acc {

private:
    int conv_layer_number;

public:
    conv_acc() : conv_layer_number(0) {conv_layer_number = 0;};

///////////////////////------------------conv accelerator----------------//////////////////////////
    void conv_layer_acc(
        int N, //input feature number
        int K, //input kernel size
        int M, // output feature number
        int R, // output Row
        int C, // output column
        int S, // stride size
        int P, // padding size
        T *in_data, // in_data[N][(R-1)*S + K][(C-1)*S + K] --> [N][(R-1)*S + K - 2*P][(C-1)*S + K - 2*P]
        W *layer_weights, //w[M][N][K][K]
        W *layer_bias, // b[M]
        G *out_data){ // out[M][R][C]

            //buffer local data before computation
            T in_buf[Tn][(Tr-1)*S + K][(Tc-1)*S + K];
            G out_buf[Tm][Tr][Tc];
            W w_buf[Tm][Tn][K][K];
            W b_buf[Tm];

            for(int r = 0; r < R; r += Tr){
                for(int c = 0; c < C; c += Tc){
                    for(int m = 0; m < M; m += Tm){
                        for(int n = 0; n < N; n += Tn){

                            // load input data
                            for(int i = n; i < n+Tn; i++){
                                for(int j = r*S - P; j < (r+Tr-1)*S + K - P; j++){
                                    for(int k = c*S - P; k < (c+Tc-1)*S + K - P; k++){
                                        if(j < 0 || j >= ((R-1)*S + K - 2*P) || k < 0 || k >= ((C-1)*S + K - 2*P)){
                                            in_buf[i-n][j-r*S+P][k-c*S+P] = 0;}
                                        else{
                                            in_buf[i-n][j-r*S+P][k-c*S+P] = *(in_data + i*((R-1)*S+K - 2*P)*((C-1)*S+K - 2*P) + j*((C-1)*S+K - 2*P) +k);}
//            							    in_buf[i-n][j-r*S][k-c*S] = *(in_data + i*((R-1)*S+K)*((C-1)*S+K) + j*((C-1)*S+K) +k);
                                    }
                                }
                            }
#if _C_DEBUG_MODE_
#if _KERNEL_DEBUG_
                            ofstream conv_acc;
                            conv_acc.open("conv_in_data.txt", ios::app);
                            conv_acc <<"conv input: "<< endl;
                            for (int i = 0; i < std::min(Tn,n); i++) {
                                for (int j = 0; j < (Tr-1)*S + K; j++) {
                                    for(int k = 0; k < (Tc-1)*S + K; k++){
                                        conv_acc << in_buf[i][j][k] << " ";
                                    }
                                    conv_acc << endl;
                                }
                                conv_acc << endl;
                            }
                            conv_acc.close();
#endif
#endif
                            // load input weights
                            for(int i = m; i < m+Tm; i++){
                                for(int j = n; j < n+Tn; j++){
                                    for(int k1 = 0; k1 < K; k1++){
                                        for(int k2 = 0; k2 < K; k2++){
                                            w_buf[i-m][j-n][k1][k2] = *(layer_weights + i*N*K*K + j*K*K + k1*K + k2);
                                        }
                                    }
                                }
                                b_buf[i-m] = *(layer_bias + i);
                            }
#if _C_DEBUG_MODE_
#if _KERNEL_DEBUG_
                            ofstream conv_w;
                            conv_w.open("conv_in_weights.txt", ios::app);
                            for (int i = 0; i < Tm; i++) {
                                for (int j = 0; j < Tn; j++) {
                                    for(int k = 0; k < K; k++){
                                        for(int l = 0; l < K; l++){
                                            conv_w << w_buf[i][j][k][l] << " ";
                                            cout << w_buf[i][j][k][l] << " ";
                                        }
                                        conv_w << endl;
                                        cout << endl;
                                    }
                                    conv_w << endl;
                                    cout << endl;
                                }
                                conv_w << endl;
                                cout << endl;
                            }
                            conv_w.close();
#endif
#endif
                            for(int i=0; i<Tm; i++){
                                for(int j=0; j<Tr; j++){
                                    for(int k=0; k<Tc; k++){
                                        out_buf[i][j][k] = 0;
                                    }
                                }
                            }
                            // convolutional accelerator
                            for(int i=0; i<K; i++){
                                for(int j=0; j<K; j++){
                                    for(int tr=0; tr+r<min(R, r+Tr); tr++){
                                        for(int tc=0; tc+c<min(C, c+Tc); tc++){
#pragma HLS PIPELINE
                                            for(int tm=0; tm<Tm; tm++){ // unroll loop kernel
#pragma HLS UNROLL
                                                for(int tn=0; tn<Tn; tn++){ // unroll loop kernel
#pragma HLS UNROLL
                                                    out_buf[tm][tr][tc] += w_buf[tm][tn][i][j]*in_buf[tn][S*tr+i][S*tc+j];
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // transfer output data
                        ofstream conv_out_buf;
                        conv_out_buf.open("conv_out_buf.txt", ios::app);
                        int flag1=0;
                        int flag2=0;
                        if(R<r+Tr){
                            flag1=1;
                        }if(C<c+Tc){
                            flag2=1;
                        }
                        for(int i = 0; i < Tm; i++){
                            for(int j=0; j < (flag1>0?(R%Tr):Tr); j++){
                                for(int k=0; k < (flag2>0?(C%Tc):Tc); k++){

                                    if ((out_buf[i][j][k] + b_buf[i]) >= 0) {
                                        conv_out_buf << (out_buf[i][j][k] + b_buf[i]) << " ";
                                        *(out_data + (i + m) * R * C + (j + r) * C + k + c) = (out_buf[i][j][k] + b_buf[i]);
                                        out_buf[i][j][k] = 0;
                                    }
                                    else{
                                        conv_out_buf << 0 << " ";
                                        *(out_data + (i + m) * R * C + (j + r) * C + k + c) = 0;
                                        out_buf[i][j][k] = 0;
                                    }
                                }
                                conv_out_buf << endl;
                            }
                            conv_out_buf << endl;
                        }
                        conv_out_buf.close();
                    }
                }
            }
#if _C_DEBUG_MODE_
#if _KERNEL_DEBUG_
            ofstream conv_out;
            conv_out.open("conv_out_data.txt", ios::app);
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < R; j++) {
                    for(int k = 0; k < C; k++){
                        conv_out << *(out_data + i*R*C + j*C + k) << " ";
                    }
                    conv_out << endl;
                }
                conv_out << endl;
            }
            conv_out.close();
#endif
#endif
        }
};

#endif