#include<stdio.h>
#include <stdlib.h>

void printform(char*form[],char* sc);
int check(char sc[3][3]);
int computer_think(char* sc); 

int main(void)
{
	char sc[3][3]={	
	'1','2','3',
	'4','5','6',
	'7','8','9'
	};
	char* form[]={
	"%c|%c|%c\n",
	"-+-+-\n",
	"%c|%c|%c\n",
	"-+-+-\n",
	"%c|%c|%c\n"
	};
	char turn ='O';
	int k,i=0;
	while(i<9){
		printform(form,(char*)sc);
		if(turn == 'X'){
			printf(">>turn[%c]:",turn);
			scanf("%d",&k);
		}
		else{
			printf(">>turn[%c]:",turn);
			scanf("%d",&k);
		}
		((char*)sc)[k-1]=turn;//將二維陣列視為一維處理 
		if(check(sc)) 
		{
			printf("winner[%c]!",turn);
			break;
		}
		i++;
		turn=(turn=='O')?'X':'O';//turn是否為O，是的話回傳X，不是則回傳O

	}
	if(i==9)//到達局數上限 
	{
		printf("平手!\n");
	}
}

void printform(char* form[],char* sc)
{
	int i,j=0;
	for(i=0;i<5;i++){
		if(i%2){//i=1,3時 
			printf(form[i]);
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
