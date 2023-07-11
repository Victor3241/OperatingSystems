#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define PIPE_RESP "RESP_PIPE_96183"
#define PIPE_REQ "REQ_PIPE_96183"

#define PATH_MAX 50

void msg_req(int fd_resp) {
    const char* variant = "VARIANT";
    int nr = 96183;
    const char* value = "VALUE";
    int size_variant = strlen(variant);
    int size_value = strlen(value);
    write(fd_resp, &size_variant, 1);
    write(fd_resp, variant, strlen(variant));
    write(fd_resp, &nr, sizeof(nr));
    write(fd_resp, &size_value, 1);
    write(fd_resp, value, strlen(value));
}

int size_shared_memory;

void create_shm(int fd_resp, unsigned int* addr, int size_shared_memory) {
    const char* name = "/MUQs4pb";
    const int permissions = 0644;
    //const int size = 2739776;
    const char* create = "CREATE_SHM";
    const char* success = "SUCCESS";
    const char* error = "ERROR";

    int size_create = strlen(create);
    int size_success = strlen(success);
    int size_error = strlen(error);

    write(fd_resp, &size_create, 1);
    write(fd_resp, create, strlen(create));

	
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, permissions);
    if (shm_fd == -1) {
        write(fd_resp, &size_error, 1);
        write(fd_resp, error, strlen(error));
        return;
    }

    if (ftruncate(shm_fd, size_shared_memory) == -1) {
        write(fd_resp, &size_error, 1);
        write(fd_resp, error, strlen(error));
        shm_unlink(name);
        return;
    }
    printf("%d\n", size_shared_memory);
    addr = (unsigned int*)mmap(NULL, size_shared_memory, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (addr == MAP_FAILED) {
        write(fd_resp, &size_error, 1);
        write(fd_resp, error, strlen(error));
        shm_unlink(name);
        return;
    }
    write(fd_resp, &size_success, 1);
    write(fd_resp, success, strlen(success));
}

void write_to_shm(int offset, int value, int fd_resp, int fd_req) {
	const char* create = "WRITE_TO_SHM";
	const char* success = "SUCCESS";
	const char* error = "ERROR";
    const int size = 2739776;
	
	
	int size_create = strlen(create);
	int size_success = strlen(success);
	int size_error = strlen(error);
			
	
	write(fd_resp, &size_create, 1);
	write(fd_resp, create, strlen(create)); 

    if (offset >= size || offset < 0 || ((offset + sizeof(offset)) >= size)) {
        write(fd_resp, &size_error, 1);
        write(fd_resp, error, strlen(error));
        return;
    }
    
    	int shm_fd = shm_open("/MUQs4pb", O_CREAT | O_RDWR, 0644);
	//addr = (unsigned int*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	
	//addr[offset] = value;
	//memcpy(addr + offset, &value, sizeof(unsigned int));
	lseek(shm_fd, offset, SEEK_SET);
	write(shm_fd, &value, sizeof(unsigned int));
    	
    	
    	write(fd_resp, &size_success, 1);
	write(fd_resp, success, strlen(success));
	
    	//memcpy(addr + offset, &value, sizeof(unsigned int));
    	//addr[offset / sizeof(unsigned int)] = value; 
}

void map_file(char* file_name, int fd_resp) {
	const char* create = "MAP_FILE";
    const char* success = "SUCCESS";
    const char* error = "ERROR";
    
	int size_create = strlen(create);
    int size_success = strlen(success);
    int size_error = strlen(error);

	write(fd_resp, &size_create, 1);
    write(fd_resp, create, size_create);
	//printf("ok\n");
	
	
	
    char current_dir[PATH_MAX];
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        write(fd_resp, "ERROR", strlen("ERROR"));
        return;
    }

    char full_path[PATH_MAX + 1];
    snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, file_name);

	
    int fd = open(file_name, O_CREAT | O_RDWR);
    //printf("%s\n", file_name);
    if (fd == -1) {
    //printf("ERROR\n");
        write(fd_resp, &size_error, 1);
        write(fd_resp, error, strlen(error));
        return;
    }

	//printf("ok\n");
    struct stat st;
    if(fstat(fd, &st) == -1){
    	close(fd);
    	write(fd_resp, &size_error, 1);
        write(fd_resp, error, strlen(error));
        return;
    }
    
    off_t file_size = st.st_size;
    void* addr = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        close(fd);
        write(fd_resp, &size_error, 1);
        write(fd_resp, error, strlen(error));
        return;
    }
	//printf("ok\n");
    close(fd);
    write(fd_resp, &size_success, 1);
    write(fd_resp, success, size_success);
}

