import json
import sys
import math
import helping_functions

EOL = "\n"
EOS = ";"
SPACE = " "
PARAMETER_BEGIN = "("
PARAMETER_END = ")"
BODY_BEGIN = "{"
BODY_END = "}"
SEPARATER = "   "
ARRAY_BEGIN = "["
ARRAY_END = "]"
EQUAL = " = "
MULT = " * "
NULL = "NULL"
FOR = "for"
LESS = " < "
COMMA_SPACE = ", "
INCREMENT = "++"
PREP_ELSE = "#else"
PREP_ENDIF = "#endif"
KERNEL = "#if _KERNEL_DEBUG_"
HLS = "#if _HLS_MODE_"
BATCH_NORM = "#if _BATCH_NORM_"
SCALE = "#if _SCALE_"

def generate(generated_file_name="ff_test.cpp"):
    paraJS = open("parameter2.json", "r")
    json_data = json.load(paraJS)
    str1 = ""

    arr = helping_functions.read_params(sys.argv[1])
    str1 += generate_import(json_data["import"])
    str1 += generate_function(json_data["function"], "loadfile")
    str1 += generate_fn_load()
    str1 += generate_function(json_data["function"], "main")
    str_body = generate_body(json_data["body"], json_data["out"], json_data["comments"], arr)
    str1 += str_body

    with open("../example/test_demo/" + generated_file_name, "w") as generated_file:
        generated_file.write(str1)


def generate_import(import_json):
	import_str = EOL
	for import_sen in import_json["include"]:
        	import_str += import_sen + EOL
	import_str += EOL
	for import_sen in import_json["namespace"]:
        	import_str += import_sen + EOL
	
	return import_str

def generate_function(fn_json, nm):
	fn_str = EOL
	for fn_sen in fn_json[nm]:
        	fn_str += fn_sen["return_type"] + SPACE + fn_sen["function_name"] + PARAMETER_BEGIN 
		for i, p in enumerate(fn_sen["parameters"]):
			fn_str += p["type"] + p["name"]
			if (i+1) != len(fn_sen["parameters"]):
				fn_str += COMMA_SPACE
		fn_str += PARAMETER_END + SPACE + BODY_BEGIN + EOL
	return fn_str

def generate_fn_load(prefix=SEPARATER):
	fn_bd = prefix
	fn_bd += "std::ifstream fs(file.c_str(), std::ios::binary);" + EOL + prefix +\
	         "fs.seekg(0, std::ios::end);" + EOL + prefix +\
	         "size = fs.tellg();" + EOL + prefix +\
		 "char * data = new char[size];" + EOL + prefix +\
		 "fs.seekg(0);" + EOL + prefix +\
		 "fs.read(data, sizeof(char) * size);" + EOL + prefix +\
		 "fs.close();" + EOL + prefix +\
		 "return (unsigned char *)data;" + EOL +\
		 BODY_END + EOL
	return fn_bd

