.global _start
_start:
    # Direccion para indicar PASS/FAIL

    addi x10, x0, 100

    # addi

    addi x1, x0, 10
    addi x2, x0, 15

    # add

    add x3, x1, x2

    addi x4, x0, 25
    beq x3, x4, test_sub
    jal x0, fail

test_sub:

    sub x5, x3, x1

    beq x5, x2, test_and
    jal x0, fail

test_and:

    and x6, x1, x2

    beq x6, x1, test_or
    jal x0, fail

test_or:

    or x7, x1, x2

    beq x7, x2, test_slt
    jal x0, fail

test_slt:

    slt x8, x1, x2

    addi x9, x0, 1

    beq x8, x9, test_store
    jal x0, fail

test_store:
    sw x3, 0(x10)

    lw x11, 0(x10)

    beq x11, x3, test_jal
    jal x0, fail

test_jal:

    jal x12, success

    jal x0, fail

success:

    addi x13, x0, 1
    sw x13, 0(x10)

fail:

    # Fin del programa
