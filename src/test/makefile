tar1 = sender
tar2 = sniffer
src = $(wildcard *.c)
obj = $(patsubst %.c, %.o, $(src))

all : $(tar1) $(tar2)

$(tar1) : sender.o
	cc $^ -o $@ -Wall
$(tar2) : sniffer.o
	cc $^ -o $@ -Wall

%.o:$(src)%.c
	cc -c $< -o $@
%.o : %.c
	cc -c $< -o $@ -Wall

.PHONY : clean
clean :
	rm -rf $(tar1) $(tar2) $(obj1) $(obj2)
