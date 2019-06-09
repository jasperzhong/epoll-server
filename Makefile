CC=g++
Target = yzmond 
.PHONY: all clean 
HEADFILE= common/common.h common/packet_head.h common/server_to_client.h common/client_to_server.h logger.h server_conf.h async_server.h client_descriptor.h 
INCLUDEFILE=  common/packet_head.cpp common/server_to_client.cpp common/client_to_server.cpp logger.cpp server_conf.cpp async_server.cpp client_descriptor.cpp 
CXXFLAGS= -std=c++11 -g -I /usr/include/mysql -L /usr/lib64/mysql -lmysqlclient
all:$(Target)
$(Target):%:%.cpp $(HEADFILE) $(INCLUDEFILE)
	$(CC) $(CXXFLAGS) -o $@ $< $(HEADFILE) $(INCLUDEFILE)
clean:
	-rm -rf $(Target) *.out