//
// Created by 26473 on 2022/11/2.
//

#ifndef MYMEMPOOL_SRC_MEMPOOL_MEMPOOL_H_
#define MYMEMPOOL_SRC_MEMPOOL_MEMPOOL_H_
#pragma once
#include "common.h"
#include <string.h>

class MemPool {
 public:
  MemPool() = delete;
  ~MemPool() = delete;
  void *AllocMemory(size_t sMemorySize);
  void FreeMemory(void *ptrMemoryBlock);

  friend void CreateMemoryPool(void *pBuf, size_t sBufSize, MemPool *&mem_pool);

 private:
  void *memory;
  size_t size;
  memory_block *pmem_map;
  memory_chunk *pfree_mem_chunk;
  memory_chunk *pfree_mem_chunk_pool;
  size_t mem_used_size; // 记录内存池中已经分配给用户的内存的大小
  size_t mem_map_pool_count; // 记录链表单元缓冲池中剩余的单元的个数，个数为0时不能分配单元给pfree_mem_chunk
  size_t free_mem_chunk_count; // 记录 pfree_mem_chunk链表中的单元个数
  size_t mem_map_unit_count; //
  size_t mem_block_count; // 一个 mem_unit 大小为 MINUNITSIZE

 public:
  void *&getMemory() {
    return memory;
  }
};

void CreateMemoryPool(void *pBuf, size_t sBufSize, MemPool *&mem_pool);
//void reateMemoryPool(void *pBuf, size_t sBufSize, MemPool *&mem_pool) {}

#endif //MYMEMPOOL_SRC_MEMPOOL_MEMPOOL_H_
