//
// Created by 26473 on 2022/11/2.
//

#ifndef MYThreadMemPool_SRC_ThreadMemPool_ThreadMemPool_H_
#define MYThreadMemPool_SRC_ThreadMemPool_ThreadMemPool_H_
#pragma once
#include "common.h"
#include <string.h>
#include <iostream>

class ThreadMemPool {
 public:
  /**
   * 禁止使用 ThreadMemPool 类的构造函数和析构函数，因为该类的实例由内存空间类型转换得到，该内存空间的申请与释放应由CentralMemPool管理
   */
  ThreadMemPool() = delete;
  ~ThreadMemPool() = delete;

  /**
   * 在ThreadMemPool中申请固定大小的内存空间
   * @param sMemorySize 申请空间的大小
   * @return 申请空间的地址，nullptr代表申请失败
   */
  void *AllocMemory(size_t sMemorySize);

  /**
   * 释放指定地址的内存空间
   * @param ptrMemoryBlock 释放空间的地址
   */
  void FreeMemory(void *ptrMemoryBlock);

  /**
   * 打印ThreadMemPool的当前信息
   */
  void PrintInfo();

  /**
   * 打印ThreadMemPool的Chunk信息
   */
  void PrintChunkInfo();

  /**
   * 获得ThreadMemPool的最大block数量
   * @return ThreadMemPool的最大block数量
   */
  size_t MaxAllocSize();

  /**
   * 友元函数， 根据内存空间的地址和大小，创建一个 ThreadMemPool的实例
   * @param pBuf 内存空间的地址
   * @param sBufSize 内存空间的大小
   * @param mem_pool 待创建的ThreadMemPool的指针
   */
  friend void CreateMemoryPool(void *pBuf, size_t sBufSize, ThreadMemPool *&mem_pool);

 private:
  // 用于向线程分配的内存空间的起始地址（数据区）
  void *memory_;
  // 用于向线程分配的内存空间的大小
  size_t size_;

  // 内存映射表的起始地址
  map_unit *chunk_map_;
  // 内存映射表的表项数目
  size_t chunk_map_count_;

  // chunk池，双向链表的头指针，用于给free_chunk_ 提供 chunk
  memory_chunk *chunk_pool_;
  // chunk池中chunk的数量
  size_t chunk_pool_count_;

  // free_chunk_链表的头指针，free_chunk_用于给线程进行分配
  memory_chunk *free_chunk_;
  // free_chunk_链表中chunk的个数
  size_t free_chunk_count_;

  // 记录内存池中已经分配给用户的内存的大小
  size_t mem_used_size_;

  // 数据区有效block的数量
  size_t mem_block_count_;

 public:
  /**
   * 获得数据区的地址
   * @return 数据区地址
   */
  void *&getMemory() {
    return memory_;
  }

  /**
   * 获得已分配内存空间大小
   * @return 已分配内存空间大小
   */
  size_t UsedSize() {
    return mem_used_size_;
  }
};

/**
 * 根据内存空间的地址和大小，创建一个 ThreadMemPool的实例
 * @param pBuf 内存空间的地址
 * @param sBufSize 内存空间的大小
 * @param mem_pool 待创建的ThreadMemPool的指针
 */
void CreateMemoryPool(void *pBuf, size_t sBufSize, ThreadMemPool *&mem_pool);

#endif //MYThreadMemPool_SRC_ThreadMemPool_ThreadMemPool_H_
