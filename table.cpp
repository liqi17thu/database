﻿#include "table.h"
#include <sstream>
#include <utility>
#include <iostream>
#include <algorithm>
#include <list>
#include <map>


std::string Table::getRow(Data* data) {
	while (data->getPre() != NULL)
		data = data->getPre();

	std::string row;

	while (data->getSuc() != NULL) {
		row += data->getData() + ",";
		data = data->getSuc();
	}
	row += data->getData();

	return row;
}


void Table::init(std::string & _info) {
	std::istringstream info(_info);
	std::string data;
	getline(info, data, '(');
	while (getline(info, data, ',')) {
		if (*data.rbegin() == ')') data.pop_back();
		//设置主键
		bool iskey = false;
		int x = data.find("PRIMARY KEY");
		if (x != data.npos) {
			int x1 = data.find_first_of('(');
			int x2 = data.find_first_of(')');
			std::string name = data.substr(x1 + 1, x2 - x1 - 1);
			for (size_t i = 0; i < attrs.size(); i++) {
				if (attrs[i]->getName() == name) {
					key = i;
					iskey = true;
					break;
				}
			}
			continue;
		}

		bool notnull = false;
		x = data.find("NOT NULL");
		if (x != data.npos) {
			//设置非空项
			notnull = true;
			not_null.push_back(attr_num);
			//删除NOT NULL
			data.erase(x - 1, 9);
		}

		//添加属性
		x = data.find(' ');
		std::string name = data.substr(0, x);
		std::string type = data.substr(x + 1);
		Attr *attr = new Attr(name, type);
		if(notnull)	attr->notNull();
		if (iskey) attr->setKey();
		attrs.push_back(attr);
		attr_num++;
	}
}

void Table::addData(std::string & _info) {
	
	std::istringstream info(_info);
	std::string data;
	getline(info, data, '(');
	getline(info, data, ')');

	std::vector<int> attrId;
	int x;
	std::string name;
	//添加属性
	while ((x = data.find_first_of(',')) != data.npos) {
		name = data.substr(0, x);
		data = data.substr(x + 1);

		for (int i = 0; i < attr_num; i++) {
			if (attrs[i]->getName() == name) {
				attrId.push_back(i);
				break;
			}
		}
	}
	for (int i = 0; i < attr_num; i++) {
		if (attrs[i]->getName() == data) {
			attrId.push_back(i);
			break;
		}
	}

	//检查notnull
	for (auto it : not_null) {
		if (find(attrId.begin(), attrId.end(), it) == attrId.end()) {
			std::cout << "ERROR:'"<<attrs[it]->getName() << "'不允许为空"<< std::endl;
			return;
		}
	}


	//数据
	std::string new_row;
	std::vector<Data*> new_attr;
	getline(info, data, '(');
	getline(info, data, ')');
	//检查主键
	if (find(attrId.begin(), attrId.end(), key) != attrId.end())
	{
		int j;
		for (j = 0; j < attrId.size(); j++) {
			if (attrId[j] == key) {
				break;
			}
		}
		std::string tp = data;
		int index = -1;
		while (j--)
		{
			index = tp.find(',', index + 1);
		}
		int index2 = tp.find(',', index + 1);
		if (index2 == tp.npos) index2 = tp.size();
		tp = tp.substr(index+1, index2 - index-1);
		for (auto it : attrs[key]->getDatas())
		{
			if (it->getData() == tp)
			{
				std::cout << "ERROR:主键重复，不可插入" << std::endl;
				return;
			}
		}
	}
	row_num++;
	for (int i = 0; i < attr_num; i++) {
		//查找是否有相应的数据
		size_t j;
		for (j = 0; j < attrId.size(); j++) {
			if (attrId[j] == i) {
				break;
			}
		}
		//赋值
		if (j != attrId.size()) {
			new_row += getData(data, j);
			Data *p;
			if (attrs[i]->getType() == "INT") { p = new IntData(getData(data, j)); }
			else if (attrs[i]->getType() == "DOUBLE") { p = new DoubleData(getData(data, j)); }
			else { p = new Data(getData(data, j)); }
			new_attr.push_back(p);
		}
		else {
			Data *p;
			if (attrs[i]->getType() == "INT") { p = new IntData(""); }
			else if (attrs[i]->getType() == "DOUBLE") { p = new DoubleData(""); }
			else { p = new Data(""); }
			new_attr.push_back(p);
		}
		if (i != attr_num - 1)
			new_row += ",";
	}
	rows.push_back(new_row);

	//加入到attr中
	for (size_t i= 0; i < new_attr.size(); i++) {
		if (i == 0) {
			new_attr[i]->setSuc(new_attr[i + 1]);
			attrs[i]->addData(new_attr[i]);
		}
		else if (i == new_attr.size() - 1) {
			new_attr[i]->setPre(new_attr[i - 1]);
			attrs[i]->addData(new_attr[i]);
		}
		else {
			new_attr[i]->setPre(new_attr[i - 1]);
			new_attr[i]->setSuc(new_attr[i + 1]);
			attrs[i]->addData(new_attr[i]);
		}
	}
}


