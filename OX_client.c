/*
/*
** client.c -- 一個 stream socket client 的 demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "3490" // Client 所要連線的 port
#define MAXDATASIZE 100 // 我們一次可以收到的最大位原組數（number of bytes）

// 取得 IPv4 或 IPv6 的 sockaddr：
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
void printform(char* form[],char* sc)
{
	int i,j=0;
	for(i=0;i<5;i++){
		if(i%2){//i=1,3時 
			printf("%s", form[i]);
		}
		else{//i=0,2,4時 
			printf(form[i],sc[j],sc[j+1],sc[j+2]); 
			j+=3;
		}
	}
}	

int check(char sc[3][3])
{	
	int i;
	for(i=0;i<3;i++){
		if((sc[i][0]==sc[i][1]&&sc[i][1]==sc[i][2])||//列判斷 
		   (sc[0][i]==sc[1][i]&&sc[1][i]==sc[2][i]))//行判斷 
			{
				return 1;
			}
	}
	if((sc[0][0]==sc[1][1]&&sc[1][1]==sc[2][2])||//對角判斷 
	   (sc[0][2]==sc[1][1]&&sc[1][1]==sc[2][0]))
	   {
		   return 1;
	   }
	return 0;
}

int main(int argc, char *argv[])
{
  int sockfd, numbytes, sd_numbytes;
  char buf[MAXDATASIZE];
  struct addrinfo hints, *servinfo, *p;
  int rv;
  int num_rcv;
  char s[INET6_ADDRSTRLEN];
  char sd_msg[MAXDATASIZE];
  char * frm[] = {
	"%c|%c|%c\n",
	"-+-+-\n",
	"%c|%c|%c\n",
	"-+-+-\n",
	"%c|%c|%c\n" };
  char sc[3][3] = { '1', '2', '3',
	'4', '5', '6',
	'7', '8', '9' };
  char turn = 'O';
  int pos, round = 0;

  if (argc != 2) {
    fprintf(stderr,"usage: client hostname\n");
    exit(1);
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // 用迴圈取得全部的結果，並先連線到能成功連線的
   for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
      p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);

  printf("client: connecting to %s\n", s);

  freeaddrinfo(servinfo); // 全部皆以這個 structure 完成

  printform(frm, (char*)sc);

      while(round < 9) {

        if(recv(sockfd, buf, sizeof(buf), 0) < 0) {
          perror("received from server failed !");
          exit(1);
        }

        if(turn == 'X') {

          sscanf(buf, "%d", &pos);
          printf("turn[%c]>>%s\n", turn,buf);
        
        }
        else {
          printf("turn[%c]>>", turn);
          sscanf(buf, "%d", &pos);
        }
        
        ((char*)sc)[pos-1] = turn;

        printform(frm, (char*)sc); //?

        if(check(sc)) {

          printf("%c win!", turn);
          break;

        }
        round++;
        turn = (turn=='O') ? 'X' : 'O';

      }
      if(round==9) {
 
        printf("平手！！");

      }
    /* Send message to the server */
    /* if((sd_numbytes = send(sockfd, sd_msg, strlen(sd_msg), 0))==-1) {
      perror("ClientERROR");
      exit(1);
    } */
  
  close(sockfd);

  return 0;

}
