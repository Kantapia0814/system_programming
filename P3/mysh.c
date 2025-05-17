#define _POSIX_C_SOURCE 200809L // necessary for line 526 to work (idk why :()
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <dirent.h>
#define NUMBER 3
const char *PATHS[NUMBER] = {"/usr/local/bin/", "/usr/bin/", "/bin/"};

char * wildcard = NULL;
char * wildcardprefix = NULL;
char * wildcardsuffix = NULL;
int wildReplaceBool = 0;

int readFunction(char *line, char **args) {
    int pos;
    int linelen = strlen(line);
    int segstart = 0, segend = 0, seglen = 0;
    int index = 0;
    char *token = NULL;

    while (segstart < linelen && isspace(line[segstart])) {
        segstart++;
    }
    for (pos = segstart; pos <= linelen; pos++) {
        if (pos == linelen || isspace(line[pos])) {
            segend = pos - 1;

            while (segend >= segstart && isspace(line[segend])) {
                segend--;
            }
            if (segend >= segstart) {
                seglen = segend - segstart + 1;
                token = malloc(seglen + 1);
                memcpy(token, line + segstart, seglen);
                token[seglen] = '\0';
                args[index++] = token;
            }
            while (pos < linelen && isspace(line[pos])) pos++;
            segstart = pos;
        }
    }
    args[index] = NULL;
    return index;
}

void makeFree(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        free(args[i]);
    }
}

int changeDirectory(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "cd: No such file or directory\n");
        return 1;
    } 
    if (chdir(args[1]) != 0) {
        perror("cd");
        return 1;
    }
    return 0;
}

int printDirectory(char **args) {
    char cwd[100];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
        return 0;
    } else {
        perror("pwd");
        return 1;
    }
}

int printError(char **args) {
    if (args[1]) {
        fprintf(stderr, "%s\n", args[1]);
    } else {
        fprintf(stderr, "die: command error\n");
    }
    makeFree(args);
    exit(1);
}
 
int printWhich(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "which: command error\n");
        return 1;
    } else {
        int count = 0;
        for (int i = 0; i < NUMBER; i++) {
            char which_is_mine[100];
            strcpy(which_is_mine, PATHS[i]);
            strcat(which_is_mine, args[1]);
            // check excute permission using access function
            if (access(which_is_mine, X_OK) == 0) {
                printf("%s\n", which_is_mine);
                count = 1;
                break;
            }
        }
        if (count != 1) {
            printf("Cannot find %s\n", args[1]);
            return 1;
        }
    }
    return 0;
}

