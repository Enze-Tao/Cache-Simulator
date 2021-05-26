#include "cachelab.h"
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <limits.h>

typedef struct
{
    int tag;
    int valid_bit;
    int lru_time;//用于记录每行最近使用的时间
}cacheline;

int s;//组标志位数
int e;//每组包含的行数
int b;//块偏移位数
int setnum;//cache中包含的组数
int linenum;//组中包含的行数
int set_v;//是否在指令中加入-v参数
int hit,miss,evic;//分别记录hit,miss,eviction的次数
char* addr;//记录读取trace文件的位置
cacheline** cache;//模拟缓存

void readcommand(int argc, char **argv){   
    char ch;
    while((ch=getopt(argc,argv,"vs:E:b:t:"))!=-1){
        switch (ch)
        {
            case 'v':
                set_v=1;
                break;
            case 's':
                s=atoi(optarg); 
                break;
            case 'E':
                e=atoi(optarg);  
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                addr=optarg; 
                break;     
            default:
                printf("Wrong command!");
                break;
        }
    }
    return;
}//从命令行中取得程序的输入参数

void create_cache()
{
    setnum=1<<s;
    linenum=e;
    cache=(cacheline**)malloc(sizeof(cacheline*)*setnum);
    if(!cache){
        printf("Cache memory allocating error!\n");
        exit(0);
    }
    for(int i=0;i<setnum;i++){
        cache[i]=(cacheline*)malloc(sizeof(cacheline)*linenum);
        for(int j=0;j<linenum;j++){
            cache[i][j].lru_time=0;
            cache[i][j].valid_bit=0;
        }        
    }
    return;
}//为cache分配内存空间

void free_cache()
{
    for(int i=0;i<setnum;i++)
        free(cache[i]);
    free(cache);
}//释放分配给cache的内存

int getTag(int s,int b,int address){
    return address=address>>(s+b);
}//从trace文件读取的指令的地址项中取得tag标志位

int getSet(int s,int b,int address){
    int ret,mask;
    address=address>>b;
    mask=(1<<s)-1;
    ret=address&mask;
    return ret;
}//从trace文件读取的指令的地址项中取得组索引位

void LRUupdate(){
    for(int i=0;i<setnum;i++)
        for(int j=0;j<linenum;j++)
            if(cache[i][j].valid_bit==1)
                cache[i][j].lru_time++;
}//执行完一条trace指令后用于更新每行对应的LRU计数

int LRUreplace(int set){
    int max=INT_MIN,res;
    for(int i=0;i<linenum;i++){
        if(cache[set][i].lru_time>max){
            max=cache[set][i].lru_time;
            res=i;
        }
    }
    return res;    
}//用于确定LRU替换算法的替换行

int isHit(int set_bit,int tag_bit){
    for(int i=0;i<linenum;i++){
        if(cache[set_bit][i].valid_bit==1){
            if(cache[set_bit][i].tag==tag_bit){
                cache[set_bit][i].lru_time=0;
                return 1;
            }
        }
    }
    return 0;    
}//用于判断是否命中，若命中完成对应行的lru计数更新并返回1，否则返回0

int find_line(int set_bit){
    int line=-1;
    for(int i=0;i<linenum;i++){
        if(cache[set_bit][i].valid_bit==0){
            line=i;
            break;
        }
    }
    return line;
}//在未命中情况下用于返回对应组可用的空行，若无空行返回-1

void access_mem(int set_bit,int tag_bit){
	int index1,index2;

	if(isHit(set_bit,tag_bit)){
        hit++;
        if(set_v)
            printf("hit ");
        return;
    }//cache hit后的处理

    miss++;//若未命中则miss计数+1
    if(set_v){
        printf("miss ");
    }
    index1=find_line(set_bit);//寻找对应组是否有可用的空行
    if(index1!=-1){
        cache[set_bit][index1].valid_bit = 1;
		cache[set_bit][index1].tag = tag_bit;
		cache[set_bit][index1].lru_time = 0;            		
		return ;
    }
	
    evic++;//未命中且对应组没有空行，eviction计数+1
	if(set_v){
        printf("eviction ");
    }    
    index2=LRUreplace(set_bit);//确定对应组需要被替换的行
	cache[set_bit][index2].tag = tag_bit;
	cache[set_bit][index2].lru_time = 0;    
	return ;
   
}//模拟完成一次访问内存操作

int main(int argc, char **argv)
{
    FILE* fp;
    char oper;
    long address,bytes;
    readcommand(argc,argv);
    create_cache();
    fp=fopen(addr,"r");
    while (fscanf(fp," %c %lx,%ld",&oper,&address,&bytes)!=EOF){
        if(oper=='I')
            continue;
        int set,tag;
        set=getSet(s,b,address);//取得address中的set位
        tag=getTag(s,b,address);//取得address中的tag位
        if(set_v)
            printf("%c %lx,%ld ",oper,address,bytes);
        switch (oper)
        {
            case 'L':
                access_mem(set,tag);
                break;
            case 'S':
                access_mem(set,tag);
                break;
            case 'M':
                access_mem(set,tag);
                access_mem(set,tag);
                break;
        }
        LRUupdate();//每次执行指令后更新cache中的全部有效行
        if(set_v)
            printf("\n");
    }     
    fclose(fp);
    free_cache();
    printSummary(hit, miss, evic);
    return 0;
}
