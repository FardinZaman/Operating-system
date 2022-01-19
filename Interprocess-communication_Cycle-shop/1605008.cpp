#include<iostream>
#include<pthread.h>
#include<cstdio>
#include<unistd.h>
#include<semaphore.h>
#include<cstring>
#include<time.h>
#include<random>
#include<queue>

using namespace std;

#define cyclists 10
#define servicemen 3
#define bill_counters 2

pthread_mutex_t service_room[servicemen] ;
pthread_mutex_t service_finished , paid_bill ;

sem_t bill_counter_full ;

queue<char*> served ;
queue<char*> paid ;

void* garage(void* the_cycle)
{
    int service_time;

    for(int i=0 ; i<servicemen ; i++)
    {
        if(i==0)
        {
            while(true)
            {
                pthread_mutex_lock(&service_room[i]);

                pthread_mutex_lock(&paid_bill);
                if(paid.size() == 0)
                {
                    pthread_mutex_unlock(&paid_bill);
                    break;
                }

                pthread_mutex_unlock(&paid_bill);
                pthread_mutex_unlock(&service_room[i]);
            }
        }

        printf("%s started taking service from serviceman %d\n" , (char*)the_cycle , i+1);
        service_time = random()%4;
        sleep(service_time);
        printf("%s finished taking service from serviceman %d\n", (char*)the_cycle , i+1);

        if(i != servicemen-1)
        {
            pthread_mutex_lock(&service_room[i+1]);
        }

        pthread_mutex_unlock(&service_room[i]);
    }


    pthread_mutex_lock(&service_finished);
    served.push((char*)the_cycle);
    pthread_mutex_unlock(&service_finished);


    sem_wait(&bill_counter_full);

    pthread_mutex_lock(&service_finished);
    char* top_cycle = served.front();
    served.pop();
    pthread_mutex_unlock(&service_finished);

    int bill_time;

    printf("%s started paying the service bill\n" , top_cycle);

    bill_time = random()%5;
    sleep(bill_time);

    pthread_mutex_lock(&paid_bill);
    paid.push(top_cycle);
    pthread_mutex_unlock(&paid_bill);

    printf("%s finished paying the service bill\n" , top_cycle);

    sem_post(&bill_counter_full);


    for(int i=0 ; i<servicemen ; i++)
    {
        pthread_mutex_lock(&service_room[i]);
    }

    pthread_mutex_lock(&paid_bill);
    char* cleared_cycle = paid.front();
    paid.pop();
    pthread_mutex_unlock(&paid_bill);

    int pass_time;
    pass_time = random()%2;
    sleep(pass_time);

    printf("%s has departed\n" , cleared_cycle);

    for(int i=0 ; i<servicemen ; i++)
    {
        pthread_mutex_unlock(&service_room[i]);
    }


    return NULL;

}

int main()
{
    int res ;

    for(int i=0 ; i<servicemen ; i++)
    {
        res = pthread_mutex_init(&service_room[i] , NULL);

        if(res != 0)
        {
            printf("failed initializing serviceman %d mutex\n" , i+1);
            return 0;
        }
    }

    res = pthread_mutex_init(&service_finished , NULL);
    if(res != 0)
    {
        printf("failed initializing served queue access mutex\n");
        return 0;
    }

    res = pthread_mutex_init(&paid_bill , NULL);
    if(res != 0)
    {
        printf("failed initializing paid queue access mutex\n");
        return 0;
    }

    res = sem_init(&bill_counter_full , 0 , bill_counters);
    if(res != 0)
    {
        printf("failed initializing bill counter semaphore\n");
        return 0;
    }


    pthread_t cycles[cyclists];
    for(int i=0 ; i<cyclists ; i++)
    {
        char* cyclist = new char[3];
        strcpy(cyclist , to_string(i+1).c_str());
        res = pthread_create(&cycles[i] , NULL , garage , cyclist);

        if(res != 0)
        {
            printf("failed creating cyclist %d process/thread\n" , i+1);
            return 0;
        }
    }

    for(int i=0 ; i<cyclists ; i++)
    {
        void* result;
        res = pthread_join(cycles[i] , &result);

        if(res != 0)
        {
            printf("failed joining thread for number %d cyclist\n" , i+1);
            return 0;
        }
    }


    for(int i=0 ; i<servicemen ; i++)
    {
        res = pthread_mutex_destroy(&service_room[i]);

        if(res != 0)
        {
            printf("failed destroying serviceman %d mutex\n" , i+1);
            return 0;
        }
    }

    res = pthread_mutex_destroy(&service_finished);
    if(res != 0)
    {
        printf("failed destroying served queue access mutex\n");
        return 0;
    }

    res = pthread_mutex_destroy(&paid_bill);
    if(res != 0)
    {
        printf("failed destroying paid queue access mutex\n");
        return 0;
    }

    res = sem_destroy(&bill_counter_full);
    if(res != 0)
    {
        printf("failed destroying bill counter semaphore\n");
        return 0;
    }


    return 0;
}
