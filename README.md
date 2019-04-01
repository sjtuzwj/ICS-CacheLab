# ICS-CacheLab

# Part1思路
valid && tag == ->hit    
!valid -> miss    
valid -> tag != -> evict    

LRU的正确做法是使用链表+MTF前移编码策略，这样能始终保证队尾的出现是最后，并且能控制在常数级。   
但是硬件上似乎由于难度过大，采用了counter？总之就是遍历一遍找counter最小的，时间复杂度O（e）。

# Part2思路
64：减少长纵向（64）的读写，每次的纵向范围都控制在32的大小。    
32/61： 单独处理对角线元素。    
