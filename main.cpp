#include "Simulador.h"

#include <iostream>
#include <sstream>
#include <string>
#include <chrono>

int main(int argc, char* argv[]) {

    std::string archivo;

    if (argc == 2) {
        archivo = argv[1];
    } else {
        std::cout << "Ingrese la ruta del archivo del programa: ";
        std::getline(std::cin, archivo);

        if (archivo.empty()) {
            std::cout << "No se ingreso ningun archivo.\n";
            return 1;
        }
    }

    Simulador sim;

    if (!sim.cargarPrograma(archivo)) {
        std::cout << "No se pudo abrir el archivo.\n";
        return 1;
    }

    std::cout << "\"" << archivo << "\" cargado a memoria.\n";

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

            std::cout << "Instruccion ejecutada:\n";
            std::cout << sim.getInstrHex()
                      << "    "
                      << sim.getInstrAsm()
                      << '\n';
        }
        else if (cmd == "run") {

            // Ejecuta durante un tiempo (100 ms por defecto)

            int tiempo = 100;

            ss >> tiempo;

            auto inicio = std::chrono::steady_clock::now();

            while (!sim.terminado()) {

                sim.step();

                auto ahora = std::chrono::steady_clock::now();

                auto transcurrido =
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                                ahora - inicio);

                if (transcurrido.count() >= tiempo)
                    break;
            }

            std::cout << "Ultima instruccion ejecutada:\n";
            std::cout << sim.getInstrHex()
                      << "    "
                      << sim.getInstrAsm()
                      << '\n';

            if (sim.terminado())
                std::cout << "Programa finalizado.\n";
            else
                std::cout << "Tiempo agotado.\n";
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

            char formato;
            uint32_t inicio;
            uint32_t fin;

            if (!(ss >> formato)) {
                std::cout << "Uso: mem <x|d> <inicio> <fin>\n";
                continue;
            }

            if (formato == 'x' || formato == 'X') {

                if (!(ss >> std::hex >> inicio >> fin)) {
                    std::cout << "Uso: mem x <inicio> <fin>\n";
                    continue;
                }

            } else if (formato == 'd' || formato == 'D') {

                if (!(ss >> std::dec >> inicio >> fin)) {
                    std::cout << "Uso: mem d <inicio> <fin>\n";
                    continue;
                }

            } else {

                std::cout << "Formato invalido. Use 'x' (hexadecimal) o 'd' (decimal).\n";
                continue;
            }

            if (inicio > fin) {
                std::cout << "Rango invalido.\n";
                continue;
            }

            sim.mostrarMemoria(inicio, fin);
        }
        else {

            std::cout << "Comando invalido.\n";
        }
    }

    return 0;
}