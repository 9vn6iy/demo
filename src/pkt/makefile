tar1 = PKTV3Sender
tar2 = PKTV3Sniffer
src = $(wildcard *.c)
deps = $(wildcard ../../iclude/*.h)
obj = $(patsubst %.c, %.o, $(src))

all : $(tar1) $(tar2)

$(tar1) : PKTV3Sender.o
	cc -pthread $^ -o $@ -Wall
$(tar2) : PKTV3Sniffer.o
	cc -pthread $^ -o $@ -Wall

%.o:$(src)%.c
	cc -c $< -o $@
%.o : %.c $(deps)
	cc -c $< -o $@ -Wall

.PHONY : clean
clean :
	rm -rf $(tar1) $(tar2) $(obj1) $(obj2)



