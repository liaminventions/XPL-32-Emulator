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

#include "6551.hpp"

Plus4::ACIA6551 acia;

m6522_t state;
uint64_t pins;

uint64_t* pinn = &pins;

string fname("ROM.BIN");

bool verbose = true;

bool r = true;

uint64_t cycles;
uint32_t speed = 1;

uint8_t viaAddr;
uint8_t aciaAddr;

void MemWrite(uint16_t addr, uint8_t byte)
{	
  *pinn |= ((uint64_t)0) << 24; // rw
  if(verbose){
    cout<<"w   ";
    cout << std::hex << addr << "    ";
    cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte << endl; 
  }
  //cout << "w" << endl;
  if(addr < 0xC000){
    memory[addr] = byte;
  }
  if(addr >= 0x8000 && addr <= 0x87FF) {
    acia.writeRegister(addr-0x8000, byte);
    if(addr == 0x8000) {
      cout << char(byte) << endl;
    }
  }
  
  if(addr >= 0xB000 && addr <= 0xB7FF){
    pins |= ((uint64_t)1) << 40; // cs1 
    viaAddr = addr - 0xB000;
    _m6522_write(&state, viaAddr, byte);
  } else {
    pins |= ((uint64_t)0) << 40; // cs1 
  }
	return;
}

uint8_t MemRead(uint16_t addr)
{
  
	*pinn |= ((uint64_t)1) << 24; // rw
  //if(verbose){
  //  cout << "r   ";
  //  cout << std::hex << addr << "    ";
  //  cout << std::hex << std::setw(2) << std::setfill('0') << (int)memory[addr] << endl;
  //}
  // cout << std::hex << addr;
  if(addr >= 0x8000 && addr <= 0x87FF) {
    memory[addr] = acia.readRegister(addr-0x8000);
  }
  
  if(addr >= 0xB000 && addr <= 0xB7FF){
    pins |= ((uint64_t)1) << 40; // cs1
    viaAddr = addr - 0xB000;
    memory[addr] = _m6522_read(&state, viaAddr);
  } else {
    pins |= ((uint64_t)0) << 40; // cs1
  }
	return memory[addr];
}

void half(){
  while(true) {
    
  }
}

int main() {
  for(int i = 0; i <= 65536; i++){
    memory[i] = 0xff;
  }

  ifstream inputData;
	inputData.open(fname);
	if (inputData) {
		inputData.read(reinterpret_cast<char *>(memory + sizeof(uint8_t) * 32768), sizeof(uint8_t) * 32768);
		inputData.close();
  } else {
    cout << "Error Loading ROM!" << endl;
    exit(0);
  }

  pins |= ((uint64_t)1) << 41; // cs2 always low
  
  mos6502 cpu(MemRead, MemWrite);

  cpu.Reset();
  acia.reset();
  m6522_init(&state);
  m6522_reset(&state);

  int irqcount = 0;
  uint64_t irqstate = 0;
  uint64_t lastirq = 0;

  while(true){
    cpu.Run(speed, cycles);            // w65c02
    acia.runOneCycle();                // r65c51
    pins = m6522_tick(&state, pins);   // w65c22
    irqstate = pins & M6522_IRQ;
    pins |= ((uint64_t)1) << 41; // cs2 always low
    if(irqstate != lastirq && memory[0xB00E] != 0) {
      if(irqstate != lastirq && _m6522_read(&state, M6522_REG_IER) != 0) {
	      cout << irqcount << " IRQ " << endl;
	      irqcount++;
		    cpu.IRQ();
      }
    }
    lastirq = irqstate;
  }
}