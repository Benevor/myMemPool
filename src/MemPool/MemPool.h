//
// Created by 26473 on 2022/11/2.
//

#ifndef MYMEMPOOL_SRC_MEMPOOL_MEMPOOL_H_
#define MYMEMPOOL_SRC_MEMPOOL_MEMPOOL_H_
#pragma once
#include "common.h"
#include <string.h>
#include <iostream>

class MemPool {
 public:
  MemPool() = delete;
  ~MemPool() = delete;
  void *AllocMemory(size_t sMemorySize);
  void FreeMemory(void *ptrMemoryBlock);
  void PrintInfo();
  void PrintChunkInfo();

  friend void CreateMemoryPool(void *pBuf, size_t sBufSize, MemPool *&mem_pool);

 private:
  void *memory_;
  size_t size_;

  map_unit *chunk_map_;     // 内存映射表
  size_t chunk_map_count_;  // 映射表单元格个数

  memory_chunk *chunk_pool_;
  size_t chunk_pool_count_;   // 记录链表单元缓冲池中剩余的单元的个数，个数为0时不能分配单元给 free_chunk_

  memory_chunk *free_chunk_;  // chunk 链表
  size_t free_chunk_count_;   // 记录 free_chunk_ 链表中的单元个数

  size_t mem_used_size_;    // 记录内存池中已经分配给用户的内存的大小
  size_t mem_block_count_;  // 一个 mem_unit 大小为 MINUNITSIZE

 public:
  void *&getMemory() {
    return memory_;
  }
};

void CreateMemoryPool(void *pBuf, size_t sBufSize, MemPool *&mem_pool);

#endif //MYMEMPOOL_SRC_MEMPOOL_MEMPOOL_H_
