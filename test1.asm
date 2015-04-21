# This is test program 1.  This program does nothing useful.
	
	add $s0, $0, $zero
	addi $t0, $t0, 100#test comment
	addi $a0, $a0,100
test:	add $s0, $s0, $a0 # this is a comment
	addi $a0, $a0, -1
	bne $a0, $0, test	# this is another comment
	addi $a0, $a0, 100
	
test1: 
    #test comment

    
      
lw	$a0, 8($a1)
	sw $a0,4($a1)
  j test1		
jr	$ra
	jal test1
	slt$t0,$a0,$a1
	beq$t0,$t1,test
	sub $t3, $t1, $t1




	
        sll $a0, $a1, 2
