#include<stdio.h>
#include<stdlib.h>
#include<sys/utsname.h>
#include<unistd.h>
#include <netdb.h>
#include<sys/types.h>
#include<string.h>
#include<wait.h>
#include<sys/stat.h>
#include<signal.h>
#include<fcntl.h>

int fg=0;
char home[101];
struct process{
	pid_t pid;
	char name[100];
	int flag;
}phistory[1000];
int plen=0;


void jobs()
{
	int i,j,count=1;
	char a[10],c,b[10];
	for(i=0;i<plen;i++)
	{
		//flag =1 implies the job is running.
		if(phistory[i].flag==1)
		{
			printf("[%d]  %s  [%d]\n",count,phistory[i].name,phistory[i].pid);
			count++;
		}
	}
}

void pinfo(int pid)
{
	int temp=pid,i=0,j;
	char a[100],str[10],arr[1000],str1[10],c,b[100];;
	str[0]=0;
	strcpy(a,"/proc/");
	//get interger to string
	while(temp!=0)
	{
		str[i]=('0'+temp%10);
		temp=temp/10;
		i++;
	}
	for(j=0;j<i;j++)
		str1[j]=str[i-j-1];
	str1[j]='\0';
	strcat(a,str1);
	strcpy(b,a);
	strcat(b,"/exe");
	strcat(a,"/status");
	int f=open(a,O_RDONLY);
	if(f>0)
	{
		j=0;
		//read from the file and store in array.
		while(read(f,&c,1)!=0)
			arr[j++]=c;
		arr[j]='\0';
		char *test;
		//break the array at \n positions.
		test = strtok (arr,"\n");
		i=0;
		while (1)
		{
			if(i==11 || i==1 || i==3)
			{
				if(i==11)
				{
					// to check if the attribute is of size or not.
					if(strncmp(test,"VmSize",6)==0)
						write(1,test,strlen(test));
					else
						write(1,"No memory for this pid",22);
				}
				else
					write(1,test,strlen(test));
				write(1,"\n",1);
			}
			//to get line by line in file
			test= strtok (NULL, "\n");
			if(test==NULL)
				break;
			i++;
		}
		char arrr[101];arrr[100]='\0';
		int ll=readlink(b,arrr,100);
		write(1,"Executable Path: ",16);
		if(ll!=-1)
		write(1,arrr,ll);
		else
			write(1,"Permission denied",17);
		write(1,"\n",1);
	}
	else
		write(2,"No process with the given  pid\n",32);
}

void signal_handler_int(int sig)
{
	if(sig==SIGINT)
	{
		if(fg!=1)
		{write(1,"\n",1);
		print_shellprompt(home);}
	}
	return;
}

void  signal_handler_child(int sig)
{
	int status;
	int z = waitpid(WAIT_ANY,&status,WUNTRACED | WNOHANG);
	int i;
	for(i = 0; i< plen; i++)
	{
		if (phistory[i].pid == z)
		{
			if(WIFEXITED(status))
				printf("\n%s with pid %d exited normally\n",phistory[i].name,phistory[i].pid);
			if(WIFSTOPPED(status))
				printf("\n%s with pid %d is stopped by a signal\n",phistory[i].name,phistory[i].pid);
			if(WIFSIGNALED(status))
				printf("\n%s with pid %d terminated by a signal\n",phistory[i].name,phistory[i].pid);
			phistory[i].flag = 0;
			if(WIFCONTINUED(status))
			{
				printf("\n%s with pid %d resumed by a signal\n",phistory[i].name,phistory[i].pid);
				phistory[i].flag=1;
			}
			print_shellprompt(home);
			break;
		}
	}
	return ;
}

