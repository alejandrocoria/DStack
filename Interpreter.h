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
	std::map<Number, std::string> strings;
	std::string source;
	std::string sourceParsed;
	std::map<Number, PositionInfo> positionMap;
	Status status;
	ErrorInfo errorInfo;
	bool debugMode;
	std::string debugOutput;
	std::mt19937 randomEngine;
};
