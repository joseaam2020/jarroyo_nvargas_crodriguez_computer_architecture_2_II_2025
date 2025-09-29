#include <iostream>
#include <vector>
#include <cstdint>

class Interconnect{
private:
    uint64_t memory[512];

public:
    Interconnect() {
        for (int i = 0; i < 512; i++ ){
            memory[i] = 0;
        }

    }

//Read every PE request to access 
    uint64_t request_read(int id_pe, uint32_t address){

    }

 // Read every value from the PE   
    void request_write(int id_pe,uint32_t address, uint64_t value){

    }

 //Direct lecture of the memory   
    uint64_t memory_read(uint32_t address){


    }

//Direct write to the memery   
    void memory_write(uint32_t address,uint64_t value){


    }

};

