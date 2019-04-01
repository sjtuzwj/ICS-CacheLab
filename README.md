# ICS-CacheLab

# Part1思路
valid && tag == ->hit    
!valid -> miss    
valid -> tag != -> evict    

# Part2思路
64：减少长纵向（64）的读写，每次的纵向范围都控制在32的大小。    
32/61： 单独处理对角线元素。    
