#include <iostream>
using namespace std;
void cong(double a, double b) {
	cout << "kết quả: " << a + b;
}
void tru(double a, double b) {
	cout << "kết quả: " << a - b;
}
void nhan(double a, double b) {
	cout << "kết quả: " << a * b;
}
void chia(double a, double b) {
	cout << "kết quả: " << a / b;
}
int main() {
	double a;
	double b;
    string ask;
	cout << "nhập phần tử a: ";
	cin >> a;
	cout << "nhập phần tử b: ";
	cin >> b;
	cout << "cộng(1) trừ(2) nhân(3) chia(4):"<<endl;
	cin >> ask;
	if (ask == "1") {
		cong(a, b);
	}
	else if (ask == "2")
	{
		tru(a, b);
	}
	else if (ask == "3") {
		nhan(a, b);
	}
	else if (ask == "4") {
		chia(a, b);
	}
}
