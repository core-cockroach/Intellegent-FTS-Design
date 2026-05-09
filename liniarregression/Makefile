CC      = gcc
CFLAGS  = -Wall -O2
LDFLAGS = -lm

TARGET  = fault_predictor
SRCS    = fault_predictor.c rtos_fault_model.c rtos_linear_regressor.c
OBJS    = $(SRCS:.c=.o)

# 'all' is the first target → default when you run 'make' or 'make all'
.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)