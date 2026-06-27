#include "Simulador.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>


// Constructor
Simulador::Simulador(size_t tamanoMem)
    : pc(0),
    mem(tamanoMem),
    fin(false)
{
    regFile[0] = 0;
    reset();
}

//simuolador
void Simulador::reset() {
    pc = 0;
    fin = false;
    instrActualAsm = "";
    instrActualHex = 0;

    for (int i = 0; i < 32; ++i)
        regFile[i] = 0;
    for (size_t i = 0; i < mem.size(); ++i)
        mem[i] = 0;

    // Inicializar el stack pointer al final de la memoria
    regFile[2] = static_cast<uint32_t>(mem.size()) & ~0xF;
}

bool Simulador::cargarPrograma(const std::string& archivo) {
    std::ifstream file(archivo, std::ios::binary);

    if (!file.is_open())
        return false;

    reset();
    file.read(reinterpret_cast<char*>(mem.data()), mem.size());
    return true;
}

// Execution
void Simulador::step() {
    if (fin)
        return;
    uint32_t instr = fetch();
    decodeExecute(instr);
}

void Simulador::run() {
    while (!fin)
        step();
}

bool Simulador::terminado() const {
    return fin;
}


// Fetch
uint32_t Simulador::fetch() {
    return readWord(pc);
}