int isbuiltin(char *token){
    if (strcmp(token, "cd") && strcmp(token, "pwd") && strcmp(token, "which") && strcmp(token, "exit") && strcmp(token, "die")) {
        return 0;
    }            
    return 1;   
}
int iswild(char *token){
    int i = 0;
    while(token[i]){
        if(token[i] == '*'){
            return i;
        }
        i++;
    }
    return -1;
}
int hasslash(char *token){
    int i = iswild(token);
    while(i >= 0){
        if(token[i] == '/' ){
            return i;
            }
        i--;
    }
    return -1;
}
char* getWildPrefix(char* wildcard){
    char *tokenprefix = NULL;
    if(hasslash(wildcard) != -1){
        tokenprefix = realloc(tokenprefix, iswild(wildcard) - hasslash(wildcard));
        strncpy(tokenprefix, wildcard + hasslash(wildcard) + 1, iswild(wildcard) - hasslash(wildcard) -1);
        tokenprefix[iswild(wildcard) - hasslash(wildcard) - 1] = '\0';
    }else{
        tokenprefix = realloc(tokenprefix, iswild(wildcard) + 1);
        strncpy(tokenprefix,wildcard, iswild(wildcard));
        tokenprefix[iswild(wildcard)] = '\0';     //////////////// weird bug line
    }
    return tokenprefix;
}
char* getWildSuffix(char* wildcard){
    char *tokensuffix = NULL;
    int wildIndex = iswild(wildcard);
    int suffix_len;
    if(hasslash(wildcard) != -1){
        suffix_len = strlen(wildcard) - wildIndex - 1;
        tokensuffix = realloc(tokensuffix, suffix_len + 1);
        strncpy(tokensuffix, wildcard + wildIndex + 1, suffix_len);
        tokensuffix[suffix_len] = '\0';
    }
    else{
        suffix_len = strlen(wildcard) - wildIndex - 1;
        tokensuffix = realloc(tokensuffix, suffix_len + 1);
        strncpy(tokensuffix, wildcard + wildIndex + 1, suffix_len);
        tokensuffix[suffix_len] = '\0';
    }
    return tokensuffix;
}
void updateargs(char * token, int tokenindex, char** args, int argc){
    if (wildReplaceBool) {
        for (int i = argc; i > tokenindex; i--) {
            args[i] = strdup(args[i - 1]);
        }
        free(args[tokenindex]);
        args[tokenindex] = strdup(token);  
        args[argc + 1] = NULL;
    } else {
        free(args[tokenindex]);
        args[tokenindex] = strdup(token);
        args[argc] = NULL;
        wildReplaceBool = 1;
    }
    /*for (int k = 0; args[k]; k++) {
        printf("args[%d]: %s  , ", k, args[k]);
    }
    printf("\n");*/
}
void processwild(char *wildcardprefix, char * wildcardsuffix, int tokenindex, char** args, int argc){
    char *searchdir = NULL;
    char *prefix = NULL;
    char *suffix = NULL;
    if(hasslash(wildcard) != -1){
        searchdir = realloc(searchdir, strlen(wildcard) + 1);
        strcpy(searchdir, wildcard);
        searchdir[hasslash(wildcard)] = '\0';
        DIR *dir = opendir(searchdir);
        struct dirent *dp;
        while ((dp = readdir(dir)) != NULL) {
            if (!dir) {
                perror("Cannot open directory");
                return;
            }
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
                continue;
            }
            if(strlen(dp->d_name) >= strlen(wildcardprefix) + strlen(wildcardsuffix)){
                if(strlen(wildcardprefix) > 0){  
                    prefix  = realloc(prefix, iswild(wildcard) - hasslash(wildcard));
                    strncpy(prefix, dp->d_name, strlen(wildcardprefix));
                    prefix[iswild(wildcard) - hasslash(wildcard) -1] = '\0';
                } 
                if(strlen(wildcardsuffix) > 0){
                    suffix  = realloc(suffix, strlen(wildcard) - iswild(wildcard));
                    strncpy(suffix, dp->d_name + strlen(dp->d_name) - strlen(wildcardsuffix), strlen(wildcardsuffix));
                    suffix[strlen(wildcardsuffix)] = '\0';
                }    
                if((prefix == NULL && suffix == NULL) || 
                (prefix == NULL && strcmp(wildcardsuffix, suffix) == 0) || 
                (suffix == NULL && strcmp(wildcardprefix, prefix) == 0) || 
                ((prefix != NULL && suffix != NULL) && (strcmp(wildcardprefix, prefix) == 0 && strcmp(wildcardsuffix, suffix) == 0))){
                    //printf("%s\n", dp->d_name);
                    if(wildReplaceBool){
                        updateargs(dp->d_name, tokenindex, args, argc);
                        argc++;
                    }
                    else updateargs(dp->d_name, tokenindex, args, argc);
                }
        }
        if(suffix != NULL) {
            free(suffix); 
        }
        if(prefix != NULL) {
            free(prefix);
        }
        suffix = NULL, prefix = NULL;
    }
    closedir(dir);
    }else{
        char cwd[100];
        if(getcwd(cwd, sizeof(cwd)) != NULL) {
            DIR *dir = opendir(cwd);
            struct dirent *dp;
            while ((dp = readdir(dir)) != NULL) {
                if (!dir) {
                    perror("Cannot open directory");
                    return;
                }
                if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
                    continue;
                }
                if(strlen(dp->d_name) >= strlen(wildcardprefix) + strlen(wildcardsuffix)){
                    if(strlen(wildcardprefix) > 0){
                        prefix  = realloc(prefix, iswild(wildcard) + 1);
                        strncpy(prefix, dp->d_name, strlen(wildcardprefix));
                        prefix[iswild(wildcard)] = '\0';
                    }
                    if(strlen(wildcardsuffix) > 0){
                        suffix  = realloc(suffix, strlen(wildcard) - iswild(wildcard));
                        strncpy(suffix, dp->d_name + strlen(dp->d_name) - strlen(wildcardsuffix), strlen(wildcardsuffix));
                        suffix[strlen(wildcard) - iswild(wildcard) - 1] = '\0';
                        }
                    if((prefix == NULL && suffix == NULL) || 
                    (prefix == NULL && strcmp(wildcardsuffix, suffix) == 0) || 
                    (suffix == NULL && strcmp(wildcardprefix, prefix) == 0) || 
                    ((prefix != NULL && suffix != NULL) && (strcmp(wildcardprefix, prefix) == 0 && strcmp(wildcardsuffix, suffix) == 0))){
                        //printf("%s\n", dp->d_name);
                        if(wildReplaceBool){
                            updateargs(dp->d_name, tokenindex, args, argc);
                            argc++;
                        }
                        else updateargs(dp->d_name, tokenindex, args, argc);
                    }
                }
                if(suffix != NULL) {
                    free(suffix);
                }
                if(prefix != NULL){
                    free(prefix);
                }     
                suffix = NULL, prefix = NULL;
            }
            closedir(dir);
        }
    }
    if(searchdir) free(searchdir);
}

