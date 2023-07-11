#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_PATH_LEN 1000


void list_dir(char *dirPath, int permission, int size)
{
    DIR *dir;
    struct dirent *dirEntry;
    struct stat inode;
    char name[MAX_PATH_LEN];

    lstat(dirPath, &inode);

    if (!S_ISDIR(inode.st_mode))
    {
        if(permission){
		if(inode.st_mode & S_IEXEC){
			printf("%s\n", dirPath);
			exit(7);
		}
        }
        else {
        printf("%s\n", dirPath);
        exit(7);
        }
    }
    printf("SUCCESS\n");

    dir = opendir(dirPath);
    if (dir == 0)
    {
        printf("ERROR\ninvalid directory path\n");
        exit(4);
    }

    while ((dirEntry = readdir(dir)) != 0)
    {
        snprintf(name, MAX_PATH_LEN, "%s/%s", dirPath, dirEntry->d_name);
	
    lstat(name, &inode);

       if (inode.st_mode & S_IEXEC && permission){
	       if (strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0){
		    printf("%s\n", name);
		}
	}
	else {
        if (size >= 0 && !permission)
        {
            lstat(name, &inode);
            if (S_ISREG(inode.st_mode) && inode.st_size > size)
            {
                printf("%s\n", name);
            }
        }
        else if (strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0 && !permission)
            printf("%s\n", name);
            }
    }

    closedir(dir);
}

void recursive_list_dir(char *dirPath, int first, int permission, int size)
{
    DIR *dir;
    struct dirent *dirEntry;
    struct stat inode;
    char name[MAX_PATH_LEN];

    lstat(dirPath, &inode);

    if (!S_ISDIR(inode.st_mode))
    {
    	if(permission){
		if(inode.st_mode & S_IEXEC){
			printf("%s\n", dirPath);
			exit(7);
		}
        }
        else {
        printf("%s\n", dirPath);
        exit(7);
        }
    }
    if (first)
        printf("SUCCESS\n");

    dir = opendir(dirPath);
    if (dir == 0)
    {
        printf("%s\n", dirPath);
        printf("ERROR\ninvalid directory path\n");
        exit(4);
    }

    while ((dirEntry = readdir(dir)) != 0)
    {

        snprintf(name, MAX_PATH_LEN, "%s/%s", dirPath, dirEntry->d_name);

        lstat(name, &inode);
        if (S_ISDIR(inode.st_mode) && strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0)
        {
            recursive_list_dir(name, 0, permission, size);
        }
        if (inode.st_mode & S_IEXEC && permission){
	       if (strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0){
		    printf("%s\n", name);
		}
	}
	else {
        if (size >= 0 && !permission)
        {
            lstat(name, &inode);
            if (S_ISREG(inode.st_mode) && inode.st_size > size)
            {
                printf("%s\n", name);
            }
        }
        else if (strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0 && !permission)
            printf("%s\n", name);
            }
    }

    closedir(dir);
}

void parse(char* path)
{
	int fd = open(path, O_RDONLY);
	if(fd == -1)
	{
		printf("File not open\n");
		close(fd);
		return;
	}	
	
	char magic[] = "0000";
	lseek(fd, -4, SEEK_END);
	read(fd, magic, 4);
	if(strcmp(magic, "P3A8"))
	{
		printf("ERROR\nwrong magic");
		close(fd);
		return;	
	}
	close(fd);
	
	short header_size = 0;
	fd = open(path, O_RDONLY);
	lseek(fd, -6, SEEK_END);
	read(fd, &header_size, 2);
	
	short version;
	lseek(fd, -header_size, SEEK_END);
	read(fd, &version, 2);
	if(version < 42 || version > 125)
	{
		printf("ERROR\nwrong version");
		close(fd);
		return;
	}
	
	int nr_of_sections = 0;
	lseek(fd, -header_size + 2, SEEK_END);
	read(fd, &nr_of_sections, 1);
	if(nr_of_sections < 7 || nr_of_sections > 16)
	{
		printf("ERROR\nwrong sect_nr");
		close(fd);
		return;
	}
	
	int type = 0;
	lseek(fd, -header_size + 3, SEEK_END);
	for(int s = 0; s < nr_of_sections; s++)
	{
		lseek(fd, 15, SEEK_CUR);
		read(fd, &type, 4);
		if(type != 51 && type != 40 && type != 76 && type != 62)
		{
			printf("ERROR\nwrong sect_types");
			close(fd);
			return;
		}
		lseek(fd, 8, SEEK_CUR);
	}
	char* name = (char*)malloc(4*sizeof(int) + 1);
	int size = 0, offset = 0;
	lseek(fd, -header_size, SEEK_END);
	read(fd, &version, 2);
	read(fd, &nr_of_sections, 1);
	printf("SUCCESS\nversion=%d\nnr_sections=%d\n", version, nr_of_sections);
	for(int i = 0; i < nr_of_sections; i++)
	{
		strcpy(name, "");
		read(fd, name, 15);
		read(fd, &type, 4);
		read(fd, &offset, 4);
		read(fd, &size, 4);
		printf("section%d: %s %d %d\n", i+1, name, type, size);
	}
	free(name);
	close(fd);
	return;
}

