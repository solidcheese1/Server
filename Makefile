
CC = g++
CFLAGS = -Wall -g -I./Coms_parser -I./Socket -I./TCP -I./UDP -I.

SRC_DIRS = Coms_parser Socket TCP UDP ./

SOURCES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
OBJECTS = $(SOURCES:%.cpp=%.o)

OBJ_DIR = obj
TARGET = my_program


all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^

$(OBJ_DIR)/%.o: %.cpp
	mkdir -p $(OBJ_DIR)  
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)
