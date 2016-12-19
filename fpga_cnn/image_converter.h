//Move all the image pre-processing functions here to simplify the main function.
//re-construct the array data of images into 3d
tensor_t in_2_3D(vec_t& data_in) {
	tensor_t data_out;
	data_out.push_back(data_in);
	return data_out;
}

//re-construct input array to 2d matrix
std::vector<tensor_t> in_2_2D_conv(int& input_size, tensor_t in) {
	vec_t in_data; //original input data
	vec_t vec1;    //input row vector
	tensor_t vec2;
	std::vector<tensor_t> in_data2D;
	for (int j = 0; j < in.size(); j++) {
		in_data = in[j];
		for (int i = 0; i < in_data.size(); i++)//����һάת��ά
		{
			if (in_data.size() < input_size*input_size) {//�ж������
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