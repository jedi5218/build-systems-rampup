.PHONY= clean
# _server_objs = main.o
# server_objs = $(addprefix obj/server/, $(_server_objs))

CFLAGS = \
    -Wall -Wextra -pedantic -Wformat -Wformat-security\
    -Wunused-but-set-variable -Wno-deprecated-declarations\
    -fsanitize=address -fno-omit-frame-pointer

server_objs = $(subst src/,obj/, $(patsubst %.cpp,%.o, $(shell find src/server/ -type f -name *.cpp)))
client_objs = $(subst src/,obj/, $(patsubst %.cpp,%.o, $(shell find src/client/ -type f -name *.cpp)))
common_objs = $(subst src/,obj/, $(patsubst %.cpp,%.o, $(shell find src/common/ -type f -name *.cpp)))


client_headers = $(shell find src/client/ -type f -name *.hpp)
server_headers = $(shell find src/server/ -type f -name *.hpp)
common_headers = $(shell find src/common/ -type f -name *.hpp)

build: server client

client: $(client_objs) $(common_objs)
	$(CXX) $(CFLAGS) -o bin/client $^

server: $(server_objs) $(common_objs)
	$(CXX) $(CFLAGS) -o bin/server $^


obj/client/%.o: src/client/%.cpp $(client_headers) $(common_headers)
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -c -o $@ $< -I src/

obj/server/%.o: src/server/%.cpp $(server_headers) $(common_headers)
	echo $(common_headers)
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -c -o $@ $< -I src/

obj/common/%.o: src/common/%.cpp $(common_headers)
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -c -o $@ $< -I src/

clean:
	rm -rf bin/* obj/*
