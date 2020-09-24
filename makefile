.PHONY= clean
# _server_objs = main.o
# server_objs = $(addprefix obj/server/, $(_server_objs))

server_objs = $(subst src/,obj/, $(patsubst %.cpp,%.o, $(shell find src/server/ -type f -name *.cpp)))
build: server

client: $(client_objs)
	$(CXX) $(CFLAGS) -o bin/client $(client_objs)

server: $(server_objs)
	$(CXX) $(CFLAGS) -o bin/server $(server_objs)

obj/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -c -o $@ $< 

clean:
	rm -r bin/* obj/*