// Decode + Execute
void Simulador::decodeExecute(uint32_t instr) {

    uint32_t opcode =  instr        & 0x7F;
    uint32_t rd      = (instr >> 7) & 0x1F;
    uint32_t funct3  = (instr >>12) & 0x07;
    uint32_t rs1     = (instr >>15) & 0x1F;
    uint32_t rs2     = (instr >>20) & 0x1F;
    uint32_t funct7  = (instr >>25) & 0x7F;

    // inmediatos
    int32_t immI;
    int32_t immS;
    int32_t immB;
    int32_t immU;
    int32_t immJ;

    immI = (int32_t)instr >> 20;

    immS =((instr >> 25) << 5) |((instr >> 7) & 0x1F);
    if (immS & 0x800)
        immS |= 0xFFFFF000;

    immB =  (((instr >> 31) & 0x1) << 12) |
            (((instr >> 7)  & 0x1) << 11) |
            (((instr >>25) & 0x3F) << 5) |
            (((instr >> 8) & 0xF) << 1);
    if (immB & 0x1000)
        immB |= 0xFFFFE000;

    immU = instr & 0xFFFFF000;

    immJ =  (((instr >>31) & 1) <<20) |
            (((instr >>12) &0xFF) <<12) |
            (((instr >>20) &1) <<11) |
            (((instr >>21) &0x3FF) <<1);
    if (immJ & 0x100000)
        immJ |= 0xFFE00000;

    // Guardar la instruccion cruda
    instrActualHex = instr;

    // Valores de registros fuente
    uint32_t uRs1 = readReg(rs1);
    uint32_t uRs2 = readReg(rs2);
    int32_t  sRs1 = static_cast<int32_t>(uRs1);
    int32_t  sRs2 = static_cast<int32_t>(uRs2);

    switch (opcode) {

        // LOAD (I-type)
        case 0x03: {
            uint32_t addr = static_cast<uint32_t>(sRs1 + immI);
            switch (funct3) {
                case 0x0: // lb
                    writeReg(rd, static_cast<uint32_t>(static_cast<int8_t>(readByte(addr))));
                    instrActualAsm = "lb " + ("x" + std::to_string(rd)) + ", " + std::to_string(immI) + "(" + ("x" + std::to_string(rs1)) + ")";
                    break;
                case 0x1: // lh
                    writeReg(rd, static_cast<uint32_t>(static_cast<int16_t>(readHalf(addr))));
                    instrActualAsm = "lh " + ("x" + std::to_string(rd)) + ", " + std::to_string(immI) + "(" + ("x" + std::to_string(rs1)) + ")";
                    break;
                case 0x2: // lw
                    writeReg(rd, readWord(addr));
                    instrActualAsm = "lw " + ("x" + std::to_string(rd)) + ", " + std::to_string(immI) + "(" + ("x" + std::to_string(rs1)) + ")";
                    break;
                case 0x4: // lbu
                    writeReg(rd, static_cast<uint32_t>(readByte(addr)));
                    instrActualAsm = "lbu " + ("x" + std::to_string(rd)) + ", " + std::to_string(immI) + "(" + ("x" + std::to_string(rs1)) + ")";
                    break;
                case 0x5: // lhu
                    writeReg(rd, static_cast<uint32_t>(readHalf(addr)));
                    instrActualAsm = "lhu " + ("x" + std::to_string(rd)) + ", " + std::to_string(immI) + "(" + ("x" + std::to_string(rs1)) + ")";
                    break;
            }
            pc += 4;
            break;
        }

        // IMMEDIATE (I-type)
        case 0x13: {
            switch (funct3) {
                case 0x0: // addi
                    writeReg(rd, static_cast<uint32_t>(sRs1 + immI));
                    instrActualAsm = "addi " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + std::to_string(immI);
                    break;
                case 0x1: // slli
                    writeReg(rd, uRs1 << (immI & 0x1F));
                    instrActualAsm = "slli " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + std::to_string(immI & 0x1F);
                    break;
                case 0x2: // slti
                    writeReg(rd, sRs1 < immI ? 1u : 0u);
                    instrActualAsm = "slti " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + std::to_string(immI);
                    break;
                case 0x3: // sltiu
                    writeReg(rd, uRs1 < static_cast<uint32_t>(immI) ? 1u : 0u);
                    instrActualAsm = "sltiu " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + std::to_string(immI);
                    break;
                case 0x4: // xori
                    writeReg(rd, uRs1 ^ static_cast<uint32_t>(immI));
                    instrActualAsm = "xori " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + std::to_string(immI);
                    break;
                case 0x5:
                    if (funct7 == 0x00) { // srli
                        writeReg(rd, uRs1 >> (immI & 0x1F));
                        instrActualAsm = "srli " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + std::to_string(immI & 0x1F);
                    } else {             // srai
                        writeReg(rd, static_cast<uint32_t>(sRs1 >> (immI & 0x1F)));
                        instrActualAsm = "srai " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + std::to_string(immI & 0x1F);
                    }
                    break;
                case 0x6: // ori
                    writeReg(rd, uRs1 | static_cast<uint32_t>(immI));
                    instrActualAsm = "ori " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + std::to_string(immI);
                    break;
                case 0x7: // andi
                    writeReg(rd, uRs1 & static_cast<uint32_t>(immI));
                    instrActualAsm = "andi " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + std::to_string(immI);
                    break;
            }
            pc += 4;
            break;
        }

        // U-TYPE
        case 0x37: // lui
            writeReg(rd, static_cast<uint32_t>(immU));
            instrActualAsm = "lui " + ("x" + std::to_string(rd)) + ", " + std::to_string(immU >> 12);
            pc += 4;
            break;

        case 0x17: // auipc
            writeReg(rd, pc + static_cast<uint32_t>(immU));
            instrActualAsm = "auipc " + ("x" + std::to_string(rd)) + ", " + std::to_string(immU >> 12);
            pc += 4;
            break;

        // STORE (S-type)
        case 0x23: {
            uint32_t addr = static_cast<uint32_t>(sRs1 + immS);
            switch (funct3) {
                case 0x0: // sb
                    writeByte(addr, static_cast<uint8_t>(uRs2 & 0xFF));
                    instrActualAsm = "sb " + ("x" + std::to_string(rs2)) + ", " + std::to_string(immS) + "(" + ("x" + std::to_string(rs1)) + ")";
                    break;
                case 0x1: // sh
                    writeHalf(addr, static_cast<uint16_t>(uRs2 & 0xFFFF));
                    instrActualAsm = "sh " + ("x" + std::to_string(rs2)) + ", " + std::to_string(immS) + "(" + ("x" + std::to_string(rs1)) + ")";
                    break;
                case 0x2: // sw
                    writeWord(addr, uRs2);
                    instrActualAsm = "sw " + ("x" + std::to_string(rs2)) + ", " + std::to_string(immS) + "(" + ("x" + std::to_string(rs1)) + ")";
                    break;
            }
            pc += 4;
            break;
        }

        // R-TYPE
        case 0x33: {
            switch (funct3) {
                case 0x0:
                    if (funct7 == 0x00) { // add
                        writeReg(rd, static_cast<uint32_t>(sRs1 + sRs2));
                        instrActualAsm = "add " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + ("x" + std::to_string(rs2));
                    } else {              // sub
                        writeReg(rd, static_cast<uint32_t>(sRs1 - sRs2));
                        instrActualAsm = "sub " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + ("x" + std::to_string(rs2));
                    }
                    break;
                case 0x1: // sll
                    writeReg(rd, uRs1 << (uRs2 & 0x1F));
                    instrActualAsm = "sll " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + ("x" + std::to_string(rs2));
                    break;
                case 0x2: // slt
                    writeReg(rd, sRs1 < sRs2 ? 1u : 0u);
                    instrActualAsm = "slt " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + ("x" + std::to_string(rs2));
                    break;
                case 0x3: // sltu
                    writeReg(rd, uRs1 < uRs2 ? 1u : 0u);
                    instrActualAsm = "sltu " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + ("x" + std::to_string(rs2));
                    break;
                case 0x4: // xor
                    writeReg(rd, uRs1 ^ uRs2);
                    instrActualAsm = "xor " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + ("x" + std::to_string(rs2));
                    break;
                case 0x5:
                    if (funct7 == 0x00) { // srl
                        writeReg(rd, uRs1 >> (uRs2 & 0x1F));
                        instrActualAsm = "srl " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + ("x" + std::to_string(rs2));
                    } else {              // sra
                        writeReg(rd, static_cast<uint32_t>(sRs1 >> (uRs2 & 0x1F)));
                        instrActualAsm = "sra " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + ("x" + std::to_string(rs2));
                    }
                    break;
                case 0x6: // or
                    writeReg(rd, uRs1 | uRs2);
                    instrActualAsm = "or " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + ("x" + std::to_string(rs2));
                    break;
                case 0x7: // and
                    writeReg(rd, uRs1 & uRs2);
                    instrActualAsm = "and " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + ("x" + std::to_string(rs2));
                    break;
            }
            pc += 4;
            break;
        }

        // BRANCH (B-type)
        case 0x63: {
            bool taken = false;
            switch (funct3) {
                case 0x0: taken = (sRs1 == sRs2);
                    instrActualAsm = "beq " + ("x" + std::to_string(rs1)) + ", " + ("x" + std::to_string(rs2)) + ", " + std::to_string(immB);
                    break; // beq
                case 0x1: taken = (sRs1 != sRs2);
                    instrActualAsm = "bne " + ("x" + std::to_string(rs1)) + ", " + ("x" + std::to_string(rs2)) + ", " + std::to_string(immB);
                    break; // bne
                case 0x4: taken = (sRs1 <  sRs2);
                    instrActualAsm = "blt " + ("x" + std::to_string(rs1)) + ", " + ("x" + std::to_string(rs2)) + ", " + std::to_string(immB);
                    break; // blt
                case 0x5: taken = (sRs1 >= sRs2);
                    instrActualAsm = "bge " + ("x" + std::to_string(rs1)) + ", " + ("x" + std::to_string(rs2)) + ", " + std::to_string(immB);
                    break; // bge
                case 0x6: taken = (uRs1 <  uRs2);
                    instrActualAsm = "bltu " + ("x" + std::to_string(rs1)) + ", " + ("x" + std::to_string(rs2)) + ", " + std::to_string(immB);
                    break; // bltu
                case 0x7: taken = (uRs1 >= uRs2);
                    instrActualAsm = "bgeu " + ("x" + std::to_string(rs1)) + ", " + ("x" + std::to_string(rs2)) + ", " + std::to_string(immB);
                    break; // bgeu
            }
            pc = taken ? static_cast<uint32_t>(static_cast<int32_t>(pc) + immB)
                       : pc + 4;
            break;
        }

        // JUMP
        case 0x67: // jalr
            {
                uint32_t ret = pc + 4;
                pc = static_cast<uint32_t>((sRs1 + immI) & ~1);
                writeReg(rd, ret);
                instrActualAsm = "jalr " + ("x" + std::to_string(rd)) + ", " + ("x" + std::to_string(rs1)) + ", " + std::to_string(immI);
            }
            break;

        case 0x6F: // jal
            writeReg(rd, pc + 4);
            pc = static_cast<uint32_t>(static_cast<int32_t>(pc) + immJ);
            instrActualAsm = "jal " + ("x" + std::to_string(rd)) + ", " + std::to_string(immJ);
            break;

        default:
            // Instruccion desconocida – detiene la simulacion
            instrActualAsm = "unknown";
            fin = true;
            break;
    }
}

