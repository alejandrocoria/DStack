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

#include "Interpreter.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>

namespace {

bool toLower(char &ch){
    if ((ch >= 'A') && (ch <= 'Z')){
        ch = ch - 'A' + 'a';
        return true;
    }
    return false;
}

void printNumber(Number n){
    std::cout << " " << toString(n);
    char ch = toChar(n);
    if ((ch >= 32) && (ch <= 126)) // is printable
        std::cout << " (" << ch << ")";
}

void printStack(const Stack &stack){
    bool first = true;
    for (Number n : stack){
        if (first)
            first = false;
        else
            std::cout << ",";
        printNumber(n);
    }
    std::cout << "\n";
}

void printLine(unsigned int n, char ch = '-'){
    for (unsigned int i = 0; i < n; ++i)
        std::cout << ch;
}

template<class T>
std::string interpolate(std::string s, const T &n1, const T &n2){
    std::string::size_type pos;

    while ((pos = s.find("#")) != std::string::npos)
        s.replace(pos, 1, std::string() + n1);

    while ((pos = s.find("$")) != std::string::npos)
        s.replace(pos, 1, std::string() + n2);

    return s;
}

void pushString(const std::string &s, Stack &stack){
    for (char ch : s)
        stack.push_back(Number(ch));
}

}

Interpreter::Interpreter(bool debug):
reg         (0),
pos         (0),
status      (Status::Normal),
debugMode   (debug),
randomEngine(std::chrono::high_resolution_clock::now().time_since_epoch().count()){
	stackA.push_back(Number(0));
	stackB.push_back(Number(0));
}

bool Interpreter::load(const std::string &path){
	std::ifstream t(path.c_str());
	if (t){
		std::stringstream buffer;
		buffer << t.rdbuf();
		source = buffer.str();
		source += '\n';
		parse();
		if (status == Status::Error){
            showError();
            return false;
		} else{
            return true;
		}
	}

	std::cout << "The file could not be opened\n";
	return false;
}

bool Interpreter::execute(){
    if (sourceParsed.empty())
        return true;

	std::pair<char, char> pair;
	Stack *first;
	Stack *second;
	do {
		first = &stackA;
		second = &stackB;

        if (debugMode)
            printCurrentStatus();

		if (!getPair(pair))
			continue;

        toLower(pair.first);
        bool alt = toLower(pair.second);

        Opcodes code = toOpcode(pair, alt);

        if (debugMode)
            printCurrentOpcode(code, alt);

        bool increment;
		if (alt)
            increment = execute(pair, code, *second, *first);
        else
            increment = execute(pair, code, *first, *second);

        if (increment)
            ++pos;
	} while (status == Status::Normal);

    if (status == Status::Error)
        showError();

	return status != Status::Error;
}

namespace {
enum class Stage {Code, Comment, StringBegin, String, StringEnd, MultiComment, MultiCommentEnd};
}

