/// @file shared_memory.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#include "shared_memory.h"


//Shared memory
SharedMemory* createSharedMemory(size_t size){ //, int shm_flags ){
  SharedMemory *mem = malloc( sizeof(SharedMemory) );
  mem->id = alloc_shared_memory(IPC_PRIVATE, size);
  mem->ptr = get_shared_memory(mem->id, 0); // shm_flags);  
  return mem;
}
 
void destroySharedMemory(SharedMemory *shm){
  //TODO: check order free/remove
  free_shared_memory(shm->ptr);
  remove_shared_memory(shm->id);
  free(shm);
}



MutexSharedMemory* createMutexSharedMemory(size_t size, int sem_cnt ){
  MutexSharedMemory *mx_mem = malloc( sizeof(MutexSharedMemory) );
  mx_mem->mem = createSharedMemory(size);
  mx_mem->semaphore = createSemaphore(sem_cnt);
  return mx_mem;
}


void destroyMutexSharedMemory(MutexSharedMemory* sh_mem){
  destroySharedMemory(sh_mem->mem);
  destroySemaphore(sh_mem->semaphore);
  free(sh_mem);
}




//utility methods for SHM 
int alloc_shared_memory(key_t shmKey, size_t size) {
    // get, or create, a shared memory segment
    int shmid = shmget(shmKey, size, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmid == -1)
        errExit("shmget failed");

    return shmid;
}

void *get_shared_memory(int shmid, int shmflg) {
    // attach the shared memory
    void *ptr_sh = shmat(shmid, NULL, shmflg);
    if (ptr_sh == (void *)-1)
        errExit("shmat failed");

    return ptr_sh;
}

void free_shared_memory(void *ptr_sh) {
    // detach the shared memory segments
    if (shmdt(ptr_sh) == -1)
        errExit("shmdt failed");
}

void remove_shared_memory(int shmid) {
    // delete the shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        errExit("shmctl failed");
}
