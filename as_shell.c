#include "as_shell.h"


void as_print_help ()
{
    int help_fd = open("help.txt",O_CREAT | O_RDONLY,0644);
    if(help_fd<0)
    {
        perror("Failed to open help.txt !!!\n");
    }

    char* help_ptr = (char*) mmap(NULL,sysconf(_SC_PAGE_SIZE),PROT_READ,MAP_SHARED,help_fd,0);
    if(help_ptr == NULL)
    {
        perror("Failed to get page for help.txt !!!\n");
    }

    printf("\n %s \n",help_ptr);

    munmap(help_ptr,sysconf(_SC_PAGE_SIZE));
    close(help_fd);
}

void as_make_tokens(char** tokens,char* input)
{
    int token_index = 0;
    const char * delimeter = " ";
    char * myptr  = strtok(input,delimeter);
    while(myptr != NULL)
    {
       tokens[token_index]= strdup(myptr);
        ++token_index;
        myptr= strtok(NULL,delimeter);
    } 
}

void as_get_token_count (char* input,int*  token_count)
{
    for(int i = 0; i < LINE_SIZE; ++i)
    {
        if(input[i] == ' ')
        {
            if(input[i+1] != ' ' && input[i+1] != '\0')
             *token_count+=1;
        }

        if(input[i] == '\0')
        {
            break;
        }
    } 
}

 void as_free(char** tokens,int token_count)
{
    for(int i = 0;i<token_count;++i)
    {
        free(tokens[i]);
        tokens[i] = NULL;
    }

    free(tokens);
    tokens = NULL;

}

void  as_print_tokens (char** tokens,int token_count)
{
     for(int i = 0;i<token_count;++i)
    {
        printf("%s\n",tokens[i]);
    }
}

void as_cd (char* PWD)
{
   

    struct Pair* temp = (struct Pair*) malloc(sizeof(struct Pair));
    if(temp == NULL)
    {
        perror("Failed to allocate memory for temporary Pair(as_cd) !!! \n");
    }
     

    int pwd_fd = open("set.txt",O_CREAT | O_RDWR,0664);
    if(pwd_fd < 0)
    {
      perror("Failed open file for setting PWD !!!\n");
    }

    read(pwd_fd,temp,sizeof(struct Pair));
    
    strcat(temp->value,"/");
    strcat(temp->value,PWD);  
    
    int dir_fd = open(temp->value,__O_DIRECTORY | O_RDONLY,0664);
    if(dir_fd < 0 )
    {
        printf("No such file or directory !!!\n");
    }

    else 
    {
       lseek(pwd_fd,SEEK_SET,0);
       write(pwd_fd,temp,sizeof(struct Pair));
       fsync(pwd_fd);
       printf("Entered to => %s\n",temp->value); 

    }

    read(pwd_fd,temp,sizeof(struct Pair));


    free(temp);
    temp = NULL;
    
  
    close(dir_fd);
    close(pwd_fd);
 
 }

 void as_exit()
 {
    kill(getppid(),SIGINT);
    printf("\nSee you later !!!\n");
 }

void as_echo(char** tokens,int token_count)
{
    for(int i = 1; i < token_count; ++i)
    {
        printf("%s ",tokens[i]);
    }

    printf("\n");

}

void as_set_initial_PWD ()
{
    char pwd_key [] = "PWD";
    char pwd_value [] = ".";          
    int pwd_fd = open("set.txt",O_CREAT | O_RDWR,0644);
    if(pwd_fd<0)
      {
        perror("Failed open file for setting initial PWD !!!\n");
      }

    struct Pair pwd_obj;
    strcpy(pwd_obj.key,pwd_key);
    strcpy(pwd_obj.value,pwd_value);
    write(pwd_fd,&pwd_obj,sizeof(pwd_obj));  
   
    close(pwd_fd);
}

void as_set (char* token)
{
    if(strchr(token,'=') != NULL)
    {
      char** set=malloc(2 * sizeof(char*));
      int set_index = 0;
      const char * st_delimeter = "=";
      char * myptr  = strtok(token,st_delimeter);
      while(myptr != NULL)
      {
         set[set_index]= strdup(myptr);
         ++set_index;

         myptr= strtok(NULL,st_delimeter);
      } 

      

      int fd = open("set.txt",O_CREAT | O_RDWR | O_APPEND,0644);
      if(fd<0)
      {
        perror("Failed to open set.txt !!!\n");
      }

     struct Pair obj = {'\0'};
     strcpy(&obj.key[0],set[0]);
     strcpy(&obj.value[0],set[1]);
     write (fd, &obj, sizeof (obj));
         
     close (fd); 
     free(set);
     set= NULL;

    }

     else
    {
        printf("Invalid expression for set !!!/n set key=value !!! \n");
    }
}

void as_history_write (char* input)
{
    int fd = open("command_history.txt",O_CREAT | O_RDWR | O_APPEND,0644);
    if(fd<0)
      {
        perror("Failed to open command_history.txt !!!\n");
      }

    int input_size = strlen(input);
    write(fd,input,input_size);

    close(fd);  
}