void extract(char* path, int section, int line)
{
	int fd = open(path, O_RDONLY);
	if(fd == -1)
	{
		printf("ERROR\ninvalid file");
		close(fd);
		return;
	}	
	
	char magic[] = "0000";
	lseek(fd, -4, SEEK_END);
	read(fd, magic, 4);
	if(strcmp(magic, "P3A8"))
	{
		printf("ERROR\ninvalid file");
		close(fd);
		return;
	}
	close(fd);
	
	short header_size = 0;
	fd = open(path, O_RDONLY);
	lseek(fd, -6, SEEK_END);
	read(fd, &header_size, 2);
	
	short version;
	lseek(fd, -header_size, SEEK_END);
	read(fd, &version, 2);
	if(version < 42 || version > 125)
	{
		printf("ERROR\ninvalid file");
		close(fd);
		return;
	}
	
	int nr_of_sections = 0;
	lseek(fd, -header_size + 2, SEEK_END);
	read(fd, &nr_of_sections, 1);
	if(nr_of_sections < 7 || nr_of_sections > 16)
	{
		printf("ERROR\ninvalid file");
		close(fd);
		return;
	}
	
	int type = 0;
	lseek(fd, -header_size + 3, SEEK_END);
	for(int s = 0; s < nr_of_sections; s++)
	{
		lseek(fd, 15, SEEK_CUR);
		read(fd, &type, 4);
		if(type != 51 && type != 40 && type != 76 && type != 62)
		{
			printf("ERROR\ninvalid file");
			close(fd);
			return;
		}
		lseek(fd, 8, SEEK_CUR);
	}
	if(nr_of_sections < section)
	{
		printf("ERROR\ninvalid section");
		close(fd);
		return;	
	}
	
	char name[16] = "";
	int size = 0, offset = 0;
	lseek(fd, -header_size, SEEK_END);
	read(fd, &version, 2);
	read(fd, &nr_of_sections, 1);
	for(int i = 0; i < nr_of_sections; i++)
	{
		read(fd, name, 15);
		name[15] = '\0';
		read(fd, &type, 4);
		read(fd, &offset, 4);
		read(fd, &size, 4);
		if(i == (section - 1)){
			i = nr_of_sections + 1;
		}
	}
	
	lseek(fd, offset, SEEK_SET);
	char section_text[size + 1];
	read(fd, section_text, size);
	int nr_lines = 1;
	
	for(int i = 0; i < size; i++)
	{
		if(section_text[i] == '\n')
		{
			nr_lines = nr_lines + 1;
		}
	}
	
	if(nr_lines < line || nr_lines < 1)
	{
		printf("ERROR\ninvalid line");
		close(fd);
		return;
	}
	printf("SUCCESS");
	int found = 1;
	for(int i = 0; i < size - 1; i++)
	{
		if(section_text[i] == '\n')
		{
			found = found + 1;
		}
		if(found == (nr_lines - line + 1))
		{
			printf("%c", section_text[i]);
		}
	}
	close(fd);
	return;
}

int verif(char* path)
{
	int fd = open(path, O_RDONLY);
	if(fd == -1)
	{
		
		close(fd);
		return 0;
	}	
	
	char magic[] = "0000";
	lseek(fd, -4, SEEK_END);
	read(fd, magic, 4);
	if(strcmp(magic, "P3A8"))
	{
		close(fd);
		return 0;	
	}
	close(fd);
	
	short header_size = 0;
	fd = open(path, O_RDONLY);
	lseek(fd, -6, SEEK_END);
	read(fd, &header_size, 2);
	
	short version;
	lseek(fd, -header_size, SEEK_END);
	read(fd, &version, 2);
	if((int)version < 42 ||  (int)version > 125)
	{
		close(fd);
		return 0;
	}
	
	int nr_of_sections = 0;
	lseek(fd, -header_size + 2, SEEK_END);
	read(fd, &nr_of_sections, 1);
	if(nr_of_sections < 7 || nr_of_sections > 16)
	{
		close(fd);
		return 0;
	}
	
	int type = 0;
	lseek(fd, -header_size + 3, SEEK_END);
	int offset[nr_of_sections];
	int size[nr_of_sections];
	for(int s = 0; s < nr_of_sections; s++)
	{
		lseek(fd, 15, SEEK_CUR);
		read(fd, &type, 4);
		if(type != 51 && type != 40 && type != 76 && type != 62)
		{
			close(fd);
			return 0;
		}
		read(fd, &offset[s], 4);
		read(fd, &size[s], 4);
	}
	int ok = -1;
	for(int i = 0; i < nr_of_sections; i++)
	{
		char section[size[i] + 1];
		lseek(fd, offset[i], SEEK_SET);
		read(fd, section, size[i]);
		int nr_lines = 1;
		for(int j = 0; j < size[i]; j++)
		{
			if(section[j] == '\n')
			{
				nr_lines = nr_lines + 1;
			}
		}
		if(nr_lines == 15)
		{
			ok = ok + 1;
		}
	}
	close(fd);
	if(ok > 0){
		return 1;
	}
	else 
	{
		return 0;
	}
}


