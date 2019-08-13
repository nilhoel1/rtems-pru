.origin 0
.entrypoint START
START:
ZERO &r0, 120
MOV r31, (1 << 5) 
HALT
