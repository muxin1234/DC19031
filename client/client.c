#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define N 32
#define M 128
#define R 0x1 //管理员模式
#define L 0x2 //普通用户模式
#define Q 0x3  //管理员按人名查找
#define S 0x4 //管理员查找所有
#define X 0x7  //管理员修改库
#define T 0x8  //管理员添加数据
#define D 0x9   //管理删除用户

#define H 0x5  //管理员查看历史记录
#define E 0x6 //退出




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
int do_register(int sockfd,MSG *msg);
int do_register1(int sockfd,MSG *msg);
void do_query(int sockfd,MSG *msg);
void do_query2(int sockfd,MSG *msg);
void do_query3(int sockfd,MSG *msg);
void do_updata(int sockfd,MSG *msg);
void do_history(int sockfd,MSG *msg);
int main(int argc, const char *argv[])
{
	int p;
	int sockfd;
	struct sockaddr_in serveraddr;
	socklen_t len=sizeof(SA);
	int cmd;
	char clean[M]={0};
	MSG msg;
	if(argc!=3)
	{
		printf("User:%s <IP> <port>\n",argv[0]);
		return -1;
	}

	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		err_log("fail to socket");
	}
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr=inet_addr(argv[1]);

	if(connect(sockfd,(SA*)&serveraddr,len)<0)// 客户端执行马上连接单词服务器
	{
		err_log("fail to connect");
	}

BEF:
	while(1)//一级的界面
	{
		 puts("**********************************************");	
		 puts("1.管理员模式       2.普通用户模式     3.退出");
		 puts("**********************************************");
		 puts("请输入您的选择（数字）>>>");
		  if(scanf("%d",&cmd)!=1)// 检测输入的格式对否
		 {
				puts("input error");
				fgets(clean,M,stdin);
				continue;
		 }
		switch(cmd)//根据不同的输入命令执行不同的行为
		 {
			case 1:
				if(do_register(sockfd,&msg)==1)
				{
					puts("欢迎管理员登陆");
					goto GUANLI;
					
				}
				else
				{
					
					break;
				}
				
			case 2:
				if(do_register1(sockfd,&msg)==1)
				{
					puts("欢迎员工登陆");
					goto PUTONG;
					break;
				}
				else
				{
					
					break;
				}
			
			case 3:
					goto EXIT;
			default:
				puts("input error");
				break;

		 }
	}


EXIT:  
	msg.type=E;
	send(sockfd,&msg,LEN_SMG,0);
	close(sockfd);  //客户端退出的处理   
	exit(0);



GUANLI:
   while(1)//管理员一级界面
   {
	puts("**********************************************");
	puts("1：查询  2：修改 3：添加用户  4：删除用户  5：查询历史记录 6：退出");
	puts("**********************************************");
	puts("请输入您的选择（数字）>>>");
	if(scanf("%d",&cmd)!=1)
		 {
				puts("input error");
				fgets(clean,M,stdin);
				continue;
		 }
		 switch(cmd)
		 {
		    case 1:
			goto GUANLI2;
                
				 break;
			case 2:
				goto GUANLI3;  //管理员修改库
			
				break;
			case 3:
				goto GUANLI4;    //管理员添加用户
				 break;
			case 4:
				goto  GUANLI5;   //管理删除用户
				 break;
				 
			case 5:     //管理员查看历史记录
				 do_history(sockfd,&msg);
				 break;
			case 6:
				goto BEF;
				break;
			default:
				puts("cmd error");
				break;
		 }
	 }
	 
	 
GUANLI2:
	while(1)//管理员二级界面
   {
	puts("**********************************************");
	puts("1：按人名查找  	2：查找所有 	3：退出");
	puts("**********************************************");
	puts("请输入您的选择（数字）>>>");
	if(scanf("%d",&cmd)!=1)
		 {
				puts("input error");
				fgets(clean,M,stdin);
				continue;
		 }
		 switch(cmd)
		 {
		    case 1:
                 do_query(sockfd,&msg);
				 break;
			case 2:
				 do_query2(sockfd,&msg);
				break;
			case 3:
				goto BEF;
				break;
			default:
				puts("cmd error");
				break;
		 }


   }