void Table::setRows() {
	//清除rows
	rows.clear();
	std::list<Data*> temp = attrs[key]->getDatas();
	for (auto it : temp) {
		rows.push_back(getRow(it));
	}
}

void Table::setRows(int i) {
	rows.clear();
	std::list<Data*> temp = attrs[i]->getDatas();
	for (auto it : temp) {
		rows.push_back(getRow(it));
	}
}

std::vector<Data*> Table::separateRow(std::string _row) {
	std::istringstream row(_row);
	std::vector<Data*> datas;
	std::string data;
	int i = 0;
	while (getline(row, data, ',')) {
		Data * p;
		if (attrs[i]->getType() == "INT") { p = new IntData(data); }
		else if (attrs[i]->getType() == "DOUBLE") { p = new DoubleData(data); }
		else { p = new Data(data); }
		datas.push_back(p);
		i++;
	}
	//建立连接
	for (size_t i = 0; i < datas.size(); i++) {
		if (i == 0) {
			datas[i]->setSuc(datas[i + 1]);
		}
		else if (i == datas.size() - 1) {
			datas[i]->setPre(datas[i - 1]);
		}
		else {
			datas[i]->setPre(datas[i - 1]);
			datas[i]->setSuc(datas[i + 1]);
		}
	}
	return datas;
}

void Table::setAttrs() {
	for (auto it : attrs) {
		it->cleanData();
	}

	for (auto it : rows) {
		std::vector<Data*> datas = separateRow(it);
		for (int i = 0; i < attr_num; i++) {
			attrs[i]->addData(datas[i]);
		}
	}
}

void Table::select(std::string & _info, std::string & Clause) {
	if (row_num == 0)
		return;
	Sort();
	std::istringstream info(_info);
	std::vector<std::string> names;
	std::string name;

	if (_info == "*") {
		for (auto it : attrs) {
			names.push_back(it->getName());
		}
	}
	else {
		while (getline(info, name, ',')) {
			names.push_back(name);
		}
	}

	std::vector<int> output;
	for (size_t i = 0; i < names.size(); i++) {
		for (int j = 0; j < attr_num; j++) {
			if (attrs[j]->getName() == names[i]) {
				output.push_back(j);
			}
		}
	}
		
	//输出表头
	for (auto it = names.begin(); it != names.end(); it++) {
		if (it != --names.end())
			std::cout << *it << '\t';
		else {
			std::cout << *it;
		}
	}
	std::cout << std::endl;

	//输出数据
	std::list<Data*> attr = attrs[key]->getDatas();
	for (auto data : attr) {
		if (WC.whereclause(getRow(data), Clause, attrs)) {
			size_t i = 0;
			for (; i < output.size(); i++) {
				Data* p = get_Data_i(data, output[i]);
				if (p->getType() == "INT") {	p = dynamic_cast<IntData*>(p);	}
				else if (p->getType() == "DOUBLE") { p = dynamic_cast<DoubleData*>(p); }

				if (i == output.size() - 1) {
					std::cout << p->showData() << std::endl;
				}
				else {
					std::cout << p->showData() << "\t";
				}
			}
		}
	}
}

void Table::Sort() {
	attrs[key]->Sort();
	setRows();
	setAttrs();
}

void Table::Delete(std::string & Clause) {
	for (auto it = rows.begin(); it != rows.end();) {
		if (WC.whereclause(*it, Clause,attrs)) {
			it = rows.erase(it);
			row_num--;
		}
		else {
			it++;
		}
	}
	setAttrs();
}

