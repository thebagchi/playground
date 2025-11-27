#include <cstdio>
#include <iostream>
#include <memory>

struct FileDeleter {
  void operator()(FILE* fp) const {
    if (fp) {
      std::cout << "Custom deleter: closing file" << std::endl;
      fclose(fp);
    }
  }
};

std::unique_ptr<FILE, FileDeleter> openFile(const char* filename) {
  FILE* fp = fopen(filename, "w");
  return fp ? std::unique_ptr<FILE, FileDeleter>(fp) : nullptr;
}

int main() {
  std::cout << "std::unique_ptr Custom Deleter Demo" << std::endl;
  {
    auto file_ptr = openFile("/tmp/test.txt");
    if (file_ptr) {
      fprintf(file_ptr.get(), "Hello!\n");
      std::cout << "File written" << std::endl;
    }
  }
  std::cout << "Scope ended - deleter called!" << std::endl;
  return 0;
}