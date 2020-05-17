CC = g++
OBJS = cacheSim.o
EXEC = cacheSim
COMP_FLAG = --std=c++11

$(EXEC) : $(OBJS)
	$(CC) $(OBJS) -o $@

chaceSim.o:
	$(CC) -c $(COMP_FLAG) $*.cpp

clean:
	rm -f $(OBJS) $(EXEC)

