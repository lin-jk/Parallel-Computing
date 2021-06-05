#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#include <assert.h>
#include <time.h>

enum STATUS{Running, Send, Receive, Finish};
void main(int argc, char* argv[])
{
	enum STATUS Status = Running;

	if(argc == 1){
		printf("请在命令行输入需要计算的数\n");
		assert(0);//也可以用abort()或者exit(1) 
	}
	char *input_argv = (argv[1]);
    int core_num, core_id, num_to_compute;
	sscanf(input_argv, "%d", &num_to_compute); //先将命令行参数存入 num_to_compute  ,  将字符串转换成int可以用sscanf这个函数 
//	printf("num_to_compute = %d\nStatus is %d\n", num_to_compute, Status);
//	fflush(stdout);
    long long sum = 0, i = 0;
    int decision_whether_to_send = 2;
    int which_id_to_comm = 1;
    
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &core_id);
	MPI_Comm_size(MPI_COMM_WORLD, &core_num);
	
	clock_t start, finish;
	int batch_size = (num_to_compute / core_num); //因为是int类型，所以结果会是整数   另外这句话不能放在MPI_Init之前，因为core_num初始化为0，会出现整数除以0的情况 
    int start_num = batch_size * core_id;
	int end_number; 
	int round_bound = (int)ceil(sqrt(core_num));
	int round = 0;
	
	if(core_id == 0) start = clock();
	
	if(core_id == (core_num - 1)) {
		end_number = num_to_compute + 1;
	}
	else {
		end_number = start_num + batch_size; //判断一下 end_number是多少 
	}
	printf("core_id %d----------start_num = %d----------end_num = %d\n", core_id, start_num, end_number);
	fflush(stdout);
	for(i = start_num; i < end_number; i++){
		sum += i;
	}
    while(Status != Finish){
    	if(core_id % decision_whether_to_send == 0) Status = Receive;
    	else Status = Send;
    	switch(Status){
    		case Send :
    		{	int comm_id = core_id - which_id_to_comm;//计算消息要发给谁 
    			MPI_Send(&sum, 1, MPI_INT, comm_id, 0, MPI_COMM_WORLD);
    			Status =  Finish; //这里应该加一步判断消息是否发送成功，如果发送成功才将Status设置为Finish 
    			printf("core_id %d send sum = %d\n", core_id, sum);
    			fflush(stdout);
    			break;
    		}//如果在某个case下面声明变量的话，对象的作用域是在俩个花括号之间 也就是整个switch语句，其他的case语句也能看到，这样的话就可能导致错误,所以加了一个括号 
    		case Receive :
    		{	long long temp = 0;//temp用来存放接受的数 
    			int id_to_receive = core_id + which_id_to_comm;//计算接受哪个id的消息 
    			if(id_to_receive >= core_num){//如果开的进程数为7，经过计算发现core_6需要与core_7通信，但core_7并不存在，所以就会报错。但是不能用core_id是否等于core_num-1来决定是否break 
    				decision_whether_to_send *= 2;//因为如果开的进程数为6，发现第一轮的通信是没有问题的，第二轮时，core_4需要与core_6通信，core_6并不存在，就会报错 
    				which_id_to_comm *= 2;
    				break;
				} 
				MPI_Recv(&temp, 10, MPI_INT, id_to_receive, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
				sum += temp;
				decision_whether_to_send *= 2;//将这两个变量乘以2，用来做下一轮计算（decision_whether_to_send用来计算消息要发给谁，which_id_to_comm用来计算接受哪个id的消息） 
				which_id_to_comm *= 2;
				printf("core_id %d receive the sum = %d from core_%d\n", core_id, temp, id_to_receive);
				fflush(stdout);
				break;
			}
			default : break;
		}
		
		round ++;
		if(round >= round_bound) break;//这一步是core_0用来判断自己是否应该结束 
		
	}
	printf("core_id %d finish the task\n", core_id);
	if(core_id == 0){ //最后输出sum时一定是由0号进程输出的 
		finish = clock();
		printf("the sum is %d, and the computation time is %fs\n", sum, (double)(finish - start) / CLOCKS_PER_SEC);
	}
    MPI_Finalize(); 
} /* end main */
