#include "Interpreter.h"

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

void printLine(unsigned int n){
    for (unsigned int i = 0; i < n; ++i)
        std::cout << '-';
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

bool Interpreter::parse(){
    const std::string valids = "dstckDSTCK0123456789";
    const std::string ignorable = " \t\n\r";

    std::string::size_type line = 1;
    std::string::size_type col = 0;
    Number position = 0;

    bool inComment = false;
    for (char ch : source){
        ++col;
        if (ch == '\n'){
            ++line;
            col = 0;
        }

        if (inComment){
            if (ch == '\n')
                inComment = false;
        } else {
            if (ignorable.find(ch) != std::string::npos)
                continue;
            if (ch == '/'){
                inComment = true;
            } else if (valids.find(ch) != std::string::npos){
                sourceParsed += ch;
                positionMap.emplace(position, PositionInfo{line, col});
                ++position;
            } else{
                status = Status::Error;
                errorInfo.position = {line, col};
                errorInfo.error = "Invalid character";
                return false;
            }
        }
    }
    return true;
}

bool Interpreter::getPair(std::pair<char, char> &pair){
	if (pos + 1 >= sourceParsed.length()){
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
        case Opcodes::Sub:      reg = first.back() - second.back(); break;
        case Opcodes::Mul:      reg = first.back() * second.back(); break;
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
        case Opcodes::And:      reg = Number(first.back() && second.back()); break;
        case Opcodes::Or:       reg = Number(first.back() || second.back()); break;
        case Opcodes::Not:      reg = !first.back(); break;

        case Opcodes::Rand:     if (first.back() <= second.back())
                                    reg = getRandom(first.back(), second.back());
                                break;
        case Opcodes::Min:      reg = std::min(first.back(), second.back()); break;
        case Opcodes::Max:      reg = std::max(first.back(), second.back()); break;

        case Opcodes::Push:     first.push_back(reg); break;
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

        case Opcodes::PrintN:   print(toString(reg)); break;
        case Opcodes::PrintC:   print(toChar(reg)); break;
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
    std::cout << "\n*********************************\n";
    std::cout << errorInfo.error << " in ";
    std::cout << errorInfo.position.line << ":" << errorInfo.position.col << "\n";
    std::cout << "*********************************\n";
}
