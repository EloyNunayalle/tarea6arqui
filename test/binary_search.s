.global _start

_start:
    # Construir arreglo dinamicamente en memoria (direccion 0x1000)
    # Valores: 2, 5, 8, 12, 15, 20, 23, 30, 35 ( ordenado)
    lui s0, 1                # s0 = 0x1000
    
    addi t0, x0, 2
    sw t0, 0(s0)             # mem[0x1000] = 2
    addi t0, x0, 5
    sw t0, 4(s0)             # mem[0x1004] = 5
    addi t0, x0, 8
    sw t0, 8(s0)             # mem[0x1008] = 8
    addi t0, x0, 12
    sw t0, 12(s0)            # mem[0x100c] = 12
    addi t0, x0, 15
    sw t0, 16(s0)            # mem[0x1010] = 15
    addi t0, x0, 20
    sw t0, 20(s0)            # mem[0x1014] = 20
    addi t0, x0, 23
    sw t0, 24(s0)            # mem[0x1018] = 23
    addi t0, x0, 30
    sw t0, 28(s0)            # mem[0x101c] = 30
    addi t0, x0, 35
    sw t0, 32(s0)            # mem[0x1020] = 35
    

    # Llamar binary_search(base, left, right, target)
    # target = 20, en un arreglo que va de indice 0 al 8
    addi a0, s0, 0           # a0 = base del arreglo (0x1000)
    addi a1, x0, 0           # a1 = indice izquierdo (left = 0)
    addi a2, x0, 8           # a2 = indice derecho (right = 8)
    addi a3, x0, 20          # a3 = objetivo de busqueda (target = 20)
    jal ra, binary_search    # Ejecutar busqueda
    
fin:
    beq x0, x0, fin          # Terminar con bucle infinito

# Funcion: binary_search
# Busca recursivamente un valor en un arreglo ordenado.
# Parametros: a0 = base, a1 = left, a2 = right, a3 = target.
# Retorno: a0 = indice encontrado, o -1 si no existe.
# Usa ra, sp, s0-s3 y t0-t4.
# Divide el rango por la mitad hasta encontrar el valor o agotar la busqueda.
binary_search:
    # Prologo
    addi sp, sp, -32         # Abrir frame en pila
    sw ra, 28(sp)            # Respaldar direccion de retorno
    sw s0, 24(sp)            # Respaldar base
    sw s1, 20(sp)            # Respaldar left
    sw s2, 16(sp)            # Respaldar right
    sw s3, 12(sp)            # Respaldar target
    
    addi s0, a0, 0           # Copiar parametros a registros seguros s
    addi s1, a1, 0
    addi s2, a2, 0
    addi s3, a3, 0
    
    # Condicion base recursiva: si left > right , retornar -1
    slt t0, s2, s1           # t0 = 1 si right < left
    bne t0, x0, binary_search_not_found
    
    # Calcular mid = (left + right) / 2
    add t0, s1, s2           # Sumar los indices
    srai t0, t0, 1           # Dividir entre 2 conservando signo (t0 = mid)
    addi t4, t0, 0           # Guardar localmente 'mid' en t4 temporalmente
    
    # Cargar arr[mid] desde memoria
    slli t0, t4, 2           # Multiplicar indice mid por 4 bytes
    add t0, s0, t0           # Sumarle la direccion base
    lw t1, 0(t0)             # t1 obtendra el valor del arreglo (arr[mid])
    
    # Casos de valor: if arr[mid] == target
    beq t1, s3, binary_search_found
    
    # if target < arr[mid], ir a la mitad izquierda
    slt t0, s3, t1           # t0 = 1 si target < arr[mid]
    beq t0, x0, binary_search_right # Si t0 es 0, eso significa target > arr[mid], ir a la derecha
    
binary_search_left:
    # Preparar parametros de llamada recursiva: limitando al intervalo [left, mid - 1]
    addi a0, s0, 0           # a0 = base
    addi a1, s1, 0           # a1 = left se mantiene igual
    addi a2, t4, -1          # a2 = mid - 1 (nuevo right)
    addi a3, s3, 0           # a3 = target
    jal ra, binary_search
    jal x0, binary_search_end # Tras finalizar recursividad, a0 tiene el indice, ir a epilogo

binary_search_right:
    # Preparar parametros de llamada recursiva: limitando al intervalo [mid + 1, right]
    addi a0, s0, 0           # a0 = base
    addi a1, t4, 1           # a1 = mid + 1 (nuevo left)
    addi a2, s2, 0           # a2 = right se mantiene igual
    addi a3, s3, 0           # a3 = target
    jal ra, binary_search
    jal x0, binary_search_end # Ir a epilogo con la respuesta ya en a0

binary_search_found:
    addi a0, t4, 0           # El indice mid encontrado es cargado como retorno
    jal x0, binary_search_end

binary_search_not_found:
    addi a0, x0, -1          # No se hallo el target, retornar -1

binary_search_end:
    # Epilogo
    lw ra, 28(sp)            # Restaurar estado previo
    lw s0, 24(sp)
    lw s1, 20(sp)
    lw s2, 16(sp)
    lw s3, 12(sp)
    addi sp, sp, 32          # Cerrar frame
    jalr x0, 0(ra)           # Retornar a punto de llamada original