char *slice_command(char *command, int first, int command_len) {
    char *new_command = malloc(command_len + 1);
    if (new_command == NULL) {
        perror("malloc error");
        exit(1);
    }
    memcpy(new_command, &command[first], command_len);
    new_command[command_len] = '\0';
    return new_command;
}

int externalFuction(char **args, int input, int output, int batch_mode) {
    int count = 0;
    char this_is_mine[100];
    for (int i = 0; i < NUMBER; i++) {
        char which_is_mine[100];
        strcpy(which_is_mine, PATHS[i]);
        strcat(which_is_mine, args[0]);
        if (access(which_is_mine, X_OK) == 0) { // Is executable?
            count = 1;
            strcpy(this_is_mine, which_is_mine);
            break;
        }
    }
    if (count != 1) {
        fprintf(stderr, "%s: wrong command. Enter the other command.\n", args[0]);
        return 1;
    }
    pid_t pid = fork();
    if (pid == 0) {
        // close stdin if batch mode
        if (batch_mode) {
            close(STDIN_FILENO);
        }
        // if input file is enabled, connect to standard input
        if (input != -1) {
            dup2(input, STDIN_FILENO);
            close(input);
        }
        // if output file is enabled, connect to standard output
        if (output != -1) {
            dup2(output, STDOUT_FILENO);
            close(output);
        }
        if (execv(this_is_mine, args) == -1) {
            perror("execv error");
            exit(1);
        }
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (status == 0) {
            return 0;
        } else {
            return 1;
        }
    } else {
        perror("fork error");
        return 1;
    }
    return 1;
}

// split line with '|'
int splitPipe(char *line, char *command_box[]) {
    int length = strlen(line);
    int first = 0;
    int cmdIndex = 0;
    for (int i = 0; i <= length; i++) {
        if (line[i] == '|' || line[i] == '\0') {
            int cmdLen = i - first;
            if (cmdLen > 0) {
                command_box[cmdIndex] = slice_command(line, first, cmdLen);
                cmdIndex = cmdIndex + 1; 
            }
            first = i + 1;
        }
    }
    command_box[cmdIndex] = NULL;
    return cmdIndex;
}