def generate_body(body_json, out_json, comm_json, arr, prefix=SEPARATER):
	col_gray = ""
	while (col_gray != "color" and col_gray != "grayscale"):
        	col_gray = raw_input("\nPlease enter color specification input (color, grayscale): ")
		if col_gray == "color":
			chn = 3
		elif col_gray == "grayscale":
			chn = 1
		else:
			print "Please enter \"color\" for colored image and \"grayscale\" for grayscaled one "

	sz = "sizeof"
	ms = "memset"
	body_str = EOL
	body_str1 = ""
	alloc_str = EOL + prefix + comm_json[11] + EOL
	body_str += prefix + out_json[0] + EOL
	body_str += EOL + prefix + comm_json[10] 
	body_str += EOL
	value = ""
	arr1, arr1_str = helping_functions.extraction(arr)
	arrr = arr1[arr1_str.index("in_data_mem_size")].split(" * ")
	arr2 = arr1[arr1_str.index("conv_weight_size")].split(" + ")
	arr3 = ""
	if "conv_bias_size" in arr1_str:
		arr3 = arr1[arr1_str.index("conv_bias_size")].split(" + ")
	if "fc_bias_size" in arr1_str:
		arr4 = arr1[arr1_str.index("fc_weight_size")].split(" + ")
		arr5 = arr1[arr1_str.index("fc_bias_size")].split(" + ")
	n_layers = arr1[arr1_str.index("n")]
	layers_order = arr1[arr1_str.index("layers_order")]

	if "fc_bias_size" in arr1_str:
		fc_out = "fc_" + str(n_layers) + "_out"

	'''make only one nn_batch_norm_size and nn_scale_size declaration'''
	repeat1 = False
	repeat2 = False
	for k, var_sen in enumerate(body_json["var_init"]):
		if var_sen["name"] in arr1_str:
			if var_sen["name"] == "nn_batch_norm_size":
				if not repeat1:
					body_str += prefix + var_sen["type"] + SPACE
					body_str += var_sen["name"] 
					repeat1 = True
					body_str += EQUAL + PARAMETER_BEGIN 
					body_str += arr1[arr1_str.index(var_sen["name"])] 
					body_str += PARAMETER_END + MULT + sz +\
						PARAMETER_BEGIN + var_sen["size"] + PARAMETER_END +\
						EOS + EOL
			elif var_sen["name"] == "nn_scale_size":
				if not repeat2:
					body_str += prefix + var_sen["type"] + SPACE
					body_str += var_sen["name"] 
					repeat2 = True
					body_str += EQUAL + PARAMETER_BEGIN 
					body_str += arr1[arr1_str.index(var_sen["name"])] 
					body_str += PARAMETER_END + MULT + sz +\
						PARAMETER_BEGIN + var_sen["size"] + PARAMETER_END +\
						EOS + EOL
			else:
				body_str += prefix + var_sen["type"] + SPACE
				if var_sen["name"] == "fc_out_size":
					body_str += fc_out + "_size"
				else:
					body_str += var_sen["name"] 
				body_str += EQUAL + PARAMETER_BEGIN 
				body_str += arr1[arr1_str.index(var_sen["name"])] 
				body_str += PARAMETER_END + MULT + sz +\
						PARAMETER_BEGIN + var_sen["size"] + PARAMETER_END +\
						EOS + EOL

	print "\nPlease make sure the Tm and Tn can be divided by the number of ports!"
	port_num = int(helping_functions.prompt("\nPlease enter the number of ports: "))
	with open("parameter3.json","w") as f:
		json.dump(str(port_num),f)

	maximum = ""
	if int(arrr[1])*int(arrr[1]) > int(math.ceil(float(arr1[arr1_str.index("maximum")])/port_num)):
		maximum = int(arrr[1])*int(arrr[1])
	else:
		maximum = int(math.ceil(float(arr1[arr1_str.index("maximum")])/port_num))
	for i in range(0,2):
		for j in range(1,port_num + 1):
			body_str += prefix + "unsigned int" + SPACE
			body_str += "out_size_" + str(i) + "_" + str(j)
			body_str += EQUAL + PARAMETER_BEGIN 
			body_str += str(maximum)
			body_str += PARAMETER_END + MULT + sz +\
						PARAMETER_BEGIN + "data_type_o" + PARAMETER_END +\
						EOS + EOL	
	if "Eltwise" in layers_order or "Concat" in layers_order:
			for j in range(1,port_num + 1):
				body_str += prefix + "unsigned int" + SPACE
				body_str += "out_size_2_" + str(j)
				body_str += EQUAL + PARAMETER_BEGIN 
				body_str += str(maximum)
				body_str += PARAMETER_END + MULT + sz +\
							PARAMETER_BEGIN + "data_type_o" + PARAMETER_END +\
							EOS + EOL	

	ker = 0
	for k, var_sen in enumerate(body_json["var_init"]):
		if (var_sen["memory"] == "fc_8_out_mem_int"):
			alloc_str += KERNEL + EOL
			ker = 1
		
		if var_sen["name"] in arr1_str:
			alloc_str += prefix + var_sen["size"] + SPACE + "*"

			if var_sen["name"] == "fc_out_size":
					cond1 = "fc_" + str(n_layers) + "_out_mem_int"
					alloc_str += "fc_" + str(n_layers) + "_out_mem_int"
					fcout = "fc_" + str(n_layers) + "_out_size"
			else:
					cond1 = var_sen["memory"]
					alloc_str += var_sen["memory"]
					fcout = var_sen["name"]
			alloc_str += EQUAL + PARAMETER_BEGIN + var_sen["size"] + "*" + PARAMETER_END +\
				     "malloc" + PARAMETER_BEGIN + fcout + PARAMETER_END + EOS + EOL
				
			cond = cond1 + " == " + NULL
			alloc_str += prefix + helping_functions.generate_if(cond, [out_json[1] + var_sen["memory"] + "\\n\"" +\
				     PARAMETER_END + EOS], ["printf(\"" + var_sen["location"] + "= 0x%x \\n\", " + cond1 + PARAMETER_END + EOS], 1)
			if ker == 1:
				alloc_str += PREP_ENDIF + EOL
				ker = 0
	
	for i in range(0,2):
		for j in range(1,port_num + 1):
			alloc_str += prefix + "data_type_o *" + "temp_out_" + str(i) + "_" + str(j)
			alloc_str += EQUAL + PARAMETER_BEGIN + "data_type_o *" + PARAMETER_END +\
			     "malloc" + PARAMETER_BEGIN + "out_size_" + str(i) + "_" + str(j) + PARAMETER_END + EOS + EOL 
			cond = "temp_out_" + str(i) + "_" + str(j) + " == " + NULL
			alloc_str += prefix + helping_functions.generate_if(cond, [out_json[1] + "temp_out_" + str(i) + "_" + str(j) + "\\n\"" +\
			     PARAMETER_END + EOS], ["printf(\"" + "temp_out_" + str(i) + "_" + str(j) + " memory location" + "= 0x%x \\n\", " + "temp_out_" + str(i) + "_" + str(j) + PARAMETER_END + EOS], 1)
	if "Eltwise" in layers_order or "Concat" in layers_order:
		for j in range(1,port_num + 1):
			alloc_str += prefix + "data_type_o *" + "temp_out_2_" + str(j)
			alloc_str += EQUAL + PARAMETER_BEGIN + "data_type_o *" + PARAMETER_END +\
			     "malloc" + PARAMETER_BEGIN + "out_size_2_" + str(j) + PARAMETER_END + EOS + EOL 
			cond = "temp_out_2_" + str(j) + " == " + NULL
			alloc_str += prefix + helping_functions.generate_if(cond, [out_json[1] + "temp_out_2_" + str(j) + "\\n\"" +\
			     PARAMETER_END + EOS], ["printf(\"" + "temp_out_2_" + str(j) + " memory location" + "= 0x%x \\n\", " + "temp_out_2_" + str(j) + PARAMETER_END + EOS], 1)

	body_str1 += KERNEL + EOL
	body_str1 += prefix + out_json[2] + EOL
	if "fc_bias_size" in arr1_str:
		body_str1 += prefix + "memset(fc_" + str(n_layers) + "_out_mem_int, 0, fc_" + str(n_layers) + "_out_size);" + EOL
	else:
		body_str1 += prefix + "memset(out_mem_int, 0, out_size);" + EOL
	if "nn_batch_norm_size" in arr1_str:
		body_str1 += prefix + "memset(batch_norm_mem_port_param1, 0, nn_batch_norm_size);" + EOL
		body_str1 += prefix + "memset(batch_norm_mem_port_param2, 0, nn_batch_norm_size);" + EOL
	if "nn_scale_size" in arr1_str:
		body_str1 += prefix + "memset(scale_mem_port_param1, 0, nn_scale_size);" + EOL
		body_str1 += prefix + "memset(scale_mem_port_param2, 0, nn_scale_size);" + EOL
	
	for i in range(0,2):
		for j in range(1,port_num + 1):
			body_str1 += prefix + "memset" + PARAMETER_BEGIN + "temp_out_" + str(i) + "_" + str(j)
			body_str1 += ", 0, " + "out_size_" + str(i) + "_" + str(j) + PARAMETER_END + EOS + EOL
	if "Eltwise" in layers_order or "Concat" in layers_order:
		for j in range(1,port_num + 1):
			body_str1 += prefix + "memset" + PARAMETER_BEGIN + "temp_out_2_" + str(j)
			body_str1 += ", 0, " + "out_size_2_" + str(j) + PARAMETER_END + EOS + EOL

	body_str1 += PREP_ENDIF + EOL*2

	body_str1 += prefix + comm_json[0] + EOL
	body_str1 += HLS + EOL
	body_str1 += prefix + "const char* weight_src = \"net_weights.txt\";" + EOL
	body_str1 += PREP_ELSE + EOL
	body_str1 += prefix + "const char* weight_src = \"net_inputs/net_weights.txt\";" + EOL
	body_str1 += PREP_ENDIF + EOL

	#body_str1 += HLS + EOL
	#body_str1 += prefix + "ifstream ifs(\"val.txt\");" + EOL
	#body_str1 += PREP_ELSE + EOL
	#body_str1 += prefix + "ifstream ifs(\"net_inputs/val.txt\");" + EOL
	#body_str1 += PREP_ENDIF + EOL
	#body_str1 += prefix + "string val_name[10];" + EOL + prefix + "float val_class[10];" +\
	#	     EOL + prefix + "string str;" + EOL
	#body_str1 += prefix + helping_functions.generate_if("!ifs", [out_json[6], "getchar();"], "", 1)
	#body_str1 += prefix + "int num = 0;" + EOL
	#body_str1 += prefix + helping_functions.generate_while("ifs >> str&&num<20", 
	#			             [helping_functions.generate_if("num % 2 == 1", ["val_class[num / 2] = int(atof(str.c_str()));"], 
	#				     ["val_name[num / 2] = str;"], 2), "num++;"], 1)
	#body_str1 += prefix + "ifs.close();" + EOL*2
	indata_mem = arr1[arr1_str.index("in_data_mem_size")].split(" * ")
	if chn == 3:
		body_str1 += prefix + comm_json[1] + EOL
		body_str1 += HLS + EOL
		body_str1 += prefix + "ifstream ifs1(\"net_mean.txt\");" + EOL
		body_str1 += PREP_ELSE + EOL
		body_str1 += prefix + "ifstream ifs1(\"net_inputs/net_mean.txt\");" + EOL
		body_str1 += PREP_ENDIF + EOL	
		body_str1 += EOL*2
		body_str1 += prefix + "float channel_mean[3] = { 0 };" + EOL +\
		     prefix + "string str1;" + EOL +\
		     prefix + "string y1 = \"[\";" + EOL +\
		     prefix + "string y2 = \"]\";" + EOL +\
		     prefix + helping_functions.generate_if("!ifs1", [out_json[5], "getchar();"], "", 1) 
		body_str1 += prefix + "int index = 0;" + EOL
		body_str1 += prefix + helping_functions.generate_while("ifs1 >> str1", ["int p1 = str1.find(y1, 0);", 
		     helping_functions.generate_if("p1 >= 0", ["str1.erase(p1, y1.length());"], "", 2),
		     "int p2 = str1.find(y2, 0);", 
		     helping_functions.generate_if("p2 >= 0", ["str1.erase(p2, y2.length());"], "", 2),
		     "float f = atof(str1.c_str());",
		     "channel_mean[index] = f;",
		     "index++;"], 1)
		body_str1 += prefix + "ifs1.close();" + EOL*2
		body_str1 += prefix + comm_json[2] + EOL
		height = helping_functions.prompt("Please enter the height of the image: ")
		width = helping_functions.prompt("Please enter the width of the image: ")
		
		body_str1 += prefix + comm_json[3] + EOL
		body_str1 += KERNEL + EOL + HLS + EOL +\
			     prefix + "string image_dir = \"" + sys.argv[2] + "\";" + EOL + PREP_ELSE + EOL +\
			     prefix + "string image_dir = \"./net_inputs/test_imgs/" + sys.argv[2] + "\"" + EOS + EOL +\
			     PREP_ENDIF + EOL
		
		body_str1 += prefix + "float in_data_3D_channel_swap[3" + ARRAY_END +\
			     ARRAY_BEGIN + height + ARRAY_END + ARRAY_BEGIN + width +"] = { 0 };" +\
			     EOL + prefix + "float in_data_3D[3" + ARRAY_END +\
			     ARRAY_BEGIN + indata_mem[1] + ARRAY_END + ARRAY_BEGIN + indata_mem[2] +\
			     "] = { 0 };" + EOL +\
			     prefix + "int crop_w = " + arrr[1] + ";"+ EOL + prefix + "int crop_h = " +\
			     arrr[1] + ";" + EOL + prefix + "int w;" + EOL + prefix + "int h;" + EOL +\
			     prefix + "int channels;" +\
			     EOL + prefix + "int size;" + EOL + prefix +\
			     "const unsigned char * data = loadfile(image_dir, size);" + EOL +\
			     prefix + "const unsigned char * image_orig = stbi_load_from_memory(data, size, &w, &h, &channels, 3);" +\
			     EOL 
	
		body_str1 += prefix + helping_functions.generate_for_loop("i", "int", 0, 3, [helping_functions.generate_for_loop("j", "int", "i", "w*h*3", 
			     ["in_data_3D_channel_swap[2 - i][j / (w * 3)][(j % (w * 3) - i) / 3] = (float)image_orig[j]; //range:0--255"], 2, 3)], 1, 1)
		body_str1 += prefix + helping_functions.generate_for_loop("i", "int", 0, 3, [helping_functions.generate_for_loop("j", "int", 0, "h", 
			     [helping_functions.generate_for_loop("k", "int", 0, "w", ["in_data_3D_channel_swap[i][j][k] /= 255;// range:0--1"], 3, 1)], 2, 1)], 1, 1)
		body_str1 += prefix + "resize_image(in_data_3D_channel_swap, h, w, in_data_3D);" + EOL
		body_str1 += prefix + helping_functions.generate_for_loop("i", "int", 0, 3, [helping_functions.generate_for_loop("j", "int", 0, "crop_h", 
			     [helping_functions.generate_for_loop("k", "int", 0, "crop_w", ["in_data_3D[i][j][k] = in_data_3D[i][j][k] * 255 - channel_mean[i];"], 
			     3, 1)], 2, 1)], 1, 1)
		body_str1 += prefix + out_json[10] + EOL

		for_str = ""
		for l in range(1,port_num + 1):
			for_str += "if(i+" + str(l-1) + "<3){" + EOL +prefix*5
			for_str += "temp_out_0_" + str(l) + "[in_data_size]" + EQUAL + "(data_type)in_data_3D" + ARRAY_BEGIN + "i+" + str(l-1) + ARRAY_END + "[j][k];" + EOL +prefix*4 + BODY_END

		body_str1 += prefix + out_json[11] + EOL + prefix + out_json[11] + EOL +\
			     prefix + "int in_data_size=0;" + EOL +\
			     prefix + helping_functions.generate_for_loop("i", "int", 0, 3, [helping_functions.generate_for_loop("j", "int", 0, "crop_h", 
			     [helping_functions.generate_for_loop("k", "int", 0, "crop_w", [for_str,
			     "in_data_size++;"], 3, 1)], 2, 1)], 1, port_num) 
		body_str1 += prefix + out_json[12] + EOL*2
		body_str1 += PREP_ENDIF + EOL*2
	else:
		body_str1 += KERNEL + EOL + HLS + EOL
		body_str1 += prefix + "string image_dir = \"" + sys.argv[2] + "\";" + EOL + PREP_ELSE + EOL
		body_str1 += prefix + "string image_dir = \"./net_inputs/test_imgs/" + sys.argv[2] + "\"" + EOS + EOL + PREP_ENDIF + EOL
		body_str1 += prefix + "int w;" + EOL + prefix + "int h;" + EOL + prefix + "int channels;" + EOL + prefix + "int size;" + EOL + prefix +\
			     "const unsigned char * data = loadfile(image_dir, size);" + EOL +\
			     prefix + "const unsigned char * image_orig = stbi_load_from_memory(data, size, &w, &h, &channels, 1);" +\
			     EOL 
		body_str1 += prefix + "int in_data_size=0;" + EOL
		body_str1 += prefix + "ofstream indata;" + EOL + prefix + "indata.open(\"in_data.txt\");"+ EOL
		body_str1 += prefix + helping_functions.generate_for_loop("i", "int", 0, 1, [helping_functions.generate_for_loop("j", "int", 0, indata_mem[2], [helping_functions.generate_for_loop("k", "int", 0, indata_mem[2], ["indata << image_orig[i *" + indata_mem[2] + "*" + indata_mem[2] + " + " + indata_mem[2] + "*j + k] << \" \";"], 3, 1), "indata << endl;"], 2, 1), "indata << endl;"], 1, 1)
		body_str1 += prefix + "indata.close();" + EOL*2

		body_str1 += prefix + "cout << \"Writing data to input data memory space ... ... ...\" << endl;" + EOL
		body_str1 += prefix + helping_functions.generate_for_loop("i", "int", 0, 1, [helping_functions.generate_for_loop("j", "int", 0, indata_mem[2], [helping_functions.generate_for_loop("k", "int", 0, indata_mem[2], ["temp_out_0_1[in_data_size] = (data_type)image_orig[i*" + indata_mem[2] + "*" + indata_mem[2] + " + " + indata_mem[2] + "*j + k];", "in_data_size++;"], 3, 1)], 2, 1)], 1, 1)
		body_str1 += prefix + "cout << \"Finished writing data to input data memory space ... ...\" << endl;" + EOL + PREP_ENDIF + EOL

	body_str1 += prefix + "char tan_h = 't';" + EOL +\
		     prefix + "char relu = 'r';" + EOL +\
		     prefix + "char none = 'i';" + EOL +\
		     prefix + "int in_number_conv = 0;" + EOL +\
		     prefix + "int in_number_fc = 0;" + EOL +\
		     prefix + "int in_number_pooling = 0;" + EOL*2 

	body_str1 += generate_weights_biases(len(arr2), "conv", arr2, arr3)
	if "fc_bias_size" in arr1_str:
		body_str1 += generate_weights_biases(len(arr4), "fc", arr4, arr5)

	if "nn_batch_norm_size" in arr1_str:
		body_str1 += prefix + 'get_batch_norm_mean("net_inputs/batch_norm_mean.txt",batch_norm_mem_port_param1);' + EOL
		body_str1 += prefix + 'get_batch_norm_denominator("net_inputs/batch_norm_denominator.txt",batch_norm_mem_port_param2);' + EOL
	if "nn_scale_size" in arr1_str:
		body_str1 += prefix + 'get_batch_norm_gamma("net_inputs/scale_gamma.txt",scale_mem_port_param1);' + EOL
		body_str1 += prefix + 'get_batch_norm_beta("net_inputs/scale_beta.txt",scale_mem_port_param2);' + EOL + EOL

	body_str1 += KERNEL + EOL
	if "fc_bias_size" in arr1_str:
		body_str1 += prefix + "float fc_" + str(n_layers) + "_out[" + arr1[arr1_str.index("fc_out_size")] + "] = { 0 };" + EOL
	else:
		body_str1 += prefix + "float out[" +arr1[arr1_str.index("out_size")] + "] = { 0 };" + EOL
	body_str1 += prefix + "clock_t start, finish, inf_start, inf_finish;" + EOL +\
		     prefix + "double totaltime, inf_time;" + EOL +\
		     prefix + "start = clock();" + EOL +\
		     PREP_ENDIF + EOL*2
	body_str1 += prefix + comm_json[4] + EOL
	body_str1 += prefix + "inference_net(" + EOL + prefix + comm_json[7] + EOL + prefix + "conv_weight_mem_port," + EOL
	if "conv_bias_size" in arr1_str:
		body_str1 += prefix + "conv_bias_mem_port," + EOL 
	if "fc_bias_size" in arr1_str:
		body_str1 += prefix + "fc_weight_mem_port," + EOL
		body_str1 += prefix + "fc_bias_mem_port," + EOL 
	if "nn_batch_norm_size" in arr1_str:
		body_str1 += prefix + "batch_norm_mem_port_param1," + EOL + prefix + "batch_norm_mem_port_param2," + EOL
	if "nn_scale_size" in arr1_str:
		body_str1 += SCALE + EOL + prefix + "scale_mem_port_param1," + EOL + prefix + "scale_mem_port_param2," + EOL + PREP_ENDIF + EOL
		     
	if "fc_bias_size" in arr1_str:
		body_str1 += KERNEL + EOL + prefix + comm_json[8] + EOL + prefix + "fc_" + str(n_layers) + "_out_mem_int," + EOL
	else:
		body_str1 += KERNEL + EOL + prefix + comm_json[8] + EOL + prefix + "out_mem_int," + EOL

	if "Eltwise" in layers_order or "Concat" in layers_order:
		for i in range(0,2):
			for j in range(1,port_num + 1):
				body_str1 += prefix + "temp_out_" + str(i) + "_" + str(j) + "," + EOL
		for j in range(1,port_num + 1):
				if j == port_num:
					body_str1 += prefix + "temp_out_2_" + str(j) + ");" + EOL*2
				else:
					body_str1 += prefix + "temp_out_2_" + str(j) + "," + EOL
	else:
		for i in range(0,2):
			for j in range(1,port_num + 1):
				if i == 1 and j == port_num:
					body_str1 += prefix + "temp_out_" + str(i) + "_" + str(j) + ");" + EOL*2
				else:
					body_str1 += prefix + "temp_out_" + str(i) + "_" + str(j) + "," + EOL

	body_str1 += prefix + "finish = clock();" + EOL + prefix +\
  		     "totaltime = (double)(finish - start) / CLOCKS_PER_SEC;" +\
		     EOL + prefix + out_json[7] + EOL
    
	if "fc_bias_size" in arr1_str:
		body_str1 += prefix + helping_functions.generate_for_loop("i", "int", 0, arr1[arr1_str.index("fc_out_size")], ["fc_" + str(n_layers) + "_out[i]=(float)(fc_" + str(n_layers) + "_out_mem_int[i]);"], 1, 1)
		body_str1 += prefix + "softmax(" + fc_out + ", " + arr1[arr1_str.index("fc_out_size")] + ");" +\
			     EOL + prefix + "predict(" + fc_out +", " + arr1[arr1_str.index("fc_out_size")] + ");" + EOL +\
			     PREP_ENDIF + EOL*2
	else:
		body_str1 += prefix + helping_functions.generate_for_loop("i", "int", 0, arr1[arr1_str.index("out_size")], ["out[i]=(float)(" + "out_mem_int[i]);"], 1, 1)
		body_str1 += prefix + "softmax(out, " + arr1[arr1_str.index("out_size")] + ");" +\
			     EOL + prefix + "predict(out, " + arr1[arr1_str.index("out_size")] + ");" + EOL +\
			     PREP_ENDIF + EOL*2
	body_str1 += prefix + "return 0;" + EOL*2 + BODY_END


	return body_str + alloc_str + body_str1

