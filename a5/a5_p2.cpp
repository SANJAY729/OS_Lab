#include <stdio.h>
#include <iostream>
#include <unistd.h> 
#include <stdlib.h> 
#include <time.h>
#include <pthread.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/time.h>
#include <sys/syscall.h>
#include <chrono>
#include <bits/stdc++.h>
using namespace std;

#define MAX_SIZE_PRIORITY_QUEUE 100
#define gettid() syscall(SYS_gettid)

int max_queue_size = 8;

typedef struct {
    int thread_id;
    int producer_number;
    int priority;
    int compute_time;
    int job_id;
} job;

typedef struct {
    job job_queue[MAX_SIZE_PRIORITY_QUEUE]; //using as indexed from 1
    int size;
    int job_created;
    int job_completed;
    pthread_mutex_t lock;
} data;

typedef struct {
    data* jobs_info;
    int thread_number;
    int total_jobs;
} param;

data initialize() {
    data jobs_info;
    jobs_info.size = 0;
    jobs_info.job_created = 0;
    jobs_info.job_completed = 0;
    pthread_mutexattr_t lock_attr;
	pthread_mutexattr_init(&lock_attr);
	pthread_mutexattr_setpshared(&lock_attr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&jobs_info.lock, &lock_attr);
    return jobs_info;
}

void insert_job(data* jobs_info, job x) {
    jobs_info->size = jobs_info->size + 1;
    int i  = jobs_info->size - 1;
    jobs_info->job_queue[i] = x;
    while (i != 0 && jobs_info->job_queue[i].priority > jobs_info->job_queue[(i - 1)/2].priority) {
        job temp = jobs_info->job_queue[i];
        jobs_info->job_queue[i] = jobs_info->job_queue[(i - 1)/2];
        jobs_info->job_queue[(i - 1)/2] = temp;
        i = (i - 1)/2;
    }
    return;
}

void heapify(data* jobs_info, int i){
    int l = 2 * i + 1;
    int r = 2 * i + 2;
    int max = i;
    if (r < jobs_info->size && jobs_info->job_queue[i].priority < jobs_info->job_queue[r].priority)
        max = r;
    if (l < jobs_info->size && jobs_info->job_queue[max].priority < jobs_info->job_queue[l].priority)
        max = l;
    if (max != i){
        job temp = jobs_info->job_queue[max];
        jobs_info->job_queue[max] = jobs_info->job_queue[i];
        jobs_info->job_queue[i] = temp;
        heapify(jobs_info,max);
    }
}

job delete_job(data* jobs_info) {
    if (jobs_info->size == 1){
        jobs_info->size = 0;
        return jobs_info->job_queue[0];
    }
    job root = jobs_info->job_queue[0];
    jobs_info->job_queue[0] = jobs_info->job_queue[jobs_info->size - 1];
    jobs_info->size = jobs_info->size - 1;
    heapify(jobs_info,0);
    return root;
}

job make_job(int producer_number, int thread_id) {
    job x;
    x.compute_time = rand()%4 + 1;
    x.job_id = rand()%100000 + 1;
    x.priority = rand()%10 + 1;
    x.producer_number = producer_number;
    x.thread_id = thread_id;
    return x; 
}

void print(job x) {
    cout<<"JOB ID "<<x.job_id<<endl;
    cout<<"PRODUCER THREAD ID "<<x.thread_id<<endl;
    cout<<"PRIORITY "<<x.priority<<endl;
    cout<<"PRODUCER NUMBER "<<x.producer_number<<endl;
    cout<<"COMPUTE TIME "<<x.compute_time<<endl;
}

void lag(int time) {
    clock_t start = clock();
    while(clock() < start + time * CLOCKS_PER_SEC);
}

void* produce_job(void* arg){
    //extracting the parameters from void* arg
    param* function_param = (param*)arg;
    data* jobs_info = function_param->jobs_info;
    int producer_number = function_param->thread_number;
    int thread_id = gettid();
    int total_jobs = function_param->total_jobs;
    while(1) {
        lag(rand()%4);
        pthread_mutex_lock(&jobs_info->lock);
        if (jobs_info->job_created >= total_jobs) {
            pthread_mutex_unlock(&jobs_info->lock);
            break;
        }
        if (jobs_info->size < max_queue_size) {
            job x = make_job(producer_number,thread_id);
            cout<<"***JOB PRODUCED***"<<endl;
            print(x);
            jobs_info->job_created++;
            insert_job(jobs_info,x);
        }
        pthread_mutex_unlock(&jobs_info->lock);
    }
}

void* consume_job(void* arg){
    //extracting the parameters from void* arg
    param* function_param = (param*)arg;
    data* jobs_info = function_param->jobs_info;
    int consumer_number = function_param->thread_number;
    int thread_id = gettid();
    int total_jobs = function_param->total_jobs;
    while(1) {
        lag(rand()%4);
        pthread_mutex_lock(&jobs_info->lock);
        if (jobs_info->job_completed >= total_jobs) {
            pthread_mutex_unlock(&jobs_info->lock);
            break;
        }
        if (jobs_info->size > 0) {
            job x = delete_job(jobs_info);
            cout<<"***JOB CONSUMED***"<<endl;
            cout<<"CONSUMER NUMBER "<<consumer_number<<endl;
            cout<<"CONSUMER THREAD ID "<<thread_id<<endl;
            print(x);
            lag(x.compute_time);
            jobs_info->job_completed++;
        }
        pthread_mutex_unlock(&jobs_info->lock);
    }
}

//Global Variables which are used as shared memory
data info = initialize();

int main() {
    srand(time(0));
    int NP, NC, total_jobs;
    cin>>NP;
    cin>>NC;
    cin>>total_jobs;

    auto begin = std::chrono::high_resolution_clock::now();

    pthread_t tids[NP + NC + 1]; //using as indexed from 1
    param args_threads[NP + NC + 1]; //using as indexed from 1

    for(int i = 1; i <= NP; i++) {
        args_threads[i].jobs_info = &info;
        args_threads[i].thread_number = i;
        args_threads[i].total_jobs = total_jobs;
        
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&tids[i],&attr, produce_job, &args_threads[i]);
    }
    for(int i = NP + 1; i <= NP + NC; i++) {
        args_threads[i].jobs_info = &info;
        args_threads[i].thread_number = i - NP;
        args_threads[i].total_jobs = total_jobs;
        
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&tids[i],&attr, consume_job, &args_threads[i]);
    }

    for (int i = 1; i <= NP+NC; i++)
        pthread_join(tids[i], NULL);

    auto end = std::chrono::high_resolution_clock::now();
    auto time_spent = std::chrono::duration_cast<std::chrono::microseconds>(end-begin);
    cout<<"Time spent in seconds is "<<time_spent.count()/1000000<<endl;
    return 0;
}