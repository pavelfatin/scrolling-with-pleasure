all: MouseLogger.c TearingExample.c
	gcc MouseLogger.c -DWINVER=0x0501 -o MouseLogger
	gcc TearingExample.c -lgdi32 -w -o TearingExample

clean:
	del MouseLogger.exe TearingExample.exe
