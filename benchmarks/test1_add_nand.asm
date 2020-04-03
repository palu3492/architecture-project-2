 lw 1 0 one # load reg1 with 1
 lw 2 0 limit # load reg1 with 2000 (symbolic address)
 lw 3 0 0 # load reg3 with 0
start add 3 1 3 # var ++
 add 1 2 4
 nand 1 2 4
 beq 1 2 start # go back to the beginning of the loop
 halt
limit .fill 2000
one .fill 1