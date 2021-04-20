#include<pthread.h>
#include<time.h>

int s = 0;

void *add(void *args){
    int i;
    for(i = 0; i < 500000; i++){
        s ++;
    } 
    return NULL;
}



int main(){
    pthread_t th1, th2;
    pthread_create(&th1, NULL, add, NULL);
    pthread_create(&th2, NULL, add, NULL);
    pthread_join(th1, NULL);
    pthread_join(th2, NULL);
    printf("%d\n", s);
    return 0;
}
/*注1：这一段代码最后的输出为557193，而不是1000000，这恰恰证明了两个线程是并行的（假设s=0时，第一个线程先读s，然后第二个线程再读s，接着第一个线程另s+=1，第二个线程也另s+=1，最后第一个线程将s=1写回
内存，此时内存中s=1.接着第二个线程也将s=1写入内存，此时s还是等于1，也就是说两个线程分别做了s+=1操作后，s=1）
注2：如果另for循环执行5000次，最后结果会是10000，也就是说此时两个线程没有并行，可能是循环次数太少，编译的时候没有将两个线程分配到两个核上
