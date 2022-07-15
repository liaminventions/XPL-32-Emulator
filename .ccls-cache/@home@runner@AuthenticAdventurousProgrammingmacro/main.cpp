#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;

#include "mos6502.h"

#define CHIPS_IMPL

uint8_t memory[65536]; // 64K ram (not)

#include "6522.h"

string fname("ROM.BIN");

bool verbose = true;

bool r = false;

unsigned long cycles = 1;
uint32_t speed = 100000;

void via_assert(bool r);
void via_disassert(bool r);

void via_assert(bool r) {
  r = true;
}
void via_disassert(bool c) {
  r = false;
}

void MemWrite(uint16_t addr, uint8_t byte)
{	
  if(addr < 0xC000){
    memory[addr] = byte;
  }
  
  if(addr >= 0xB000 && addr <= 0xB7FF){
    via_assert(r);
  } else {
    via_disassert(r);
  }
	return;
}

uint8_t MemRead(uint16_t addr)
{
  cout << std::hex << addr << "      ";
  cout << std::hex << std::setw(2) << std::setfill('0') << (int)memory[addr] << endl;
	return memory[addr];
}

int main() {
  for(int i = 0; i <= 65536; i++){
    memory[i] = 0xEA;
  }

  ifstream inputData;
	inputData.open("ROM.BIN");
	if (inputData) {
		inputData.read(reinterpret_cast<char *>(memory + sizeof(uint8_t) * 32768), sizeof(uint8_t) * 32768);
		inputData.close();
  } else {
    cout << "Error Loading ROM!" << endl;
    exit(0);
  }

  m6522_t states;
  m6522_t* state = &states;
  uint64_t pins;
  
  mos6502 cpu(MemRead, MemWrite);

  cpu.Reset();
  m6522_reset(state);
  m6522_init(state);
  
  while(true){
    cpu.Run(speed, cycles);
    pins = m6522_tick(state, pins);
  }
}