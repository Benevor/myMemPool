//
// Created by 26473 on 2022/11/2.
//

#ifndef MYMEMPOOL_SRC_COMMON_H_
#define MYMEMPOOL_SRC_COMMON_H_
#pragma once
#include <stdlib.h>
#include <vector>

#define MINUNITSIZE 64
#define ADDR_ALIGN 8
#define SIZE_ALIGN MINUNITSIZE

struct memory_chunk;
// 任意一块大小的内存都会被向上取整到block大小的整数倍
typedef struct map_unit {
  size_t count; // 该block后面的与该block同属于1个chunk的block的数目（包含自己），chunk的第一个unit有效
  size_t start; // 该block所在chunk的起始block索引，chunk的最后一个unit有效
  memory_chunk *chunk_addr; // chunk的第一个unit有效
} map_unit;

// 多个连续的block组成1个chunk
typedef struct memory_chunk {
  map_unit *map_addr; // 指向本chunk在内存映射表中的位置，其实就是起始 block 对应的 map_unit, 不为空代表是 free_chunk
  memory_chunk *pre;
  memory_chunk *next;
} memory_chunk;

#endif //MYMEMPOOL_SRC_COMMON_H_
