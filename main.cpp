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

#include <cstring>
#include <iostream>

void usage();

int main(int argc, char *argv[]) {
    bool debug = false;
    char *file;

    if (argc < 2){
        usage();
        exit(0);
    } else if (argc > 3){
        std::cout << "too many arguments\n\n";
        usage();
        exit(0);
    }

    if (argc == 3){
        if (std::strcmp(argv[1], "-d") == 0){
            debug = true;
            file = argv[2];
        } else if (std::strcmp(argv[2], "-d") == 0){
            debug = true;
            file = argv[1];
        } else{
            std::cout << "error in arguments\n\n";
            usage();
            exit(0);
        }
    } else{
        file = argv[1];
    }

	Interpreter interpreter{debug};

	if(!interpreter.load(file))
		return 2;

	if(!interpreter.execute())
		return 3;

	return 0;
}

void usage(){
    std::cout << "dstack [-d] file\n\n";
    std::cout << "    -d\tDisplay debugging information while running\n";
    std::cout << "    file\tName of the file to be executed\n\n";
}
