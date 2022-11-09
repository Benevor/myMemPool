//
// Created by 26473 on 2022/11/2.
//

#ifndef MYMEMPOOL_SRC_COMMON_H_
#define MYMEMPOOL_SRC_COMMON_H_
#pragma once
#include <stdlib.h>

#define MINUNITSIZE 64
#define ADDR_ALIGN 8
#define SIZE_ALIGN MINUNITSIZE

struct memory_chunk;
// 任意一块大小的内存都会被向上取整到block大小的整数倍
typedef struct memory_block {
  size_t count; // 该block后面的与该block同属于1个chunk的block的数目
  size_t start; // 该block所在chunk的起始block索引
  memory_chunk *pmem_chunk;
} memory_block;

// 多个连续的block组成1个chunk
typedef struct memory_chunk {
  memory_block *pfree_mem_addr; // 指向chunk在内存映射表中的位置
  memory_chunk *pre;
  memory_chunk *next;
} memory_chunk;

#endif //MYMEMPOOL_SRC_COMMON_H_
