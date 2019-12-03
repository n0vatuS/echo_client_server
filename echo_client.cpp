#include <stdio.h> // for perror
#include <string.h> // for memset
#include <unistd.h> // for close
#include <arpa/inet.h> // for htons
#include <netinet/in.h> // for sockaddr_in
#include <sys/socket.h> // for socket
#include <stdlib.h> // for exit
#include <vector>
#include <thread> // for print msg

using namespace std;

const static int BUFSIZE = 1024;
int sockfd;

void recv_msg() {
	while(true) {
		char buf[BUFSIZE];
		ssize_t received = recv(sockfd, buf, BUFSIZE - 1, 0);
		if (received == 0 || received == -1) {
			perror("recv failed");
			exit(1);
		}
		buf[received] = '\0';
		printf("%s\n", buf);
	}
}

void usage() {
	printf("syntax : echo_client <host> <port>\n");
	printf("sample : echo_client 127.0.0.1 1234\n");
}

int main(int argc, char ** argv) {
	if(argc != 3) {
		usage();
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket failed");
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2]));
	if(!inet_aton(argv[1], &addr.sin_addr)) {
		printf("Conversion Error!\n");
		exit(1);
	}
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

	int res = connect(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr));
	if (res == -1) {
		perror("connect failed");
		return -1;
	}
	printf("connected\n");
	
	thread t(recv_msg);
	
	while (true) {
		char buf[BUFSIZE];

		scanf("%s", buf);
		if (strcmp(buf, "quit") == 0) break;

		ssize_t sent = send(sockfd, buf, strlen(buf), 0);
		if (sent == 0) {
			perror("send failed");
			break;
		}
	}
	close(sockfd);
}
