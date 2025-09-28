#include <iostream>
using namespace std;
class person {
private:
	string name;
public:
	void setname(string a) {
		name = a;
	}
	string getname() {
		return name;
	}

};
int main() {
	string namex;
	cout << "tên của bạn là gì?: ";
	cin >> namex;
	person show;
	show.setname(namex);
	string x = show.getname();
	cout << "xin chào " << x;
}
