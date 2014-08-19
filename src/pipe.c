/* 
 * File:    pipe1.c 
 * Author:  Josh Taylor 
 * Created: 15 August, 2014 
 *
 * Uses the GNU pipe functions to send individual floating point number between 
 * threads.
 * 
 * Simple program to  to investigate the speed of passing information between 
 * two threads. The producer thread generates 1,000,000,000 random floating 
 * point numbers. The consumer thread receives 1,000,000,000 floating point 
 * number and summarises them.
 */

// #include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
// #include <unistd.h>
#include <Math.h>

int const Number_Count = 1000*1000;

void *producer(void *argument) 
{
    int a =  632559378;  /* \ Wichmann-Hill (2006) */
    int b = 1436901506;  /* | 4-cycle random */
    int c =  244914893;  /* | generator */
    int d = 1907460368;  /* / state. */

    double const recip_a /* 1/2 147 483 579.0 */ =
    0.00000000046566130226972971885062316464865503867958;
    double const recip_b /* 1/2 147 483 543.0 */ =
    0.00000000046566131007598599324865699331694482745566;
    double const recip_c /* 1/2 147 483 423.0 */ =
    0.00000000046566133609684213147940094716158374741503;
    double const recip_d /* 1/2 147 483 123.0 */ =
    0.00000000046566140114899519981000567798175892812360;

    double w; /* The random number */

    int i; /* Loop counter */

    int fd = *((int *) argument);

    for (i = 0; i < Number_Count; i++) {
        w  = recip_a * (double)(a = (int)((a * 11600LL) % 2147483579));
        w += recip_b * (double)(b = (int)((b * 47003LL) % 2147483543));
        w += recip_c * (double)(c = (int)((c * 23000LL) % 2147483423));
        w += recip_d * (double)(d = (int)((d * 33000LL) % 2147483123));
        if (w >= 2.0) w -= 2.0;
        if (w >= 1.0) w -= 1.0;

        /* <<< somehow send w to the other thread >>> */
        write(fd, &w, sizeof w);
    }

    return NULL;
}

void *consumer(void *argument) 
{
    int i;
    double x, v;
    double mean = 0.0, sum2 = 0.0;

    int fd = *((int *) argument);

    for (i = 0; i < Number_Count; i++) {

        /* <lt;lt; somehow receive x from the other thread >>> */
        read(fd, &x, sizeof x);

        v = (x - mean)/(i+1);
        sum2 += ((i+1)*v)*(i*v);
        mean += v;
    }
    printf("Mean = %g, standard deviation = %g\n", mean, sqrt(sum2/(i-1)));

    return NULL;
}

int main (void)
{
    int rc;
    pthread_t p, c;
    int mypipe[2];
     
    /* Create the pipe. */
    if (pipe (mypipe)) {
       printf("Pipe failed.\n");
       return EXIT_FAILURE;
    }

    rc = pthread_create(&c, NULL, consumer, (void *)&mypipe[0]);
    if (rc != 0) {
        printf("Failed to create comsumer.\n");
        exit(EXIT_FAILURE);
    }
    rc = pthread_create(&p, NULL, producer, (void *)&mypipe[1]);
    if (rc != 0) {
        printf("Failed to create producer.\n");
        exit(EXIT_FAILURE);
    }
    rc = pthread_join(p, NULL);
    if (rc != 0) {
        printf("Failed to join producer.\n");
        exit(EXIT_FAILURE);
    }
    rc = pthread_join(c, NULL);
    if (rc != 0) {
        printf("Failed to join consumer.\n");
        exit(EXIT_FAILURE);
    }

    close(mypipe[0]);
    close(mypipe[1]);

    return EXIT_SUCCESS;
}
