/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 

/*
name: 朱文杰
ID: ics517021910799
*/
#include <stdio.h>
#include "cachelab.h"
#define BLOCKSIZE 32
#define SENTINEL -777
int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    
//优化1:
//思路:使得导致block过期的元素最后一个处理
/*
@step: block size
*/
    //Event Distribution
    if ((M == 32 && N == 32) || (M ==61 && N ==67)) {
        //块的大小，这里值正好凑成整数，而且变量数足够，就直接写了。
        int step = (M + N)/8;
        //四个循环变量 + 三个临时存储变量 + 一个blocksize，合计8个。
        //从A中取出一块block
        for (int i = 0; i < N; i += step) {
            for (int j = 0; j < M; j += step) {
                //对block进行逐元素处理
                for (int x = i; x < i + step && x < N; x++) {
                    //存储对角线处元素（否则B[i][j]必然导致冲突),使用-777作为哨兵。
                    int crossIndex = SENTINEL, crossValue, curValue;
                    for (int y = j; y < j + step && y < M; y++) {
                        curValue= A[x][y];
                        //y！=x，正常转置
                        if (y ^ x) {
                            B[y][x] = curValue;
                        }
                        //横纵坐标相等，会导致冲突，从而使原取出的Ablock过期，因此存起来最后一个处理。
                        else {
                            crossIndex = x;
                            crossValue = curValue;
                        }
                    }
                    //存在对角线处元素塞过去
                    if (crossIndex != -777) {
                        B[crossIndex][crossIndex] = crossValue;
                    }
                }
            }
        }
    }

    
//优化2:
//思路:在cache的长度不够纵向全部存储的情况下，避免超长纵向导致的大面积evict冲突
/*
@step: block size
*/
    if (M == 64 && N == 64) { 
        //三个循环变量 + 八个临时存储变量 + 一个blocksize，合计12个。
        int step=8;
        for (int i = 0; i < N; i += step) {
                for (int j = 0; j < M; j += step) {
        //分成8*8的块，但是因为数组长度是64，因此和32*32的数组不同，同块仍然存在冲突(箭头向下时)
                    for (int x = i; x < i + step/2; x++) {
                        //西北
                        int tmp0 = A[x][j],
                            tmp1 = A[x][j + 1],
                            tmp2 = A[x][j + 2],
                            tmp3 = A[x][j + 3],
                        //东北
                            tmp4 = A[x][j + 4],
                            tmp5 = A[x][j + 5],
                            tmp6 = A[x][j + 6],
                            tmp7 = A[x][j + 7];
        /* A Load
        -----------------> 
        ----------------->
        ----------------->
        ----------------->
        . . . .   . . . .
        . . . .   . . . .
        . . . .   . . . .
        . . . .   . . . .                            
        */

       /* B Store
        x         x+4
   1     | | | |   | | | |
   2     | | | |   | | | |
   3     | | | |   | | | |
   4     | | | |   | | | |
        . . . .   . . . .
        . . . .   . . . .
        . . . .   . . . .
        . . . .   . . . .
        */
                        //对称轴为西北方向，故 西北 to 西北（就位）
                        B[j][x] = tmp0;
                        B[j + 1][x] = tmp1;
                        B[j + 2][x] = tmp2;
                        B[j + 3][x] = tmp3;
                        //东北 to 东北（由于上文所说同块间冲突，因此不能放到西南，故先放到东北）
                        B[j + 3][x + 4] = tmp4;
                        B[j + 2][x + 4] = tmp5;
                        B[j + 1][x + 4] = tmp6;
                        B[j][x + 4] = tmp7;
                    }
                    
                    for (int y= 0; y < step/2 ; y++) { 
                        //从中间向两侧按列读取
                        //西南
                        int tmp0 = A[i + 4][j +  step/2 -1 - y];
                        int tmp1 = A[i + 5][j +  step/2 -1 - y];
                        int tmp2 = A[i + 6][j +  step/2 -1 - y];
                        int tmp3 = A[i + 7][j +  step/2 -1 - y];
                        //东南
                        int tmp4 = A[i + 4][j + step/2 + y];
                        int tmp5 = A[i + 5][j + step/2 + y];
                        int tmp6 = A[i + 6][j + step/2 + y];
                        int tmp7 = A[i + 7][j + step/2 + y];
        /* A Load
         . . . .   . . . .
         . . . .   . . . .
         . . . .   . . . .
         . . . .   . . . .
       4 | | | |   | | | |
       5 | | | |   | | | |
       6 | | | |   | | | |
       7 | | | |   | | | |
        */ 
        /* B Store
        . . . .   --------
        . . . .   --------
        . . . .   --------
        . . . .   --------
        . . . .   --------
        . . . .   --------
        . . . .   --------
        . . . .   --------                           
        */

                        //在B的块内部，先前存在东北角的数据向西南对换，避免了上述所说的纵向冲突。
                        //西南 to 东北  东南 to 东南， 均为按行读取，不会发生冲突。
                        B[j + step/2 + y][i] = B[j +  step/2 -1 - y][i + 4];
                        B[j +  step/2 -1 - y][i + 4] = tmp0;
                        B[j + step/2 + y][i + 1] = B[j +  step/2 -1 - y][i + 5];
                        B[j +  step/2 -1 - y][i + 5] = tmp1;
                        B[j + step/2 + y][i + 2] = B[j +  step/2 -1 - y][i + 6];
                        B[j +  step/2 -1 - y][i + 6] = tmp2;
                        B[j + step/2 + y][i + 3] = B[j +  step/2 -1 - y][i + 7];
                        B[j +  step/2 -1 - y][i + 7] = tmp3;

                        B[j + step/2 + y][i + 4] = tmp4;
                        B[j + step/2 + y][i + 5] = tmp5;
                        B[j + step/2 + y][i + 6] = tmp6;
                        B[j + step/2 + y][i + 7] = tmp7;
                    }
                }
            }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

