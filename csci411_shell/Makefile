CXX = g++
CXXFLAGS = -Wall
TARGET = myshell
SRC = SimpleShell.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o myshell_history.txt
