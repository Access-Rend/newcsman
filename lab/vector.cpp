#include<vector>
#include<iostream>
using namespace std;

vector<vector<int> > a;
int main(){
	a.reserve(3);
	a[0].push_back(2);
	cout<<a[0][0];
	return 0;
}