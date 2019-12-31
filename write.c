#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define KEY 24600

// union semun {
//   int              val;    /* Value for SETVAL */
//   struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
//   unsigned short  *array;  /* Array for GETALL, SETALL */
//   struct seminfo  *__buf;  /* Buffer for IPC_INFO
//                               (Linux-specific) */
// };

int main() {
  printf("Trying to get in\n");
  int semd = semget(KEY, 1, 0);
  struct sembuf sb;
  sb.sem_num = 0;
  sb.sem_op = -1;
  semop(semd, &sb, 1);

  //Get the last line
  int shmd = shmget(KEY, 10, 0);
  int *data = shmat(shmd, 0, 0);
  int file = open("story.txt", O_RDWR);
  int check = lseek(file, -1 * (*data), SEEK_END);
  if (check == -1) {
    printf("lseek failed\n");
  }
  if (*data == 0) {
    printf("Last addition:\n");
  } else {
    char last_line[*data];
    check = read(file, last_line, *data);
    if (check == -1) {
      printf("read failed\n");
    }
    printf("Last addition: %s", last_line);
  }

  printf("Your addition: ");
  char input[256];
  fgets(input, 256, stdin);
  //Update size in shared memory
  *data = strlen(input);
  //Write to file
  check = lseek(file, 0, SEEK_END);
  if (check == -1) {
    printf("lseek failed\n");
  }
  check = write(file, input, *data);
  if (check == -1) {
    printf("write failed\n");
  }
  //Detaching data variable
  int shm_check = shmdt(data);
  if (shm_check == -1) {
    printf("Detaching the data variable failed, strerror: %s\n", strerror(errno));
    exit(0);
  }
  close(file);
  //Release semaphore
  sb.sem_op = 1;
  semop(semd, &sb, 1);

  return 0;
}
