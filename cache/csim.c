#include "cachelab.h"
#include <string.h>
# include <stdio.h>
# include <stdlib.h>
/*
name: 朱文杰
ID: ics517021910799
*/
int hitn=0, missn=0, evictn=0;


//正确写法是MTF，不过C写起来太累了，这里直接记录最后一次访问的位次n，然后遍历集找n最小的地方,时间复杂度O(e)。
int visittime=0;

typedef struct Information
{
    char* file;
    size_t s;
    size_t E;
    size_t b;
} Info;

Info ParseShell(int argc, char* argv[])
{
    Info info;
    if(argc==10){
        //verbose
        info.file=argv[9];
        info.s=atoi(argv[3]);
        info.E=atoi(argv[5]);
        info.b=atoi(argv[7]);
    }
    else if(argc==9){
        //normal
        info.file=argv[8];
        info.s=atoi(argv[2]);
        info.E=atoi(argv[4]);
        info.b=atoi(argv[6]);
    }
    else{
        printf("Wrong params!");
    }
    return info;
}

size_t GetBlockIndex(size_t address,Info* info)
{
    return address & (2<<(info->b-1));//取后b位
}

size_t GetSetIndex(size_t address,Info* info)
{
    return address>>info->b & (2<<(info->s-1));//取中间s位
}

size_t GetTag(size_t address,Info* info)
{
    return address>>(info->b+info->s);//取前32-b-s位
}

typedef unsigned char byte;

typedef struct Line{
    size_t visit;
    size_t b;
    size_t tag;
    int valid;
    byte* blocks;//vector
}Line;

typedef struct Set{
    size_t e;
    Line* lines;//list
}Set;

typedef struct Cache{
    size_t s;
    Set* sets;//vector
}Cache;


void GenerateLine(Line* ret,size_t b)
{
    ret->visit=0;
    ret->b=b;
    ret->tag=0;
    ret->valid=0;
    ret->blocks=byte*(malloc(b));
}

void GenerateSet(Set* ret , size_t e, size_t b)
{
    ret->e = e;
    ret->lines=Line*(malloc(sizeof(Line)*e));
    for(int i=0; i<e; i++)
        GenerateLine(ret->lines[i],b);
}

Cache* GenerateCache(size_t s, size_t e, size_t b)
{
    Cache* cache=Cache*(malloc(sizeof(Cache)));
    cache->s= s;
    cache->sets= Set*(malloc(sizeof(Set)*s));
    for(int i=0; i<s; i++)
        GenerateSet(cache->sets[i],e,b);
    return cache;
}

Line* GetLRU(Set* set, size_t e){
    int cur=0, minv=0, mini=0;
    for(int i=0;i<e;i++){
        cur=set->lines[i].visit;
        if(cur<minv){
            mini=i;
            minv=cur;
        }
    }
    return set->lines[mini];
}

int checkHit(size_t address, Line* line)
{   
    visittime++;
    line->visit=visittime;
    int tag=GetBlockIndex(address,info);
    if(line->valid==1)
    {
        if(line->tag==tag)
        return 1;//hit
        else{
            line->tag=tag;
            return -1;//evict
        }
    }
    else{
        line->tag=tag;//miss
        line->valid=1;
        return 0;
    }
}

int visit(Info* info, Cache* cache, size_t address)
{
    int si=GetSetIndex(address,info);
    Set* curset= cache->sets[si];
    Line* curline= GetLRU(curset,info->E);
    return checkHit(address,curline);
}


int main(int argc,char *argv[]) 
{
    Info info= ParseShell(argc,argv);
    printSummary(hit, miss, evict);
    return 0;
}
