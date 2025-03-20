#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg, ...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg, ...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void *threadfunc(void *thread_param) {

    struct thread_data *td = (struct thread_data *) thread_param;

    // sleep before obtaining lock
    usleep(td->wait_to_obtain_ms * 1000);

    // obtain lock
    int lock_rc = pthread_mutex_lock(td->mutex);

    if (lock_rc != 0) {
        syslog(LOG_INFO, "pthread_mutex_lock failed with %d", lock_rc);
        td->thread_complete_success = false;
        return thread_param;
    }
    // sleep after obtaining lock
    usleep(td->wait_to_release_ms * 1000);

    // unlock
    int unlock_rc = pthread_mutex_unlock(td->mutex);

    if (unlock_rc != 0) {
        syslog(LOG_INFO, "pthread_mutex_unlock failed with %d", unlock_rc);
        td->thread_complete_success = false;
        return thread_param;
    }

    // set success
    td->thread_complete_success = true;

    return thread_param;
}


bool
start_thread_obtaining_mutex(
        pthread_t *thread,
        pthread_mutex_t *mutex,
        int wait_to_obtain_ms,
        int wait_to_release_ms) {

    // allocate memory for thread data
    struct thread_data *td = malloc(sizeof(struct thread_data));

    if (NULL == td) {
        syslog(LOG_ERR, "Could not allocate memory for thread_data");
        return false;
    }

    td->mutex = mutex;
    td->wait_to_obtain_ms = wait_to_obtain_ms;
    td->wait_to_release_ms = wait_to_release_ms;
    td->thread_complete_success = false;

    // start the thread
    int rc = pthread_create(thread, NULL, threadfunc, td);

    if (rc != 0) {
        syslog(LOG_INFO, "pthread_create failed with %d", rc);
        return false;
    }

    return true;
}

