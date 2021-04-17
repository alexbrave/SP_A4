// ADD FILE HEADER COMMENT


#include "../inc/chatClient.h"

// ADD METHOD HEADER COMMENT
int getOrCreateCursorSem(void)
{
    // We define our acquire and release operation structs
    struct sembuf acquire_operation = { 0, -1, SEM_UNDO };
    struct sembuf release_operation = { 0, 1, SEM_UNDO };

    // Our semaphore initial value is 1, indicating that only 1 process can use the resource at a time
    unsigned short init_values[NUM_OF_SEMS] = { SEM_INITAL_VALUE };

    // We want to get a key using ftok for our semaphore to be public for all processes
    // that want to use it
    key_t semaphoreKey = 0;

    // We'll use the semaphore key that we got to then get the semaphore ID
    int semaphoreID = 0;

    // Here we'll get the key via which we'll be able to get the semaphore
    semaphoreKey = ftok(CURRENT_DIRECTORY, SEM_FTOK_ID);

    // Check if the semaphore already exists
    semaphoreID = semget(semaphoreKey, NUM_OF_SEMS, CHECK_SEM_EXISTS);

    // Create a semaphore if one doesn't already exist
    if(semaphoreID == OPERATION_FAILED)
    {
        semaphoreID = semget(semaphoreKey, NUM_OF_SEMS, (IPC_CREAT | 0660));
        if (semaphoreID == OPERATION_FAILED)
        {
            return OPERATION_FAILED;
        }

        // Initialize the semaphore to 1, the value in our init_values
        if (semctl(semaphoreID, 0, SETALL, init_values) == OPERATION_FAILED) 
        {
            return OPERATION_FAILED;
        }
    } 

    // If we made it this far, that means the semaphore exists, 
    // or was created and initialized successfully!
    // Attempt to acquire the semaphore
    
    // We will try to get the semaphore, if that fails, we'll assume it's gone
    // and return that we were unsuccessfull
    if (semop(semaphoreID, &acquire_operation, NUM_SOP_STRUCTS) == OPERATION_FAILED)
    {
        return OPERATION_FAILED;
    }

    // Otherwise the semaphore was created and obtained successfully
    return OPERATION_SUCCESS;

}


/*
* FUNCTION 		: releaseLogSem
* DESCRIPTION 	: Releases the log file semaphore currently held by this process
* PARAMETERS 	: void : this function takes no parameters
* RETURNS 		: int : returns whether the operation succeeded or failed
*/
int releaseCursorSem(void)
{
    // Here we define our acquire and release semaphore operation structs
    struct sembuf acquire_operation = { 0, -1, SEM_UNDO };
    struct sembuf release_operation = { 0, 1, SEM_UNDO };

    // We want to get a key using ftok for our semaphore to be public for all processes
    // that want to use it
    key_t semaphoreKey = 0;

    // We'll use the semaphore key that we got to then get the semaphore ID
    int semaphoreID = 0;

    // We use this counter while trying to get the semaphore
    int getSemAttmptCntr = 0;

    semaphoreKey = ftok(CURRENT_DIRECTORY, SEM_FTOK_ID);

    // Check if the semaphore already exists, hopefully it does
    semaphoreID = semget(semaphoreKey, NUM_OF_SEMS, CHECK_SEM_EXISTS);

    // Attempt to release the semaphore
    if (semop (semaphoreID, &release_operation, 1) == OPERATION_FAILED) 
    {
        // If releasing the semaphore is unsucessful, there is a problem
        return OPERATION_FAILED;
    }
    else
    {
        return OPERATION_SUCCESS;
    }

}


// ADD FUNCTION TO DELETE SEMAPHORE
void deleteCursorSem(void)  
{
    // We want to get a key using ftok for our semaphore to be public for all processes
    // that want to use it
    key_t semaphoreKey = 0;

    // We'll use the semaphore key that we got to then get the semaphore ID
    int semaphoreID = 0;

    int returnCode = 0;
    
    semaphoreKey = ftok(CURRENT_DIRECTORY, SEM_FTOK_ID);

    // Check if the semaphore already exists, hopefully it does
    semaphoreID = semget(semaphoreKey, NUM_OF_SEMS, CHECK_SEM_EXISTS);

    // Delete semaphore
    returnCode = semctl(semaphoreID, 0, IPC_RMID);
}
