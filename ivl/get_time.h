#ifndef GET_TIME_H
#define GET_TIME_H

#include <time.h>
#include <sys/time.h>

struct compile_time {
    struct timeval r_time;
    clock_t u_time;
    float real_time;
    float user_time;
};

void start_time(struct compile_time *ct);
float get_real_time(struct compile_time *ct);
float get_user_time(struct compile_time *ct);
void end_time(struct compile_time *ct);
void print_time(struct compile_time *ct, const char* compile_stage);

inline void start_time(struct compile_time *ct)
{
    gettimeofday(&(ct->r_time), NULL);
    ct->u_time = clock();
}

inline float get_real_time(struct compile_time *ct)
{
    struct timeval end;
    if (gettimeofday(&end, NULL)) {
        return 0;
    }
    float ret = (end.tv_sec - ct->r_time.tv_sec) + (float)(end.tv_usec - ct->r_time.tv_usec) / 1000000;
    return ret;
}

inline float get_user_time(struct compile_time *ct)
{
    clock_t end = clock();
    float ret = (float)(end - ct->u_time) / CLOCKS_PER_SEC;
    return ret;
}

inline void end_time(struct compile_time *ct)
{
    ct->real_time = get_real_time(ct);
    ct->user_time = get_user_time(ct);
}

inline void print_time(struct compile_time *ct, const char* compile_stage)
{
    printf("XctProfile--%-8s \t  real  %-f(s)\t user  %-f(s)\n", compile_stage, ct->real_time, ct->user_time);
}

#endif