void Table::updateData(std::istringstream& info)
{
	//UPDATE xxtable SET attrName = attrValue WHERE XXXXX;
	std::string data;
	std::string value;
	info >> data;//"SET"
	info >> std::ws;
	getline(info, data, '=');
	int index = 0;
	while (attrs[index]->getName() != data && index < attr_num)index++;
	if (index == attr_num)
	{
		std::cout << "ERROR:未找到'" <<data<<"'"<< std::endl;
		return;
	}
	info >> std::ws;//'='
	info >> value;//得到value
	info >> data;//"WHERE"
	if (data != "WHERE") data = "";
	else getline(info, data);//得到条件语句

	for (std::list<std::string>::iterator it = rows.begin(); it != rows.end(); it++)
	{
		if (WC.whereclause(*it, data, attrs))//遍历链表，根据WHERECLAUSE找到行
		{
			std::string& tmprow = *it;
			if (attrs.size() > 1) {
				int x1 = -1;
				for (int kk = 0; kk < index; kk++)
				{
					x1 = tmprow.find(',', x1 + 1);
				}
				int x2 = tmprow.find(',', x1 + 1);
				if (x2 == tmprow.npos) x2 = tmprow.size();
				if (tmprow.substr(x1 + 1, x2 - x1 - 1) != value)
				{
					if (index == key)
					{
						bool t = false;
						for (auto itt : attrs[key]->getDatas())
						{
							if (itt->getData() == value)
							{
								std::cout << "ERROR:主键重复，不可更新" << std::endl;
								t = true;
								break;
							}
						}
						if (t) continue;
					}

					tmprow.replace(x1 + 1, x2 - x1 - 1, value);
				}
			}
			else if (tmprow != value)
			{
				if (index == key)
				{
					bool t = false;
					for (auto itt : attrs[key]->getDatas())
					{
						if (itt->getData() == value)
						{
							std::cout << "ERROR:主键重复，不可更新" << std::endl;
							t = true;
							break;
						}
					}
					if (t) continue;
				}
				tmprow = value;
			}
		}
	}
	setAttrs();
}

#define TAB 8
#define SPACE 1
void Table::show_table_colums() {
	std::cout << "Field\tType\tNull\tKey\tDefault\tExtra" << std::endl;
	int delayed = 0;
	for (int i = 0; i < attr_num; i++) {
		std::string name = attrs[i]->getName();
		std::cout <<name;
		if (name.size() < TAB) std::cout << "\t";
		else
		{
			std::cout << " ";
			delayed = name.size() - TAB + SPACE;
		}

		std::string type = attrs[i]->getType();
		transform(type.begin(), type.end(), type.begin(), ::tolower);
		if (type == "int")
		{
			std::cout << "int(11)";
			if (7 + delayed < TAB) std::cout << "\t";
			else std::cout << " ";
		}
		else if (type == "char")
		{
			std::cout << "char(1)";
			if (7 + delayed < TAB) std::cout << "\t";
			else std::cout << " ";
		}
		else
		{
			std::cout << type;
			if (type.size()+delayed < TAB) std::cout << "\t";
			else
			{
				std::cout << " ";
				delayed = type.size() +delayed- TAB + SPACE;
			}
		}
		//输出NOT NULL信息
		auto inotnull = find(not_null.begin(), not_null.end(), i);
		if (inotnull != not_null.end()) {
			std::cout << "NO";
			if (2 + delayed < TAB) std::cout << "\t";
			else
			{
				std::cout << " ";
				delayed= 2 + delayed - TAB + SPACE;
			}
		}
		else { 
			std::cout << "YES";
			if (3 + delayed < TAB) std::cout << "\t";
			else
			{
				std::cout << " ";
				delayed = 3 + delayed - TAB + SPACE;
			}
		}
		//输出主键信息
		if (i == key) {
			std::cout << "PRI";
			if (3 + delayed < TAB) std::cout << "\t";
			else
			{
				std::cout << " ";
				delayed = 3 + delayed - TAB + SPACE;
			}
		}
		else {
			if (delayed < TAB) std::cout << "\t";
			else
			{
				std::cout << " ";
				delayed = delayed - TAB + SPACE;
			}
		}
		//DEFAULT AND EXTRA??
		std::cout << "NULL" << std::endl;
	}
}