GUANLI3:  //管理员修改库
	msg.type=X;
	while(1)
	{
		puts("请输入您要修改的工号(必须是工号)：");
		scanf("%d",&msg.id);	
		send(sockfd,&msg,LEN_SMG,0);
		recv(sockfd,&msg,LEN_SMG,0);
		if(strncmp(msg.text,"OK",2)==0)//服务器查到了
		{		
			
		   while(1)
		   {
		    puts("**********************************************");
			puts("1：年龄 	2：退出");
			puts("**********************************************");
			puts("请输入您的选择（数字）>>>");
			
			if(scanf("%d",&cmd)!=1)
			{
				puts("input error");
				fgets(clean,M,stdin);
				continue;
			}
			switch(cmd)
			{
		   
			case 1:
				puts("请输入年龄：");
				 scanf("%d",&msg.age);
				 send(sockfd,&msg,LEN_SMG,0);
				 puts("数据库修改成功!修改结束.");
				break;
			case 2:
				goto GUANLI;
				break;
			
			
			}
			
			}		
			
				
	
		}
	}

GUANLI4: //管理员添加用户
	
	msg.type=T;
	while(1)
	{
		puts("***************热烈欢迎新员工***************");
		puts("请输入工号");
		scanf("%d",&msg.id);
		puts("请输入姓名");
		scanf("%s",msg.name);
		puts("请输入密码");
		scanf("%s",msg.text);
		puts("请输入年龄");
		scanf("%d",&msg.age);
		puts("是否为管理员：(0.是 1.否)?");
		scanf("%d",&msg.usertype);
		
		send(sockfd,&msg,LEN_SMG,0);
		 puts("数据库修改成功!是否继续添加:(1.是 2.否)");
		 scanf("%d",&p);
		 if(p == 1)
		 {
			 continue;
			 
		 }
		 else
		 {
			 goto GUANLI;
			 break;
		 }
		 
		 
	}	

GUANLI5://管理员删除用户
	msg.type=D;
	while(1)
	{
		
		puts("请输入要删除的用户工号：");
		scanf("%d",&msg.id);
		puts("请输入要删除的用户名:");
		scanf("%s",msg.name);   
		send(sockfd,&msg,LEN_SMG,0);
		printf("数据库修改成功!删除工号为：%d 的用户",msg.id);
		 puts("数据库修改成功!是否继续删除(1.是 2.否)");
		 scanf("%d",&p);
		 if(p == 1)
		 {
			 continue;
			 
		 }
		 else
		 {
			 goto GUANLI;
			 break;
		 }
		 
		
		
	}
		
PUTONG:
	while(1)//普通一级界面
   {
	   
	puts("**********************************************");
	puts("1：查询  	2：修改 	3：退出");
	puts("**********************************************");
	puts("请输入您的选择（数字）>>>");
	if(scanf("%d",&cmd)!=1)		
	 {
				puts("input error");
				fgets(clean,M,stdin);
				continue;
	 }
		 switch(cmd)
		 {
		    case 1:
			    do_query3(sockfd,&msg);          //普通查询
                
				 break;
			case 2:
				goto PUTONG1;                    //普通修改
			
				break;
			case 3:
				goto BEF;
				break;
			default:
				puts("cmd error");
				break;
		 }



   
   
   
   
   }	

PUTONG1:
	msg.type=X;
	while(1)
	{
		puts("请输入您要修改的工号(必须是工号)：");
		scanf("%d",&msg.id);	
		send(sockfd,&msg,LEN_SMG,0);
		recv(sockfd,&msg,LEN_SMG,0);
		if(strncmp(msg.text,"OK",2)==0)//服务器查到了
		{		
			
		   while(1)
		   {
		    puts("**********************************************");
			puts("  1：年龄  2：退出");
			puts("**********************************************");
			puts("请输入您的选择（数字）>>>");
			
			if(scanf("%d",&cmd)!=1)
			{
				puts("input error");
				fgets(clean,M,stdin);
				continue;
			}
			switch(cmd)
			{
		    case 1:
                 puts("请输入年龄：");
				 scanf("%d",&msg.age);
				 send(sockfd,&msg,LEN_SMG,0);
				 puts("数据库修改成功!修改结束.");
				 
			
			case 2:
				goto PUTONG;
				break;
			
			
			}
			
			}		
			
				
	
		}
	}




 
	return 0;

	
}



