#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <threads.h>

atomic_size_t acnt;
size_t cnt;

int f(void *thr_data) {
    for(int n = 0; n < 1000; ++n) {
        cnt--;
        acnt--;
    }
    return 0;
}

int main(void) {
    acnt = 20000;
    cnt = 20000;
    int nthreads = 10;
    int i;
    // thrd_t thr[10];
    // for(int n = 0; n < 10; ++n)
    //     thrd_create(&thr[n], f, NULL);
    // for(int n = 0; n < 10; ++n)
    //     thrd_join(thr[n], NULL);
    pthread_t pthreads[nthreads];
    for(i = 0; i < nthreads; i++) {
        pthread_t p;
        pthreads[i] = p;
    }
    for(i = 0; i < nthreads; i++) {
        pthread_create(&pthreads[i], NULL, f, NULL);
    }
    for(i = 0; i < nthreads; i++) {
        pthread_join(pthreads[i], NULL);
    }

    printf("The atomic counter is %zu\n", acnt);
    printf("The non-atomic counter is %zu\n", cnt);
}
