# Implementacion de QuickSort recursivo


.global _start

_start:
    # Construir arreglo dinamicamente en memoria (direccion 0x1000)
    # Valores: 6, 4, 3, 2, 1, 8, 9
    lui s0, 1                # s0 = 0x1000
    
    addi t0, x0, 6           # t0 = 6
    sw t0, 0(s0)             # mem[0x1000] = 6
    addi t0, x0, 4           # t0 = 4
    sw t0, 4(s0)             # mem[0x1004] = 4
    addi t0, x0, 3           # t0 = 3
    sw t0, 8(s0)             # mem[0x1008] = 3
    addi t0, x0, 2           # t0 = 2
    sw t0, 12(s0)            # mem[0x100c] = 2
    addi t0, x0, 1           # t0 = 1
    sw t0, 16(s0)            # mem[0x1010] = 1
    addi t0, x0, 8           # t0 = 8
    sw t0, 20(s0)            # mem[0x1014] = 8
    addi t0, x0, 9           # t0 = 9
    sw t0, 24(s0)            # mem[0x1018] = 9
    

    # Llamar a la funcion quicksort(base, 0, 6)
    addi a0, s0, 0           # a0 = base del arreglo (0x1000)
    addi a1, x0, 0           # a1 = 0 (indice low)
    addi a2, x0, 6           # a2 = 6 (indice high)
    jal ra, quicksort        # Llamada a quicksort
    
fin:
    beq x0, x0, fin          # Bucle infinito final


# Funcion: quicksort
# Ordena un arreglo mediante QuickSort recursivo.
# Parametros: a0 = base, a1 = low, a2 = high.
# Retorno: ninguno.
# Usa ra, sp, s0-s3 y t0.
quicksort:
    # Prologo
    addi sp, sp, -32         # Reservar 32 bytes en la pila
    sw ra, 28(sp)            # Guardar ra
    sw s0, 24(sp)            # Guardar s0 (base)
    sw s1, 20(sp)            # Guardar s1 (low)
    sw s2, 16(sp)            # Guardar s2 (high)
    sw s3, 12(sp)            # Guardar s3 (pivot index)

    # if (low < high)
    slt t0, a1, a2           # t0 = 1 si low < high
    beq t0, x0, quicksort_end # Si low >= high, terminar recursion
    
    # Respaldar argumentos en registros s para las llamadas
    addi s0, a0, 0           # s0 = base
    addi s1, a1, 0           # s1 = low
    addi s2, a2, 0           # s2 = high
    
    # Llamar partition(base, low, high)
    # a0, a1, a2 ya contienen base, low y high, por lo que llamamos directo
    jal ra, partition
    
    # Guardar indice del pivot devuelto en a0
    addi s3, a0, 0           # s3 = pivot index (pi)
    
    # Llamar quicksort(base, low, pi - 1)
    addi a0, s0, 0           # a0 = base
    addi a1, s1, 0           # a1 = low
    addi a2, s3, -1          # a2 = pi - 1
    jal ra, quicksort
    
    # Llamar quicksort(base, pi + 1, high)
    addi a0, s0, 0           # a0 = base
    addi a1, s3, 1           # a1 = pi + 1
    addi a2, s2, 0           # a2 = high
    jal ra, quicksort
    
quicksort_end:
    lw ra, 28(sp)            # Restaurar ra
    lw s0, 24(sp)            # Restaurar s0
    lw s1, 20(sp)            # Restaurar s1
    lw s2, 16(sp)            # Restaurar s2
    lw s3, 12(sp)            # Restaurar s3
    addi sp, sp, 32          # Liberar espacio en pila
    jalr x0, 0(ra)           # Retornar a llamador