int do_register(int sockfd,MSG *msg)  //管理员
{
	msg->type=R;
	puts("请输入用户名");
	scanf("%s",msg->name);
	puts("请输入密码>>>");
	scanf("%s",msg->text);

	send(sockfd,msg,LEN_SMG,0);//用户名密码发给服务器
	recv(sockfd,msg,LEN_SMG,0); //等待服务器回应
	if(strncmp(msg->text,"OK",2)==0)
	{
		 puts("亲爱的管理员，欢迎您登陆员工管理系统！");
		 return 1;
	}
	else
	{
		puts("登陆失败");
		return -1;
	}
		
}

int do_register1(int sockfd,MSG *msg)  //普通用户
{
	msg->type=L;
	puts("请输入用户名");
	scanf("%s",msg->name);
	puts("请输入密码>>>");
	scanf("%s",msg->text);

	send(sockfd,msg,LEN_SMG,0);//用户名密码发给服务器
	recv(sockfd,msg,LEN_SMG,0); //等待服务器回应
	if(strncmp(msg->text,"OK",2)==0)
	{
		 puts("亲爱的员工，欢迎您登陆员工管理系统！");
		 return 1;
	}
	else
	{
		puts("登陆失败");
		return -1;
	}
		
}


void do_query(int sockfd,MSG *msg)
{
	msg->type=Q;
		int i=0;
	while(1)
	{
		puts("请输入您要查找的用户名：");
		scanf("%s",msg->name);
		if(strncmp(msg->name,"##",2)==0)
		{
			break;
		}
		send(sockfd,msg,LEN_SMG,0);
		recv(sockfd,msg,LEN_SMG,0);
		if(strncmp(msg->text,"OK",2)==0)//服务器查到了
		{
		   while(1)
			{
			recv(sockfd,msg,LEN_SMG,0);//等待服务器发送的历史记录条目
			i++;
			if(strncmp(msg->text,"over",4)==0)//记录结束跳出循环
			{
				break;
			}

			printf("%-15s",msg->text);
			if(i%5==0)//每5个换行
				putchar(10);
			}
			puts("------------------------------------");
			puts("Query ok!");
			return;

		}
		else
		{
				
					puts("Query fail!");
					return;
		}

			
	}

}

void do_query2(int sockfd,MSG *msg)
{
	
	msg->type=S;
	int i=0;
	while(1)
	{
	
		send(sockfd,msg,LEN_SMG,0);
		recv(sockfd,msg,LEN_SMG,0);
		if(strncmp(msg->text,"OK",2)==0)//服务器查到了
		{
		   while(1)
			{
			recv(sockfd,msg,LEN_SMG,0);//等待服务器发送的历史记录条目
			i++;
			if(strncmp(msg->text,"over",4)==0)//记录结束跳出循环
			{
				break;
			}

			printf("%-15s",msg->text);
			if(i%5==0)//每5个换行
				putchar(10);
			}
			puts("------------------------------------");
			puts("Query ok!");
			return;

		}
		else
		{
				
					puts("Query fail!");
					return;
		}

			
	}
	
	
	
}

void do_query3(int sockfd,MSG *msg)
{
	msg->type=Q;
	int i=0;
	while(1)
	{
		
		send(sockfd,msg,LEN_SMG,0);
		recv(sockfd,msg,LEN_SMG,0);
		if(strncmp(msg->text,"OK",2)==0)//服务器查到了
		{
		   while(1)
			{
			recv(sockfd,msg,LEN_SMG,0);//等待服务器发送的历史记录条目
			i++;
			if(strncmp(msg->text,"over",4)==0)//记录结束跳出循环
			{
				break;
			}

			printf("%-15s",msg->text);
			if(i%5==0)//每5个换行
				putchar(10);
			}
			puts("------------------------------------");
			puts("Query ok!");
			return;

		}
		else
		{
				
					puts("Query fail!");
					return;
		}

			
	}
	
	
	
}


void do_history(int sockfd,MSG *msg)
{
	msg->type=H;
	int i=0;
	send(sockfd,msg,LEN_SMG,0);
	recv(sockfd,msg,LEN_SMG,0);
	if(strncmp(msg->text,"OK",2)==0)
	{
			
		while(1)
		{
			recv(sockfd,msg,LEN_SMG,0);//等待服务器发送的历史记录条目
			i++;
			if(strncmp(msg->text,"over",4)==0)//记录结束跳出循环
			{
				break;
			}

			printf("%-15s",msg->text);
			if(i%3==0)//每三个换行
				putchar(10);
		}
		puts("------------------------------------");
		puts("History ok!");
		return;
	}
	else
	{
		puts("History fail!");	
		return;
	}
}
