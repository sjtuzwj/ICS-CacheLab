#include "cachelab.h"
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define True 1
#define False !True
#define Hit 1
#define Miss 0
#define Evict -1

/*
name: 朱文杰
ID: ics517021910799
*/


//记录内存访问状态次数
int hitn = 0, missn = 0, evictn = 0;

//正确写法是MTF，这里直接记录最后一次访问的位次n，然后遍历集找n最小的地方,时间复杂度O(e)。
int visittime = 0;

//存储信息
struct Info
{
    char* file;
    unsigned long s;
    unsigned long E;
    unsigned long b;
};
//获取命令行参数
struct Info* ParseShell(int argc, char* argv[])
{
    struct Info * info = (struct Info*)malloc(sizeof(struct Info));
    info->file = argv[8];
    info->s = atoi(argv[2]);
    info->E = atoi(argv[4]);
    info->b = atoi(argv[6]);
    return info;
}
//获取集合索引
unsigned long GetSetIndex(unsigned long address, struct Info* info)
{
    return (address & ((0x1 << (info->s+ info->b)) - 1)) >> info->b;//取中间s位
}
//获取标签
unsigned long GetTag(unsigned long address, struct Info* info)
{
    return  address >> (info->b + info->s);//取前32-b-s位
}

//模拟一行
struct Line {
    unsigned long visit;
    unsigned long b;
    unsigned long tag;
    int valid;
    char* blocks;//vector
};

//模拟一个集合
struct Set {
    unsigned long e;
    struct Line* lines;//list
};

//模拟cache
struct Cache {
    unsigned long s;
    struct Set* sets;//vector
};

//行的构造函数
void GenerateLine(struct Line* ret, unsigned long b)
{
    ret->visit = 0;
    ret->b = b;
    ret->tag = 0;
    ret->valid = False;
    ret->blocks = (char*)(malloc(1 << b));
}
//集合的构造函数
void GenerateSet(struct Set* ret, unsigned long e, unsigned long b)
{
    ret->e = e;
    ret->lines = (struct Line*)(malloc(sizeof(struct Line)*e));
    for (int i = 0; i<e; i++)
        GenerateLine(&ret->lines[i], b);
}
//cache的构造函数
struct Cache* GenerateCache(unsigned long s, unsigned long e, unsigned long b)
{
    struct Cache* cache = (struct Cache*)(malloc(sizeof(struct Cache)));
    cache->s = s;
    int size = 1 << s;
    cache->sets = (struct Set*)(malloc(sizeof(struct Set)*size));
    for (int i = 0; i<size; i++)
        GenerateSet(&cache->sets[i], e, b);
    return cache;
}
//获取LRU行的指针
struct Line* GetLRU(struct Set* set, unsigned long e) {
    int cur = 0, minv = INT_MAX, mini = -1;
    for (int i = 0; i<e; i++) {
        cur = set->lines[i].visit;
        if (cur<minv) {
            mini = i;
            minv = cur;
        }
    }
    return &set->lines[mini];
}
//检查内存访问状态
int checkHit(struct Info* info, unsigned long address, struct  Line* line)
{
    visittime++;
    line->visit = visittime;
    int tag = GetTag(address, info);
    if (line->valid == True)
    {
        if (line->tag == tag)
            return Hit;//hit
        else {
            line->tag = tag;
            return Evict;//evict and miss
        }
    }
    else{
        line->tag = tag;//miss
        line->valid = True;
        return Miss;
    }
}

//模拟内存访问，首先看是否击中。
int visit(struct Info* info, struct Cache* cache, unsigned long address)
{
    int si = GetSetIndex(address, info);
    struct Set* curset = &cache->sets[si];
    struct Line* curline;
    //hit
    for (int i = 0; i < info->E; i++)
    {
        curline = &curset->lines[i];
        if (curline->valid == True && curline->tag == GetTag(address, info))
            return checkHit(info, address, curline);
    }
    //miss
    for (int i = 0; i < info->E; i++)
    {
        curline = &curset->lines[i];
        if (curline->valid == False)
            return checkHit(info, address, curline);
    }
    //evict
    curline = GetLRU(curset, info->E);
    return checkHit(info, address, curline);
}

//根据内存访问状态得到对应响应
void ParseFlag(int flag)
{
    switch (flag)
    {
    case Evict:
        evictn++;
        missn++;
        //printf("miss eviction ");
        return;
    case Miss:
        missn++;
        //printf("miss ");
        return;
    case Hit:
        hitn++;
        //printf("hit ");
    }
}

//解析内存指令
void ParseInstruction(struct Info* info, struct Cache*cache, char op, long address, int size)
{
    if (op == 'L' || op == 'S') {
        ParseFlag(visit(info, cache, address));
    }
    else if (op == 'M') {
        ParseFlag(visit(info, cache, address));
        ParseFlag(visit(info, cache, address));
    }
    else return;
}

//GC
void clean(struct Info* info, struct Cache* cache){
    for(int i=0;i<info->s; i++){
        for(int j=0; j<info->E; j++){
                free(cache->sets[i].lines[j].blocks);
        }
        free(cache->sets[i].lines);
    }
    free(cache->sets);
    free(cache);
}

//先命令行
//再初始化
//然后处理输入
//最后输出结果并GC
int main(int argc, char *argv[])
{
    struct Info*  info = ParseShell(argc, argv);
    freopen(info->file, "r", stdin);
    struct Cache* cache = GenerateCache(info->s, info->E, info->b);
    char op;
    long addr;
    int size;
    while (scanf("%c %lx,%x", &op, &addr, &size) != EOF)
    {
        printf("%c %lx,%x ", op, addr, size);
        ParseInstruction(info, cache, op, addr, size);
    }
    clean(info,cache);
    printSummary(hitn, missn, evictn);
}
