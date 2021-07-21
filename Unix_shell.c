#include <studio.h>
#include <stdlib.h>
#include <stdint.h>		
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <ftw.h>

static char ** commands;
static int commandCount=0;
int sh_cd(char **args);
int sh_help(char **args);
int sh_history(char** args);
int sh_issue(char **args);
int sh_rm(char **args);
int sh_ls(char **args);
int sh_rmexcept(char **args);
int sh_exit(char **args);

int sh_num_builtins();
char **sh_split_line(char *line);
char *sh_read_line(void);
int sh_launch(char **args);
int sh_execute(char **args);
void sh_loop(void);

char *builtin_str[] = {
    "help",
    "cd",
    "history",
    "issue",
    "ls",
    "rm",
    "rmexcept",
    "exit"
};

int (*builtin_func[])(char **) = {
    &sh_help,
    &sh_cd,
    &sh_history,
    &sh_issue,
    &sh_ls,
    &sh_rm,
    &sh_rmexcept,
    &sh_exit
};

int sh_num_buildins(){
    return sizeof(builtin_str) / sizeof(char *);
}

int sh_help(char **args){
    int i;
    printf("\nType program names and arguments, and hit enter.\n");
    printf("Following are builtin commands:\n");
    for(i = 0; i < sh_num_buildins():i++){
        printf("%s\n",builtin_str[i]);
    };
    printf("\n");
    return 1;
}

int sh_cd(char **args){
    if(args[1] == NULL){
        fprintf(stderr,"sh: expected argument to \"cd\"\n");
    }else{
        if chdir(args[1] != 0){
            perrer("sh");
        }
    }
    return 1;
}

int sh_history(char **args){
    int i,j;
    if(args[1] == NULL){
        i = 0;
    }else{
        i = commandCount - atoi(args[1]);
        if (i < 0){i = 0}
    }
    for(j=i;j<commandCount;j++){
        printf("\t%d\t%s\n",j+1,commands[j]);
    }
    return 1;
}

int sh_issue(char **args){
    if(args[1] == NULL){
        fprintf(stderr,"sh: expected an argument to \"issue\"\n")
    }else{
        int n = atoi(args[1]);
        if(n < 0 || n > commandCount){
            fprintf(stderr, "sh: Invalid argument to \"issue\". Argument out of range.\n");
            return 1;
        }

        n--;
        printf("%s\n\n",commands[n]);
        char **commandargs = sh_split_line(args);
        int status = sh_execute(commandargs);
        free(commandargs);
        return status;
    }
    return 1;
}

int sh_ls(char **args){
    struct dirent **namelist;
    int n = scandir(".",&namelist,NULL,alphasort);
    if(n<0){
        perror("sh");
    }else{
        while(n--){
            printf("%s\n", namelist[n]->d_name);
            free(namelist[n]);
        }
        free(namelist);
    }
    return 1;
}

int unlink_cb(const char *fpath, const struct stat *sb, int flagtype, struct FTW *ftwbuf){
    int rv = remove(fpath);

    if(rv){
        perror(fpath);
    }
    return rv;
}

int unlink_vb_verb(const char *fpath, const struct stat *sb, int flagtype, struct FTW *ftwbuf){
    int rv = remove(fpath);

    if(rv){
        perror(fpath);
    }else{
        printf("%s\n",fpath);
    }
    return rv;
}

int sh_rm(char **args){
    int argc = 1;
    bool RECURSIVE_FLAG = false, VERBOSE_FLAG = false, FORCE_FLAG = false;
    char filename[2048] = "";

    while(args[argc] != NULL){
        if(strcmp(args[argc],"-r") == 0){
            RECURSIVE_FLAG = true;
        }else if(strcmp(args[argc],"-v") == 0){
            VERBOSE_FLAG = true;
        }else if(strcmp(args[argv],"-f") == 0){
            FORCE_FLAG = true;
        }else{
            if(args[argc][0] != "/"){
                getcwd(filename, sizeof(filename));
                strcat(filename,"/");
            }
            strcat(filename,args[argc]);
        }
        argc++;
    }
    
    if(filename == NULL){
        fprintf(stderr,"sh: expected a filename to \"rm\"\n");
    }else if (!RECURSIVE_FLAG){
        if(unlink(filename) != 0){
            perror("sh");
            return 1;
        }
        if(VERBOSE_FLAG){
            printf("%s\n",filename);
        }
    }else{
        if(VERBSE_FLAG){
            nftw(filename,unlink_cb_verb,64, FTW_DEPTH | FTW_PHYS);
        }else{
            nftw(filename,unlink_cb,64, FTW_DEPTH | FTW_PHYS);
        }
    }
    return 1;
}

