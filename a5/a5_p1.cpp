#include <stdio.h>
#include <iostream>
#include <unistd.h> 
#include <stdlib.h> 
#include <time.h>
#include <pthread.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/time.h>
#include <chrono>
#include <bits/stdc++.h>
using namespace std;

#define MAX_SIZE_PRIORITY_QUEUE 100

int max_queue_size = 8;

typedef struct {
    int process_id;
    int producer_number;
    int priority;
    int compute_time;
    int job_id;
} job;

typedef struct {
    job job_queue[MAX_SIZE_PRIORITY_QUEUE];
    int size;
    int job_created;
    int job_completed;
    pthread_mutex_t lock;
} data;

data* initialize(int shmid) {
    data* jobs_info = (data*)shmat(shmid,(void*)0,0);
    jobs_info->size = 0;
    jobs_info->job_created = 0;
    jobs_info->job_completed = 0;
    pthread_mutexattr_t lock_attr;
	pthread_mutexattr_init(&lock_attr);
	pthread_mutexattr_setpshared(&lock_attr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&jobs_info->lock, &lock_attr);
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

job make_job(int producer_number, int process_id) {
    job x;
    x.compute_time = rand()%4 + 1;
    x.job_id = rand()%100000 + 1;
    x.priority = rand()%10 + 1;
    x.producer_number = producer_number;
    x.process_id = process_id;
    return x; 
}

void print(job x) {
    cout<<"JOB ID "<<x.job_id<<endl;
    cout<<"PRODUCER PROCESS ID "<<x.process_id<<endl;
    cout<<"PRIORITY "<<x.priority<<endl;
    cout<<"PRODUCER NUMBER "<<x.producer_number<<endl;
    cout<<"COMPUTE TIME "<<x.compute_time<<endl;
}

void lag(int time) {
    clock_t start = clock();
    while(clock() < start + time * CLOCKS_PER_SEC);
}

void produce_job(data* jobs_info, int producer_number, int process_id, int total_jobs) {
    while(1) {
        lag(rand()%4);
        pthread_mutex_lock(&jobs_info->lock);
        if (jobs_info->job_created >= total_jobs) {
            pthread_mutex_unlock(&jobs_info->lock);
            break;
        }
        if (jobs_info->size < max_queue_size) {
            job x = make_job(producer_number,process_id);
            cout<<"***JOB PRODUCED***"<<endl;
            print(x);
            jobs_info->job_created++;
            insert_job(jobs_info,x);
        }
        pthread_mutex_unlock(&jobs_info->lock);
    }
    return;
}

void consume_job(data* jobs_info, int consumer_number, int process_id, int total_jobs) {
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
            cout<<"CONSUMER PROCESS ID "<<process_id<<endl;
            print(x);
            lag(x.compute_time);
            jobs_info->job_completed++;
        }
        pthread_mutex_unlock(&jobs_info->lock);
    }
    return;
}

int main() {
    srand(time(0));
    int NP, NC, total_jobs;
    cin>>NP;
    cin>>NC;
    cin>>total_jobs;

    key_t key = 777;
    int shmid = shmget(key, sizeof(data), 0660|IPC_CREAT);
    if (shmid < 0) {
        cout<<"lol1"<<endl;
        exit(0);
    }
    data* jobs_info = initialize(shmid);
    auto begin = std::chrono::high_resolution_clock::now();
    pid_t pid;
    for (int i = 1; i<=NP; i++) {
        pid = fork();
        if (pid < 0) {
            cout<<"lol2"<<endl;
            exit(0);
        }
        else if (pid == 0) { //inside child process
            srand(time(0) ^ i*2);
            produce_job(jobs_info,i,getpid(),total_jobs);
            exit(0);
        }
    }
    for (int i = 1; i<=NC; i++) {
        pid = fork();
        if (pid < 0) {
            cout<<"lol3"<<endl;
            exit(0);
        }
        else if (pid == 0) { //inside child process
            srand(time(0) ^ i*3);
            consume_job(jobs_info,i,getpid(),total_jobs);
            exit(0);
        }
    }
    while(1) {
        lag(2);
        pthread_mutex_lock(&jobs_info->lock);
        if (jobs_info->job_created == jobs_info->job_completed && jobs_info->job_created == total_jobs) {
            auto end = std::chrono::high_resolution_clock::now();
            auto time_spent = std::chrono::duration_cast<std::chrono::microseconds>(end-begin);
            cout<<"Time spent in seconds is "<<time_spent.count()/1000000<<endl;
            pthread_mutex_unlock(&jobs_info->lock);
            break;
        }
        pthread_mutex_unlock(&jobs_info->lock);
    }
    return 0;
}