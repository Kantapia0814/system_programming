#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>    
#include <sys/types.h>  
#include <sys/stat.h>   
#include <unistd.h>     
#include <fcntl.h>
#include <ctype.h>

#ifndef DEBUG
#define DEBUG 0
#endif

typedef struct wordList{    //list of words and count in a file
    char* word;
    int wordCount;
    struct wordList* next;
} wordList;

typedef struct fileInfo{    //list of files, and their wordLists
    char* name;
    int wordTotal;
    wordList* words;
    struct fileInfo* next;
} fileInfo;

fileInfo* realFileList = NULL;

fileInfo* tempFileList = NULL;

fileInfo* getFileinfo(const char* filename){
    fileInfo* newInfo = malloc(sizeof(fileInfo));
    int len = strlen(filename) + 1;
    char* copy = malloc(len);
    memcpy(copy, filename, len);
    newInfo->name = copy;
    newInfo->wordTotal = 0;
    newInfo->words = NULL;
    newInfo->next = NULL;
    return newInfo; 
}

char * update_extension(const char* filename){
    char *newfilename = malloc(strlen(filename) + 5);
    strcpy(newfilename, filename);
    strcat(newfilename, ".txt");
    return newfilename;
}

char * whyisthisnotafunctioninthestandardclibrarywtf(char * string){
    for(int i = 0; i < strlen(string); i++){
        if(isspace(string[i])){
            //printf("THERE IS A SPACE IN YOUR STRING, BOZO\n");
            return NULL;
        }
        string[i] = tolower(string[i]);
    }
    return string;
}

int has_txt_extension(const char *filename) {
    const char *ext = strrchr(filename, '.');  // Find last occurrence of '.'
    return (ext && strcmp(ext, ".txt") == 0);
}

int is_left_forbidden(char c) {
    return (c == '(' || c == '[' || c == '{' || c == '"' || c == '\'');
}

int is_right_forbidden(char c) {
    return (c == ')' || c == ']' || c == '}' || c == '"' || c == '\'' || c == ',' || c == '.' || c == '!' || c == '?');
}

void countingWord(fileInfo* file, const char* word) {  //adds a particular word to a particular file. if it doesn't exist, create a new root node.
    int size = strlen(word) + 1;
    char* copy = malloc(size);
    memcpy(copy, word, size);

    for (int i = 0; i < size; i++) {
        copy[i] = tolower(copy[i]);
    }

    wordList* getWords = file->words;
    while (getWords != NULL) {
        if (strcmp(getWords->word, copy) == 0) {
            getWords->wordCount++;
            free(copy);
            return;
        }
        getWords = getWords->next;
    }

    wordList* newWords = malloc(sizeof(wordList));
    newWords->word = copy;
    newWords->wordCount = 1;
    newWords->next = file->words;
    file->words = newWords;
    file->wordTotal++;
}

int countEverywords(fileInfo* file) {   //counts the total number of words in all files
    int count = 0;
    fileInfo* current = file;
    while (current != NULL) {
        count += current->wordTotal;
        current = current->next;
    }
    return count;
}

int countEachwords(fileInfo* file, const char* word) { //counts the total number of a word in all files
    int count = 0;
    fileInfo* current = file;
    while (current != NULL) {
        wordList* words = current->words;
        while (words != NULL) {
            if (strcmp(words->word, word) == 0) {
                count = count + words->wordCount;
                break;
            }
            words = words->next;
        }
        current = current->next;
    }
    return count;
}

void printResult(fileInfo* files) {
    int total_num_words = countEverywords(files);
    fileInfo* file = files;
    while (file != NULL) {
        float max_relatvie_freq = 0.0;
        wordList* word_list = file->words;
        wordList* answer = NULL;

        while (word_list != NULL) {
            int word_count_one_file = word_list->wordCount;
            int word_total_one_file = file->wordTotal;
            int word_count_whole_file = countEachwords(files, word_list->word);
            
            float one_file_freq = (float) word_count_one_file / word_total_one_file;
            float whole_file_freq = (float) word_count_whole_file / total_num_words;
            float relative_freq = one_file_freq / whole_file_freq;

            if (max_relatvie_freq < relative_freq) {
                max_relatvie_freq = relative_freq;
                answer = word_list;
            }
            if (max_relatvie_freq == relative_freq) {
                int result = strcmp(word_list->word, answer->word);
                if (result < 0) {
                    max_relatvie_freq = relative_freq;
                    answer = word_list;
                }
            }
            word_list = word_list->next;
        }
        if (answer != NULL) {
            if(has_txt_extension(file->name)){
                char* copy = file->name;
                copy[strlen(copy) - 4] = '\0';
                printf("%s: %s\n", copy, answer->word);
            }
            else{
                printf("%s: %s\n", file->name, answer->word);
            }          
        }
        file = file->next;
    }
}