void read_from_file_offset(int offset, int no_of_bytes, int fd_resp, unsigned int* addr) {
    const char* create = "READ_FROM_FILE_OFFSET";
    const char* success = "SUCCESS";
    const char* error = "ERROR";
    const int size = 2739776;
    int create_size = strlen(create);
    int size_success = strlen(success);
    int size_error = strlen(error);

	write(fd_resp, &create_size, 1);
    write(fd_resp, create, create_size);
    printf("%d\n%d\n", offset, no_of_bytes);
    
    printf("ok\n");

    if (addr == NULL) {
        write(fd_resp, &size_error, 1);
        write(fd_resp, error, strlen(error));
        return;
    }

	printf("ok\n");
    int file_size = no_of_bytes + offset;
    if (file_size > size) {
        write(fd_resp, &size_error, 1);
        write(fd_resp, error, strlen(error));
        return;
    }
	printf("ok\n");
    //memcpy(addr, (addr + offset), no_of_bytes);


    write(fd_resp, &size_success, 1);
    write(fd_resp, success, size_success);
}


int main() {
    int fd_resp;
    if (mkfifo(PIPE_RESP, 0666) == -1) {
        printf("ERROR\ncannot create the response pipe\n");
        unlink(PIPE_RESP);
        unlink(PIPE_REQ);
        return 0;
    }

    int fd_req = open(PIPE_REQ, O_RDONLY);
    if (fd_req < 0) {
        printf("ERROR\ncannot open the request pipe\n");
        unlink(PIPE_RESP);
        unlink(PIPE_REQ);
        return 0;
    }

    fd_resp = open(PIPE_RESP, O_WRONLY);
    if (fd_resp < 0) {
        printf("ERROR\ncannot open the response pipe\n");
        close(fd_req);
        unlink(PIPE_RESP);
        unlink(PIPE_REQ);
        return 0;
    }

    const char* st = "START";
    int size = strlen(st);
    write(fd_resp, &size, 1);
    if ((write(fd_resp, st, strlen(st))) == 5) {
        printf("SUCCESS\n");
    }

    char s[250];
    int size_s = 0;
    unsigned int* addr = NULL;
    while (1) {
    	read(fd_req, &size_s, 1);
    	printf("%d\n", size_s);
        read(fd_req, &s, size_s);
        s[size_s] = '\0';
        printf("%s\n", s);

        if (strcmp(s, "VARIANT") == 0) {
            msg_req(fd_resp);
        } else if (strcmp(s, "CREATE_SHM") == 0) {
        
      		int size_shared_memory = 0;
      		read(fd_req, &size_shared_memory, sizeof(size_shared_memory));
      		printf("%d\n", size_shared_memory);
      		
            create_shm(fd_resp, addr, size_shared_memory);
        } else if (strcmp(s, "WRITE_TO_SHM") == 0) {
            unsigned int offset = -1;
            unsigned int value = -1;
            //int size_offset;
            //int size_value;
            //read(fd_req, &size_offset, 1);
            read(fd_req, &offset, sizeof(offset));
            //read(fd_req, &size_value, 1);
            read(fd_req, &value, sizeof(value));
            printf("\n%s\n%d\n%d\n", s, offset, value);
            write_to_shm(offset, value, fd_resp, fd_req);
        } else if(strcmp(s, "MAP_FILE") == 0){
        	 int size_map = 0;
        	 read(fd_req, &size_map, 1);
        	 //printf("%d\n", size_map);
        	 char* file_name = (char*)malloc((size_map));
        	 read(fd_req, file_name, size_map);
        	 file_name[size_map] = '\0';
        	 //printf("%s\n", file_name);
        	 map_file(file_name, fd_resp);
        }else if (strcmp(s, "EXIT") == 0) {
            close(fd_req);
            close(fd_resp);
            unlink(PIPE_RESP);
            unlink(PIPE_REQ);
            return 0;
        } else if(strcmp(s, "READ_FROM_FILE_OFFSET") == 0){
            unsigned int offset = -1;
            unsigned int nr_bytes = -1;
            read(fd_req, &offset, sizeof(offset));
            read(fd_req, &nr_bytes, sizeof(nr_bytes));
            read_from_file_offset(offset, nr_bytes, fd_resp, addr);
        }else {
            close(fd_req);
            close(fd_resp);
            unlink(PIPE_RESP);
            unlink(PIPE_REQ);
            return 0;
        }
    }

    close(fd_req);
    close(fd_resp);
    unlink(PIPE_RESP);
    unlink(PIPE_REQ);
    return 0;
}

