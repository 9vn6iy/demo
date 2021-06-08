tar1 = sender
tar2 = sniffer
src1 = $(wildcard *.c)
src2 = $(wildcard ../*.c)
deps = $(wildcard ../../iclude/*.h)
obj1 = $(patsubst %.c, %.o, $(src1))
obj2 = $(patsubst %.c, %.o, $(src2))

all : $(tar1) $(tar2)

$(tar1) : sender.o $(obj2)
	cc $^ -o $@ -Wall
$(tar2) : sniffer.o $(obj2)
	cc $^ -o $@ -Wall

%.o:$(src1)%.c
	cc -c $< -o $@
%.o:$(src2)%.c
	cc -c $< -o $@
%.o : %.c $(deps)
	cc -c $< -o $@ -Wall

.PHONY : clean
clean :
	rm -rf $(tar1) $(tar2) $(obj1) $(obj2)
