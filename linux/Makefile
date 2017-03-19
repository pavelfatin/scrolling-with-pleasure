all: MouseLogger.c ScrollingLogger.c
	gcc MouseLogger.c -lX11 -lXi -o MouseLogger
	gcc ScrollingLogger.c -lX11 -lXi -o ScrollingLogger

clean:
	rm MouseLogger ScrollingLogger