void as_history_print()
{
    int fd = open("command_history.txt",O_CREAT | O_RDWR,0644);
    if(fd<0)
      {
        perror("Failed to open command_history.txt !!!\n");
      } 

    char* ad = mmap(NULL,sysconf(_SC_PAGE_SIZE),PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    printf("%s",ad);

    munmap(ad,sysconf(_SC_PAGE_SIZE));
    close(fd);

}

void as_check_variables (char** token,int token_count)
{
    for(int i = 0; i < token_count;++i)
    {
        if(token[i][0] == '$')
        {

            int fd = open("set.txt",O_CREAT | O_RDWR ,0644);
            if(fd<0)
            {
            perror("Failed to open set.txt !!!\n");
            }

            struct Pair* obj = (struct Pair*) malloc(sizeof(struct Pair));
             
            int var_size = strlen(token[i]);
            char var[var_size];
            for(int k = 0,j=1;j<strlen(token[i]);++k,++j)
            {
                var[k] = token[i][j];
            }
            var[var_size - 1] = '\0';

            bool flag = false;

            while(true)
            {
                int count = read(fd,obj,sizeof(struct Pair));
                if(count > 0)
                {
                    if(!strcmp(obj->key,var))
                    {
                        strcpy(token[i],obj->value);
                        flag = true;

                    }
                }

                else
                {
                    break;
                }    
            }    

            if(!flag)
            {
                printf("There are no variable named '%s'\nCan be created by 'set' keyword ",var);
            }
 
            free(obj);
            obj = NULL;
            close(fd);
        }
    }
}

void as_unset (char * token)
{
    int fd = open("set.txt",O_CREAT | O_RDWR ,0644);
    if(fd < 0)
    {
        perror("Failed to open set.txt !!!\n");
    }

    struct Pair* obj = (struct Pair*) malloc(sizeof(struct Pair));

    bool flag = false;

    while(true)
    {
        int count = read(fd,obj,sizeof(struct Pair));
        if(count > 0)
        {
            if(!strcmp(obj->key,token))
            {
                struct Pair ob = {' '};
                lseek(fd,-100,SEEK_CUR);
                write (fd, &ob, sizeof (ob));
                flag = true;

                break;
            }
        }

        else
        {
            break;
        }

    }
    
    if(flag)
    {
        printf("Variable '%s' removed \n",token);
    }

    else
    {
        printf("There is no variable which name is '%s'\n",token);
    }

    free(obj);
    obj = NULL;
    close(fd);
    
}

void as_print_pwd ()
{
   

    struct Pair* temp = (struct Pair*) malloc(sizeof(struct Pair));
    if(temp == NULL)
    {
        perror("Failed to allocate memory for temporary Pair(my_cd) !!! \n");
    }
     

    int pwd_fd = open("set.txt",O_CREAT | O_RDWR,0664);
    if(pwd_fd<0)
    {
      perror("Failed open file for setting PWD !!!\n");
    }

    
    read(pwd_fd,temp,sizeof(struct Pair));

    printf("%s => %s\n",temp->key,temp->value);  


    free(temp);
    temp = NULL;
    close(pwd_fd);
  
}

void as_parser(char **tokens,int token_count)
{
  as_check_variables(tokens,token_count);

  if(!strcmp(tokens[0], "cd"))
  {
    as_cd(tokens[1]);
  }

  else if (!strcmp(tokens[0], "set"))
  { 
    as_set(tokens[1]);
  }

  else if (!strcmp(tokens[0], "unset"))
  { 
    as_unset(tokens[1]);
  }   

  else if(!strcmp(tokens[0], "history"))
  {
    as_history_print();
  }

  else if(!strcmp(tokens[0], "help"))
  {
    as_print_help();
  }

  else if(!strcmp(tokens[0], "pwd"))
  {
    as_print_pwd();
  }

  else if(!strcmp(tokens[0], "exit"))
  {
    as_exit();
  }
 
  else if(!strcmp(tokens[0], "echo"))
  {
    as_echo(tokens,token_count);
  }
  
  else 
  {

    int res = execvp(tokens[0],tokens);

    if(res == -1)
    {
      printf("%s: command not found\n",tokens[0]);
    }
     
  }

}

void start ()
{

          
  as_set_initial_PWD();
  char* greetings = "\nWelcome to as_shell created by A.Saroyan !!!\n\n";
  printf("%s",greetings);
    
  while(true)
  {

    int pid = fork();

    if(pid == 0)
    {
    char* input =(char*) malloc(sizeof(char) * LINE_SIZE);  
    if(input == NULL)
    {
      perror("Failed to create input ");
    }
   
    printf("as_shell=> ");  
    fgets(input,LINE_SIZE,stdin);

    as_history_write(input);

    input[strlen(input) - 1] = '\0';
    
    int token_count =1;  
    as_get_token_count(input,&token_count);

    char** tokens = malloc(token_count * sizeof(char*));
    if(tokens == NULL)
    {
       perror("Failed to allocate memory for tokens(array) !!! \n");
    }


    as_make_tokens(tokens,input);
    as_parser(tokens,token_count);
 
    as_free(tokens,token_count);
    free(input);
    input = NULL;

    exit(0); 

  }


  else 
  {
    wait(NULL);
  }

  }

}