// Memory
uint8_t Simulador::readByte(uint32_t addr) const {
    return mem[addr];
}

uint16_t Simulador::readHalf(uint32_t addr) const {
    return
        static_cast<uint16_t>(mem[addr]) |
        (static_cast<uint16_t>(mem[addr + 1]) << 8);
}

uint32_t Simulador::readWord(uint32_t addr) const {
    return
        static_cast<uint32_t>(mem[addr]) |
        (static_cast<uint32_t>(mem[addr + 1]) << 8) |
        (static_cast<uint32_t>(mem[addr + 2]) << 16) |
        (static_cast<uint32_t>(mem[addr + 3]) << 24);
}

void Simulador::writeByte(uint32_t addr, uint8_t value) {
    mem[addr] = value;
}

void Simulador::writeHalf(uint32_t addr, uint16_t value) {
    mem[addr] = value & 0xFF;
    mem[addr + 1] = (value >> 8) & 0xFF;
}

void Simulador::writeWord(uint32_t addr, uint32_t value) {
    mem[addr] = value & 0xFF;
    mem[addr + 1] = (value >> 8) & 0xFF;
    mem[addr + 2] = (value >> 16) & 0xFF;
    mem[addr + 3] = (value >> 24) & 0xFF;
}


// Registers
uint32_t Simulador::readReg(uint32_t reg) {
    return regFile[reg];
}

