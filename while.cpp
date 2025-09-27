//dạng đúng
#include <iostream>
using namespace std;
int main() {
	int i = 10;
	while (i <= 10) {
		cout << i;
	}
}
// dạng sai
#include <iostream>
using namespace std;
int main() {
	int i = 20;
	while (i <= 10) {
		cout << i;
	}
}
// => dạng sai sẽ không lặp lại inf giá trị i
