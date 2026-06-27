# Simulador del ISA RISC-V RV32I

## 1. Introducción

El presente proyecto consiste en la implementación de un simulador del subconjunto base **RV32I** del ISA RISC-V, desarrollado en **C++**. El objetivo del simulador es emular el comportamiento de una CPU RISC-V que ejecuta instrucciones de 32 bits, permitiendo cargar programas binarios, ejecutarlos paso a paso o de forma continua, e inspeccionar el estado completo del procesador (registros, memoria y contador de programa).

Un simulador de este tipo es útil en el contexto del curso de Arquitectura de Computadores porque permite comprender el ciclo de vida de una instrucción —fetch, decode y execute— sin necesidad de hardware físico, además de facilitar la depuración de programas en ensamblador RISC-V.

## 2. Arquitectura general

El proyecto está organizado en tres archivos fuente principales:

- **`simulador.h`**: Declaración de la clase `Simulador`, que encapsula todo el estado del procesador simulado (memoria, registros, PC) y los métodos para operar sobre él.
- **`simulador.cpp`**: Implementación completa del simulador: manejo de memoria, registros, ciclo fetch–decode–execute, y funciones de depuración.
- **`main.cpp`**: Interfaz de usuario tipo REPL (Read-Eval-Print Loop) que interpreta comandos ingresados por el usuario y los traduce a llamadas a los métodos del `Simulador`.

El flujo de ejecución comienza en `main()`, donde se instancia un objeto `Simulador` y se carga un archivo binario en memoria mediante `cargarPrograma()`. Luego se entra en un bucle infinito que lee comandos desde la entrada estándar. Cada comando (`step`, `run`, `regs`, `mem`, `pc`, `exit`) invoca métodos específicos del simulador para avanzar la ejecución o consultar el estado interno.

El diseño sigue una arquitectura modular simple: la lógica de simulación está completamente separada de la interfaz de usuario, lo que permite que el simulador pueda ser reutilizado o integrado con otras interfaces sin modificaciones.

## 3. Funcionamiento general

El simulador opera en tres grandes fases: carga del programa, inicialización del procesador y ejecución.

### 3.1 Carga del programa

Al iniciar, `cargarPrograma()` abre el archivo binario en modo lectura binaria (`std::ifstream` con `std::ios::binary`). Antes de leer, invoca `reset()` para restaurar el procesador a su estado inicial. Luego utiliza `file.read()` para volcar el contenido completo del archivo directamente sobre el arreglo que representa la memoria, comenzando desde la dirección 0. No existe un loader que distinga secciones de código y datos; el binario se copia byte a byte en la memoria a partir de la dirección base. Si el archivo no puede abrirse, la función retorna `false` y el programa termina.

### 3.2 Inicialización del procesador

El método `reset()` establece el estado inicial del procesador simulado:

- El PC se fija en 0.
- Todos los 32 registros se ponen en 0, excepto `x2` (el stack pointer, SP), que se inicializa al final del espacio de memoria alineado a 16 bytes (`regFile[2] = (mem.size() & ~0xF)`).
- La memoria completa se llena con ceros.
- La bandera `fin` se establece en `false`.
- Las variables `instrActualHex` e `instrActualAsm` se limpian.

Esta inicialización garantiza que cada ejecución comience desde un estado conocido y reproducible.

### 3.3 Ejecución del programa

La ejecución se realiza mediante el método `step()`, que implementa el ciclo clásico de una CPU:

1. **Fetch**: Se lee una palabra de 32 bits desde la dirección apuntada por el PC mediante `readWord(pc)`. La memoria es little-endian, por lo que los 4 bytes consecutivos se ensamblan correctamente.
2. **Decode y Execute**: La instrucción obtenida se pasa a `decodeExecute()`, que extrae los campos (opcode, rd, funct3, rs1, rs2, funct7, inmediatos), selecciona la operación mediante una estructura `switch`, la ejecuta y actualiza el resultado.
3. **Actualización del PC**: Al finalizar la ejecución, el PC se incrementa en 4 bytes (salvo en instrucciones de salto o branch, donde se calcula una dirección destino).

El programa termina cuando ocurre una de estas condiciones:

- Se ejecuta una instrucción con opcode no reconocido, lo que activa la bandera `fin = true`.
- Se alcanza un bucle infinito (como los loops `beq x0, x0, fin` incluidos en los programas de prueba).
- El usuario detiene la ejecución manualmente (en el modo `run` con tiempo límite).