def generate_w_b(nm, arr, s, c, s1, prefix=SEPARATER):
	wb_str = ""
	wb_str = prefix + "float *" + nm + " = (float*)malloc(" + arr[c] + " * sizeof(float));" + EOL
	wb_str += prefix + "memset(" + nm + ", 0, " + arr[c] + " * sizeof(float));" + EOL
	wb_str += prefix + "load_" + s + "_" + s1 + "(" + EOL + "weight_src, " + EOL + nm + "," + EOL + " weight_bias_record,"  + EOL + " nn_channel_size_" + s1 + ", " + EOL +\
	         " nn_in_number_" + s1 + ","  + EOL + " nn_out_number_" + s1 + ","  + EOL +" in_number_" + s1 + ");" + EOL
	if c == 0:
		wb_str += prefix + "int " + s1 + "_" + s + "_num=0;" + EOL
		
	wb_str += prefix + "cout << \"Loading " + s1 + " " + s + " " + str(c + 1) + " to memory space, starting at: \" <<" + s1 + "_" + s +"_num << '\\n';" + EOL
	wb_str += prefix + helping_functions.generate_for_loop("i", "int", 0, arr[c], 
	                               [s1 + "_" + s + "_mem_port[" + s1 + "_" + s +"_num] = (data_type_w)" + nm + "[i];", 
				       s1 + "_" + s + "_num++;"], 1, 1)
	wb_str += prefix + "free(" + nm + ");" + EOL

	return wb_str

