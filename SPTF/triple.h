#pragma once
#include<string>

using namespace std;

enum vertexEnum
{
	USER = 0,
	ITEM = 1,
	TYPE = 2
};

class Vertex{
public:
	Vertex(){
		this->str = "";
		this->type = 0;
	}
	Vertex(string str, vertexEnum vt) {
		this->type = 0;
		this->str = str;
		this->vertexType = vt;
	}
	Vertex(int t, vertexEnum vt) {
		this->str = "";
		this->type = t;
		this->vertexType = vt;
	}
	void setUser(string str) {
		vertexType = USER;
		this->str = str;
	}
	void setItem(string str) {
		vertexType = ITEM;
		this->str = str;
	}
	void setType(int t) {
		vertexType = TYPE;
		this->type = t;
	}
	string getUser() {
		return str;
	}
	string getItem() {
		return str;
	}
	int getType() {
		return type;
	}
	bool operator<(const Vertex& other)  const {
		if (vertexType == 1 || vertexType == 0)return this->str < other.str;
		return this->type < other.type;
	}

private:
	vertexEnum vertexType;
	string str;
	int type;
};

class Triple {
public:
	Triple(std::string u, std::string i ,int t) {
		this->userID.setUser(u);
		this->itemID.setItem(i);
		this->type.setType(t);
	}
	Vertex &getUserID() {
		return userID;
	}
	Vertex &getItemID() {
		return itemID;
	}
	Vertex &getType() {
		return type;
	}
private:
	Vertex userID;
	Vertex itemID;
	Vertex type;
};