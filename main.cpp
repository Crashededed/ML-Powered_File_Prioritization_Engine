#include <sys/stat.h>

#include <iostream>

int main() {
  const char* path = "Text.txt";
  struct stat fileStat;

  if (stat(path, &fileStat) == 0) {
    std::cout << "File size: " << fileStat.st_size << " bytes" << std::endl;
  } else {
    std::cerr << "Error retrieving file information." << std::endl;
  }
  return 0;
}
