#include <stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#define max_history 100
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
extern char**environ;
int main() {
    char input[1024];
    char*args[100];
    char history[max_history][1024];
    int count=0;
    while(1){
        printf("\033[1;36mmy shell>> \033[0m");
        if(fgets(input,sizeof(input),stdin)==0){
            break;
        }
        input[strcspn(input, "\n")] = '\0';
        if(input[0]=='<'){
            int n=atoi(input+1);
            int index=count-n;
            if(n<=0 || n>count){
                printf("You exceeded command range");
            }
            strcpy(input,history[index]);
        }
        if(strlen(input)>0){
            if(count<max_history){
                strcpy(history[count],input);
                count++;
            }
        }
        char*token=strtok(input," ");
        int i=0;
        while(token!=NULL){
            args[i++]=token;
            token=strtok(NULL," ");
        }
        args[i]=NULL;
        if(args[0]==NULL){
            continue;
        }
        
        else if(strcmp(args[0],"exit")==0){
            printf("GOOD BYEE...\n");
            break;
        }
        else if(strcmp(args[0],"pwd")==0){
            char cwd[1024];
            if(getcwd(cwd,sizeof(cwd))!=NULL){
                printf("%s\n",cwd);
                continue;
            }
            else{
                perror("pwd");
            }
        }
       else if(strcmp(args[0],"cd")==0){
           if(args[1]==NULL){
               printf("usage dictonary\n");
           }
           else if (chdir(args[1])!=0){
               perror("cd");
           }
           continue;
       }
       else if(strcmp(args[0],"env")==0){
           for(int j=0;environ[j]!=NULL;j++){
               printf("%s\n",environ[j]);
           }
           continue;
       }
       else if(strcmp(args[0],"printenv")==0){
           if(args[1]==NULL){
               for(int j=0;environ[j]!=NULL;j++){
                   printf("%s\n",environ[j]);
               }
               continue;
           }
           else{
               char *value=getenv(args[1]);
               if(value!=NULL){
                   printf("%s\n",value);
                   continue;
               }
           }
       }
      else if(strcmp(args[0],"history")==0){
          for(int k=0;k<count;k++){
              printf("%s\n",history[k]);
          }
          continue;
      }
      else if(strcmp(args[0],"calc")==0){
          if(args[1]==NULL){
              printf("Usage: calc [add|sub|mul|avg|max|min]\n");
                continue;
          }
          if(args[2]==NULL){
              printf("please provide numbers\n");
              continue;
          }
          else if(strcmp(args[1],"add")==0){
              int sum=0;
              for(int l=2;args[l]!=NULL;l++){
                  sum=sum+atoi(args[l]);
              }
              printf("%d\n",sum);
          }
          else if(strcmp(args[1],"sub")==0){
              int result=atoi(args[2]);
              for(int i=3;args[i]!=NULL;i++){
                  result=result-atoi(args[i]);
              }
              printf("%d\n",result);
          }
          else if(strcmp(args[1],"multi")==0){
              int multi=1;
              for(int i=2;args[i]!=NULL;i++){
                  multi=multi*atoi(args[i]);
              }
              printf("%d\n",multi);
          }
          else if(strcmp(args[1],"avg")==0){
              int sum=0;
              int count=0;
              for(int i=2;args[i]!=NULL;i++){
                  sum=sum+atoi(args[i]);
                  count++;
              }
              printf("%.4f\n",(float)sum/count);
          }
          else if(strcmp(args[1],"max")==0){
              int max=atoi(args[2]);
              for(int i=3;args[i]!=NULL;i++){
                  if(atoi(args[i])>max){
                      max=atoi(args[i]);
                  }
              }
              printf("%d\n",max);
          }
          else if(strcmp(args[1],"min")==0){
              int min=atoi(args[2]);
              for(int i=3;args[i]!=NULL;i++){
                  if(atoi(args[i])<min){
                      min=atoi(args[i]);
                  }
              }
              printf("%d\n",min);
          }
          
          else{
              printf("unknown calc operator\n");
          }
          continue;
      }
      else if(strcmp(args[0],"help")==0){
              printf("history\n");
              printf("cd\n");
              printf("pwd\n");
              printf("exit\n");
              printf("calc\n");
              printf("news\n");
              printf("env\n");
              printf("printenv\n");
              continue;
          }
      if(strcmp(args[0], "weather") == 0)
        {
            if(args[1] == NULL)
            {
                printf("Usage: weather city\n");
                continue;
            }

            char command[512];
            snprintf(command, sizeof(command), "python3 weather.py \"%s\" 2>/dev/null", args[1]);
            
            if(system(command) != 0)
            {   
                printf("Error: weather.py not found or failed. Make sure python3 and weather.py exist.\n");
            }
            continue;
        }
        if(strcmp(args[0], "news") == 0)
        {
            if(args[1] == NULL)
            {
                printf("Usage: news [world|india|technology|sports]\n");
                continue;
            }

            char command[512];
            snprintf(command, sizeof(command), "python3 news.py %s 2>/dev/null", args[1]);
            
            if(system(command) != 0)
            {
                printf("Error: news.py not found or failed. Make sure python3 and news.py exist.\n");
            }
            continue;
        }
        else{
            pid_t pid=fork();
            if(pid<0){
                perror("fork");
            }
            else if(pid==0){
                for(int i=0;args[i]!=NULL;i++){
                    if(strcmp(args[i],"<")==0){
                        int fd=open(args[i+1],O_RDONLY);
                        if(fd<0){
                            perror("input redirection");
                            exit(1);
                        }
                        dup2(fd,STDIN_FILENO);
                        close(fd);
                        args[i]=NULL;
                    }
                    else if(strcmp(args[i],">")==0){
                        int fd=open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if(fd<0){
                            perror("output redirection");
                            exit(1);
                        }
                        dup2(fd,STDOUT_FILENO);
                        close(fd);
                        args[i]=NULL;
                    }
                    else if(strcmp(args[i],">>")==0){
                        int fd = open(args[i+1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                        dup2(fd,STDOUT_FILENO);
                        close(fd);
                        args[i]=NULL;
                    }
                }
                execvp(args[0],args);
                perror("execvp");
                exit(1);
            }
            else{
                wait(NULL);
            }
        }
        
    }

    return 0;
} 
