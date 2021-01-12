


#include <iostream>

#include <vector>
#include <string>
#include <algorithm>
#include <fstream>


using namespace std;

//基于文件流迭代器统计文件的行数
int count_lines(const string& file){
  ifstream in(file);
  return count(istreambuf_iterator<char>(in),
               istreambuf_iterator<char>(),
               '\n');
}

vector<int> count_lines_in_files(const vector<string>& files){
  return files | transform(count_lines);
}



