/*
DStack - Interpreter for the esoteric programming language DStack.
Copyright (C) 2015 Alejandro O. Coria Bayer

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

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
        exit = (bool) ss >> number;
    } while (!exit);

    return number;
}

Number readChar(std::istream &stream){
    char ch;
    if (stream.get(ch)){
        return static_cast<Number>(ch);
    } else{
        stream.clear();
        return Number(0);
    }
}
