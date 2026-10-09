#make run IMG_PATH=path/to/image.png

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 $(shell pkg-config --cflags opencv4)
LDLIBS = $(shell pkg-config --libs opencv4)

TARGET = main
SRC = main.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

run: $(TARGET)
	./$(TARGET) "$(IMG_PATH)"

clean:
	rm -f $(TARGET)