void Simulador::writeReg(uint32_t reg, uint32_t value) {
    if (reg != 0)
        regFile[reg] = value;
}

// Debug
void Simulador::mostrarPC() const {
    std::cout << "PC = 0x"
              << std::hex
              << std::setw(8)
              << std::setfill('0')
              << pc
              << std::dec
              << std::setfill(' ')
              << '\n';
}

void Simulador::mostrarRegistro(uint32_t reg) const {
    if (reg >= 32)
        return;
    std::cout << "x" << reg
              << " = 0x"
              << std::hex
              << std::setw(8)
              << std::setfill('0')
              << regFile[reg]
              << std::dec
              << std::setfill(' ')
              << '\n';
}

void Simulador::mostrarRegistros() const {
    for (int i = 0; i < 32; i++) {
        std::cout << "x"
                  << std::setw(2)
                  << i
                  << " = 0x"
                  << std::hex
                  << std::setw(8)
                  << std::setfill('0')
                  << regFile[i]
                  << std::dec
                  << std::setfill(' ')
                  << '\n';
    }
}

void Simulador::mostrarMemoria(uint32_t inicio, uint32_t fin) const {

    // Alinear al inicio de una palabra
    inicio &= ~0x3;

    if (inicio >= mem.size())
        return;

    // Asegurar que no se lea fuera de memoria
    if (fin >= mem.size() - 3)
        fin = mem.size() - 4;

    // Alinear el final
    fin &= ~0x3;

    for (uint32_t addr = inicio; addr <= fin; addr += 4) {

        uint32_t value = readWord(addr);

        std::cout << "0x"
                  << std::hex
                  << std::setw(8)
                  << std::setfill('0')
                  << addr
                  << " (" << std::dec << addr << ")"
                  << ": 0x"
                  << std::hex
                  << std::setw(8)
                  << std::setfill('0')
                  << value
                  << " (" << std::dec << value << ")"
                  << '\n';
    }

    std::cout << std::setfill(' ');
}

// Instruccion actual
const std::string& Simulador::getInstrAsm() const {
    return instrActualAsm;
}

std::string Simulador::getInstrHex() const {
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase
        << std::setw(8) << std::setfill('0') << instrActualHex;
    return oss.str();
}