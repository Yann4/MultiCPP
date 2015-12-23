SERVER_FILES=Server.cpp
CLIENT_FILES=Client.cpp
COMMON_HEADERS=Socket.h

LIB_FILES=Ws2_32.lib

SERVER_EXE=server.exe
CLIENT_EXE=client.exe

EXECUTABLES=$(SERVER_EXE) $(CLIENT_EXE)
OBJ_FILES = $(SERVER_FILES:cpp=obj) $(CLIENT_FILES:cpp=obj)

CPP = cl
rm = del

all: server client

cleanmake: clean all

server: $(SERVER_FILES) $(COMMON_HEADERS)
	$(CPP) $(SERVER_FILES) /Fe$(SERVER_EXE) $(LIB_FILES)

client: $(CLIENT_FILES) $(COMMON_HEADERS)
	$(CPP) $(CLIENT_FILES) /Fe$(CLIENT_EXE) $(LIB_FILES)

clean:
	$(rm) $(OBJ_FILES) $(EXECUTABLES)
