#pragma once

#include<vector>
#include<random>
#include<cmath>

#include <Eigen/Dense>

#include<dataParser.h>


#define SIGMOID_BOUND 6

static inline double nor(double value) {
	if (value > 1.0)value = 1.0;
	if (value < -1.0)value = -1.0;
	return value;
}

class SPTF {
public:
	SPTF():ver1(), ver2(), ver3(), ver4(), verUser(), verItem(){

	}
	~SPTF() {
	}
	void setConfig(string inputFileName, int dim, int graStep, int negSamples, double initLr,
	string outUser, string outItem, string outType) {
		inputData = inputFileName;
		this->dim = dim;
		this->graStep = graStep;
		this->negSamples = negSamples;
		this->initLr = initLr;
		this->outUser = outUser;
		this->outItem = outItem;
		this->outType = outType;
	}
	void loadData() {
		DataParser *data = new DataParser(inputData);
		data->csv2data(ver1, ver2, ver3, ver4, verUser, verItem, tri1, tri2, tri3, tri4);
		delete data;
	}
	void outputStatus() {
		cout << "#############SPTF状态输出#############" << endl;
		cout << "Vertex关系集合状态" << "ver1:" << ver1.size() << "###" << "ver2:" << ver2.size() << "###"
			<< "ver3:" << ver3.size() << "###" << "ver4:" << ver4.size() << "###" << endl;
		cout << "Vertex不同种类集合状态" << "verUser:" << verUser.size() << "###" << "verItem:" 
			<< verItem.size() << "###"<< endl;
		cout << "Triple集合状态" << "tri1:" << tri1.size() << "###" << "tri2:" << tri2.size() << "###"
			<< "tri3:" << tri3.size() << "###" << "tri4:" << tri4.size() << "###" << endl;
		cout << "参数状态:" << endl;
		cout << "输入训练集文件名:" << inputData << endl;
		cout << "输出用户参数文件名:" << outUser << endl;
		cout << "输出产品参数文件名:" << outItem << endl;
		cout << "输出动作参数文件名:" << outType << endl;
		cout << "矩阵分解维度:" << dim << endl;
		cout << "迭代次数:" << graStep << endl;
		cout << "负例样本抽取数量:" << negSamples << endl;
		cout << "用户参数MAP:" << userArgs.size()<<endl;
		cout << "产品参数MAP:" << itemArgs.size() << endl;
		cout << "动作参数MAP:" << actionArgs.size() << endl;
		cout << "用户Pop概率生成结果:" << popUsampler.size() << endl;
		cout << "产品Pop概率生成结果:" << popIsampler.size() << endl;
	}
	void training() {
		genPop();
		initVec();
		trainPerType(tri1, ver1);
		trainPerType(tri2, ver2);
		trainPerType(tri3, ver3);
		trainPerType(tri4, ver4);
	}
private:
	//记得要把指针回收！！！！！！！！！！！！！！
	map<Vertex,vector<Vertex> *>  ver1, ver2, ver3, ver4;
	map<Vertex,int> verUser, verItem;
	vector<Triple *> tri1, tri2, tri3, tri4;

	vector<double> popUsampler, popIsampler;

	map<string, vector<double> *> userArgs, itemArgs;
	map<int, vector<double> *> actionArgs;

