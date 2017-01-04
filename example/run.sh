#!/bin/bash

rm -rf fftest out_conv.txt out_pool.txt out_fc.txt

max=60
for (( i=2; i <= max; ++i))
do
	echo -e "\n"
done


g++ -g -Wall -Wextra -pedantic -fpermissive -I ../ -O0 ff_test.cpp -o fftest
