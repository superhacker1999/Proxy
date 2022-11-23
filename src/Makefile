.PHONY: clean server_build server
SRC_SERVER=Server/Server.cc NetHandler/NetHandler.cc Client/Client.cc Logger/Logger.cc
CC=gcc
FLAGS=-lstdc++ -Wall -Werror -Wextra

clean:
	rm -rf server_build

server_build: clean
	$(CC) $(FLAGS) $(SRC_SERVER) -o server_build

server: server_build
	./server_build $(SERV_PORT) $(DB_IP) $(DB_PORT)

# make server SERV_PORT=54544 DB_IP=0.0.0.0 DB_PORT=3306

style_check:
	clang-format -i Server/* Client/* NetHandler/* Logger/*