## 4. Implementación del estado arquitectónico

### 4.1 Memoria

La memoria se implementa mediante `std::vector<uint8_t>`, un arreglo dinámico de bytes. El tamaño por defecto es de 1 MiB (1.048.576 bytes), configurable en el constructor de la clase. Cada byte es direccionable individualmente, lo que refleja el modelo de memoria byte-addressable de RISC-V.

El acceso a la memoria se realiza a través de métodos específicos:

| Método             | Descripción                                      |
|--------------------|--------------------------------------------------|
| `readByte(addr)`   | Retorna `mem[addr]` (1 byte).                    |
| `readHalf(addr)`   | Retorna 2 bytes consecutivos ensamblados en little-endian. |
| `readWord(addr)`   | Retorna 4 bytes consecutivos ensamblados en little-endian. |
| `writeByte(addr, v)` | Escribe un byte en `mem[addr]`.                |
| `writeHalf(addr, v)` | Descompone un valor de 16 bits en 2 bytes little-endian. |
| `writeWord(addr, v)`  | Descompone un valor de 32 bits en 4 bytes little-endian. |

El diseño little-endian implica que, por ejemplo, el valor `0x12345678` en la dirección `A` se almacena como:
- `mem[A]   = 0x78`
- `mem[A+1] = 0x56`
- `mem[A+2] = 0x34`
- `mem[A+3] = 0x12`

No existen validaciones de límites más allá de depender del comportamiento del `std::vector`; el simulador asume que las direcciones generadas por los programas están dentro del rango de memoria asignado.

### 4.2 Registros

El banco de registros se representa con un arreglo fijo `uint32_t regFile[32]`, donde cada elemento almacena un valor de 32 bits. Los registros se numeran de `x0` a `x31`, siguiendo la convención RISC-V.

El acceso a registros se realiza mediante dos métodos:

- `readReg(reg)`: Retorna el valor almacenado en `regFile[reg]`.
- `writeReg(reg, value)`: Asigna `value` a `regFile[reg]`, con la salvedad de que **el registro `x0` está cableado a cero**: si `reg == 0`, la escritura se ignora. Esto reproduce fielmente el comportamiento hardware del ISA RISC-V, donde `x0` siempre lee 0 y cualquier escritura es descartada.

### 4.3 Program Counter (PC)

El PC se almacena como un `uint32_t` miembro de la clase, inicializado en 0 tras `reset()`. Durante la ejecución:

- En instrucciones aritméticas, lógicas, de carga y de almacenamiento, el PC se incrementa en 4: `pc += 4`.
- En instrucciones de branch, el PC se actualiza condicionalmente: si la condición se cumple, se suma el inmediato B (signado) al PC actual; en caso contrario, se avanza a `pc + 4`.
- En instrucciones de salto (`jal`, `jalr`), el PC se carga con la dirección destino calculada, y la dirección de retorno (`pc + 4`) se escribe en el registro `rd`.

No existe lógica de predicción de saltos ni pipeline; cada instrucción completa su ciclo antes de que comience la siguiente.

### 4.4 Stack Pointer (SP)

El registro `x2` (SP) se inicializa en la dirección más alta de la memoria alineada a 16 bytes. Esto permite que los programas utilicen la pila desde el final del espacio de memoria hacia direcciones decrecientes, que es la convención estándar en RISC-V. Los programas de prueba —especialmente los recursivos como `binary_search`, `quicksort` y `symmetric_tree`— dependen de esta inicialización para poder hacer push y pop de registros (`sw` y `lw` con offset negativo desde SP) sin sobrescribir el código ni los datos.

### 4.5 Reset

El método `reset()` restaura todos los componentes del procesador a su estado inicial:

- Registros: todos a 0, excepto SP (`x2`) al final de la memoria.
- Memoria: todos los bytes a 0.
- PC: 0.
- Bandera `fin`: `false`.
- Variables de depuración: instrucción actual a vacío/cero.

Cada vez que se carga un nuevo programa con `cargarPrograma()`, `reset()` se invoca automáticamente, garantizando que no queden residuos de ejecuciones anteriores.

## 5. Decodificación y ejecución

### 5.1 Extracción de campos

Cuando `decodeExecute()` recibe una instrucción de 32 bits, extrae los campos estándar de RISC-V mediante operaciones de enmascaramiento y desplazamiento:

