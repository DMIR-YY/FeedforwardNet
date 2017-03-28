//#if _HLS_MODE_
//#pragma HLS INTERFACE ap_bus port=in_data_3D
//#pragma HLS RESOURCE core=AXI4M variable=in_data_3D
//
//#pragma HLS INTERFACE ap_bus port=conv_1_weight_a
//#pragma HLS RESOURCE core=AXI4M variable=conv_1_weight_a
//
//#pragma HLS INTERFACE ap_bus port=conv_1_bias_a
//#pragma HLS RESOURCE core=AXI4M variable=conv_1_bias_a
//
//#pragma HLS INTERFACE ap_bus port=conv_2_weight_a
//#pragma HLS RESOURCE core=AXI4M variable=conv_2_weight_a
//
//#pragma HLS INTERFACE ap_bus port=conv_2_bias_a
//#pragma HLS RESOURCE core=AXI4M variable=conv_2_bias_a
//
//#pragma HLS INTERFACE ap_bus port=fc_1_weight_a
//#pragma HLS RESOURCE core=AXI4M variable=fc_1_weight_a
//
//#pragma HLS INTERFACE ap_bus port=fc_1_bias_a
//#pragma HLS RESOURCE core=AXI4M variable=fc_1_bias_a
//
//#pragma HLS INTERFACE ap_bus port=fc_1_out_a
//#pragma HLS RESOURCE core=AXI4M variable=fc_1_out_a
//#endif

//// layer weights and bias inputs ------- LeNet-5
//float        conv_1_weight_a[6][5][5],
//float        conv_1_bias_a[6],
//float        pool_1_weight_a[24],
//float        pool_1_bias_a[6],
//float        conv_2_weight_a[96][5][5],
//float        conv_2_bias_a[16],
//float        pool_2_weight_a[64],
//float        pool_2_bias_a[16],
//float        conv_3_weight_a[1920][5][5],
//float        conv_3_bias_a[120],
//float        fc_1_weight_a[1200][1][1],
//float        fc_1_bias_a[10],

/*****************************************************************************************/
////construct network --------------LeNet-5 example 1
//conv_layer<float, 32, 5, 1, 6> C1;      //1@32x32-in, 6@28x28-out
//pool_layer<float, 28, 2, 6> P2;         //6@28x28-in, 6@14x14-out
//conv_layer<float, 14, 5, 6, 16> C3;     //6@14x14-in, 16@10x10-out
//pool_layer<float, 10, 2, 16> P4;        //16@10x10-in, 16@5x5-out
//conv_layer<float, 5, 5, 16, 120> C5;    //16@5x5-in, 120@1x1-out
//fc_layer<float, 120, 1, 10> F6;         //120@1x1-in, 10@1x1-out

//										//temp storage space for LeNet-5
//float  conv_1_out_a[6][28][28] = { 0 };
//float  pool_1_out_a[6][14][14] = { 0 };
//float  conv_2_out_a[16][10][10] = { 0 };
//float  pool_2_out_a[16][5][5] = { 0 };
//float  conv_3_out_a[120][1][1] = { 0 };

////Forward propagation process
//C1.conv_layer_a(activation_type, in_data_3D, conv_1_weight_a, conv_1_bias_a, conv_1_out_a);
//P2.pooling_layer_a(activation_type, conv_1_out_a, pool_1_weight_a, pool_1_bias_a, pool_1_out_a);
//C3.conv_layer_a_table(activation_type, has_connection_table[1], pool_1_out_a, conv_2_weight_a, conv_2_bias_a, conv_2_out_a);
//P4.pooling_layer_a(activation_type, conv_2_out_a, pool_2_weight_a, pool_2_bias_a, pool_2_out_a);
//C5.conv_layer_a(activation_type, pool_2_out_a, conv_3_weight_a, conv_3_bias_a, conv_3_out_a);
//F6.fc_layer_a(activation_type, conv_3_out_a, fc_1_weight_a, fc_1_bias_a, fc_1_out_a);
/******************************************************************************************/

//// layer weights and bias inputs ------- conv2pool2fc
//float        conv_1_weight_a[6][5][5],
//float        conv_1_bias_a[6],
//float        conv_2_weight_a[96][5][5],
//float        conv_2_bias_a[16],
//float        fc_1_weight_a[160][5][5],
//float        fc_1_bias_a[10],

/******************************************************************************************/
////construct network --------------LeNet-5 example 2
/// conv(1-6) + max Pooling + conv(6-16) + max pooling + fc
//conv_pool_layer<float, 32, 5, 0, 1, 2, 0, 2, 1, 6> C1P2;
//conv_pool_layer<float, 14, 5, 0, 1, 2, 0, 2, 6, 16> C3P4;
//fc_layer<float, 16, 5, 10> F5;
//
////temp storage space
//float  pool_1_out[6][14][14] = { 0 };
//float  pool_2_out[16][5][5] = { 0 };
//
////Forward propagation process
//C1P2.conv_layer_w_pool_a(activation_type, in_data_3D, conv_1_weight_a, conv_1_bias_a, pool_1_out);
//C3P4.conv_layer_w_pool_a(activation_type, pool_1_out, conv_2_weight_a, conv_2_bias_a, pool_2_out);
//F5.fc_layer_a(activation_type, pool_2_out, fc_1_weight_a, fc_1_bias_a, fc_1_out_a);

/******************************************************************************************/
////construct network --------------LeNet-5 example 3
////conv(1-6) + max Pooling + conv(6-16) + max pooling + fc
/*conv_pool_layer<float, 32, 5, 0, 1, 2, 0, 2, 1, 6> C1P2;
conv_pool_layer<float, 14, 5, 0, 1, 2, 0, 2, 6, 16> C3P4;*/