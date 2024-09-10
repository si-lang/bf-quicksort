#include <stdlib.h> // atoi
#include <iostream> // For cout


void replaceAll(std::string& str, const std::string& from, const std::string& to) {
  if(from.empty())
    return;
  size_t start_pos = 0;
  while((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
}

int main(int argc, char const *argv[]) {
  std::string fncStr = "";
  int start = 0;
  int end = 0;

  if (argc > 1) {
    fncStr = std::string(argv[1]);
  }

  if (argc > 2) {
    start = atoi(argv[2]);
  }

  if (argc > 3) {
    end = atoi(argv[3]);
  }

  //std::cout << "Input:\nString: " << fncStr << "; Number: " << end << '\n';

  for (int i = start; i <= end; i++) {
    std::string temp = fncStr;
    replaceAll(temp, "%X", std::to_string(i));
    std::cout << temp << '\n';
  }

  return 0;
}
