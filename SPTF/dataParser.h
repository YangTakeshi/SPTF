#pragma once
#include <iostream>    
#include<vector>
#include<map>

//#include <Eigen/Dense> 
#include <csv.h>

#include<triple.h>

//using namespace Eigen;
using namespace std;

class DataParser{
public:
	//DataParser(MatrixXd * mat, string fileName) {
	DataParser(string fileName) {
		csvReader = new io::CSVReader<3>(fileName);
		csvReader->read_header(io::ignore_extra_column, "user", "item", "action");
	}
	~DataParser() {
		delete csvReader;
	}
	void csv2data(map<Vertex, vector<Vertex> *> &ver1, map<Vertex, vector<Vertex> *> &ver2, map<Vertex, vector<Vertex> *> &ver3,
		map<Vertex, vector<Vertex> *> &ver4, map<Vertex, int> &verUser, map<Vertex, int> &verItem, vector<Triple *> &tri1, vector<Triple *> &tri2,
		vector<Triple *> &tri3, vector<Triple *> &tri4) {
		std::string user, item; int action;
		while (csvReader->read_row(user, item, action)) {
			Triple * newTriple = new Triple(user, item, action);
			switch (action) {
			case 1:
				tri1.push_back(newTriple);
				insert(ver1, newTriple, 0, verUser, verItem);
				insert(ver1, newTriple, 1, verUser, verItem);
				break;
			case 2:
				tri2.push_back(newTriple);
				insert(ver2, newTriple, 0, verUser, verItem);
				insert(ver2, newTriple, 1, verUser, verItem);
				break;
			case 3:
				tri3.push_back(newTriple);
				insert(ver3, newTriple, 0, verUser, verItem);
				insert(ver3, newTriple, 1, verUser, verItem);
				break;
			case 4:
				tri4.push_back(newTriple);
				insert(ver4, newTriple, 0, verUser, verItem);
				insert(ver4, newTriple, 1, verUser, verItem);
				break;
			default:
				std::cout << "error:can not insert to map##############" << user << ":" << item << ":" <<action << endl;
			}
		}
	}
private:
	io::CSVReader<3> * csvReader;
	void insert(map<Vertex, vector<Vertex> *> &verMap, Triple *tri, int flag,
		map<Vertex, int> &verUser, map<Vertex, int> &verItem) {
		Vertex f;
		if (!flag) {
			f = tri->getUserID();
			insertVec(f, verUser);
		}
		else {
			f = tri->getItemID();
			insertVec(f, verItem);
		}
		auto  iter = verMap.find(f);
		if (iter == verMap.end()) {
			vector<Vertex> *newVec = new vector<Vertex>();
			newVec->push_back(f);
			verMap.insert(pair<Vertex, vector<Vertex> *>(f, newVec));
		}
		else {
			iter->second->push_back(f);
		}
	}
	void insertVec(Vertex &f, map<Vertex, int> &maps) {
		auto iter = maps.find(f);
		if (iter == maps.end()) {
			maps.insert(pair<Vertex, int>(f, 1));
		}
		else {
			iter->second++;
		}
	}
};