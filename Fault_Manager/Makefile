CC         = gcc
CFLAGS     = -Wall -O2
LDFLAGS    = -lm

BUILD_DIR  = build
TARGET     = $(BUILD_DIR)/fault_manager_app

SRCS       = faultmanagermain.c \
             fault_predictor.c \
             fault_manager.c \
             rtos_fault_model.c \
             rtos_linear_regressor.c

# Object files: same names with .o, inside BUILD_DIR
OBJS       = $(SRCS:%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean

all: $(TARGET)

# Create the build directory before anything else
$(BUILD_DIR):
	mkdir -p $@

# Link all objects into the final binary
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile each .c into a .o inside the build directory
# The order-only prerequisite (| $(BUILD_DIR)) ensures the directory exists
# but does not trigger a rebuild every time.
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)