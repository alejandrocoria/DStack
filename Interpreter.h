#pragma once

#include "Number.h"
#include "Opcodes.h"

#include <map>
#include <random>
#include <string>
#include <utility>
#include <vector>

typedef std::vector<Number> Stack;

class Interpreter{
public:
	Interpreter(bool debug = false);
	bool load(const std::string &path);
	bool execute();

private:
    bool parse();
    bool getPair(std::pair<char, char> &pair);
	bool execute(std::pair<char, char> pair, Opcodes code, Stack &first, Stack &second);
	Number getRandom(Number min, Number max);

	template<class T>
	void print(const T &output);
	void printCurrentStatus();
	void printCurrentOpcode(Opcodes code, bool alt);

	void showError();

    enum class Status {Normal, EoF, Error};

    struct PositionInfo{
        std::string::size_type line;
        std::string::size_type col;
    };

    struct ErrorInfo{
        PositionInfo position;
        std::string error;
    };

	Stack stackA;
	Stack stackB;
	Number reg;
	Number pos;
	std::string source;
	std::string sourceParsed;
	std::map<Number, PositionInfo> positionMap;
	Status status;
	ErrorInfo errorInfo;
	bool debugMode;
	std::string debugOutput;
	std::mt19937 randomEngine;
};