int usingPipe(char *line, int batch_mode) {
    char *commands[32];
    int count = splitPipe(line, commands); 
    
    int prev_info = -1;

    for (int i = 0; i < count; i++) {
        int pipe_info[2] = {-1, -1};
        if (i < count) {
            if (pipe(pipe_info) == -1) {
                perror("pipe error");
                return 1;
            }
        }
        pid_t pid = fork();
        // child process
        if (pid == 0) {
            if (batch_mode) {
                close(STDIN_FILENO);
            }
            if (i == 0) {
                if (count > 1) {
                    close(pipe_info[0]);
                    dup2(pipe_info[1], STDOUT_FILENO);
                    close(pipe_info[1]);
                }
            }
            else if (i < count - 1) {
                dup2(prev_info, STDIN_FILENO);
                close(prev_info);
                close(pipe_info[0]);
                dup2(pipe_info[1], STDOUT_FILENO);
                close(pipe_info[1]);
            }
            else if (i == count - 1) {
                if (prev_info != -1) {
                    dup2(prev_info, STDIN_FILENO);
                    close(prev_info);
                }
            }
            char *args[64];
            int argc = readFunction(commands[i], args);
            for (int k = 0; args[k] != NULL; k++) {
                if (iswild(args[k]) != -1) {
                    wildReplaceBool = 0;
                    wildcard = strdup(args[k]);
                    wildcardprefix = getWildPrefix(wildcard);
                    wildcardsuffix = getWildSuffix(wildcard);
                    processwild(wildcardprefix, wildcardsuffix, k, args, argc);
                    free(wildcard); free(wildcardprefix); free(wildcardsuffix);
                    wildcard = NULL; wildcardprefix = NULL; wildcardsuffix = NULL;
                    break;
                }
            }
            char realpath[128];
            int count = 0;

            for (int j = 0; j < NUMBER; j++) {
                snprintf(realpath, sizeof(realpath), "%s%s", PATHS[j], args[0]);
                if (access(realpath, X_OK) == 0) {
                    count = 1;
                    break;
                }
            }
            if (!count) {
                fprintf(stderr, "%s: command not found\n", args[0]);
                exit(1);
            }
            execv(realpath, args);
            perror("execvp error");
            exit(1);
        }
        // parents process
        if (pid > 0) {
            if (i == 0) {
                if (count > 1) {
                    prev_info = pipe_info[0];
                    close(pipe_info[1]);
                }
            }
            else if (i < count - 1) {
                close(prev_info);
                prev_info = pipe_info[0];
                close(pipe_info[1]);
            }
            else if (i == count - 1) {
                if (prev_info != -1) {
                    close(prev_info);
                }
            }
            char *args[64];
            readFunction(commands[i], args);
            makeFree(args);
        } else {
            perror("fork error");
            return 1;
        } 
    }
    for (int i = 0; i < count; i++) {
        wait(NULL);
        free(commands[i]);
    }
    return 0;
}

int checkAndOr(char *line) {
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == '&') {
            if (line[i + 1] == '&') {
                return 1;
            }
        }
        else if (line[i] == '|') {
            if (line[i + 1] == '|') {
                return 2;
            }
        }
    }
    return 0;
}

int supportInternal(char **args) {
    if (strcmp(args[0], "cd") == 0) {
        return changeDirectory(args);
    } 
    else if (strcmp(args[0], "pwd") == 0) {
        return printDirectory(args);
    } 
    else if (strcmp(args[0], "die") == 0) {
        printError(args);
    } 
    else if (strcmp(args[0], "which") == 0) {
        return printWhich(args);
    }
    return -1;
}