	string inputData, outUser, outItem, outType;
	int dim, graStep, negSamples;
	double initLr;
	void genPop() {//存在BUG
		genPopPro(verUser, popUsampler);
		genPopPro(verItem, popIsampler);
	}
	void genPopPro(map<Vertex, int> &vec, vector<double> &vecD) {
		int count = 0;
		for (auto i = vec.begin(); i != vec.end(); i++) {
			count += i->second;
		}
		auto iter = vec.begin();
		vecD.push_back(iter->second);
		int i = 1;
		while (++iter != vec.end()) {
			vecD.push_back(vecD[i-1]+(iter->second)*1.0/count);
			i++;
		}
		vecD[vecD.size() - 1] = 1.0;//vecD为递增值，最大为1
	}
	void initVec() {
		for (auto i = verUser.begin(); i != verUser.end(); i++) {
			Vertex temp = i->first;
			userArgs.insert(pair<string, vector<double> *>(temp.getUser(), genGaussian()));
		}
		for (auto i = verItem.begin(); i != verItem.end(); i++) {
			Vertex temp = i->first;
			itemArgs.insert(pair<string, vector<double> *>(temp.getItem(), genGaussian()));
		}
		for (int i = 1; i <= 4;i++) {
			actionArgs.insert(pair<int, vector<double> *>(i, genGaussian()));
		}
	}
	vector<double> *genGaussian() {
		double maxValue = 32767.0;
		vector<double> *newVector = new vector<double>(dim);
		default_random_engine e;
		normal_distribution<> d(0,1);
		for (int i = 0; i < dim; i++) {
			newVector->at(i) = ((nor(d(e))*maxValue - 0.5)/dim);
		}
		return newVector;
	}
	void trainPerType(vector<Triple *> tris, map<Vertex, vector<Vertex> *> vers) {
		default_random_engine e;
		uniform_int_distribution<int> u(0, tris.size()-1);
		int ran = u(e);
		int count = 0;
		auto i = tris.begin();
		for (; i != tris.end(); i++) {
			if (count >= ran) break;
		}
		Vertex v1 = (*i)->getUserID();
		Vertex v2 = (*i)->getItemID();
		Vertex v3 = (*i)->getType();
		negSample(v1, v2, v3, vers, 0);
		negSample(v2, v1, v3, vers, 1);
	}
	void negSample(Vertex &user, Vertex &item, Vertex type, map<Vertex, vector<Vertex> *> vers, int const &flag) {
		// flag = 0, fix user and type, sample item, PIS
		// flag = 1, fix item and type, sample user, PUS
		vector<Vertex> negVer = vector<Vertex>();
		map<Vertex, int> baseList;
		vector<double> baseSample;
		if (flag == 0) {
			baseList = verItem;
			baseSample = popIsampler;
		}
		else if (flag == 1) {
			baseList = verUser;
			baseSample = popUsampler;
		}
		else
		{
			cout << "negSample error" << endl;
		}
		int count = 0;
		while (negVer.size() < negSamples) {
			if (count < baseList.size()) {
				default_random_engine e;
				uniform_real_distribution<double> u(0.0, 1.0);
				double ran = u(e);
				int count2 = 0;
				auto i = baseList.begin();
				for (; i != baseList.end(); i++) {
					if (count2 >= baseSample.size()) break;
					if (baseSample[count2] > ran) break;
					count2++;
				}
				Vertex v = i->first;
			}
			else
			{
				break;
			}
			count++;
		}
		//update();
		if (flag == 0) {
			update(user, item, type, negVer, flag);
		}
		else if (flag == 1) {
			update(item, user, type, negVer, flag);
		}
	}
	void update(Vertex &v1, Vertex &v2, Vertex type, vector<Vertex> &negs ,int const &flag) {
		Eigen::VectorXd vecV1 = Eigen::VectorXd(dim);
		Eigen::VectorXd vecV2 = Eigen::VectorXd(dim);
		Eigen::VectorXd vecT = Eigen::VectorXd(dim);
		vector<double> *argV1;
		vector<double> *argV2;
		vector<double> *argT = actionArgs.find(type.getType())->second;;
		if (flag == 0) {
			argV1 = userArgs.find(v1.getUser())->second;
			argV2 = itemArgs.find(v2.getItem())->second;
		}
		else if (flag == 1) {
			argV1 = itemArgs.find(v1.getItem())->second;
			argV2 = userArgs.find(v2.getUser())->second;
		}
		else {
			cout << "update error1" << endl;
		}
		for (int i = 0; i < dim; i++) {
			vecV1(i) = argV1->at(i);
			vecV2(i) = argV2->at(i);
			vecT(i) = argT->at(i);
		}
		//因为是对V2进行抽样，故负例样本不影响V2，所以优先对V2进行更新
		Eigen::VectorXd x = Eigen::VectorXd(1);
		Eigen::VectorXd vecv2 = Eigen::VectorXd(dim);
		Eigen::VectorXd vecv1 = Eigen::VectorXd(dim);//计算v2对v1的影响量
		Eigen::VectorXd vect = Eigen::VectorXd(dim);//计算v2对v3的影响量
		x = vecV1.transpose() * vecV2 + vecV2.transpose()*vecT + vecT.transpose()*vecV1;
		Sigmoid(x,x);
		double g = (1 - (x.data()[0]))*initLr;
		//开始计算vecv1，vecv3
		vecv1 = g * (vecV2 + vecT);
		vect = g * (vecV2 + vecv1);
		//开始更新v2
		vecv2 = vecV2 + g * (vecV1 + vecT);
		double *vecData = vecv2.data();
		for (int i = 0; i < dim; i++) {
			(*argV2)[i] = vecData[i];
		}
		//v2更新完毕
		//开始计算neg对v1，v3的影响
		Eigen::MatrixXd negMat = Eigen::MatrixXd(negSamples, dim);
		for (int i = 0; i < negs.size(); i++) {
			for (int j = 0; j < dim; j++) {
				if (flag == 0) {
					negMat(i, j) = itemArgs[negs[i].getItem()]->at(j);
				}
				else if (flag == 1) {
					negMat(i, j) = userArgs[negs[i].getUser()]->at(j);
				}
				else {
					cout << "update error2" << endl;
				}
			}
		}
		Eigen::VectorXd xx = Eigen::VectorXd(negSamples);
		Eigen::VectorXd gg = Eigen::VectorXd(negSamples);
		Eigen::VectorXd E = Eigen::VectorXd::Ones(negSamples);
		xx = negMat * vecV1 + negMat * vecT + E*(vecT.transpose()*vecV1);
		Sigmoid(x, x);
		gg = -xx * initLr;
		vecv1 = vecv1 + gg.transpose()*(negMat + E * vecT.transpose());
		vect = vect + gg.transpose()*(negMat + E * vecV1.transpose());
		//开始更新负例影响
		negMat = negMat + gg.transpose()*(E * vecT.transpose() + E * vecV1.transpose());
		double *negData = negMat.data();
		for (int i = 0; i < negSamples; i++) {
			for (int j = 0; j < dim; j++) {
				if (flag == 0) {
					(*itemArgs[negs[i].getItem()])[j] = negData[(i - 1) * 10 + j];
				}
				else if (flag == 1) {
					(*userArgs[negs[i].getUser()])[j] = negData[(i - 1) * 10 + j];
				}
				else {
					cout << "update error3" << endl;
				}
			}
		}
		//neg影响更新完毕
		//开始对v1，v3更新
		vecV1 = vecV1 + vecv1;
		vecT = vecT + vect;
		double * v1Data = vecV1.data();
		double * vtData = vecT.data();
		for (int i = 0; i < dim; i++) {
			(*argT)[i] = vtData[i];
			(*argV1)[i] = v1Data[i];
		}
	}
	template<typename K>
	K sigmoidFunc(K x) {
		if (x > SIGMOID_BOUND)return 1.0;
		if (x < -SIGMOID_BOUND)return 0.0;
		return 1 / (1 + exp(-x));
	}
	void Sigmoid(Eigen::VectorXd& src, Eigen::VectorXd& dst)
	{
		double *src_data = src.data();
		double *dst_data = dst.data();
		for (int i = 0; i < src.size(); ++i)
		{
			dst_data[i] = sigmoidFunc(src_data[i]);
		}
	}
};