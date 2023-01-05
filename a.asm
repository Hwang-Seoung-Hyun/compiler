	      INT  0, 88
	      SUP  0, main
	      RET  0, 0
main:
	      INT  0, 20
	      LDA  1, 16
	     LITI  0, 1
	      STX  0, 0
	      POP  0, 1
	      LDA  0, 16
	      LDA  0, 12
	      LDA  1, 12
	      LOD  0, 24
	      STX  0, 0
	      STX  0, 0
	      STX  0, 0
	      POP  0, 1
	      INT  0, 12
	      LDA  0, 28
	      LDA  1, 12
	      POP  0, 5
	     ADDR  0, scanf
	      CAL  0, 0
L2:
	      LOD  1, 12
	     LITI  0, 10
	     LSSI  0, 0
	      JPC  0, L3
	      LDA  0, 12
	      LOD  0, 12
	      LOD  1, 12
	     ADDI  0, 0
	      STX  0, 0
	      POP  0, 1
	      INT  0, 12
	      LDA  0, 36
	      LOD  1, 12
	      LOD  0, 12
	      POP  0, 6
	     ADDR  0, printf
	      CAL  0, 0
L1:
	      LOD  1, 12
	      LDA  1, 12
	      LDX  0, 0
	     INCI  0, 0
	      STO  0, 0
	      POP  0, 1
	      JMP  0, L2
L3:
	      INT  0, 12
	      LDA  0, 52
	      LOD  1, 12
	      LOD  0, 12
	      POP  0, 6
	     ADDR  0, printf
	      CAL  0, 0
	      LOD  0, 16
	     LITI  0, 0
	     GTRI  0, 0
	      JPC  0, L4
	      LOD  0, 12
	     LITI  0, 41
	     GTRI  0, 0
	      JPC  0, L5
	      INT  0, 12
	      LDA  0, 68
	      POP  0, 4
	     ADDR  0, printf
	      CAL  0, 0
	      JMP  0, L6
L5:
	      INT  0, 12
	      LDA  0, 76
	      LOD  0, 12
	      POP  0, 5
	     ADDR  0, printf
	      CAL  0, 0
L6:
L4:
	      LDA  1, -4
	     LITI  0, 0
	      STO  0, 0
	      RET  0, 0
	      RET  0, 0
.literal    24 1 
.literal    28 "%d" 
.literal    36 "i=%d, a=%d\n" 
.literal    52 "i=%d, a=%d\n" 
.literal    68 "wow\n" 
.literal    76 "how %d\n" 
