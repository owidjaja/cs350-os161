#include "assignment.h"

void
consume_enter(struct resource *resource)
{
    pthread_mutex_lock(&resource->mutex);
    while (resource->num_producers * resource->ratio < (resource->num_consumers+1)){
        pthread_cond_wait(&resource->cond, &resource->mutex);
    }

    resource->num_consumers += 1;
    pthread_cond_signal(&resource->cond);
    // pthread_mutex_unlock(&resource->mutex);
}

void
consume_exit(struct resource *resource)
{
    // pthread_mutex_lock(&resource->mutex);
    // while (resource->num_producers * resource->ratio < resource->num_consumers-1){
    //     pthread_cond_wait(&resource->cond, &resource->mutex);
    // }

    resource->num_consumers -= 1;
    pthread_cond_signal(&resource->cond);
    pthread_mutex_unlock(&resource->mutex);
}

void
produce_enter(struct resource *resource)
{
    pthread_mutex_lock(&resource->mutex);
    // while ((resource->num_producers+1) * resource->ratio < resource->num_consumers){
    //     pthread_cond_wait(&resource->cond, &resource->mutex);
    // }

    resource->num_producers += 1;
    pthread_cond_signal(&resource->cond);
    // pthread_mutex_unlock(&resource->mutex);
}

void
produce_exit(struct resource *resource)
{
    // pthread_mutex_lock(&resource->mutex);
    while ((resource->num_producers-1) * resource->ratio < resource->num_consumers){
        pthread_cond_wait(&resource->cond, &resource->mutex);
    }

    resource->num_producers -= 1;
    pthread_cond_signal(&resource->cond);
    pthread_mutex_unlock(&resource->mutex);
}