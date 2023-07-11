#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <fcntl.h>

int nr_thread = 0;//for counting the nr of threads in process9
sem_t semaphore_process4_thread2;
sem_t semaphore_process4_thread5;
sem_t semaphore_process9;
sem_t semaphore_process9_thread11;
pthread_mutex_t running_threads_mutex = PTHREAD_MUTEX_INITIALIZER;//for counting the nr of threads in process9
sem_t *semaphore_p3_t1;
sem_t *semaphore_p3_t2;

void* thread_function4(void* thread_id){//function for process4
	int id = *(int*) thread_id;
	if(id == 2){
		info(BEGIN, 4, id);
		sem_post(&semaphore_process4_thread2);//lets thread5 to start
		sem_wait(&semaphore_process4_thread5);//waits for thread 5 to end
		info(END, 4, id);
	}
	else if(id == 5){
		sem_wait(&semaphore_process4_thread2);//waits for thread2 to start
		info(BEGIN, 4, id);
		info(END, 4, id);
		sem_post(&semaphore_process4_thread5);//lets thread2 to end
	}
	else if(id == 4){
		sem_wait(semaphore_p3_t2);//waits for thread2 in process3
		info(BEGIN, 4, id);
		info(END, 4, id);
		sem_post(semaphore_p3_t1);//lets thread1 in process3 start
	}
	else{
		info(BEGIN, 4, id);
		info(END, 4, id);
	}
	pthread_exit(0);
}

void* thread_function3(void* thread_id){//function for process3
	int id = *(int*) thread_id;
	if(id == 1){
		sem_wait(semaphore_p3_t1);//waits for thread4 in process 4 to end
		info(BEGIN, 3, id);
		info(END, 3, id);
	}
	else if(id == 2){
		info(BEGIN, 3, id);
		info(END, 3, id);
		sem_post(semaphore_p3_t2);//lets thread4 in process 4 to start
	}
	else{
		info(BEGIN, 3, id);
		info(END, 3, id);
	}
	pthread_exit(0);
}

void* thread_function9(void* thread_id){//function for process9
	//just the problem for process 9 with no thread 11 condition
	int id = *(int*) thread_id;
	sem_wait(&semaphore_process9);
	info(BEGIN, 9, id);
	info(END, 9, id);
	sem_post(&semaphore_process9);
	pthread_exit(0);
}

int closed = 0;//variable for knowing when to close thread11

void* thread_function9_v4(void* thread_id){//function for process9
	//attempt for solving the thread 11 problem 
	int id = *(int*) thread_id;
	if(id == 11){
		sem_wait(&semaphore_process9);
		info(BEGIN, 9, id);
	    	pthread_mutex_lock(&running_threads_mutex);
	    	nr_thread++;
		pthread_mutex_unlock(&running_threads_mutex);
		sem_wait(&semaphore_process9_thread11);
		info(END, 9, id);
		pthread_mutex_lock(&running_threads_mutex);
	    	nr_thread--;
		pthread_mutex_unlock(&running_threads_mutex);
		sem_post(&semaphore_process9);
	}
	else{
		sem_wait(&semaphore_process9);
		info(BEGIN, 9, id);
	    	pthread_mutex_lock(&running_threads_mutex);
	    	nr_thread++;
		pthread_mutex_unlock(&running_threads_mutex);
		if(nr_thread == 5){
			sem_post(&semaphore_process9_thread11);
		}
	    	info(END, 9, id);
	    	pthread_mutex_lock(&running_threads_mutex);
	    	nr_thread--;
	    	pthread_mutex_unlock(&running_threads_mutex);
	    	sem_post(&semaphore_process9);
    	}
	pthread_exit(0);
}

int main(int argc, char **argv){
	init();
	info(BEGIN, 1, 0);
	//initialize the semaphores used for process communication
	semaphore_p3_t1 = sem_open("/semaphore_p3_t1", O_CREAT, 0600, 0);
	semaphore_p3_t2 = sem_open("/semaphore_p3_t2", O_CREAT, 0600, 0);
	int p2 = fork();
	if(p2 == 0){
		info(BEGIN, 2, 0);
		int p9 = fork();
		if(p9 == 0){
			info(BEGIN, 9, 0);
			
			pthread_t threads9[39];
			int id9[39];
			
			sem_init(&semaphore_process9, 0, 5);
			sem_init(&semaphore_process9_thread11, 0, 0);
			
			int i = 0;
			for(i = 1; i <= 39; i++){
				id9[i - 1] = i;
				//pthread_create(&threads9[i - 1], NULL, thread_function9_v4, &id9[i - 1]);
				pthread_create(&threads9[i - 1], NULL, thread_function9, &id9[i - 1]);
			}
			for(i = 1; i <= 39; i++){
				pthread_join(threads9[i - 1], NULL);
			}
			sem_destroy(&semaphore_process9);
			sem_destroy(&semaphore_process9_thread11);
    			pthread_mutex_destroy(&running_threads_mutex);
			info(END, 9, 0);
			exit(0);
		}
		wait(NULL);
		info(END, 2, 0);
		exit(0);
	}
	int p3 = fork();
	if(p3 == 0){
		info(BEGIN, 3, 0);
		
		pthread_t threads3[5];
		int id3[5];
		
		int i = 0;
		for(i = 1; i <= 5; i++){
			id3[i - 1] = i;
			pthread_create(&threads3[i - 1], NULL, thread_function3, &id3[i - 1]);
		}
		for(i = 1; i <= 5; i++){
			pthread_join(threads3[i - 1], NULL);
		}
		sem_destroy(semaphore_p3_t1);
		sem_destroy(semaphore_p3_t2);
		int p5 = fork();
		if(p5 == 0){
			info(BEGIN, 5, 0);
			wait(NULL);
			info(END, 5, 0);
			exit(0);
		}
		int p6 = fork();
		if(p6 == 0){
			info(BEGIN, 6, 0);
			int p7 = fork();
			if(p7 == 0){
				info(BEGIN, 7, 0);
				wait(NULL);
				info(END, 7, 0);
				exit(0);
			}
			wait(NULL);
			info(END, 6, 0);
			exit(0);
		}
		int p8 = fork();
		if(p8 == 0){
			info(BEGIN, 8, 0);
			wait(NULL);
			info(END, 8, 0);
			exit(0);
		}
		wait(NULL);
		wait(NULL);
		wait(NULL);
		info(END, 3, 0);
		exit(0);
	}
	int p4 = fork();
	if(p4 == 0){
		info(BEGIN, 4, 0);
		
		pthread_t threads4[5];
		int id4[5];
		
		sem_init(&semaphore_process4_thread2, 0, 0);
		sem_init(&semaphore_process4_thread5, 0, 0);
		
		int i = 0;
		for(i = 1; i <= 5; i++){
			id4[i - 1] = i;
			pthread_create(&threads4[i - 1], NULL, thread_function4, &id4[i - 1]);
		}
		for(i = 1; i <= 5; i++){
			pthread_join(threads4[i - 1], NULL);
		}
		sem_destroy(&semaphore_process4_thread2);
		sem_destroy(&semaphore_process4_thread5);
		info(END, 4, 0);
		exit(0);
	}
	wait(NULL);
	wait(NULL);
	wait(NULL);
	info(END , 1, 0);
	return 0;
}

