#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#define MAXLINE 80
#define SERV_PORT 11066

int main(int argc, char *argv[])
{
	struct sockaddr_in servaddr;
	char buf[MAXLINE];
	int sockfd, n;
	char *str;


	if (argc != 3) {
		printf("protol: </device> <message>\n");
		exit(1);
	}
	str = argv[2];

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
/*	struct sockaddr_in localaddr;
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = inet_addr("92.168.60.100");
	localaddr.sin_port = 0;  // Any local port will do
	bind(sockfd, (struct sockaddr *)&localaddr, sizeof(localaddr));
*/


	//set timeout
	struct timeval tv_timeout;
	tv_timeout.tv_sec = 10;
	tv_timeout.tv_usec = 0;
	if (setsockopt
	    (sockfd, SOL_SOCKET, SO_SNDTIMEO, (void *) &tv_timeout,
	     sizeof(struct timeval)) < 0) {
		perror("setsockopt error set send time \n");
	}
	tv_timeout.tv_sec = 10;
	tv_timeout.tv_usec = 0;
	if (setsockopt
	    (sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *) &tv_timeout,
	     sizeof(struct timeval)) < 0) {
		perror("setsockopt error set receive time\n");
	}

	char ip[32] = { 0 };
	char *ifname = argv[1];
	int s32Ret = 0;
	char ipaddr[48];
	struct sockaddr_in *sin;
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	memcpy(ifr.ifr_name, ifname, strlen(ifname));

	if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) {
		perror("ioctl error!\n");
		s32Ret = -2;
		goto FAILED;
	} else {
		struct sockaddr_in *addr;
		char *address;
		addr = (struct sockaddr_in *) &(ifr.ifr_addr);
		address = inet_ntoa(addr->sin_addr);
		printf("inet addr: %s\n ", address);


		struct ifreq ifr;
		char *ifname = argv[1];
//		memset(&ifr, 0x00, sizeof(ifr));
		strcpy(ifr.ifr_name, ifname);

//		setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(char *)&ifr,sizeof(ifr));
		int err = setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE,
				     (char *) &ifr,
				     sizeof(ifr));

		if (err == 0) {
			printf("setsockopt success\n");
		} else {
			printf("setsockopt failure %d\n",err);
		}
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	//47.93.55.33
	inet_pton(AF_INET, "20.40.3.14", &servaddr.sin_addr);
	servaddr.sin_port = htons(SERV_PORT);
/***********************/
	struct ifreq ifrset;
	socklen_t opt_len = sizeof(ifrset);
	int err2 = getsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE,
			      (char *) &ifrset, &opt_len);
	if (err2 == 0) {
		printf("current ifname = %s\n", ifrset.ifr_name);
	}
/*******************/
	int conn_re = connect(sockfd, (struct sockaddr *) &servaddr,
			      sizeof(servaddr));

	if (conn_re == 0) {

		printf("socket connect success \n");
		write(sockfd, str, strlen(str));

	} else {
		printf("socket connect failure\n");
	}

//      n = read(sockfd, buf, MAXLINE);
//      printf("Response from server:\n");
//      write(STDOUT_FILENO, buf, n);
	printf("will be closed after 3s\n");
	sleep(3);
	close(sockfd);

      FAILED:
	close(sockfd);

	return 0;
}
