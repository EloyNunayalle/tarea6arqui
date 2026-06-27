#ifndef SIMULADOR_H
#define SIMULADOR_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

class Simulador {

private:
    uint32_t pc; // program counter
    uint32_t regFile[32]; // register file

    std::vector<uint8_t> mem; // main memory
    bool fin; // end flag

    uint32_t fetch(); // fetch instr
    void decodeExecute(uint32_t instr); // decode + execute

    // Memory
    uint8_t  readByte(uint32_t addr);
    uint16_t readHalf(uint32_t addr); 
    uint32_t readWord(uint32_t addr);

    void writeByte(uint32_t addr, uint8_t value);
    void writeHalf(uint32_t addr, uint16_t value);
    void writeWord(uint32_t addr, uint32_t value);

    // Registers
    uint32_t readReg(uint32_t reg);
    void writeReg(uint32_t reg, uint32_t value); 

public:

    Simulador(size_t tamanoMem = 1024 * 1024);
    ~Simulador() = default;

    // Simulator
    void reset(); 
    bool cargarPrograma(const std::string& archivo); 

    // Execution
    void step(); // exec one instr
    void run(); // exec until end
    bool terminado() const; // check end

    // Debug
    void mostrarPC() const;  // show pc
    void mostrarRegistro(uint32_t reg) const; // show reg
    void mostrarRegistros() const;  // show regs
    void mostrarMemoria(uint32_t inicio, uint32_t fin) const; // show memory
};

#endif