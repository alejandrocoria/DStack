#pragma once

#include <cstdint>
#include <iostream>
#include <string>

using Number = uint64_t;

char toChar(Number number);
std::string toString(Number number);
Number concat(char character, Number number);
Number readNumber(std::istream &stream);
Number readChar(std::istream &stream);
