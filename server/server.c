#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sqlite3.h> //for sqlite3_open ..
#include <signal.h>
#include <time.h>//--------
//#define PATH_DICT "./dict.txt" //单词表
#define PATH_DATA "./staff_manage_system.db" //数据库
#define N 32
#define M 128
#define R 0x1 //管理员模式
#define L 0x2 //普通用户模式

#define Q 0x3  //管理员按人名查找
#define S 0x4 //管理员查找所有
#define H 0x5  //管理员查看历史记录
#define E 0x6 //退出
#define X 0x7  //管理员修改库
#define T 0x8  //管理员添加数据
#define D 0x9   //管理删除用户


typedef struct{
	int type;//消息类型
	char name[N];//用户名
	char text[M];//密码
	int id;
	int age;
	int usertype;
	
}MSG;
#define LEN_SMG sizeof(MSG)
#define err_log(log)\
	do{\
	 perror(log);\
	 exit(1);\
	}while(0)

typedef struct sockaddr SA;
void process_register(int clientfd,MSG *msg,sqlite3 *db);
void process_register1(int clientfd,MSG *msg,sqlite3 *db);

void process_query(int clientfd,MSG *msg,sqlite3 *db);
void process_query2(int clientfd,MSG *msg,sqlite3 *db);
void process_query3(int clientfd,MSG *msg,sqlite3 *db);
void process_update(int clientfd,MSG *msg,sqlite3 *db);
void process_insert(int clientfd,MSG *msg,sqlite3 *db);
void process_delete(int clientfd,MSG *msg,sqlite3 *db);

