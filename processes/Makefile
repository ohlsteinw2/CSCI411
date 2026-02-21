CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS = -lrt

# Executable names
SERVER = server
CLIENT = client

# Default target (runs when you just type "make")
all: $(SERVER) $(CLIENT)

# Build server
$(SERVER): server.cpp
	$(CXX) $(CXXFLAGS) server.cpp -o $(SERVER) $(LDFLAGS)

# Build client
$(CLIENT): client.cpp
	$(CXX) $(CXXFLAGS) client.cpp -o $(CLIENT) $(LDFLAGS)

# Clean up compiled files
clean:
	rm -f $(SERVER) $(CLIENT)
