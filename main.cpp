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
