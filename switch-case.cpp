int ngay;
cout << "Nhap so ngay (1-7): ";
cin >> ngay;
switch (ngay) {
    case 1: cout << "Chu nhat"; break;
    case 2: cout << "Thu hai"; break;
    case 3: cout << "Thu ba"; break;
    case 4: cout << "Thu tu"; break;
    case 5: cout << "Thu nam"; break;
    case 6: cout << "Thu sau"; break;
    case 7: cout << "Thu bay"; break;
    default: cout << "Khong hop le";
}
// ở phần chuỗi của switch (...) có thể biến đổi dựa trên hàm khai chuỗi | float,int,char,...
