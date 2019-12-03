#include <stdio.h> // for perror
#include <string.h> // for memset
#include <unistd.h> // for close
#include <arpa/inet.h> // for htons
#include <netinet/in.h> // for sockaddr_in
#include <sys/socket.h> // for socket
#include <stdlib.h> // for exit
#include <map>
#include <thread> // for multi connect

std::map<int, bool> valid;
std::map<int, std::thread> threads;
bool b;

void echo(int childfd) {
	if (childfd < 0) {
		perror("ERROR on accept");
		return;
	}
	printf("connected\n");

	while (true) {
		const static int BUFSIZE = 1024;
		char buf[BUFSIZE];

		ssize_t received = recv(childfd, buf, BUFSIZE - 1, 0);
		if (received == 0 || received == -1) {
			perror("recv failed");
			break;
		}
		buf[received] = '\0';
		printf("%s\n", buf);
		
		if(b) {
			for(auto it = threads.begin(); it != threads.end(); it++) {
				if(valid.find(it->first)->second) {
					ssize_t sent = send(it->first, buf, strlen(buf), 0);
					if (sent == 0) {
						perror("send failed");
					}
				}
			}
		}
		else {
			ssize_t sent = send(childfd, buf, strlen(buf), 0);
			if (sent == 0) {
				perror("send failed");
				break;
			}
		}
	}
	
	valid.find(childfd)->second = false;
}

void usage() {
	printf("syntax : echo_server <port> [-b]\n");
	printf("sample : echo_server 1234 -b\n");
}

int main(int argc, char ** argv) {
	if(!(argc == 3 && argv[2][0] == '-' && argv[2][1] == 'b') && argc != 2) {
		usage();
		exit(1);
	}
	if(argc == 3) b = true;

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket failed");
		return -1;
	}

	int optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,  &optval , sizeof(int));

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[1]));
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

	int res = bind(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr));
	if (res == -1) {
		perror("bind failed");
		return -1;
	}

	res = listen(sockfd, 2);
	if (res == -1) {
		perror("listen failed");
		return -1;
	}
	
	while (true) {
		struct sockaddr_in addr;
		socklen_t clientlen = sizeof(sockaddr);
		
		int childfd = accept(sockfd, reinterpret_cast<struct sockaddr*>(&addr), &clientlen);
		
		valid.insert(std::make_pair(childfd, true));
		threads.insert(std::make_pair(childfd, std::thread(echo, std::ref(childfd))));
	}
	close(sockfd);
}