# Funcion: partition
# Particiona el arreglo usando el ultimo elemento como pivote.
# Parametros: a0 = base, a1 = low, a2 = high.
# Retorno: a0 = indice final del pivote.
# Usa ra, sp, s0-s5 y t0-t2.
# Reordena los elementos y coloca el pivote en su posicion correcta.
partition:
    addi sp, sp, -32         # Reservar 32 bytes
    sw ra, 28(sp)            # Guardar ra
    sw s0, 24(sp)            # Guardar s0 (base)
    sw s1, 20(sp)            # Guardar s1 (low)
    sw s2, 16(sp)            # Guardar s2 (high)
    sw s3, 12(sp)            # Guardar s3 (i)
    sw s4, 8(sp)             # Guardar s4 (j)
    sw s5, 4(sp)             # Guardar s5 (pivot value)
    
    addi s0, a0, 0           # s0 = base
    addi s1, a1, 0           # s1 = low
    addi s2, a2, 0           # s2 = high
    
    # pivot = arr[high]
    slli t0, s2, 2           # t0 = high * 4
    add t0, s0, t0           # t0 = &arr[high]
    lw s5, 0(t0)             # s5 = arr[high] (pivot)
    
    # i = low - 1
    addi s3, s1, -1          # s3 = i
    
    # j = low
    addi s4, s1, 0           # s4 = j

partition_loop:
    # Condicion: j < high
    slt t0, s4, s2           # t0 = 1 si j < high
    beq t0, x0, partition_end_loop # Terminar bucle si j >= high
    
    # Obtener arr[j]
    slli t0, s4, 2           # t0 = j * 4
    add t0, s0, t0           # t0 = &arr[j]
    lw t1, 0(t0)             # t1 = arr[j]
    
    # if (arr[j] <= pivot) equivale a !(pivot < arr[j])
    slt t2, s5, t1           # t2 = 1 si pivot < arr[j]
    bne t2, x0, partition_loop_next # Si arr[j] > pivot, omitir intercambio
    
    # i = i + 1
    addi s3, s3, 1
    
    # swap(arr, i, j)
    addi a0, s0, 0           # a0 = base
    addi a1, s3, 0           # a1 = i
    addi a2, s4, 0           # a2 = j
    jal ra, swap
    
partition_loop_next:
    addi s4, s4, 1           # j = j + 1
    jal x0, partition_loop   # Repetir iteracion incondicionalmente

partition_end_loop:
    # swap(arr, i + 1, high)
    addi a0, s0, 0           # a0 = base
    addi a1, s3, 1           # a1 = i + 1
    addi a2, s2, 0           # a2 = high
    jal ra, swap
    
    # Preparar valor de retorno (i + 1)
    addi a0, s3, 1           # a0 = i + 1

    lw ra, 28(sp)
    lw s0, 24(sp)
    lw s1, 20(sp)
    lw s2, 16(sp)
    lw s3, 12(sp)
    lw s4, 8(sp)
    lw s5, 4(sp)
    addi sp, sp, 32
    jalr x0, 0(ra)

# Funcion: swap
# Intercambia dos elementos de un arreglo.
# Parametros: a0 = base, a1 = indice A, a2 = indice B.
# Retorno: ninguno.
# Usa ra, sp y t0-t3.
# Calcula ambas direcciones e intercambia sus valores.
swap:

    addi sp, sp, -16
    sw ra, 12(sp)
    
    # Direccion arr[A]
    slli t0, a1, 2           # t0 = A * 4
    add t0, a0, t0           # t0 = base + (A * 4) = &arr[A]
    
    # Direccion arr[B]
    slli t1, a2, 2           # t1 = B * 4
    add t1, a0, t1           # t1 = base + (B * 4) = &arr[B]
    
    # Cargar valores
    lw t2, 0(t0)             # t2 = arr[A]
    lw t3, 0(t1)             # t3 = arr[B]
    
    # Guardar cruzados en memoria
    sw t3, 0(t0)             # arr[A] = arr[B]
    sw t2, 0(t1)             # arr[B] = arr[A]
    

    lw ra, 12(sp)
    addi sp, sp, 16
    jalr x0, 0(ra)