int main(int argc, char *argv[]) {
    int interact_mode = 0;
    FILE *input = NULL;

    if (argc > 1) {
        input = fopen(argv[1], "r");
    } else {
        if (isatty(STDIN_FILENO)) {
            interact_mode = 1;
        }
    }
    if (interact_mode) {
        printf("Welcome to my shell!\n");
    }

    FILE *ronaldo;

    if (interact_mode == 1) {
        ronaldo = stdin;
    } else {
        ronaldo = input;
    }

    while (1) {
        if (interact_mode == 1) {
            printf("mysh> ");
            fflush(stdout);
        }
    
        char line[1024];
        int index = 0;
        char alpha;

        while (read(fileno(ronaldo), &alpha, 1) > 0) {
            if (alpha == '\n') {
                break;
            }
            line[index] = alpha;
            index = index + 1;
        }
        line[index] = '\0';

        if (index == 0) {
            break;
        }
    
        line[strcspn(line, "#")] = '\0';
        line[strcspn(line, "\n")] = '\0';
        
        if (line[0] == '\0') {
            continue;
        }        

        int andOrType = checkAndOr(line);
        if (andOrType == 1) {
            char *args[64];
            char *command1[64];
            char *command2[64];
            int argc = readFunction(line, args);
            for (int i = 0; i < argc; i++) {
                if (strcmp(args[i], "&&") == 0) {
                    break;
                }
            }
            int cmd1_len = strstr(line, "&&") - line;
            int cmd2_len = strlen(line) - (cmd1_len + 2);
            char *command1_line = slice_command(line, 0, cmd1_len);
            char *command2_line = slice_command(line, cmd1_len + 2, cmd2_len);
            readFunction(command1_line, command1);
            readFunction(command2_line, command2);
            
            int firstIn = supportInternal(command1);
            if (firstIn == -1) {
                firstIn = externalFuction(command1, -1, -1, !interact_mode);
            }

            if (firstIn == 0) {
                int result = supportInternal(command2);
                if (result == -1) {
                    externalFuction(command2, -1, -1, !interact_mode);
                }
            }
            makeFree(args);
            free(command1_line);
            free(command2_line);
            makeFree(command1);
            makeFree(command2);
            continue;
        }

        else if (andOrType == 2) {
            char *args[64];
            char *command1[64];
            char *command2[64];
            int argc = readFunction(line, args);
            for (int i = 0; i < argc; i++) {
                if (strcmp(args[i], "||") == 0) {
                    break;
                }
            }
            int cmd1_len = strstr(line, "||") - line;
            int cmd2_len = strlen(line) - (cmd1_len + 2);
            char *command1_line = slice_command(line, 0, cmd1_len);
            char *command2_line = slice_command(line, cmd1_len + 2, cmd2_len);
            readFunction(command1_line, command1);
            readFunction(command2_line, command2);

            int firstIn = supportInternal(command1);
            if (firstIn == -1) {
                firstIn = externalFuction(command1, -1, -1, !interact_mode);
            }

            if (firstIn != 0) {
                int result = supportInternal(command2);
                if (result == -1) {
                    externalFuction(command2, -1, -1, !interact_mode);
                }
            }
            makeFree(args);
            free(command1_line);
            free(command2_line);
            makeFree(command1);
            makeFree(command2);
            continue;
        }
        else if (strchr(line, '|') != NULL) {
            usingPipe(line, !interact_mode);
            continue;
        }

        char *args[64];
        int argc = readFunction(line, args);

        if (argc == 0) {
            continue;
        }
    
        /*for (int j = 0; j < argc; j++) {
            printf("  args[%d] = %s\n", j, args[j]);
        }*/

        int input = -1;
        int output = -1;
        int k = 0;
        while (args[k] != NULL) {
            if(iswild(args[k]) != -1){
                wildReplaceBool = 0;
                wildcard = realloc(wildcard, strlen(args[k]) + 1);
                strcpy(wildcard, args[k]); 
                wildcardprefix = realloc(wildcardprefix, strlen(getWildPrefix(wildcard)) + 1);
                strcpy(wildcardprefix, getWildPrefix(wildcard));
                wildcardsuffix = realloc(wildcardsuffix, strlen(getWildSuffix(wildcard)) + 1);
                strcpy(wildcardsuffix, getWildSuffix(wildcard));
                int c = k;
                processwild(wildcardprefix, wildcardsuffix, c, args, argc); 
                free(wildcard); free(wildcardprefix); free(wildcardsuffix);
                wildcard = NULL; wildcardprefix = NULL; wildcardsuffix = NULL;
                break;
            }
            if (strcmp(args[k], "<") == 0) {
                args[k] = NULL;
                input = open(args[k + 1], O_RDONLY);
                if (input < 0) {
                    perror("input redirection error");
                    makeFree(args);
                    return 1;
                }
            }
            else if (strcmp(args[k], ">") == 0) {
                args[k] = NULL;
                output = open(args[k + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (output < 0) {
                    perror("output redirection error");
                    makeFree(args);
                    return 1;
                }
            }
            k++;
        }

        if (strcmp(args[0], "cd") == 0) {
            changeDirectory(args);
            makeFree(args);
            continue;
        }
        else if (strcmp(args[0], "pwd") == 0) {
            printDirectory(args);
            makeFree(args);
            continue;
        }
        else if (strcmp(args[0], "die") == 0) {
            printError(args);
            continue;
        }
        else if (strcmp(args[0], "which") == 0) {
            printWhich(args);
            makeFree(args);
            continue;
        }
        else if (strcmp(args[0], "exit") == 0) {
            makeFree(args);
            break;
        } else {
            externalFuction(args, input, output, !interact_mode);
            makeFree(args);
        }
    }

    if (interact_mode == 1) {
        printf("Exiting my shell.\n");
    }

    if (input) {
        fclose(input);
    }

    return 0;
}
