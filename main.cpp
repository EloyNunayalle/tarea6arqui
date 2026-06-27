#include "Simulador.h"

#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cout << "Uso: " << argv[0] << " programa.bin\n";
        return 1;
    }

    Simulador sim;

    if (!sim.cargarPrograma(argv[1])) {
        std::cout << "No se pudo abrir el archivo.\n";
        return 1;
    }

    std::cout << "\"" << argv[1] << "\" cargado a memoria.\n";

    std::string linea;

    while (true) {

        std::cout << "> ";
        std::getline(std::cin, linea);
        std::stringstream ss(linea);
        std::string cmd;

        ss >> cmd;

        if (cmd == "exit") {
            break;
        }
        else if (cmd == "pc") {
            sim.mostrarPC();
        }
        else if (cmd == "step") {

            sim.step();

            std::cout << "Ejecutando instruccion.\n";
        }
        else if (cmd == "run") {

            sim.run();

            std::cout << "Programa finalizado.\n";
        }
        else if (cmd == "regs") {
            std::string reg;
            if (!(ss >> reg)) {
                sim.mostrarRegistros();
            } else {
                do {
                    if (reg.size() > 1 && reg[0] == 'x') {
                        int n = std::stoi(reg.substr(1));
                        sim.mostrarRegistro(n);
                    }
                } while (ss >> reg);
            }
        }
        else if (cmd == "mem") {

            uint32_t inicio;
            uint32_t fin;

            ss >> std::hex >> inicio >> fin;

            sim.mostrarMemoria(inicio, fin);
        }
        else {

            std::cout << "Comando invalido.\n";
        }
    }
    return 0;
}