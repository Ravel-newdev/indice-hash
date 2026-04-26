CC     = gcc
CFLAGS = -Wall -Wextra -g
TARGET = hash_extensivel
SRCS   = main.c hash.c
OBJS   = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) bucket_*.txt diretorio.txt out.txt

run: all
	./$(TARGET)

.PHONY: all clean run