bool Interpreter::parse(){
    const std::string valids = "dstackDSTACK0123456789";
    const std::string ignorable = " \t\n\r";

    std::string::size_type line = 1;
    std::string::size_type col = 0;
    Number position = 0;
    std::string stringLiteral;
    Number stringId;

    PositionInfo atPosition;

    Stage stage = Stage::Code;

    for (char ch : source){
        ++col;
        if (ch == '\n'){
            ++line;
            col = 0;
        }

        switch (stage){
            case Stage::Code:
                if (ignorable.find(ch) != std::string::npos){
                    // nothing to do
                } else if (ch == '/'){
                    stage = Stage::Comment;
                } else if (valids.find(ch) != std::string::npos){
                    sourceParsed += ch;
                    positionMap.emplace(position, PositionInfo{line, col});
                    ++position;
                } else if ((ch == '@') && (col == 1)){
                    stage = Stage::StringBegin;
                    stringLiteral.clear();
                    stringId = 0;
                    atPosition.line = line;
                    atPosition.col = col;
                } else{
                    status = Status::Error;
                    errorInfo.position = {line, col};
                    errorInfo.error = "Invalid character: ";
                    errorInfo.error += ch;
                }
                break;
            case Stage::Comment:
                if (ch == '\n')
                    stage = Stage::Code;
                break;
            case Stage::StringBegin:
                if (ch == '\n'){
                    if (col == 2){
                        stage = Stage::MultiComment;
                    } else{
                        stage = Stage::String;
                    }
                } else{
                    if (isDigit(ch)){
                        stringId = stringId * 10 + (ch - '0');
                    } else{
                        status = Status::Error;
                        errorInfo.position = {line, col};
                        errorInfo.error = "Invalid character: ";
                        errorInfo.error += ch;
                        errorInfo.error += " (Only digits are allowed)";
                    }
                }
                break;
            case Stage::String:
            case Stage::MultiComment:
                if ((ch == '@') && (col == 1)){
                    stage = Stage::StringEnd;
                } else{
                    if (stage == Stage::String)
                        strings[stringId] += ch;
                }
                break;
            case Stage::StringEnd:
            case Stage::MultiCommentEnd:
                if (ch == '\n'){
                    if (stage == Stage::StringEnd){
                        strings[stringId].pop_back();
                    }
                    stage = Stage::Code;
                } else{
                    if (stage == Stage::StringEnd){
                        strings[stringId] += '@';
                        strings[stringId] += ch;
                        stage = Stage::String;
                    } else{
                        stage = Stage::MultiComment;
                    }
                }
                break;
        }

        if (status == Status::Error)
            break;
    }

    if (stage == Stage::MultiComment){
        status = Status::Error;
        errorInfo.position = atPosition;
        errorInfo.error = "The comment is not closed before the end of file. Start";
    } else if (stage == Stage::String){
        status = Status::Error;
        errorInfo.position = atPosition;
        errorInfo.error = "The string literal is not closed before the end of file. Start";
    }

    return status != Status::Error;
}

bool Interpreter::getPair(std::pair<char, char> &pair){
	if (pos >= sourceParsed.length() - 1){
		status = Status::EoF;
		return false;
	}

	pair = {sourceParsed[pos], sourceParsed[pos + 1]};
	return true;
}

