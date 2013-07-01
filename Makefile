CC = avr-gcc

CFLAGS = -mmcu=atmega8 -Os -D__DELAY_BACKWARD_COMPATIBLE__ 
#CFLAGS = -mmcu=atmega8 -Os -DLOGGER -DPRINT -DF_CPU=1000000
#CFLAGS = -mmcu=at90s2333 -O2 -DLOGGER -DPRINT
OBP = avr-objcopy
OFLAGS = -O ihex


.c.o :
	$(CC) $(CFLAGS) -c $< -o $@


SRC = alfa-1.c 
OBJ = alfa-1.o
ELF = alfa-1 


all:   $(OBJ)
	$(CC) $(CFLAGS) -o $(ELF) $(OBJ)
	$(OBP) $(OFLAGS) $(ELF) robi.hex


