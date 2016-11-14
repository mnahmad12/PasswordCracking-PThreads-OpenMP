# Use this makefile to build all programs so long as you name them
# according to the recommended conventions

CC=gcc
CFLAGS=-I. -g -Wall -std=c99 -fopenmp 
DEPS = crack.h
PROGS      = dict_demo   md5_demo   passcrack   encrypt_all   omp_passcrack   pthread_passcrack
PROGS_OBJS = dict_demo.o md5_demo.o passcrack.o encrypt_all.o omp_passcrack.o pthread_passcrack.o
COMMON_OBJ = dict.o md5crypt_r.o crack_funcs.o parallel_funcs.o
LIBS= -lssl -lcrypto -lpthread

programs: $(PROGS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

dict_demo: $(COMMON_OBJ) dict_demo.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

md5_demo: $(COMMON_OBJ) md5_demo.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

encrypt_all: $(COMMON_OBJ) encrypt_all.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

passcrack: $(COMMON_OBJ) passcrack.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

omp_passcrack: $(COMMON_OBJ) omp_passcrack.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

pthread_passcrack: $(COMMON_OBJ) pthread_passcrack.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f *.o $(PROGS)
