#pragma once

#include <utility>

enum class Opcodes {
    Digit       = -2,
    Error       = -1,

    None        = 0,

    Add         = 1,
    Mul         = 2,
    Sub         = 3,
    Div         = 4,
    Rem         = 5,

    Zero        = 10,

    Equal       = 20,
    Unequal     = 21,
    Greater     = 22,
    GreOrEq     = 23,
    Not         = 24,
    And         = 25,
    Or          = 26,

    Rand        = 30,
    Min         = 31,
    Max         = 32,

    Push        = 40,
    Send        = 41,
    Peek        = 42,
    Pop         = 43,

    Save        = 50,
    Jump        = 51,

    PrintN      = 60,
    PrintC      = 61,
    ReadN       = 62,
    ReadC       = 63,
};

namespace {

bool isDigit(char ch){
    return (ch >= '0') && (ch <= '9');
}

}

inline Opcodes toOpcode(const std::pair<char, char> &pair, bool alt){
    if (isDigit(pair.second)){
        return Opcodes::Digit;
    }

    if (isDigit(pair.first)){
        return Opcodes::None;
    }

    if (pair.first == pair.second)
        return Opcodes::Push;

    switch (pair.first){
        case 'd': switch (pair.second){
            case 's': return alt ? Opcodes::Mul : Opcodes::Add;
            case 't': return Opcodes::Sub;
            case 'c': return Opcodes::Div;
            case 'k': return Opcodes::Rem;
        } break;
        case 's': switch (pair.second){
            case 'd': return Opcodes::Zero; // alt ?
            case 't': return alt ? Opcodes::Unequal : Opcodes::Equal;
            case 'c': return Opcodes::Greater;
            case 'k': return Opcodes::GreOrEq;
        } break;
        case 't': switch (pair.second){
            case 'd': return Opcodes::Not;
            case 's': return alt ? Opcodes::And : Opcodes::Or;
            case 'c': return Opcodes::Peek;
            case 'k': return alt ? Opcodes::Max : Opcodes::Min;
        } break;
        case 'c': switch (pair.second){
            case 'd': return Opcodes::Send;
            case 's': return Opcodes::Pop;
            case 't': return Opcodes::Rand;
            case 'k': return alt ? Opcodes::PrintN : Opcodes::PrintC;
        } break;
        case 'k': switch (pair.second){
            case 'd': return Opcodes::None;
            case 's': return Opcodes::Save;
            case 't': return Opcodes::Jump;
            case 'c': return alt ? Opcodes::ReadN : Opcodes::ReadC;
        } break;
    }

    return Opcodes::Error;
}

inline std::string toString(Opcodes code){
    switch (code) {
	    case Opcodes::Digit:    return "Digit";
	    case Opcodes::Error:    return "Error";

	    case Opcodes::None:     return "None";

        case Opcodes::Add:      return "Add";
        case Opcodes::Sub:      return "Subtract";
        case Opcodes::Mul:      return "Multiply";
        case Opcodes::Div:      return "Divide";
        case Opcodes::Rem:      return "Remainder";

        case Opcodes::Zero:     return "Zero";

        case Opcodes::Equal:    return "Equal";
        case Opcodes::Unequal:  return "Unequal";
        case Opcodes::Greater:  return "Greater";
        case Opcodes::GreOrEq:  return "Greater Or Equal";
        case Opcodes::And:      return "And";
        case Opcodes::Or:       return "Or";
        case Opcodes::Not:      return "Not";

        case Opcodes::Rand:     return "Random";
        case Opcodes::Min:      return "Minimum";
        case Opcodes::Max:      return "Maximum";

        case Opcodes::Push:     return "Push";
        case Opcodes::Send:     return "Send";
        case Opcodes::Peek:     return "Peek";
        case Opcodes::Pop:      return "Pop";

        case Opcodes::Save:     return "Save";
        case Opcodes::Jump:     return "Jump";

        case Opcodes::PrintN:   return "Print Number";
        case Opcodes::PrintC:   return "Print Character";
        case Opcodes::ReadN:    return "Read Number";
        case Opcodes::ReadC:    return "Read Character";
	}

	return "Unknown";
}




