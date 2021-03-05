/*
** server.c – 展示一個stream socket server
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define PORT "3490" // 提供給使用者連線的 port
#define BACKLOG 10 // 有多少個特定的連線佇列（pending connections queue）

#define MAXDATASIZE 100

void sigchld_handler(int s)
{
  while(waitpid(-1, NULL, WNOHANG) > 0);
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
// 取得sockaddr，IPv4或IPv6：
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
  int sockfd, new_fd; // 在 sock_fd 進行 listen，new_fd 是新的連線
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // 連線者的位址資訊 
  socklen_t sin_size;   //socklen_t: 一個跟int長度一樣的型別 
  struct sigaction sa;
  int yes=1;
  char s[INET6_ADDRSTRLEN];
  int rv;
  char buf[MAXDATASIZE];
  int num_rcv;
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

  do {

    printform(frm, (char*)sc);

    if(turn == 'X') {  //client turn
      snprintf(buf, MAXDATASIZE, "%d", pos); //?
      num_rcv=send(new_fd, buf, sizeof(buf), 0); //?
      num_rcv=recv(new_fd, buf, sizeof(buf), 0);
      sscanf(buf, "%d", &pos); //scan an int from buf and store in pos 
    }
    else {  //server play
      printf("turn[%c]>>", turn);
      scanf("%d", &pos);
    }
     
    ((char*)sc)[pos-1] = turn;

    if(check(sc)) {

      printf("%c win!", turn);
      break;

    }
    round++;
    turn = (turn=='O') ? 'X' : 'O';
    
    } while(round < 9);
 
  if(round == 9 )
  {
    printf("draw!\n");
  }



  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;  //for IPv4 => AF_INET, IPv6 = AF_INET6
  hints.ai_socktype = SOCK_STREAM; //TCP stream sockets
  hints.ai_flags = AI_PASSIVE; // 使用我的 IP
  
  // NULL can be replaced by specific address
  // getaddrinfo: return a pointer(servinfo) point to a linked list
  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  /* 以迴圈找出全部的結果，對getaddrinfo()進行錯誤檢查，並綁定（bind）到第一個能
     用的結果
     sockfd is a socket discriptor, return -1 when it's error
  */
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
      p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
      sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    return 2;
  }

  freeaddrinfo(servinfo); // 全部都用這個 structure

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  sa.sa_handler = sigchld_handler; // 收拾全部死掉的 processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  printf("server: waiting for connections...\n");

  while(1) { // 主要的 accept() 迴圈
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }

    // 取get_in_addr的ip address存到s並印出來
    inet_ntop(their_addr.ss_family,
      get_in_addr((struct sockaddr *)&their_addr),
      s, sizeof s);
    printf("server: got connection from %s\n", s);
   
    do {

      if(recv(new_fd, buf, sizeof(buf), 0) < 0) { //recieved from client
      perror("received from client failed !");
      exit(1);
      }

      turn = 'O';
      printf("turn[%c]>>%s\n", turn, buf); //print the choice of client
      sscanf(buf, "%d", &pos); //store the int form buf in pos
      ((char*)sc)[pos-1] = turn;
       
      if(check(sc)) {

          printf("%c win!", turn);
          break;

      }

      round++;

      if(round==5) {
        printf("The competition is draw !\n");
        break;
      }

      /* Server turn */

      turn = (turn == 'O') ? 'X' : 'O' ;
      printf("trun[%c]>>", turn); //'X' turn (server)
      scanf("%d", &pos);

      ((char*)sc)[pos-1] = turn;
      printform(frm, (char*)sc);

      if(check(sc)) {

          printf("%c win!", turn);
          break;

      }
      snprintf(buf, MAXDATASIZE, "%d", pos); //store pos in buf

      if(send(new_fd, buf, sizeof(buf), 0) < 0) { //send to client
        perror("send to client failed !");
        exit(1);
      }

    }while(round < 5);

    /* 直接send也可以
    send(new_fd, "Hello, world!", 13, 0); */
     
    /*pid_t fork()=-1: error, =0: child process, >0: parent process 
      fork a child process to handle the new connection*/
    if (!fork()) { // 這個是 child process
      close(sockfd); // child 不需要 listener

        close(new_fd);

        exit(0);
    }


    close(new_fd); // parent 不需要這個

  }

  return 0;
}