int print_shellprompt(char *hom)
{
	int i,j=0;
	char hostname[1024],*username,cwd[101];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);
	cwd[100]='\0';
	getcwd(cwd,100);
	//substituting the home to ~
	for(i=0;i<strlen(hom);i++)
	{
		if(hom[i]!=cwd[i])
		{
			j=1;
			break;
		}
	}
	if(j==0)
	{
		cwd[0]='~';
		for(i=strlen(hom);i<strlen(cwd);i++)
		{
			cwd[i-strlen(hom)+1]=cwd[i];
		}
			cwd[i-strlen(hom)+1]='\0';
	}
	username=getlogin();
	write(2,"<",1);
	write(2,username,strlen(username));
	write(2,"@",1);
	write(2,hostname,strlen(hostname));
	write(2,":",1);
	write(2,cwd,strlen(cwd));
	write(2,">",1);
	return 0;
}
void sig_handler_tstp(int signum)
{
		return;
}

void sig_handler_kill(int sig)
{
	exit(0);
}

int getpidval(int val)
{
	int i,count=0;
	for(i=0;i<plen;i++)
	{
		if(phistory[i].flag==1)
			count++;
		if(count==val)
			return phistory[i].pid;
	}
	return -1;
}
int main()
{

	int i,j;
	char a,buf[100];
	home[100]='\0';
	getcwd(home,100);
	if(signal(SIGTSTP, sig_handler_tstp)==SIG_ERR)
			printf("\ncan't catch SIGTSTP\n");
	if (signal(SIGINT, signal_handler_int) == SIG_ERR)
			printf("\ncan't catch SIGINT\n");
	signal(SIGQUIT,sig_handler_kill);
	signal(SIGCHLD,signal_handler_child);
	while(1)
	{	i=0;
		print_shellprompt(home);
		//reading the command on the prompt line
		while(1)
		{
			read(1,&a,1);
			if(a=='\n')
			{
				buf[i]='\0';
				break;
			}
			buf[i++]=a;
		}
		char *arr[20],*test,*rest,*ptr=buf;
		//parsing the comand on the basis of tab and space characters.
		test = strtok_r(ptr," \t",&rest);
		i=0;
		if(test!=NULL)
			arr[i++]=test;
		int flag=0,pipecount=0,pipepos[100];
		pipepos[0]=0;
		char *arin[20]={NULL},*arout[20]={NULL},z='\0',*arrin,*arrout;
		ptr=rest;
		while (1)
		{
			test= strtok_r(ptr, " \t",&rest);
			if(test==NULL)
			{
				arr[i]=NULL;
				break;
			}
			if(strcmp(test,"&")!=0)
			{
				//setting the flags if the test=< or > and storing it in z.
				if(strcmp(test,"<")==0)
				{
					if(flag==3)
						flag=4;
					else
						flag=2;
					z='<';
				}
				else if(strcmp(test,">")==0)
				{
					if(flag==2)
						flag=4;
					else
						flag=3;
					z='>';
				}
				else if(strcmp(test,"|")==0)
				{
					//remembering the position of the pipes.
					pipepos[++pipecount]=i;
				}
				else
				{
					if(z!='>' && z!='<')
						arr[i++]=test;
						//storing the file name in arout array at pipecount position if the previous cmd was >
					else if(z=='>')
					{
						arout[pipecount]=test;
						arrout=test;
						z='\0';
					}
						//storing the file name in arin array at pipecount position if the previous cmd was <
					else if(z=='<')
					{
						arin[pipecount]=test;
						arrin=test;
						z='\0';
					}
				}
			}
			else if(strcmp(test,"&")==0)
				flag=1;
			ptr=rest;
		}
		pipepos[pipecount+1]=i;
		if(arr[0]!=NULL)
		{
			
			if(strcmp(arr[0],"quit")==0)
				exit(0);
			else if(pipecount)
			{
				// creating filedescriptors equal to 2*(pipes +1) because taking as even first one takes input from a pipe.
				int fdcount[2*(pipecount+1)],l=0;
				for(i=0;i<2*pipecount+1;i=i+2)
					pipe(fdcount+i);
				int fd1,fd2;
				while(1)
				{
					if(l>pipecount)
						break;
					pid_t p=fork();
					if(arin[l]!=NULL)
					fd1=open(arin[l],O_RDONLY,S_IRWXU);
					if(arout[l]!=NULL)
					fd2=open(arout[l],O_WRONLY | O_CREAT | O_TRUNC ,S_IRWXU);
					if(p==0)
					{
						char *command[20];
						int cnt=0;
						//getting the command btn the pipes and storing it in an array.
						for(i=pipepos[l];i<pipepos[l+1];i++)
						{
							command[cnt++]=arr[i];
						}
						command[cnt]=NULL;
						//in arin the file is present if that comand was given input redirection
						if(arin[l]!=NULL)
						{
							if(fd1<0)
							{
								printf("Wrong input file!!\n");
								break;
							}
							dup2(fd1,0);
						}
						else
						{
							dup2(fdcount[2*l],0);
						}
						//in arin the file is present if that comand was given output redirection.
						if (arout[l]!=NULL)
						{
							if(fd2<0)
							{
								printf("Error opening file!!\n");
								break;
							}	
							dup2(fd2,1);
						}
						// if its the last command and it doesnt hav an output redirection then it shud be written to stdout. else it shud be written in the next pipe.
						else if(l!=pipecount)
						{
							dup2(fdcount[2*l+3],1);
						}
						close(fdcount[2*l+1]);
						if(strcmp(command[0],"jobs")==0)
						{
							jobs();
							exit(0);
						}
						//command pinfo***************
						else if(strcmp(command[0],"pinfo")==0)
						{
							if(command[1]==NULL)
								pinfo(getpid());
							else
							{
								int val=0;
								for(i=0;i<strlen(command[1]);i++)
									val=val*10+command[1][i]-'0';
								pinfo(val);
							}
							exit(0);
						}
						else if(execvp(*command,command)<0)
						{
							perror("Wrong command!!\n");
							exit(1);
						}
					}
					else
					{
						int a;
						//closing in the fd's
						if(arin[l]!=NULL)
							close(fd1);
						else if (arout[l]!=NULL)
							close(fd2);
						close(fdcount[2*l+3]);
						wait(&a);
					}
					l++;
				}
			}
			else if(flag==4)
			{
				// if no pipes and contains both input and output redirection.
				pid_t p;
				p=fork();
				if(p==0)
				{
					int fd1=open(arrin,O_RDONLY,S_IRWXU);
					int fd2=open(arrout,O_WRONLY | O_CREAT | O_TRUNC ,S_IRWXU);
					dup2(fd1,0);
					dup2(fd2,1);
					if(execvp(*arr,arr)<0)
					{
						perror("Wrong command!\n");
						exit(1);
					}
					close(fd1);
					close(fd2);
				}
				else
				{
					int aaa;
					wait(&aaa);
				}
			}
			else if(flag==2)
			{
				//if no pipes and need to redirect only the input
				pid_t p;
				p=fork();
				if(p==0)
				{
					int fd=open(arrin,O_RDONLY,S_IRWXU);
					dup2(fd,0);
					if(execvp(*arr,arr))
					{
						perror("Wrong command!\n");
						exit(1);
					}
					close(fd);
				}
				else
				{
					int aaa;
					wait(&aaa);
				}
			}
			else if(flag==3)
			{
				//if no pipes and need to redirect only the output
				pid_t p;
				p=fork();
				if(p==0)
				{
					int fd=open(arrout,O_WRONLY | O_CREAT,S_IRWXU);
					dup2(fd,1);
					//arr[remout]=NULL;
					if(execvp(*arr,arr))
					{
						perror("wrong command");
						exit(1);
					}
					close(fd);
				}
				else
				{
					int aaa;
					wait(&aaa);
				}
			}
			else if(strcmp(arr[0],"cd")==0)
			{
				char cwd[101],cwd1[101];cwd[100]=cwd1[100]='\0';
				getcwd(cwd,100);
				if(arr[1]==NULL)
					chdir(home);
				else
					chdir(arr[1]);
				if(arr[1]!=NULL && strcmp(getcwd(cwd1,100),cwd)==0)
					write(2,"no such directory\n",18);
			}
			//command kjob*****************
			else if(strcmp(arr[0],"kjob")==0)
			{
				if(arr[1]!=NULL && arr[2]!=NULL)
				{
					int val1=0,val2=0;
					for(i=0;i<strlen(arr[1]);i++)
						val1=val1*10+arr[1][i]-'0';
					val1=getpidval(val1);
					for(i=0;i<strlen(arr[2]);i++)
						val2=val2*10+arr[2][i]-'0';
					if(val1!=-1)
						kill(val1,val2);
					else
						write(1,"INVALID JOB NO.!!\n",27);
				}
				else
					write(1,"Too few arguements to kjob\n",27);
				sleep(0.2);
			}
			//command overkill*************
			else if(strcmp(arr[0],"overkill")==0)
			{
				int val=0;i=1;
				while(1)
				{	
					val=getpidval(i);
					if(val==-1)
						break;
					kill(val,9);
				}
			}
			//command fg******************
			else if(strcmp(arr[0],"fg")==0)
			{
				fg=0;
				if(arr[1]==NULL)
					write(2,"Too few arguements\n",18);
				else
				{
					int val=0,status;
					for(i=0;i<strlen(arr[1]);i++)
						val=val*10+arr[1][i]-'0';
					val=getpidval(val);
					for(i=0;i<plen;i++)
						if(val==phistory[i].pid)
						{
							phistory[i].flag=0;
							break;
						}
					kill(val,SIGCONT);
					printf("%s with pid=%d is running in foreground\n",phistory[i].name,val);
					waitpid(val,&status,WUNTRACED);
					if(WIFSTOPPED(status))
					{
							printf("%s with pid=%d is stopped by signal\n",phistory[i].name,val);
							phistory[i].flag=1;
							fg=1;
					}
					if(WIFEXITED(status))
					{
							printf("%s with pid=%d is terminated by signal\n",phistory[i].name,val);
							phistory[i].flag=0;
							fg=1;

					}
				}
			}
			//command jobs****************
			else if(strcmp(arr[0],"jobs")==0)
					jobs();
			//command pinfo***************
			else if(strcmp(arr[0],"pinfo")==0)
			{
				if(arr[1]==NULL)
					pinfo(getpid());
				else
				{
					int val=0;
					for(i=0;i<strlen(arr[1]);i++)
						val=val*10+arr[1][i]-'0';
					pinfo(val);
				}
			}
			//system commands*************
			else
			{
				pid_t pid;
				pid=fork();
				if(pid<0)
				{
					perror("Child Proccess not created\n");
					_exit(-1);
				}
				else if(pid==0)
				{
					int ret=execvp(*arr,arr);
					if(ret<0)
					{
						perror("couldn't execute\n");
						_exit(1);
					}
					_exit(0);
				}
				else
				{
					phistory[plen].pid = pid;
					phistory[plen++].flag = 0;
					if(arr[1]!=NULL)
					{
						strcat(arr[0],arr[1]);
					}
					if(flag==0)
					{
						fg=1;
						int status;
						strcpy(phistory[plen-1].name,arr[0]);
						waitpid(pid,&status,WUNTRACED);
						if(WIFSTOPPED(status))
						{
							printf("%s with pid=%d is stopped by signal\n",arr[0],pid);
							phistory[plen-1].flag=1;
						}
						if(phistory[plen-1].flag==0)
							phistory[plen-1].flag=-1;
						fg=0;
					}
					else if(flag==1)
					{
						int status;
						strcpy(phistory[plen-1].name,arr[0]);
						phistory[plen-1].flag = 1;// process is alive
						printf("command %s pid %d\n",phistory[plen-1].name,pid);
					//	kill(pid,SIGCONT);
					}
				}
			}
		}
	}
}
