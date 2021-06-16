tar1 = server
tar2 = client
src1 = $(wildcard ./*.c)
src2 = $(wildcard ../*.c)
deps = $(wildcard ../../include/*.h)
obj1 = $(patsubst %.c, %.o, $(src1))
obj2 = $(patsubst %.c, %.o, $(src2))

all : $(tar1) $(tar2)

$(tar1) : server.o $(obj2)
	cc -pthread $^ -o $@ -Wall
$(tar2) : client.o $(obj2)
	cc -pthread $^ -o $@ -Wall

%.o:$(src1)%.c
	cc -c $< -o $@
%.o:$(src2)%.c
	cc -c $< -o $@
%.o:%.c $(deps)
	cc -c $< -o $@ -Wall

.PHONY : clean
clean :
	rm -rf $(tar1) $(tar2) $(obj1) $(obj2)

