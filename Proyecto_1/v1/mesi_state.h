#ifndef MESI_STATE
#define MESI_STATE

// Estados MESI
enum class mesi_state { MODIFIED, EXCLUSIVE, SHARED, INVALID };


inline const char* mesiStateToString(mesi_state state) {
    switch(state) {
        case mesi_state::MODIFIED:  return "MODIFIED";
        case mesi_state::EXCLUSIVE: return "EXCLUSIVE";
        case mesi_state::SHARED:    return "SHARED";
        case mesi_state::INVALID:   return "INVALID";
        default:                    return "UNKNOWN";
    }


}

#endif