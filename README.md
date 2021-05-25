This is the handout directory for the Cache Lab.

## Running the autograders:

Before running the autograders, compile your code:
    linux> make

Check the correctness of your simulator:
    linux> ./test-csim

## Files:

### You will modifying and handing in these two files
csim.c       Your cache simulator

### Tools for evaluating your simulator and transpose function
Makefile     Builds the simulator and tools
README       This file
cachelab.c   Required helper functions
cachelab.h   Required header file
csim-ref*    The executable reference cache simulator
test-csim*   Tests your cache simulator
traces/      Trace files used by test-csim.c

## csim.c简介

本程序将cache中的行作为cacheline结构体，每一行有tag（标志位）、valid_bit（有效位）、lru_time（每行最近使用的时间）三个成员，而模拟的cache其实就是一个二维的cacheline结构体数组。

### 各函数功能简介：

1. *void readcommand(int argc, char argv)*：用于读取linux的命令行参数，可读取-v（可选）、-s、-E、-b、-t参数。其中-v是可选参数，-s、-E、-b、-t 则是必要参数。执行时通过命令行输入缓存参数和要读取的缓存文件，在命令行输出执行的结果。

	- -v：可选参数，用于将trace文件中的每一条指令的cache命中情况打印出来；
	- -s  <s>：64位16进制地址中的组索引位数；
	- -E  <E>：cache中每组包含的行数；
	- -b  <b>：64位16进制地址中的块（block）索引位数；
	- -t  <tracefile>：需要执行的valgrind程序生成的trace文件的地址
2. *void create_cache()*：用于根据从命令行输入中读取的s、e参数确定的组数和行数，用malloc函数为模拟cache分配内存空间。由于本程序将cache、setnum（组数）、linenum（行数）均设为全局变量，本函数不需要输入参数。
3. *void free_cache()*：用于在cache模拟结束后使用free函数释放分配给cache的内存。
4. *int getTag(int s,int b,int address)*：用于从trace文件读取的指令的地址项中取得tag标志位，其中输入参数s为地址中的组索引位数，b为地址中的块索引位数，address为地址项，返回值为tag标志位。
5. *int getSet(int s,int b,int address)*：用于从trace文件读取的指令的地址项中取得组索引位，其中输入参数s为地址中的组索引位数，b为地址中的块索引位数，address为地址项返回值为组索引位。
6. *void LRUupdate()*：执行完每一条trace指令后运行，用于更新每行对应的LRU计数（lru_time)成员。
7. *int LRUreplace(int set)*：当需要将新数据存入cache且对应组无空行时，用于确定LRU算法对应的替换行。输入参数set为对应组号，返回值为可替换的行（lru_time值最大者）。
8. *int isHit(int set_bit,int tag_bit)*：用于根据组号和tag标志在cache中遍历对应组中的每一行并匹配tag标志位，判断是否命中。输入参数set_bit为对应组号，tag_bit为需要访问的内存地址对应的标志位。若命中则完成对应行的lru_time计数更新并返回1，否则返回0。
9. *int find_line(int set_bit)*:在cache未命中的情况下，用于查找对应组可用的空行。输入参数set_bit为对应组号，若查找到空行，返回值为改空行行号，否则返回值为-1。
10. *void assess_mem(int set_bit,int tag_bit)*：用于模拟完成一次访问内存操作。输入参数set_bit为对应组号，tag_bit为需要访问的内存地址对应的标志位。
