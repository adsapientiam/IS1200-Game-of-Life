  # run.S
  # Written 2015-2017 by F Lundevall
  # Skeleton file for IS1200/IS1500 lab 1.
  # The macros PUSH and POP are in the public domain.
  # Please add your own code at the end of the file.

  #
  # Please keep the two macros PUSH and POP unchanged
  #

.macro	PUSH reg # REMARQUE LA MANQUE DE PARANTHÈSE
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

# Eux spécifiquement .globalisés car ils sont les sous-routines
# principales qui sont d'abord initialisés
.global delay
.global time2string
.global enable_interrupt

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm
  #
  # Please add your own code below this line
  #
.text
hexasc: 
	andi $a0, $a0, 0xf # so that only LSB 0-3 included
	
	# If input's LSBs constitute a value more than 9, set 1 to $t1 
	addi $t3, $0, 9
	slt $t1, $t3, $a0
	
	li $t0, 0x30 
	
	# If input's LSBs constitute a value more than 9
	# then the ASCII-encoding will skip ;:<=>@ by jumping
	# + 0x7
	addi $t4, $0, 7
	mul $t2, $t1, $t4 
	add $t0, $t0, $t2
	
	# Value + Base = ASCII value to be interpreted
	# by syscall
	add $v0, $a0, $t0
	jr $ra
	nop
	
time2string:
	PUSH $s1
	PUSH $s2
	PUSH $ra # since the hexasc will override it
	move $s1, $a0 # Since $a0 will have to be used in subroutine calls and then first in the popping
	# in the popping
	move $s2, $a1

	# Each bit of the clock, from LSB to MSB. These are entire nimbles
	# This order cuz the Stack's LIFO and the memory's supposed little endian
	# charactheristics.
	# Only have to shift to the least 4 LSB since the hexasc will remove all the rest
	
	#xX:XX
	srl $a0, $s2,12
	jal hexasc
	nop
	PUSH $v0
	
	#Xx:XX 
	srl $a0, $s2,8 
	jal hexasc
	nop
	PUSH $v0
	
	#XX:xX 
	srl $a0, $s2,4 	 		   
	jal hexasc
	nop
	PUSH $v0

	#XX:Xx
	# Receives a + 1 because the tick takes place 
	# before time2string is called
	add $a0, $0, $s2
	jal hexasc
	nop
	PUSH $v0
	
	# Add the finalizing nullspace
	andi $t0, $0, 0x0
	sb $t0, 5($s1)

	# Popping the contents into the memory
	POP $t1
	sb $t1, 4($s1)
	POP $t1
	sb $t1, 3($s1)
	
	#add the colon
	ori $t3, $0, 0x3A
	sb  $t3,  2($s1)
	
	POP $t1
	sb $t1, 1($s1)
	POP $t1
	sb $t1, 0($s1)
	
	POP $ra
	POP $s2
	POP $s1
	
	jr $ra
	nop
	
oneMiliSecond: #for loop
	slt $t2, $a2, $a1  # i < (<NOMBRE CHOISIE> : 1 ms)
	beq $t2, $0, delay # -||-
	nop
	addi $a2, $a2, 1   # i++
	j oneMiliSecond
	nop
	
delay: 
	li $a1, 4711 # Number : 1 ms
	# Set in the beginning because this corresponds to the
	# while (ms > 0)
	slt $t2, $a0, $0 # ms < 0 which...
	sub $a0, $a0, 1 # ms = ms-1
	andi $a2, $a2, 0x0 # i=0
	beq $t2, $0, oneMiliSecond # !(ms < 0) part 2
	nop
	
	jr $ra # Result of escaping the while-loop
	nop

enable_interrupt:
  	ei
  	jr $ra
  	nop