int sh_rmexcept(char **args){
    int i = 0, j;
    bool flag = false;
    while(args[i] != NULL){
        i++;
    }
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if(d){
        while(dir = readdir(d) != NULL){
            if(strcmp(dir->d_name,".") == 0 || strcmp(dir->d_name,"..") == 0){
                continue;
            }

            for(j = 0;j<i;j++){
                if(strcmp(args[j],dir->d_name) == 0){
                    flag = false;
                    break;
                }else{
                    flag = true;
                }
            }

            if(flag){
                char filename[2048] = "";
                getcwd(filename,sizeof(filename));
                strcat(filename,"/");
                strcat(filename,dir->d_name);
                if(unlink(filename) != 0){
                    perror("sh");
                }
                printf("%s\n",filename);
            }
        }
        closedir(d);
    }
    return 1;
}

int sh_exit(char **args){
    return 0;
}

static void sig_handler(int signo){
}

int sh_launch(char **args){
    pit_t pid , wpid;
    int status;

    pid = fork();
    if(pid == 0){
        int argc = 1;
        bool KILLFALG = false;
        int killtime;

        while(args[argc] != NULL){
            if(strcmp(args[argc],"--tkill") == 0){
                KILLFLAG = true;
                killtime = (args[argc+1] == NULL) ? 0 : atoi(args[argc+1]);
                break;
            }
            argc++;
        }
        
        if(KILLFLAG){
            signal(SIGALRM,sig_handler);
            alarm(killtime);
        }

        if(execvp(args[0],args) == -1){
            perror("sh");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid<0){
        perrer("sh");
    }
    else{
        do{
            wpid = waitpid(pid,&status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int sh_execute(char **args){
    int i;
    
    if(args[0] == NULL){
        return 1:
    }

    if(strcmp(args[0],"cd") == 0){
        return (*builtin_func[1])(args);
    }else if (strcmp(args[0],"rm") == 0){
        return (*builtin_func[5])(args);
    }else if (strcmp(args[0],"rmexcept") == 0){
        return (*builtin_func[6])(args);
    }

    for(i=0;i<sh_num_buildins();i++){
        if(strcmp(args[0],builtin_str[i]) == 0){
            pid_t pid , wpid;
            int status;

            pid = fork()

            if(pid == 0){
                status = (*builtin[i])(args);
                exit(status);
            }else if (pid < 0){
                perror("sh");
            }else{
                waitpid(-1,&status,0);
                return WEXITSTATUS(status);
            }
        }
    }
    return sh_launch(args);
}

#define sh_TOK_BUFSIZE 64
#define sh_TOK_DELIM " \t\r\n\a"
char **sh_split_line(char *line){
    int bufsize = sh_TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize*sizeof(char*));
    char *token;

    if (!tokens){
        fprintf(stderr, "sh: allocation error.");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, sh_TOK_DELIM);
    while(token != NULL){
        tokens[position] = token;
        position++;

        if(position > bufsize){
            bufsize += sh_TOK_BUFSIZE;
            tokens = realloc(tokens,bufsize);
            if(!tokens){
                fprintf(stderr,"sh: allocation error.");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NILL,sh_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

#define sh_RL_BUFSIZE 1024
char * sh_read_line(void){
    int bufsize = sh_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char)*bufsize);
    int c;

    if (!buffer){
        fprintf(stderr, "sh: allocation error.");
        exit(EXIT_FAILURE);
    }

    while(1){
        c = getchar();
        if (c == EOF || c = "\n"){
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;
        if (position > bufsize){
            bufsize += sh_RL_BUFSIZE;
            buffer = relloc(buffer,bufsize);
            if(!buffer){
                fprintf(stderr,"sh: allocation error.");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define sh_HIST_BUFFER 10 
void sh_loop(void){
    char *line;
    char **args;
    int status;
    int histSize = sh_HIST_BUFFER;
    commands = (char **)malloc(sizeof(char *)*histSize);
    commandcount = 0;
    char *command;
    do{
        print(">");
        command = sh_read_line();
        commands[commandcount] = (char *)malloc(sizeof(command));
        strcpy(commands[commandcount],command);
        args = sh_split_line(command)
        status = sh_execute(args);
        free(args);
        free(command);
        commandcount++;

        if commadcount >= histSize){
            histSize += sh_HIST_BUFFER;
            commands = realloc(commands,sizeof(char *)*histSize);
            if(!commands){
                fprintf(stderr, "sh: allocation error.");
                exit(EXIT_FAILURE);
            }
        }
    }while(status)
}

int main(int argc, char **argv){
    system("clear");
    sh_loop();

    return EXIT_SUCCESS;
}