def generate_weights_biases(length, s, arr1, arr2, prefix=SEPARATER):
	comm = "// Prepare weights and bias for "
	
	array = helping_functions.read_params(sys.argv[1])
	arr, arr_str = helping_functions.extraction(array)

	wb_str = ""
	for c in range(length):
		c_name = s + "_" + str(c + 1) + "_weight2D"
		b_name = s + "_" + str(c + 1) + "_bias2D"
		wb_str += prefix + comm + s + " layer " + str(c+1) + EOL
		wb_str += generate_w_b(c_name, arr1, "weight", c, s)
		if s == "conv":
			if "conv_bias_size" in arr_str:
				wb_str += generate_w_b(b_name, arr2, "bias", c, s)
		elif s == "fc":
			if "fc_bias_size" in arr_str:
				wb_str += generate_w_b(b_name, arr2, "bias", c, s)
		wb_str += prefix + "in_number_" + s +"++;" + EOL + EOL

	wb_str += prefix + "cout<<\"Finished loading " + s + " weight into memory! Total: \" <<" + s +"_weight_num  << \"... ... ...\"<<endl;" + EOL
	if s == "conv":
		if "conv_bias_size" in arr_str:
			wb_str += prefix +  "cout<<\"Finished loading " + s + " bias into memory! Total: \" <<" + s + "_bias_num  << \"... ... ...\"<<endl;" + EOL*2
	elif s == "fc":
		if "fc_bias_size" in arr_str:
			wb_str += prefix +  "cout<<\"Finished loading " + s + " bias into memory! Total: \" <<" + s + "_bias_num  << \"... ... ...\"<<endl;" + EOL*2

	return wb_str


if __name__ == "__main__":
    generate()
    