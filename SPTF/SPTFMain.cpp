// SPTF.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>    



#include<SPTF.h>

using namespace std; 

int  main(int argc, char* argv[]) {
	//MatrixXd m(2, 2);
	//m(0, 0) = 3;
	//m(1, 0) = 2.5;
	//m(0, 1) = -1;
	//m(1, 1) = m(1, 0) + m(0, 1);
	//cout << m << endl;
	SPTF *test = new SPTF();
	test->setConfig("train.csv", 64, 100000, 2, 0.025, "user.csv", "item.csv", "type.csv");
	test->loadData();
	test->training();
	test->outputStatus();
	return 0;
}