| Campo   | Bits      | Extracción                |
|---------|-----------|---------------------------|
| opcode  | [6:0]     | `instr & 0x7F`            |
| rd      | [11:7]    | `(instr >> 7) & 0x1F`     |
| funct3  | [14:12]   | `(instr >> 12) & 0x07`    |
| rs1     | [19:15]   | `(instr >> 15) & 0x1F`    |
| rs2     | [24:20]   | `(instr >> 20) & 0x1F`    |
| funct7  | [31:25]   | `(instr >> 25) & 0x7F`    |

Además, se construyen los cinco tipos de inmediatos del ISA RISC-V:

- **immI**: Extensión signada de los bits [31:20].
- **immS**: Compuesto por los bits [31:25] y [11:7], con extensión de signo de 12 bits.
- **immB**: Compuesto por los bits [31], [7], [30:25] y [11:8], con extensión de signo de 13 bits.
- **immU**: Los 20 bits superiores, desplazados a [31:12].
- **immJ**: Compuesto por los bits [31], [19:12], [20] y [30:21], con extensión de signo de 21 bits.

### 5.2 Ejecución por categorías

El opcode determina la categoría de la instrucción mediante un `switch` principal, y dentro de cada categoría, `funct3` y `funct7` seleccionan la operación concreta:

**Instrucciones aritméticas y lógicas (opcode 0x33, tipo R)**: Toman dos operandos de registros (`rs1` y `rs2`), realizan la operación (ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND) y almacenan el resultado en `rd`. Las operaciones signadas (`slt`) y no signadas (`sltu`) se manejan casteando los valores a `int32_t` o manteniéndolos como `uint32_t` según corresponda.

**Instrucciones inmediatas (opcode 0x13, tipo I)**: Similar a las R-type, pero el segundo operando proviene del inmediato `immI` en lugar de `rs2`. Incluyen ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI y SRAI. Para SRLI y SRAI, el campo funct7 distingue entre desplazamiento lógico (`funct7 == 0x00`) y aritmético (`funct7 == 0x20`).

**Instrucciones de carga (opcode 0x03, tipo I)**: Calculan una dirección efectiva como `rs1 + immI`, leen de memoria según el ancho (byte, half-word, word) con o sin extensión de signo, y almacenan en `rd`. Los formatos soportados son LB, LH, LW, LBU y LHU.

**Instrucciones de almacenamiento (opcode 0x23, tipo S)**: Calculan la dirección efectiva como `rs1 + immS` y escriben el valor de `rs2` en memoria, truncado al ancho correspondiente: SB (byte), SH (half-word), SW (word).

**Instrucciones de branch (opcode 0x63, tipo B)**: Comparan `rs1` y `rs2` según la condición especificada por funct3 (BEQ, BNE, BLT, BGE, BLTU, BGEU). Si la condición se cumple, el PC se actualiza a `pc + immB`; en caso contrario, avanza normalmente. Las comparaciones utilizan interpretación signada o no signada según la instrucción.

**Instrucciones de salto (opcode 0x6F, tipo J, y 0x67, tipo I)**: JAL guarda `pc + 4` en `rd` y salta a `pc + immJ`. JALR guarda `pc + 4` en `rd` y salta a `(rs1 + immI) & ~1`, alineando la dirección destino a 2 bytes.

**Instrucciones U-type (opcodes 0x37 y 0x17)**: LUI carga el inmediato `immU` en `rd`. AUIPC suma `immU` al PC actual y almacena el resultado en `rd`.

Si una instrucción tiene un opcode no reconocido, el simulador activa la bandera `fin = true` y detiene la ejecución, indicando que se encontró una instrucción ilegal.

## 6. Interfaz del simulador

El simulador expone una interfaz de línea de comandos con los siguientes comandos:

| Comando                | Descripción                                                                 |
|------------------------|-----------------------------------------------------------------------------|
| `step`                 | Ejecuta una instrucción y muestra su representación hexadecimal y ensamblador. |
| `run [ms]`             | Ejecuta instrucciones hasta que el programa termine o transcurran `ms` milisegundos (100 por defecto). Al finalizar, muestra la última instrucción ejecutada. |
| `pc`                   | Muestra el valor actual del contador de programa en hexadecimal.             |
| `regs`                 | Muestra los 32 registros en formato hexadecimal. Si se especifican registros individuales (e.g., `regs x5 x6`), solo muestra esos. |
| `mem x <inicio> <fin>` | Muestra el contenido de memoria entre las direcciones `inicio` y `fin` en hexadecimal. |
| `mem d <inicio> <fin>` | Muestra el contenido de memoria entre las direcciones `inicio` y `fin` en decimal. |
| `exit`                 | Finaliza el programa.                                                       |

