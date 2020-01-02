#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define KEY 24600

union semun {
  int              val;    /* Value for SETVAL */
  struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
  unsigned short  *array;  /* Array for GETALL, SETALL */
  struct seminfo  *__buf;  /* Buffer for IPC_INFO
                              (Linux-specific) */
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Please enter ./control followed by one argument (-c, -r, or -v)\n");
    exit(0);
  }
  int shmd;
  int * data;
  int semd;
  int v, r;
  if (strcmp(argv[1], "-c") == 0) {
    //Open semaphore
    semd = semget(KEY, 1, IPC_CREAT | IPC_EXCL | 0644);
    if (semd == -1) {
      printf("error %d: %s\n", errno, strerror(errno));
      semd = semget(KEY, 1, 0);
      v = semctl(semd, 0, GETVAL, 0);
      //printf("semctl returned: %d\n", v);
      if (v == -1) {
        printf("Opening semaphore failed, strerror: %s\n", strerror(errno));
      } else {
        printf("Semaphore created\n");
      }
    }
    else {
      union semun us;
      us.val = 1;
      r = semctl(semd, 0, SETVAL, us);
      //printf("semctl returned: %d\n", r);
      if (r == -1) {
        printf("Opening semaphore failed, strerror: %s\n", strerror(errno));
      } else {
        printf("Semaphore created\n");
      }
    }

    //Open shared memory
    shmd = shmget(KEY, 10, IPC_CREAT | 0644);
    if (shmd == -1) {
      printf("Opening shared memory failed, strerror: %s\n", strerror(errno));
      exit(0);
    } else {
      printf("Shared memory created\n");
    }
    data = shmat(shmd, 0, 0);
    *data = 0;
    int shm_check = shmdt(data);
    if (shm_check == -1) {
      printf("Detaching the data variable failed, strerror: %s\n", strerror(errno));
      exit(0);
    }
    //Open file
    int file = open("story.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (file == -1) {
      printf("Opening your file failed, strerror: %s\n", strerror(errno));
      exit(0);
    } else {
      printf("File created\n");
    }
    close(file);
  } else if (strcmp(argv[1], "-r") == 0) {
    //Diplay full story
    //Wait until semaphore is available, then remove it
    int semd = semget(KEY, 1, 0);
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_op = -1;
    semop(semd, &sb, 1);
    int sem_check = semctl(semd, IPC_RMID, 0);
    if (sem_check == -1) {
      printf("Removing the semaphore failed, strerror: %s\n", strerror(errno));
      exit(0);
    } else {
      printf("Semphore removed\n");
    }
    //Remove shared memory
    int shmd = shmget(KEY, 10, 0);
    int shm_check = shmctl(shmd, IPC_RMID, 0);
    if (shm_check == -1) {
      printf("Removing the shared memory failed, strerror: %s\n", strerror(errno));
      exit(0);
    } else {
      printf("Shared memory removed\n");
    }
    //Remove file
    int file_check = remove("story.txt");
    if (file_check == -1) {
      printf("Removing your file failed, strerror: %s\n", strerror(errno));
      exit(0);
    } else {
      printf("File removed\n");
    }
  } else if (strcmp(argv[1], "-v") == 0) {
    //Read file and print entire story
    int file = open("story.txt", O_RDONLY);
    //Find size of the file
    struct stat file_stat;
    stat("story.txt", &file_stat);
    int file_size = file_stat.st_size;
    char text[file_size];
    int check = read(file, text, file_size);
    if (check != file_size) {
      printf("read failed\n");
    }
    close(file);

    printf("The story so far:\n");
    printf("%s", text);
  } else {
    printf("Couldn't handle argument, please try again!\n");
    exit(0);
  }

  return 0;
}
