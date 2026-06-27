.global _start

_start:

    # Construir arreglo dinamicamente en memoria (direccion 0x1000)
    # Valores: 6, 4, 4, 1, 2, 2, 1

    lui s0, 1                # s0 = 0x1000
    
    addi t0, x0, 6
    sw t0, 0(s0)             # mem[0x1000] = 6
    addi t0, x0, 4
    sw t0, 4(s0)             # mem[0x1004] = 4
    addi t0, x0, 4
    sw t0, 8(s0)             # mem[0x1008] = 4
    addi t0, x0, 1
    sw t0, 12(s0)            # mem[0x100c] = 1
    addi t0, x0, 2
    sw t0, 16(s0)            # mem[0x1010] = 2
    addi t0, x0, 2
    sw t0, 20(s0)            # mem[0x1014] = 2
    addi t0, x0, 1
    sw t0, 24(s0)            # mem[0x1018] = 1

    # Llamar is_symmetric(base, size)
    addi a0, s0, 0           # a0 = base (0x1000)
    addi a1, x0, 7           # a1 = size (7 elementos)
    jal ra, is_symmetric     # Llamada a is_symmetric
    
fin:
    beq x0, x0, fin          # Bucle infinito para finalizar

# Funcion: is_symmetric
# Comprueba si un arbol binario representado como arreglo es simetrico.
# Parametros: a0 = base, a1 = size.
# Retorno: a0 = 1 si es simetrico, 0 en caso contrario.
# Usa ra, sp y a0-a3.
# Inicia la verificacion llamando recursivamente a mirror().
is_symmetric:
    # Prologo
    addi sp, sp, -16
    sw ra, 12(sp)
    
    # if (size == 0) return 1
    beq a1, x0, is_symmetric_return_1
    
    # Preparar parametros para llamada a mirror(base, leftIndex=1, rightIndex=2, size)
    # a0 ya posee la base (0x1000)
    addi a3, a1, 0           # a3 = size (copiado desde a1)
    addi a1, x0, 1           # a1 = leftIndex inicial es 1
    addi a2, x0, 2           # a2 = rightIndex inicial es 2
    jal ra, mirror           # Invocar subrutina recursiva
    jal x0, is_symmetric_end # a0 ya contiene el resultado de mirror, ir a epilogo

is_symmetric_return_1:
    addi a0, x0, 1           # Cargar valor true (1)

is_symmetric_end:
    # Epilogo
    lw ra, 12(sp)
    addi sp, sp, 16
    jalr x0, 0(ra)           # Retornar

# Funcion: mirror
# Comprueba recursivamente si dos subarboles son espejos.
# Parametros: a0 = base, a1 = leftIndex, a2 = rightIndex, a3 = size.
# Retorno: a0 = 1 si son simetricos, 0 en caso contrario.
# Usa ra, sp, s0-s3 y t0-t3.
# Compara ambos nodos y verifica recursivamente sus hijos cruzados.
mirror:
    # Prologo
    addi sp, sp, -32
    sw ra, 28(sp)
    sw s0, 24(sp)            # Guardar base
    sw s1, 20(sp)            # Guardar leftIndex
    sw s2, 16(sp)            # Guardar rightIndex
    sw s3, 12(sp)            # Guardar size
    
    addi s0, a0, 0
    addi s1, a1, 0
    addi s2, a2, 0
    addi s3, a3, 0
    
    # Verificar limites
    slt t0, s1, s3           # t0 = 1 si leftIndex < size
    slt t1, s2, s3           # t1 = 1 si rightIndex < size
    
    # Si ambos indices son >= size (es decir, t0 == 0 y t1 == 0), retorna 1
    or t2, t0, t1            # t2 sera 0 unicamente si t0=0 y t1=0
    beq t2, x0, mirror_return_1
    
    # Llegado aca, no son ambos fuera de limite.
    # Si solo uno es >= size, el arbol es asimetrico. Retorna 0.
    beq t0, x0, mirror_return_0
    beq t1, x0, mirror_return_0
    
    # Ahora validamos los datos en base[leftIndex] y base[rightIndex]
    slli t0, s1, 2           # Multiplicar indice izquierdo por 4
    add t0, s0, t0           # Sumar a direccion base
    lw t2, 0(t0)             # t2 = arr[leftIndex]
    
    slli t1, s2, 2           # Multiplicar indice derecho por 4
    add t1, s0, t1           # Sumar a direccion base
    lw t3, 0(t1)             # t3 = arr[rightIndex]
    
    # Comparacion
    bne t2, t3, mirror_return_0 # Si los valores difieren, no es simetrico
    
    # Primera llamada recursiva (hijo exterior izquierdo con hijo exterior derecho)
    # formula: hijo izquierdo del nodo K = 2 * K + 1
    # formula: hijo derecho del nodo K = 2 * K + 2
    addi a0, s0, 0           # a0 = base
    slli a1, s1, 1           # a1 = 2 * leftIndex
    addi a1, a1, 1           # a1 = 2 * leftIndex + 1
    slli a2, s2, 1           # a2 = 2 * rightIndex
    addi a2, a2, 2           # a2 = 2 * rightIndex + 2
    addi a3, s3, 0           # a3 = size
    jal ra, mirror
    
    # Validacion inmediata del valor devuelto (a0)
    beq a0, x0, mirror_return_0 # Si la primera llamada retorna 0, propagar false
    
    # Segunda llamada recursiva (hijo interior izquierdo con hijo interior derecho)
    addi a0, s0, 0           # a0 = base
    slli a1, s1, 1           # a1 = 2 * leftIndex
    addi a1, a1, 2           # a1 = 2 * leftIndex + 2
    slli a2, s2, 1           # a2 = 2 * rightIndex
    addi a2, a2, 1           # a2 = 2 * rightIndex + 1
    addi a3, s3, 0           # a3 = size
    jal ra, mirror
    
    # Retornamos el valor de la segunda llamada (en a0 ya hay 0 o 1)
    beq a0, x0, mirror_return_0
    jal x0, mirror_return_1

mirror_return_1:
    addi a0, x0, 1           # a0 = 1 (true)
    jal x0, mirror_end       # Saltar al final
    
mirror_return_0:
    addi a0, x0, 0           # a0 = 0 (false)
    
mirror_end:
    # Epilogo
    lw ra, 28(sp)
    lw s0, 24(sp)
    lw s1, 20(sp)
    lw s2, 16(sp)
    lw s3, 12(sp)
    addi sp, sp, 32
    jalr x0, 0(ra)
