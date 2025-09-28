void printMessage(const char* message) {
    std::cout << message << std::endl;
}

int main() {
    const char* msg = "Hello, World!";
    printMessage(msg);
    // msg không thể bị thay đổi trong hàm printMessage
}
