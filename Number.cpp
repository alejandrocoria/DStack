#include "Number.h"

#include <cmath>
#include <sstream>

namespace {

template<typename T>
std::string to_string(const T& obj){
	std::ostringstream os;
    os << obj;
    return os.str();
}

}

char toChar(Number number){
    return static_cast<char>(number % 256);
}

std::string toString(Number number){
    return to_string(number);
}

Number concat(char character, Number number){
    return number * 10 + static_cast<Number>(character - '0');
}

Number readNumber(std::istream &stream){
    Number number;
    std::string tmp;
    bool exit = false;
    do{
        std::getline(stream, tmp);
        std::stringstream ss(tmp);
        exit = ss >> number;
    } while (!exit);

    return number;
}

Number readChar(std::istream &stream){
    char ch;
    stream.get(ch);

    return static_cast<Number>(ch);
}
