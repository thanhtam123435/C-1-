#include <iostream>
#include <string>
using namespace std;

int main() {
    string cmd;
    cin >> cmd;
    if (cmd == "start") {
        cout << "Bat dau!" << endl;
      } else if (cmd == "stop") { // (1)
        cout << "Dung lai!" << endl;
    } else {
        cout << "Lenh khong hop le!" << endl;
    }
    return 0;
}
//ở phần (1) việc thêm if và sau else giúp cho sau khi việc kiểm tra hoàn thành và kết quả là else thì sẽ tiếp tục kiểm tra điều kiện if được gán ở sau else
