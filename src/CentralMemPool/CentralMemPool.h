//
// Created by 26473 on 2022/11/9.
//

#ifndef MEMPOOL_SRC_CENTRALMEMPOOL_H_
#define MEMPOOL_SRC_CENTRALMEMPOOL_H_
#pragma once

#include "common.h"
#include "ThreadMemPool.h"
#include <sys/types.h>
#include <map>
#include <mutex>
#include <shared_mutex>

class CentralMemPool {
 public:
  /**
   * 默认构造函数
   */
  CentralMemPool() = default;

  /**
   * 自实现析构函数
   */
  ~CentralMemPool();

  /**
   * 向 CentralMemPool 中添加线程相关信息和数据结构
   */
  void AddThread();

  /**
   * 从 CentralMemPool 中删除线程相关信息和数据结构
   */
  void DeleteThread();

  /**
   * 向系统申请固定大小的内存
   * @param buff_size 申请空间的大小
   * @return 申请空间的地址
   */
  void *AllocFromSys(size_t buff_size);

  /**
   * 向系统释放内存
   * @param pBuf 释放内存的地址
   * @param buff_size 释放空间的大小
   * @return 释放是否成功，true代表成功，反之失败
   */
  bool FreeToSys(void *pBuf, size_t buff_size);

  /**
   * 同 malloc 函数原型
   * @param size 申请空间的大小
   * @return 申请空间的地址
   */
  void *MyMalloc(size_t size);

  /**
   * 同 free 函数原型
   * @param p 释放空间的地址
   */
  void MyFree(void *p);

  /**
   * 释放 CentralMemPool 占用的所有系统内存空间
   */
  void FreeAllMem();

  /**
   * 获得单个 ThreadMemPool 数据区的最大 block 数量
   * @return 单个 ThreadMemPool 数据区的最大 block 数量
   */
  size_t MaxAllocSizeInMemPool(); // for test

 private:
  // 单个 ThreadMemPool 的占用大小
  size_t fix_size_ = MEMORY_POOL_BYTE_SIZE;

  // 空闲内存空间链表，每块空闲空间都是固定大小
  std::vector<void *> free_buff_;

  // 记录地址空间从哪个ThreadMemPool获得
  std::map<void *, ThreadMemPool *> ptr_to_mem_pool_;
  std::shared_mutex ptr_mutex_; // 共享读写锁

  // 记录线程占用的所有超大内存空间的地址和大小
  std::map<std::thread::id, std::vector<over_mem>> thread_to_over_;
  // 记录线程拥有的 ThreadMemPool
  std::map<std::thread::id, std::vector<ThreadMemPool *>> thread_to_mem_pool_;
  std::shared_mutex thread_mutex_; // 共享读写锁
};

#endif //MEMPOOL_SRC_CENTRALMEMPOOL_H_
