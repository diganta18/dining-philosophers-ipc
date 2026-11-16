#include <stdio.h>
#include <semaphore.h>
 
int main()
{
    char name[20];
    for (int i = 0; i < 5; i++) {
        sprintf(name, "/chopstick%d", i);
        sem_unlink(name);
    }
    sem_unlink("/room");
    printf("Semaphores removed.\n");
    return 0;
}

