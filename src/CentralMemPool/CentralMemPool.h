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

class CentralMemPool {
 public:
  void AddThread(int id);
  void DeleteThread(int id);
  void *AllocFromSys(size_t buff_size);
  bool FreeToSys(void *pBuf, size_t buff_size);

  void *MyMalloc(size_t size);
  void MyFree(void *p);

  void FreeAllMem();

 private:
  //  std::vector<ThreadMemPool *> all_thread_mem_pools_;
  size_t fix_size_ = MEMORY_POOL_BYTE_SIZE;
  std::vector<void *> free_buff_; // 空闲内存空间链表，固定大小
  std::map<void *, ThreadMemPool *> ptr_to_mem_pool_; // 记录空间从哪个ThreadMemPool获得
  std::map<std::thread::id, std::vector<over_mem>> thread_to_over_;        // 加读写锁 超大空间
  std::map<std::thread::id, std::vector<ThreadMemPool *>> thread_to_mem_pool_;  // 加读写锁
};

#endif //MEMPOOL_SRC_CENTRALMEMPOOL_H_