bool Interpreter::execute(std::pair<char, char> pair, Opcodes code, Stack &first, Stack &second){
    bool increment = true;

	switch (code) {
	    case Opcodes::Digit:    reg = concat(pair.second, reg); break;
	    case Opcodes::Error:    status = Status::Error; break;

	    case Opcodes::None:     break;

        case Opcodes::Add:      reg = first.back() + second.back(); break;
        case Opcodes::Mul:      reg = first.back() * second.back(); break;
        case Opcodes::Sub:      reg = first.back() - second.back(); break;
        case Opcodes::Pow:      reg = std::pow(first.back(), second.back()); break;
        case Opcodes::Div:      if (second.back() == 0){
                                    status = Status::Error;
                                    errorInfo.position = positionMap[pos];
                                    errorInfo.error = "Division by zero";
                                } else
                                    reg = first.back() / second.back();
                                break;
        case Opcodes::Rem:      if (second.back() == 0){
                                    status = Status::Error;
                                    errorInfo.position = positionMap[pos];
                                    errorInfo.error = "Division by zero (remainder operation)";
                                } else
                                    reg = first.back() % second.back();
                                break;

        case Opcodes::Zero:     reg = 0; break;

        case Opcodes::Equal:    reg = first.back() == second.back(); break;
        case Opcodes::Unequal:  reg = first.back() != second.back(); break;
        case Opcodes::Greater:  reg = first.back() > second.back(); break;
        case Opcodes::GreOrEq:  reg = first.back() >= second.back(); break;
        case Opcodes::Not:      reg = !first.back(); break;
        case Opcodes::And:      reg = Number(first.back() && second.back()); break;
        case Opcodes::Or:       reg = Number(first.back() || second.back()); break;
        case Opcodes::Xor:      reg = Number(!first.back() != !second.back()); break;

        case Opcodes::Rand:     if (first.back() <= second.back())
                                    reg = getRandom(first.back(), second.back());
                                break;
        case Opcodes::Min:      reg = std::min(first.back(), second.back()); break;
        case Opcodes::Max:      reg = std::max(first.back(), second.back()); break;

        case Opcodes::Push:     first.push_back(reg); break;
        case Opcodes::PushS:    if (strings.count(reg))
                                    pushString(strings[reg], first);
                                break;
        case Opcodes::PushRS:   if (strings.count(reg)){
                                    std::string tmp = strings[reg];
                                    std::reverse(tmp.begin(), tmp.end());
                                    pushString(tmp, first);
                                }
                                break;
        case Opcodes::Send:     second.push_back(first.back());
                                first.pop_back();
                                if (first.empty())
                                    first.push_back(0);
                                break;
        case Opcodes::Peek:     reg = first.back(); break;
        case Opcodes::Pop:      first.pop_back();
                                if (first.empty())
                                    first.push_back(0);
                                break;

        case Opcodes::Save:     first.push_back(pos + 1); break;
        case Opcodes::Jump:     if (reg){
                                    pos = first.back();
                                    increment = false;
                                }
                                break;
        case Opcodes::Reset:    if (reg){
                                    pos = 0;
                                    reg = 0;
                                    stackA.clear(); stackA.push_back(0);
                                    stackB.clear(); stackB.push_back(0);
                                    std::cin.clear();
                                    std::cin.sync();
                                    debugOutput.clear();
                                    increment = false;
                                }
                                break;
        case Opcodes::Halt:     if (reg)
                                     // halts the program because no program
                                     // is going to have such a high position
                                    pos = -1;
                                    increment = false;
                                break;

        case Opcodes::PrintN:   print(toString(reg)); break;
        case Opcodes::PrintC:   print(toChar(reg)); break;
        case Opcodes::PrintS:   if (strings.count(reg))
                                    print(strings[reg]);
                                break;
        case Opcodes::PrintSiN: if (strings.count(reg))
                                    print(interpolate(strings[reg],
                                          toString(first.back()), toString(second.back())));
                                break;
        case Opcodes::PrintSiC: if (strings.count(reg))
                                    print(interpolate(strings[reg],
                                          toChar(first.back()), toChar(second.back())));
                                break;
        case Opcodes::ReadN:    reg = readNumber(std::cin); break;
        case Opcodes::ReadC:    reg = readChar(std::cin); break;
	}

	return increment;
}

Number Interpreter::getRandom(Number min, Number max){
    return std::uniform_int_distribution<Number>{min, max}(randomEngine);
}

template<class T>
void Interpreter::print(const T &output){
    if (debugMode)
        debugOutput += output;
    else
        std::cout << output;
}

void Interpreter::printCurrentStatus(){
    std::cout << "stack 1:";
    printStack(stackA);

    std::cout << "stack 2:";
    printStack(stackB);

    std::cout << "register:";
    printNumber(reg);
    std::cout << "\n";

    if (!debugOutput.empty())
        std::cout << "output: " << debugOutput << "\n";

    printLine(79);
    std::cout << "\n";
}
void Interpreter::printCurrentOpcode(Opcodes code, bool alt){
    std::cout << "instruction: " << toString(code);
    std::cout << " (" << sourceParsed[pos] << sourceParsed[pos + 1] << ")";
    if (alt)
        std::cout << " stacks swapped";
    std::cout << "\n";

    std::cout << "position: " << pos << "\n";

    printLine(79);
    std::cout << "\n";
}

void Interpreter::showError(){
    printLine(79, '*');
    std::cout << "\n";
    std::cout << errorInfo.error << " in ";
    std::cout << errorInfo.position.line << ":" << errorInfo.position.col << "\n";
    printLine(79, '*');
    std::cout << "\n";
}