La interacción entre los comandos y el simulador es directa: cada comando invoca uno o más métodos del objeto `Simulador`. Por ejemplo, `step` llama a `sim.step()`, que ejecuta fetch, decode y execute, y luego muestra la instrucción mediante `getInstrHex()` y `getInstrAsm()`. El comando `run` ejecuta `sim.step()` en un bucle con control de tiempo mediante `std::chrono::steady_clock`, verificando en cada iteración si el programa ha terminado con `sim.terminado()`.

No existe un comando `reset` en la interfaz REPL, aunque el método `reset()` está implementado y se invoca automáticamente al cargar un nuevo programa.

## 7. Casos de prueba

Los archivos `.s` en la carpeta `test` contienen programas en ensamblador RV32I que verifican el funcionamiento del simulador:

- **`test_general.s`**: Prueba secuencial que valida instrucciones básicas: `addi`, `add`, `sub`, `and`, `or`, `slt`, `sw`, `lw` y `jal`. Cada operación se verifica inmediatamente con un `beq`; si el resultado no es el esperado, el programa salta a `fail`. Esto permite confirmar que la unidad aritmético-lógica y los accesos a memoria funcionan correctamente.

- **`binary_search.s`**: Implementa búsqueda binaria recursiva sobre un arreglo ordenado de 9 enteros almacenado en la dirección `0x1000`. Busca el valor 20 dentro del arreglo. La función `binary_search` utiliza la convención de llamadas RISC-V, guardando registros `ra` y `s0`–`s3` en la pila. Evalúa la corrección del manejo de la pila, los saltos y las operaciones aritméticas.

- **`quicksort.s`**: Implementa el algoritmo de ordenamiento QuickSort recursivo sobre un arreglo de 7 elementos. Incluye las funciones `quicksort`, `partition` (que utiliza el último elemento como pivote) y `swap`. Este programa pone a prueba el uso intensivo de la pila, las llamadas anidadas a subrutinas y las operaciones de carga y almacenamiento con offsets calculados dinámicamente.

- **`symmetric_tree.s`**: Verifica si un árbol binario representado como arreglo es simétrico. Utiliza la función recursiva `mirror` para comparar los subárboles izquierdo y derecho de forma cruzada. Este programa ejercita el manejo de múltiples llamadas recursivas y la evaluación de condiciones lógicas complejas.

## 8. Características implementadas

- Carga de programas binarios RV32I little-endian desde archivo.
- Ciclo completo fetch–decode–execute.
- Ejecución paso a paso con visualización de la instrucción en hexadecimal y ensamblador.
- Ejecución continua con límite de tiempo configurable.
- Conjunto completo de instrucciones del subconjunto base RV32I (39 instrucciones).
- Inspección de los 32 registros (individual o colectivamente).
- Inspección del contador de programa.
- Inspección de memoria en un rango direcciones configurable (hexadecimal o decimal).
- Manejo automático del registro `x0` como cableado a cero.
- Inicialización del stack pointer al final de la memoria.
- Detección de instrucciones inválidas con detención de la simulación.
- Almacenamiento y visualización de la última instrucción ejecutada.
- Separación entre la lógica de simulación y la interfaz de usuario.

## 9. Conclusiones

El simulador implementa de manera completa y funcional el subconjunto base RV32I del ISA RISC-V. La arquitectura del código está bien dividida en tres archivos, separando claramente la lógica de simulación de la interfaz de usuario. El ciclo fetch–decode–execute se implementa de forma directa y comprensible, utilizando estructuras `switch` anidadas que reflejan la organización del propio ISA.

Los programas de prueba cubren un espectro amplio de funcionalidades: instrucciones aritméticas básicas, acceso a memoria, branches condicionales, saltos incondicionales y algoritmos recursivos que demandan el uso correcto de la pila. La ejecución correcta de estos programas confirma que el simulador satisface los requisitos fundamentales del proyecto.

Entre las fortalezas del simulador destacan la fidelidad en la implementación del cableado a cero de `x0`, el manejo correcto de la extensión de signo en los inmediatos, y la inicialización adecuada del stack pointer para soportar programas recursivos. La interfaz REPL permite una depuración interactiva eficaz, combinando ejecución paso a paso con inspección completa del estado del procesador.