void findall(char *dirPath, int first, int permission, int size)
{
    DIR *dir;
    struct dirent *dirEntry;
    struct stat inode;
    char name[MAX_PATH_LEN];

    lstat(dirPath, &inode);

    if (S_ISREG(inode.st_mode))
    {
    	if(permission){
		if(inode.st_mode & S_IEXEC){
			printf("%s\n", dirPath);
			exit(7);
		}
        }
        else {
        printf("%s\n", dirPath);
        exit(7);
        }
    }
    if (first)
        printf("SUCCESS\n");

    dir = opendir(dirPath);
    if (dir == 0)
    {
        printf("%s\n", dirPath);
        printf("ERROR\ninvalid directory path\n");
        exit(4);
    }

    while ((dirEntry = readdir(dir)) != 0)
    {

        snprintf(name, MAX_PATH_LEN, "%s/%s", dirPath, dirEntry->d_name);

        lstat(name, &inode);
        if (S_ISDIR(inode.st_mode) && strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0)
        {
            findall(name, 0, permission, size);
        }
        if (permission){
        if(inode.st_mode & S_IEXEC){
	       if (strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0){
		    printf("%s\n", name);
		}
		}
	}
	else {
        if (size >= 0)
        {
            lstat(name, &inode);
            if (S_ISREG(inode.st_mode) && inode.st_size > size)
            {
                printf("%s\n", name);
            }
        }
        else if (strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0 && S_ISREG(inode.st_mode))
            if(verif(name) == 1){
            	printf("%s\n", name);
            }
            }
    }

    closedir(dir);
}


int main(int argc, char **argv)
{
    int perm = 0;
    int size = -1;
    if (argc >= 2)
    {
        if (strcmp(argv[1], "variant") == 0)
        {
            printf("96183\n");
        }
        if (argc >= 3)
        {
            if (strcmp(argv[1], "list") == 0)
            {
                char path[MAX_PATH_LEN] = "";
                for (int i = 5; i < strlen(argv[argc - 1]); i++)
                {
                    path[i - 5] = argv[argc - 1][i];
                }
                if (argc >= 4)
                {
                    if (argc == 5)
                    {
                        if (!strcmp(argv[2], "has_perm_execute"))
                        {
                            perm = 1;
                        }
                        else if (!strcmp(argv[3], "has_perm_execute"))
                        {
                            perm = 1;
                        }
                        if (argv[2][0] == 's')
                        {
                            size = atoi(argv[2] + 13);
                        }
                        else if(argv[3][0] == 's')
                        {
                            size = atoi(argv[3] + 13);
                        }
                    }
                    else
                    {
                        if (!strcmp(argv[2], "has_perm_execute"))
                        {
                            perm = 1;
                        }
                        if (argv[2][0] == 's')
                        {
                            size = atoi(argv[2] + 13);
                        }
                    }
                    if (strcmp(argv[2], "recursive") == 0)
                    {
                        recursive_list_dir(path, 1, perm, size);
                    }
                    else
                        list_dir(path, perm, size);
                }
                else
                    list_dir(path, perm, size);
            }
        }
    }
    if(!strcmp(argv[1], "parse")){
    	char path[MAX_PATH_LEN] = "";
    	strcpy(path, argv[2] + 5);
    	parse(path);
    }
    if(!strcmp(argv[1], "extract")){
    	char path[MAX_PATH_LEN] = "";
    	strcpy(path, argv[2] + 5);
    	int section = atoi(argv[3] + 8);
    	int line = atoi(argv[4] + 5);
    	extract(path, section, line);
    }
    if(!strcmp(argv[1], "findall"))
    {
    	char path[MAX_PATH_LEN] = "";
    	strcpy(path, argv[2] + 5);
    	findall(path, 1, 0, -1);
    }
    
    return 0;
}
