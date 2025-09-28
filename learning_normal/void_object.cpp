#include <iostream>
using namespace std;
void cout_text(string text) {
  cout << text;
}
int main(){
string name;
cout <<" tên của bạn là gì? : " << endl;
cin >> name;
  cout_text(name);
}
