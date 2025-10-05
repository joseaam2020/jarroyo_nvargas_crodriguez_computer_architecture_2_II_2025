#include "mesi_protocol.h"

std::string stateToString(MESIState state) {
    switch(state) {
        case MESIState::MODIFIED: return "MODIFIED";
        case MESIState::EXCLUSIVE: return "EXCLUSIVE";
        case MESIState::SHARED: return "SHARED";
        case MESIState::INVALID: return "INVALID";
        default: return "UNKNOWN";
    }
}

std::string transactionToString(BusTransaction trans) {
    switch(trans) {
        case BusTransaction::READ: return "READ";
        case BusTransaction::WRITE: return "WRITE";
        case BusTransaction::READ_X: return "READ_X";
        case BusTransaction::INVALIDATE: return "INVALIDATE";
        case BusTransaction::FLUSH: return "FLUSH";
        case BusTransaction::NONE: return "NONE";
        default: return "UNKNOWN";
    }
}
