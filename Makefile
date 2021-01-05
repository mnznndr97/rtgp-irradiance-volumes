# Tested on XUbuntu 20.04
# After installing packages in linux-install.sh

#name of the file
FILENAME = IrradianceVolumes

# Xcode compiler
CXX = clang++

# Include path
IDIR = include/

# Libraries path
LDIR = libs/mac/

# compiler flags:
CXXFLAGS  = -g -O0 -Wall -Wno-invalid-offsetof -Wuninitialized -std=c++17 -I$(IDIR) -m64 -msse4.1 -DDEBUG -fsanitize=address

CXXRFLAGS  = -g -O2 -Wall -Wno-invalid-offsetof -Wuninitialized -std=c++17 -I$(IDIR) -m64 -msse4.1 -DNDEBUG

# linker flags:
LDFLAGS = -L/usr/lib/x86_64-linux-gnu/ -L$(LDIR) -lGL -lglfw -lassimp -lz -lIrrXML -lX11 -lpthread -lXrandr -lXi -ldl -lfreetype -ltbb

SOURCES = include/glad/glad.c main.cpp


TARGET = $(FILENAME).out

all: debug

debug:
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SOURCES) -o $(TARGET)

asan:
	$(CXX) $(CXXRFLAGS) -fsanitize=address $(LDFLAGS) $(SOURCES) -o $(TARGET)

release:
	$(CXX) $(CXXRFLAGS) $(LDFLAGS) $(SOURCES) -o $(TARGET)

.PHONY : clean
clean :
	-rm $(TARGET) 
	-rm -R $(TARGET).dSYM
