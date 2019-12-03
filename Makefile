all: echo_server echo_client

echo_server: echo_server.cpp
	g++ -std=c++11 -o echo_server echo_server.cpp -pthread

echo_client: echo_client.cpp
	g++ -std=c++11 -o echo_client echo_client.cpp -pthread

clean:
	rm -f echo_server echo_client
