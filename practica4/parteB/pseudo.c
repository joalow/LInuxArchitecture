DEFINE_MUTEX(mtx);
//condvar prod,cons;
struct semaphore sem_cons, sem_prod;
int prod_count=0,cons_count=0;
struct kfifo cbuffer;

void fifoproc_open(bool abre_para_lectura) {
     if(abre_para_lectura){
        cons_count++;
        lock(mtx);
        while(prod_count==0)
            cond_wait(cons,mtx);
        cond_signal(cons);
        unlock(mtx);
    }else{
        prod_count++;
        lock(mtx);
        while(cons_count == 0)
            cond_wait(prod,mtx);
        cond_signal(prod);
        unlock(mtx);
    }
    
}


/************************************************************************
 * *****************************************************************
 */

void cond_wait(){
    if(down_interruptible(&sem_mtx))
      return -EINTR;

    while(condicion==false){
        nr_waiting++;
        up(&sem_mtx); /*Libera el 'mutex' */
        /*se bloquea en la cola */
        if(down_interruptible(&sem_quee)){
            down(&sem_mtx);
            nr_waiting--;
            up(&sem_mtx);
            return -EINTR;
        }
        /* Adquiere el mutx */
        if(down_interruptible(&sem_mtx))
            return -EINTR;
    }
    /* Libera el mute */
    up(&sem_mtx);
}

void cond_signal(){
    if(down_interruptible(&sem_mtx))
      return -EINTR;

    if(nr_waiting > 0){
        /* Despierta 1 de los hilos bloqueados */
        up(&sem _mutex);
        nr_waiting--;
    }

    /* Libera el mute */
    up(&sem_mtx);
}



/***********************************************************************
 * *********************************************************
 */


int fifoproc_write(char* buff,int len) {
    char kbuffer[MAX_KBUF];

    if (len> MAX_CBUFFER_LEN || len> MAX_KBUF) 
        return Error;
    if(copy_from_user(kbuffer,buff,len))
        return Error;
    
    lock(mtx);/* Esperar hastaque haya hueco para insertar(debe haber consumidores) */
    while(kfifo_avail(&cbuffer)< len && cons_count > 0){
        cond_wait(prod,mtx);
    }/* Detectar fin de comunicación por error(consumidor cierra FIFO antes) */
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
    return len;
}

void fifoproc_release(bool lectura) {
    if(lectura)
        cons_count--;
    else
        prod_count--;

    if(cons_count == 0 && prod_count == 0)
        kfifo_reset(&cbuffer)
}
