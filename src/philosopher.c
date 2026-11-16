#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
 
#define N 5
#define ROUNDS 3
 
int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <philosopher id 0-4>\n", argv[0]);
        return 1;
    }
 
    int id = atoi(argv[1]);
    if (id < 0 || id >= N) {
        printf("ID must be between 0 and %d\n", N - 1);
        return 1;
    }
 
    sem_t *chopstick[N];
    sem_t *room;
    char name[20];
 
    /* Open (or create if not present) the named semaphores */
    for (int i = 0; i < N; i++) {
        sprintf(name, "/chopstick%d", i);
        chopstick[i] = sem_open(name, O_CREAT, 0644, 1);
        if (chopstick[i] == SEM_FAILED) {
            perror("sem_open chopstick");
            return 1;
        }
    }
    room = sem_open("/room", O_CREAT, 0644, N - 1);
    if (room == SEM_FAILED) {
        perror("sem_open room");
        return 1;
    }
 
    int left = id;
    int right = (id + 1) % N;
 
    for (int r = 1; r <= ROUNDS; r++) {
        printf("Philosopher %d is thinking...\n", id);
        sleep(1 + id % 3);
 
        printf("Philosopher %d is hungry\n", id);
        sem_wait(room);               /* enter the room (max 4) */
        sem_wait(chopstick[left]);    /* pick left chopstick */
        printf("Philosopher %d picked up chopstick %d\n", id, left);
        sem_wait(chopstick[right]);   /* pick right chopstick */
        printf("Philosopher %d picked up chopstick %d\n", id, right);
 
        printf("Philosopher %d is EATING (round %d)\n", id, r);
        sleep(2);
 
        printf("Philosopher %d finished eating, putting down chopsticks\n", id);
        sem_post(chopstick[left]);    /* put down left chopstick */
        sem_post(chopstick[right]);   /* put down right chopstick */
        sem_post(room);               /* leave the room */
    }
 
    /* Close semaphores in this process */
    for (int i = 0; i < N; i++)
        sem_close(chopstick[i]);
    sem_close(room);
 
    printf("Philosopher %d is done.\n", id);
    return 0;
}

