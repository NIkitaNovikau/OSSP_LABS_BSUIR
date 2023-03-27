#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

unsigned directory_browsing( char *introducedDir, char *option, int fl )
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char pathName[PATH_MAX + 1];
    dir = opendir(introducedDir);//открываем поток каталога, и возвращает указатель на него.Поток устанавливается на первой записи в каталоге
    if( dir == NULL ) {
    	printf( "Error opening %s: %s", introducedDir, strerror(errno));//strerror печатает ошибку,в errno находится набор ошибок
    	return 0;
    }
	entry = readdir(dir);
	while(entry != NULL) {
    	struct stat entryInfo;
    	if((strncmp(entry->d_name, ".", PATH_MAX) == 0) || (strncmp(entry->d_name, "..", PATH_MAX) == 0)) {//если строки равны то 
        	entry = readdir(dir);
        	continue;//условие if пока не вернется 1
    	}
	    (void)strncpy(pathName, introducedDir, PATH_MAX);//в pathname записали изначальную директорию откуда компилилась лаба
        (void)strncat(pathName, "/", PATH_MAX);//добавили "/" в pathName
        (void)strncat(pathName, entry->d_name, PATH_MAX);
        if(lstat(pathName, &entryInfo) == 0) {//возвращают данные о файле в буфер entryInfo(возвращает структуру stat)
            if(S_ISDIR(entryInfo.st_mode)) {//st_mode-тип файла и режим доступа,тут проверяется является ли файл каталогом
			    if(strstr(option, "d") != NULL) {            
            		printf("\t-d %s\n", pathName);
			    }
                directory_browsing(pathName, option, fl);
            } 
		    else if(S_ISREG(entryInfo.st_mode)) { //тут проверяется является ли файл обычным файлом
			    if(strstr(option, "f") != NULL) {       //поиск входит ли f в option     
            		printf("\t-f %s has %lld bytes\n", pathName, (long long)entryInfo.st_size);
			    }
            } 
		    else if(S_ISLNK(entryInfo.st_mode)) { //тут проверяется является ли файл символьной ссылкой
                char targetName[PATH_MAX + 1];
                if(readlink(pathName, targetName, PATH_MAX) != -1) { //если успешно считалась символьная ссылка то идем дальше
				    if(strstr(option, "l") != NULL) {            
            			printf("\t-l %s -> %s\n", pathName, targetName);
				    }
                } 
			else {
				if(strstr(option, "l") != NULL) {            
            				printf("\t%s -> (недопустимая символическая ссылка!)\n", pathName);
				    }
                }
            }
        }    
	    else {
            printf("Error  %s: %s\n", pathName, strerror(errno));
        }
 	    entry = readdir(dir);
	}
	(void)closedir(dir);
	return 0;
}

int main(int argc, char **argv)
{
	char option[PATH_MAX];//PATH_MAX это максимальный размер который можно выделить,в моем случае это 4096
	char direct[PATH_MAX];
	int fl;
	if(argc == 3) {
		strncpy(option, argv[2], PATH_MAX);
		strcpy(direct, argv[1]);
		fl = 0;
	}
	if(argc == 1) { 
		strcpy(direct,".");
		fl = 1;
	}
	if(argc == 2) {
		if(strstr(argv[1], "-f") != NULL || strstr(argv[1], "-d") != NULL || strstr(argv[1], "-l") != NULL) { 
			strncpy(option, argv[1], PATH_MAX);//записывает длину PATH_MAX в option из argv
			strcpy(direct,".");//записывает только первую строку,тоесть до переноса и тут размер не указывается
			fl = 0;
		}
		else {
			strcpy(direct, argv[1]);
			fl = 1;
		}
	}	
    directory_browsing(direct, option, fl );
    return EXIT_SUCCESS;
}