void readFunction(const char * path, int filelength){
    
    int fd = open(path, O_RDONLY);
    char buf[filelength];
    int pos;
    char *line = NULL;
    int linelen = 0;
    int bytes;
    int segstart, segend, seglen;
    int letterbool = 0;
    while ((bytes = read(fd, buf, filelength)) > 0) {
            if (DEBUG) printf("Read %d bytes\n", bytes);
                segstart = 0;
                while(segstart < bytes && isspace(buf[segstart])){
                    segstart++;
                }
                    for (pos = segstart; pos < bytes; pos++) {
                        if((pos < bytes && isspace(buf[pos])) || (pos == bytes - 1)){ //reached first space or end
                            while(segstart < bytes && is_left_forbidden(buf[segstart])){
                                segstart++;
                            }
                            segend = pos - 1;
                            if(pos == bytes -1 && !isspace(buf[pos])){
                                segend = pos;
                            }
                            while(segend >= segstart && segend >= 0 && is_right_forbidden(buf[segend])){
                                segend--;
                            }
                        if(segend >= segstart && segend < bytes){
                            seglen = segend - segstart + 1;
                            letterbool = 0;
                            for(int i = segstart; i <= segend; i++){
                                if(i < bytes && isalpha(buf[i])){
                                    letterbool = 1;
                                    break;
                                }
                            }
                            if(letterbool){
                            line = realloc(line, seglen + 1);
                            //add is_forbidden logic.
                            memcpy(line, buf + segstart, seglen);
                            line[linelen + seglen] = '\0';
                            // do something with line
                            //printf("%s\n", line);
                            countingWord(tempFileList, line);
                            // clean up line
                            free(line);
                            line = NULL;
                            linelen = 0;
                            // prepare for next line
                            segstart = pos + 1;
                            while(segstart < bytes && isspace(buf[segstart])){
                                segstart++;
                                }
                            pos = segstart - 1;
                            }
                            else{
                            segstart = pos + 1;
                            while(segstart < bytes && isspace(buf[segstart])){
                                segstart++;
                            }
                            pos = segstart - 1;
                        }
                        }
                        else{
                            segstart = pos + 1;
                            while(segstart < bytes && isspace(buf[segstart])){
                                segstart++;
                            }
                            pos = segstart;
                        }
                    }
                }
            }
        }

void F_or_D(const char *path, int depth) {
    struct stat buffer;

    if (stat(path, &buffer) == -1) {
        if(!has_txt_extension(path) && depth == 0){
            char * newPath = update_extension(path);
            if(stat(newPath, &buffer) != -1){
                F_or_D(newPath, 0);
            }
            else{
                perror("stat");
            }
            free(newPath);
            return;
        }
        if(has_txt_extension(path) && depth == 0){
            printf("Invalid file: %s\n", path);
            return;
        }
    }

    else if (S_ISREG(buffer.st_mode) && depth == 0) {
        //printf("Valid file: %s\n", path);
        fileInfo* newFile = getFileinfo(path);    
        tempFileList = newFile;
        readFunction(path, buffer.st_size);
        tempFileList = NULL;    
        newFile->next = realFileList;
        realFileList = newFile;
    } 

    else if(has_txt_extension(path) && depth == 1){
        //printf("Valid file: %s\n", path);
        fileInfo* newFile = getFileinfo(path);
        tempFileList = newFile;
        readFunction(path, buffer.st_size);
        tempFileList = NULL;
        newFile->next = realFileList;
        realFileList = newFile;
    }
    else if(!has_txt_extension(path) && depth == 1){
        printf("Invalid file: %s\n", path);
    }

    else if (S_ISDIR(buffer.st_mode)) {
        //printf("Directory: %s\n", path);

        DIR *dir = opendir(path);
        if (!dir) {
            perror("Cannot open directory");
            return;
        }

        struct dirent *dp;
        while ((dp = readdir(dir)) != NULL) {
            // Ignore "." and ".." to prevent infinite loops
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
                continue;
            }

            // Construct full path
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, dp->d_name);

            // Recursively call F_or_D() only if it's a directory or a .txt file
            struct stat newStat;
            if (stat(full_path, &newStat) == 0) {
                if (S_ISDIR(newStat.st_mode)) {
                    F_or_D(full_path, 1);
                } else if (has_txt_extension(dp->d_name)) {
                    F_or_D(full_path, 1);
                } else {
                    printf("Ignoring non-text file: %s\n", full_path);
                }
            }
        }
        closedir(dir);
    }
}

int main(int argc, char *argv[]){
    if(argc < 2){
        printf("Ew, cringe\n");
        return EXIT_FAILURE;
    }
    for(int i = 1; i < argc; i++){
        F_or_D(argv[i], 0);
    }
    printResult(realFileList);
    
    return EXIT_SUCCESS;
}