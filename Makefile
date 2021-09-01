TARGET := processpng.out
CC     := cc
CFLAGS := -Wall -Wextra -g -O0
OUT    := ./build

$(OUT)/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

all: $(TARGET)

clean:
	rm $(TARGET)
	rm $(OUT)/*.o

OBJS = $(OUT)/main.o


$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) -lpng -lm

$(OUT)/main.o: \
	./main.c
