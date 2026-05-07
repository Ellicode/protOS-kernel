# Data section
gdtr:
    .word 0      # DW equivalent (2 bytes for limit)
    .quad 0      # DQ equivalent (8 bytes for base)

.global setGDT
.type setGDT, @function
setGDT:
   movw %di, (gdtr)        # Move 16-bit DI into the first 2 bytes (limit)
   movq %rsi, (gdtr) + 2     # Move 64-bit RSI into base (offset by 2 bytes)
   
   lgdt (gdtr)             # Load GDT using the pointer at 'gdtr'
   ret