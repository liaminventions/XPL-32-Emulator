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

m6522_t state;
uint64_t pins;

uint64_t* pinn = &pins;

string fname("ROM.BIN");

bool verbose = false;

bool r = true;

uint64_t cycles = 1;
uint32_t speed = 1;

void MemWrite(uint16_t addr, uint8_t byte)
{	
  *pinn |= ((uint64_t)0) << 24; // rw
  if(verbose){
     cout<<"wr   ";
  cout << std::hex << addr << "      ";
  cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte << endl; 
  }
  if(addr < 0xC000){
    memory[addr] = byte;
  }
  
  if(addr >= 0xB000 && addr <= 0xB7FF){
    pins |= ((uint64_t)1) << 40; // cs1 
    uint8_t viaAddr = addr - 0xB000;
    _m6522_write(&state, viaAddr, byte);
  } else {
    pins |= ((uint64_t)0) << 40; // cs1 
  }
	return;
}

uint8_t MemRead(uint16_t addr)
{
  *pinn |= ((uint64_t)1) << 24; // rw
  if(verbose){
    cout << "rd   ";
  cout << std::hex << addr << "      ";
  cout << std::hex << std::setw(2) << std::setfill('0') << (int)memory[addr] << endl;
  }
  
  if(addr >= 0xB000 && addr <= 0xB7FF){
    
  } else {
    
  }
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
  
  mos6502 cpu(MemRead, MemWrite);

  cpu.Reset();
  //m6522_reset();
  m6522_init(&state);

  int irqcount = 0;
  
  while(true){
  //for(int i=0;i<16;i++){
    if(pins & M6522_IRQ){
 //    if(verbose){
        cout << irqcount << "                                        ---IRQ---" << endl;
 //     }
      irqcount++;
      cpu.IRQ();
    }
    cpu.Run(speed, cycles);
    pins = m6522_tick(&state, pins);
    pins |= ((uint64_t)0) << 41; // cs2 always low
  }
}
