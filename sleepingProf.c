#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define STUDENT_COUNT_MIN 2
#define STUDENT_COUNT_MAX 10
#define CHAIR_COUNT 3
#define HELPS_MAX 3

sem_t student_sem, professor_sem, chair_sem;
int chair_count = CHAIR_COUNT;

void* professorThd(void* arg) {
    while (1) {
        sem_wait(&professor_sem); // Wait until awakened by a student
        printf("\033[0;92mProfessor has been awakened by a student.\033[0m\n");

        while (1) {
            sem_wait(&chair_sem); // Wait for student to occupy a chair

            if (chair_count == 0) {
                sem_post(&chair_sem);
                break;
            }

            chair_count--;
            printf("\033[0;92mStudent frees chair and enters professor's office. Remaining chairs %d\033[0m\n", chair_count);

            sem_post(&student_sem); // Signal that professor can help the student

            printf("\033[0;31mProfessor is helping a student.\033[0m\n");

            usleep(rand() % 1500000);

            printf("\033[0;31mStudent has been helped by the professor.\033[0m\n");

            sem_post(&chair_sem); // Student vacates chair
        }

        int student_count;
        sem_getvalue(&student_sem, &student_count);

        if (student_count == 0) {
            printf("\033[0;31mAll students have been assisted, professor is leaving.\033[0m\n");
            pthread_exit(NULL);
        }
    }
}

void* studentThd(void* arg) {
    int* student_num = (int*) arg;

    while (1) {
        printf("\033[0;93mStudent %d is doing the assignment.\033[0m\n", *student_num);

        usleep(rand() % 2000000);
        sem_wait(&chair_sem); // Get chair count (mutex)
        printf("\033[0;31mStudent %d needs help from the professor.\033[0m\n", *student_num);

       
        if (chair_count == 0) {
            printf("\033[0;31mChairs occupied, student %d will return later.\033[0m\n", *student_num);
            sem_post(&chair_sem);
            continue;
        }

        if (chair_count == CHAIR_COUNT) {
            sem_post(&professor_sem); // Awaken professor
        }

        chair_count--;
        printf("\033[0;92mStudent frees chair and enters professor's office. Remaining chairs %d\033[0m\n", chair_count);

        sem_post(&chair_sem); // Release mutex

        sem_wait(&student_sem); // Wait until professor can help

        printf("\033[0;92mStudent %d is getting help from the professor.\033[0m\n", *student_num);

        usleep(rand() % 1500000);

        sem_post(&chair_sem); // Student vacates chair

        //break;
    }

    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));

    printf("CS 370 - Sleeping Professor Project\n");

    int student_count;

    do {
        printf("How many students coming to professor's office? ");
        scanf("%d", &student_count);
    } while (student_count < STUDENT_COUNT_MIN || student_count > STUDENT_COUNT_MAX);

    sem_init(&student_sem, 0, 0);
    sem_init(&professor_sem, 0, 0);
    sem_init(&chair_sem, 0, 1);

    pthread_t professor;
    pthread_t* students = malloc(sizeof(pthread_t) * student_count);
    int* student_nums = malloc(sizeof(int) * student_count);

    pthread_create(&professor, NULL, professorThd, NULL);

    for (int i = 0; i < student_count; i++) {
        student_nums[i] = i + 1;
        pthread_create(&students[i], NULL, studentThd, &student_nums[i]);
    }

    pthread_join(professor, NULL);
    for (int i = 0; i < student_count; i++) {
        pthread_join(students[i], NULL);
    }

    free(students);
    free(student_nums);

    sem_destroy(&student_sem);
    sem_destroy(&professor_sem);
    sem_destroy(&chair_sem);

    return 0;
}
