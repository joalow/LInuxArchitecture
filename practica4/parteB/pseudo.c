DEFINE_MUTEX(mtx);
#define MAX_KBUF 64
#define MAX_CBUFFER_LEN 64

condvar prod,cons;
struct semaphore sem_cons, sem_prod;
int prod_count=0,cons_count=0;
struct kfifo cbuffer;

void fifoproc_open(bool abre_para_lectura) {
    if(abre_para_lectura){
         lock(mtx);
        cons_count++;
        while(prod_count==0)
            cond_wait(cons,mtx);
        cond_signal(cons);
        unlock(mtx);
    }else{
         lock(mtx);
        prod_count++;
        while(cons_count == 0)
            cond_wait(prod,mtx);
        cond_signal(prod);
        unlock(mtx);
    }   
}

int fifoproc_write(char* buff,int len) {
    char kbuffer[MAX_KBUF];

    if (len> MAX_CBUFFER_LEN || len> MAX_KBUF) 
        return Error;
    if(copy_from_user(kbuffer,buff,len))
        return Error;
    
    lock(mtx);
    /* Esperar hastaque haya hueco para insertar(debe haber consumidores) */
    while(kfifo_avail(&cbuffer)< len && cons_count > 0){
        cond_wait(prod,mtx);
    }
    /* Detectar fin de comunicación por error(consumidor cierra FIFO antes) */
    if (cons_count == 0) {
        unlock(mtx);
        return-EPIPE;
    }
    kfifo_in(&cbuffer,kbuffer,len);/* Despertar a posible consumidor bloqueado*/
    cond_signal(cons);
    unlock(mtx);
    return len;
}

int fifoproc_read(const char* buff,int len) {
     char kbuffer[MAX_KBUF];

    if (len> MAX_CBUFFER_LEN || len> MAX_KBUF) 
        return Error;

    lock(mtx);
    while(kfifo_len(&cbuffer)<len && prod_count>0){
        cond_wait(cons,mtx);
    }
    if (prod_count == 0 && kfifo_is_empty(&cbuffer)) {
        unlock(mtx);
        return 0;
    }
    kfifo_out(&cbuffer,kbuffer,len);/* Despertar a posible productor bloqueado*/
    cond_signal(prod);
    unlock(mtx);
    copy_to_user(buff,&kbuffer,len);
    return len;
}

void fifoproc_release(bool lectura) {
    lock(mtx);
    if(lectura){
        cons_count--;
        cond_signal(prod);
    }else{
        prod_count--;
        cond_signal(cons);
    }

    if(cons_count == 0 && prod_count == 0)
        kfifo_reset(&cbuffer);

    unlock(mtx);
}