void process_history(int clientfd,MSG *msg,sqlite3 *db);
void insert_history(MSG *msg,sqlite3 *db);
void get_time(const char *date);
void handler(int arg)
{
	wait(NULL);
}
int main(int argc, const char *argv[])
{
	int serverfd,clientfd;
	struct sockaddr_in serveraddr,clientaddr;
	socklen_t len=sizeof(SA);
	int cmd;
	char clean[M]={0};
	MSG msg;
	pid_t pid;
	sqlite3 *db;
	ssize_t bytes;
	if(argc!=3)
	{
		printf("User:%s <IP> <port>\n",argv[0]);
		return -1;
	}

	if(sqlite3_open(PATH_DATA,&db)!=SQLITE_OK)//打开数据库
	{
			printf("%s\n",sqlite3_errmsg(db));
			return -1;
	}
	if((serverfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		err_log("fail to socket");
	}
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr=inet_addr(argv[1]);
	if(bind(serverfd,(SA*)&serveraddr,len)<0)
	{
		err_log("fail to bind");
	}
	if(listen(serverfd,10)<0)
	{
		err_log("fail to listen");
	}
 	signal(SIGCHLD,handler);//处理僵尸进程

	while(1)
	{
		if((clientfd=accept(serverfd,(SA*)&clientaddr,&len))<0)
		{
			perror("fail to accept");
			continue;
		}
		pid=fork();
		if(pid<0)
		{
			perror("fail to fork");
			continue;
		}
		else if(pid==0) //接收客户端的请求处理过程
		{
			close(serverfd);
			while(1)
			{
				
				bytes=recv(clientfd,&msg,LEN_SMG,0);
				insert_history(&msg,db);
				if(bytes<=0)
					break;
				switch(msg.type)
				{
					case R:  //管理员模式
						process_register(clientfd,&msg,db);
						break;
					case L: //普通用户模式
						process_register1(clientfd,&msg,db);
						break;
					case Q: //管理员按人名查找 //普通用户查询
						process_query(clientfd,&msg,db);
						break;
					case S: //管理员查找所有
						process_query2(clientfd,&msg,db);
						break;
					case X: //管理员修改库
						process_update(clientfd,&msg,db);
						break;
					case T: //管理员添加数据
						process_insert(clientfd,&msg,db);
						break;
					case D: //管理删除用户
						process_delete(clientfd,&msg,db);
						break;
					case H:  //管理员查看历史记录
						process_history(clientfd,&msg,db);
						break;
					case E:
					    exit(0);
				}
			}
			close(clientfd);
			exit(1);
		}
		else
		{
			close(clientfd);
		}


	}
	
	return 0;
}
void process_register(int clientfd,MSG *msg,sqlite3 *db)  //管理员登陆
{
	
	char sql[M]={0};
	char *errmsg;
	char **rep;
	int n_row;
	int n_column;

	sprintf(sql,"select * from usrinfo where name='%s' and passwd='%s' and usertype = 0",msg->name,msg->text);
	if(sqlite3_get_table(db,sql,&rep,&n_row,&n_column,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		strcpy(msg->text,"Fail");
		send(clientfd,msg,LEN_SMG,0);
		return;
	}
	else
	{
		if(n_row==0)//查不到
		{

			strcpy(msg->text,"Fail");
			send(clientfd,msg,LEN_SMG,0);
			return;
		}
		else  //只要行数大于0，无需打印，直接返回成功
		{

			strcpy(msg->text,"OK");
			send(clientfd,msg,LEN_SMG,0);
			return;
		}
	}
	
	

	
	
}

void process_register1(int clientfd,MSG *msg,sqlite3 *db) //普通用户登陆
{
	char sql[M]={0};
	char *errmsg;
	char **rep;
	int n_row;
	int n_column;

	sprintf(sql,"select * from usrinfo where name='%s' and passwd='%s' and usertype = 1",msg->name,msg->text);
	if(sqlite3_get_table(db,sql,&rep,&n_row,&n_column,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		strcpy(msg->text,"Fail");
		send(clientfd,msg,LEN_SMG,0);
		return;
	}
	else
	{
		if(n_row==0)//查不到
		{

			strcpy(msg->text,"Fail");
			send(clientfd,msg,LEN_SMG,0);
			return;
		}
		else  //只要行数大于0，无需打印，直接返回成功
		{

			strcpy(msg->text,"OK");
			send(clientfd,msg,LEN_SMG,0);
			return;
		}
	}
	
}


void process_query3(int clientfd,MSG *msg,sqlite3 *db) //修改库的时候先调用这个小查询函数
{
	char sql[M]={0};
		char *errmsg;
		char **rep;
		int n_row;
		int n_column;
		int i,j;
		sprintf(sql,"select * from usrinfo  where staffno='%d'",msg->id);
		if(sqlite3_get_table(db,sql,&rep,&n_row,&n_column,&errmsg)!=SQLITE_OK)
		{
				printf("%s\n",errmsg);
				strcpy(msg->text,"Fail");
				send(clientfd,msg,LEN_SMG,0);
				return;
		}
		else
		{
			if(n_row==0)
			{
				
				strcpy(msg->text,"Fail");
				send(clientfd,msg,LEN_SMG,0);
				return ;
			}
			else
			{
				
				strcpy(msg->text,"OK");
				send(clientfd,msg,LEN_SMG,0);						
				return;
			}
		}
	

	
}

void process_query(int clientfd,MSG *msg,sqlite3 *db)
{
	
		char sql[M]={0};
		char *errmsg;
		char **rep;
		int n_row;
		int n_column;
		int i,j;
		sprintf(sql,"select * from usrinfo where name='%s'",msg->name);
		if(sqlite3_get_table(db,sql,&rep,&n_row,&n_column,&errmsg)!=SQLITE_OK)
		{
				printf("%s\n",errmsg);
				strcpy(msg->text,"Fail");
				send(clientfd,msg,LEN_SMG,0);
				return;
		}
		else
		{
			if(n_row==0)
			{
				
				strcpy(msg->text,"Fail");
				send(clientfd,msg,LEN_SMG,0);
				return;
			}
			else
			{
				
				strcpy(msg->text,"OK");
				send(clientfd,msg,LEN_SMG,0);
			
				for(i=0;i<n_row+1;i++)
				{
					for(j=0;j<n_column;j++)	
					{
						strcpy(msg->text,*rep++);
						send(clientfd,msg,LEN_SMG,0);
					
					}
				}
				strcpy(msg->text,"over");
				send(clientfd,msg,LEN_SMG,0);
				
				return;
			}
		}
	
	

}
void process_query2(int clientfd,MSG *msg,sqlite3 *db)
{
		char sql[M]={0};
		char *errmsg;
		char **rep;
		int n_row;
		int n_column;
		int i,j;
		sprintf(sql,"select * from usrinfo");
		if(sqlite3_get_table(db,sql,&rep,&n_row,&n_column,&errmsg)!=SQLITE_OK)
		{
				printf("%s\n",errmsg);
				strcpy(msg->text,"Fail");
				send(clientfd,msg,LEN_SMG,0);
				return;
		}
		else
		{
			if(n_row==0)
			{
				
				strcpy(msg->text,"Fail");
				send(clientfd,msg,LEN_SMG,0);
				return;
			}
			else
			{
				
				strcpy(msg->text,"OK");
				send(clientfd,msg,LEN_SMG,0);
			
				for(i=0;i<n_row+1;i++)
				{
					for(j=0;j<n_column;j++)	
					{
						strcpy(msg->text,*rep++);
						send(clientfd,msg,LEN_SMG,0);
					
					}
				}
				strcpy(msg->text,"over");
				send(clientfd,msg,LEN_SMG,0);
				
				return;
			}
		}

}



void process_update(int clientfd,MSG *msg,sqlite3 *db)
{
	
	    process_query3(clientfd,msg,db);
	    char sql[M]={0};
		char *errmsg;
		void * arg;   
	
			
		sprintf(sql,"update usrinfo set age = '%d' where staffno='%d'",msg->age,msg->id);
	   
	
	   

		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
		{
				printf("%s\n",errmsg);
				strcpy(msg->text,"Fail");
				send(clientfd,msg,LEN_SMG,0);
				return;
		}
		
	
	
}

void process_insert(int clientfd,MSG *msg,sqlite3 *db)  //管理员添加数据
{
	
	
	 char sql[M]={0};
	 char *errmsg;
	 void * arg;   
	 sprintf(sql,"insert into usrinfo (staffno,name,passwd,age,usertype) values (%d,'%s',%s,%d,%d)",msg->id,msg->name,msg->text,msg->age,msg->usertype);
	
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
		{
				printf("%s\n",errmsg);
				strcpy(msg->text,"Fail");
				send(clientfd,msg,LEN_SMG,0);
				return;
		}
	
	
	
}
void process_delete(int clientfd,MSG *msg,sqlite3 *db)
{
	char sql[M]={0};
	 char *errmsg;
	 void * arg;   
	 sprintf(sql,"delete from usrinfo where staffno = '%d'",msg->id);
	 if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
		{
				printf("%s\n",errmsg);
				strcpy(msg->text,"Fail");
				send(clientfd,msg,LEN_SMG,0);
				return;
		}
	
	
	
}

void insert_history(MSG *msg,sqlite3 *db)
{
		char sql[M]={0};
		char *errmsg;
		char date[64]={0};
		get_time(date);//获得当前的日期
		sprintf(sql,"insert into historyinfo values('%s','%s','%d')",date,msg->name,msg->type);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
		{
			printf("%s\n",errmsg);
			return;
		}
}	
void get_time(const char *date)//获得时间
{
	time_t mytime;
	struct tm *mytm;
	mytime=time(NULL);//得到秒数
	mytm=localtime(&mytime);//得到当前的时间
	sprintf(date,"%04d-%02d-%02d  %02d:%02d:%02d",mytm->tm_year+1900,mytm->tm_mon+1,mytm->tm_mday,\
			mytm->tm_hour,mytm->tm_min,mytm->tm_sec);

}
void process_history(int clientfd,MSG *msg,sqlite3 *db)
{
		char sql[M]={0};
		char *errmsg;
		char **rep;
		int n_row;
		int n_column;
		int i,j;
		sprintf(sql,"select * from historyinfo ");
		if(sqlite3_get_table(db,sql,&rep,&n_row,&n_column,&errmsg)!=SQLITE_OK)
		{
				printf("%s\n",errmsg);
				strcpy(msg->text,"Fail");
				send(clientfd,msg,LEN_SMG,0);
				return;
		}
		else
		{
			if(n_row==0)
			{
				
				strcpy(msg->text,"Fail");
				send(clientfd,msg,LEN_SMG,0);
				return;
			}
			else
			{
				
				strcpy(msg->text,"OK");
				send(clientfd,msg,LEN_SMG,0);
				for(i=0;i<n_row+1;i++)
				{
					for(j=0;j<n_column;j++)	
					{
						strcpy(msg->text,*rep++);
						send(clientfd,msg,LEN_SMG,0);
						usleep(1000);//防止粘包
					}
				}
				strcpy(msg->text,"over");
				send(clientfd,msg,LEN_SMG,0);
				return;
			}
		}
}
