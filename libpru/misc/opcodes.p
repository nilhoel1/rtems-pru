// This file lists all PRU instructions for version 2.
.origin 0
add r0, r1, r2
add r0, r1, 200
adc r31, r1, r2
adc r31, r1, 200
sub r0, r1, r2
sub r0, r1, 10
suc r0, r1, r2
suc r0, r1, 20
rsc r0, r1, r2
rsc r1, r1, 10
rsb r0, r1, r2
rsb r1, r1, 20
lsl r0, r1, 1
lsl r0, r0, r1.b0
lsl r3, r3.b0, 10
lsr r0, r1, 1
lsr r0, r1, r3.b0
lsr r0, r1, 1
and r0, r1, r2
and r3, r1.b0, r0.w0
and r3, r1.b0, r0.w1
and r3, r1.b0, r0.w2
and r3, r1.b0, r1.w0
and r3, r1.b0, r1.w1
and r3, r1.b0, r2.w2
or r0, r1, r2
or r3, r1.b0, r2.w2
or r3.b0, r3.b0, 1<<3
xor r0, r1, r2
xor r3, r1.b0, r2.w2
xor r3.b0, r3.b0, 1<<3
not r0, r1
min r0, r1, r2
max r0, r1, r2
clr r0, r1
clr r0, r1, 4
set r0, r1, r2
lmbd r0, r1, r2
mov r0, r1
mov r4, r5
ldi r0, 1
mvib r0, *r1.b0
mviw r0, *r1.b0
mvid r0, *r1.b0
lbbo r0, r1, 1, 2
sbbo r0, r1, 1, 2
lbco r0, c0, 1, 2
sbco r0, c0, 1, 2
zero 0, 8
fill 0, 8
xin 0, r0, 8
xout 0, r0, 8
xchg 0, r0, 8
jmp r0.w0
jmp r1.w0
jal r0.w0, r0.w2
jal r1.w0, r1.w2
call r0.w0
call r1.w0
ret
qbgt foo, r0.w0, 1
qbge foo, r0.w0, 1
qblt foo, r0.w0, 1
qble foo, r0.w0, 1
qbeq foo, r0.w0, 1
qbne foo, r0.w0, 1
qbne foo, r0, 0
qba foo
qbbs foo, r0, r1
qbbc foo, r0, r1
foo:wbs r31, r0
wbc r31, r0
halt
slp 0
slp 1
// V3 instructions -- not supported yet
//sxin 0, r0, 8
//sxout 0, r0, 8
//sxchg 0, r0, 8
//loop bar, 5
//iloop bar, 5
//bar:
