TARGET := processpng.out
CC     := cc
CFLAGS := -Wall -Wextra -g -O0
OUT    := ./build

$(OUT)/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

all: $(TARGET)

clean:
	rm $(OUT)/*.o
	rm $(TARGET)

OBJS = $(OUT)/main.o \
       $(OUT)/panic.o \
       $(OUT)/imgpng.o \
			 $(OUT)/hmap.o \
       $(OUT)/palettes.o \
       $(OUT)/imageprocessing.o


$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) -lpng -lm

$(OUT)/main.o: \
	./main.c \
	./panic.h \
	./imgpng.h \
	./imageprocessing.h \
	./hmap.h \
	./palettes.h

$(OUT)/panic.o: \
	./panic.c \
	./panic.h

$(OUT)/imgpng.o: \
	./imgpng.c \
	./imgpng.h

$(OUT)/imageprocessing.o: \
	./imageprocessing.c \
	./imageprocessing.h \
	./palettes.h

$(OUT)/palettes.o: \
	./palettes.c \
	./hmap.h \
	./palettes.h

$(OUT)/hmap.o: \
	./hmap.c \
	./hmap.h
