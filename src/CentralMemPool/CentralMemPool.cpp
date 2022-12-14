//
// Created by 26473 on 2022/11/9.
//

#include "CentralMemPool.h"
#include <sstream>
#include <iostream>

void CentralMemPool::AddThread() {
  auto pid = std::this_thread::get_id();
  std::stringstream ss;
  ss << "add thread : ";
  ss << pid;
  ss << "\n";
  std::cout << ss.str();
  thread_mutex_.lock();
  if (thread_to_mem_pool_.count(pid) != 0) {
    std::cout << "this thread existed !\n";
    thread_mutex_.unlock();
    return;
  }
  void *ptr = nullptr;
  // 优先从 free_buff_ 中获得空间
  if (free_buff_.empty()) {
    // free_buff_ 为空，则直接向系统申请
    ptr = AllocFromSys(fix_size_);
  } else {
    ptr = free_buff_.back();
    free_buff_.pop_back();
  }
  // 创建 ThreadMemPool 实例，并分配给该线程
  ThreadMemPool *mem_pool = nullptr;
  CreateMemoryPool(ptr, fix_size_, mem_pool);
  thread_to_mem_pool_[pid].emplace_back(mem_pool);
  thread_mutex_.unlock();
}

void CentralMemPool::DeleteThread() {
  auto pid = std::this_thread::get_id();
  std::stringstream ss;
  ss << "delete thread : ";
  ss << pid;
  ss << "\n";
  std::cout << ss.str();
  thread_mutex_.lock();
  // 释放该线程占有的所有 ThreadMemPool，释放给 free_buff_
  if (thread_to_mem_pool_.count(pid) != 0) {
    for (auto &t: thread_to_mem_pool_[pid]) {
      // 不需要清除，下次拿出来用的时候会清除内容
      free_buff_.emplace_back((void *) t);
    }
    thread_to_mem_pool_.erase(pid);
  }

  // 释放该线程占用的所有超大空间，释放给系统
  if (thread_to_over_.count(pid) != 0) {
    for (auto &t: thread_to_over_[pid]) {
      FreeToSys(t.addr, t.size);
    }
    thread_to_over_.erase(pid);
  }
  thread_mutex_.unlock();
}

void *CentralMemPool::MyMalloc(size_t size) {
  auto pid = std::this_thread::get_id();
  thread_mutex_.lock_shared();
  if (thread_to_mem_pool_.count(pid) == 0 || thread_to_mem_pool_[pid].empty()) {
    std::cout << "unexpected error !\n";
    thread_mutex_.unlock_shared();
    return nullptr;
  }
  // 申请的空间太大，直接向系统申请
  if (size > thread_to_mem_pool_[pid][0]->MaxAllocSize() * BLOCK_SIZE) {
    thread_mutex_.unlock_shared();
    thread_mutex_.lock();
    auto ptr = AllocFromSys(size);
    over_mem tmp = {ptr, size};
    thread_to_over_[pid].emplace_back(tmp);
    thread_mutex_.unlock();
    std::stringstream ss;
    ss << "thread " << pid << " malloc " << size << "from sys success, ptr: " << ptr << "\n";
    std::cout << ss.str();
    return ptr;
  }

  // 尝试在该线程现有的 ThreadMemPool 中进行申请
  auto mem_pools = thread_to_mem_pool_[pid];
  for (auto &mem_pool: mem_pools) {
    auto ptr = mem_pool->AllocMemory(size);
    if (ptr != nullptr) {
      ptr_mutex_.lock();
      ptr_to_mem_pool_.emplace(ptr, mem_pool);
      ptr_mutex_.unlock();
      thread_mutex_.unlock_shared();
      std::stringstream ss;
      ss << "thread " << pid << " malloc " << size << " from mem pool success, ptr: " << ptr << "\n";
      std::cout << ss.str();
      return ptr;
    }
  }
  thread_mutex_.unlock_shared();
  thread_mutex_.lock();
  // 现有的 ThreadMemPool 均申请失败，则为该线程添加一个 ThreadMemPool
  void *ptr = nullptr;
  if (free_buff_.empty()) {
    ptr = AllocFromSys(fix_size_);
  } else {
    ptr = free_buff_.back();
    free_buff_.pop_back();
  }
  ThreadMemPool *mem_pool = nullptr;
  CreateMemoryPool(ptr, fix_size_, mem_pool);
  thread_to_mem_pool_[pid].emplace_back(mem_pool);
  ptr = mem_pool->AllocMemory(size);
  if (ptr != nullptr) {
    ptr_to_mem_pool_.emplace(ptr, mem_pool);
  }
  thread_mutex_.unlock();

  std::stringstream ss;
  ss << "thread " << pid << " malloc " << size << " from mem pool success, after add mem pool. ptr: " << ptr << "\n";
  std::cout << ss.str();
  return ptr;
}

void CentralMemPool::MyFree(void *p) {
  auto pid = std::this_thread::get_id();
  thread_mutex_.lock_shared();
  if (thread_to_mem_pool_.count(pid) == 0 || thread_to_mem_pool_[pid].empty()) {
    std::cout << "unexpected error !\n";
    thread_mutex_.unlock_shared();
    return;
  }
  // 在该线程的 ThreadMemPool 中寻找
  if (ptr_to_mem_pool_.count(p) != 0) {
    ptr_mutex_.lock();
    auto mem_pool = ptr_to_mem_pool_[p];
    ptr_to_mem_pool_.erase(p);
    ptr_mutex_.unlock();
    mem_pool->FreeMemory(p);
    thread_mutex_.unlock_shared();
    std::stringstream ss;
    ss << "thread " << pid << " free to mem pool success, ptr: " << p << "\n";
    std::cout << ss.str();
    // 如果释放后，该 ThreadMemPool 变空，则回收该 ThreadMemPool
    if (mem_pool->UsedSize() == 0) {
      thread_mutex_.lock();
      size_t index = 0;
      for (auto &m: thread_to_mem_pool_[pid]) {
        if (m == mem_pool) {
          break;
        }
        index++;
      }
      if (thread_to_mem_pool_[pid].size() > 1) {
        // 每个线程至少留一个 ThreadMemPool
        thread_to_mem_pool_[pid].erase(thread_to_mem_pool_[pid].begin() + index);
        free_buff_.emplace_back((void *) mem_pool);

        std::stringstream ss;
        ss << "thread " << pid << " gc index:" << index << " mem pool\n";
        std::cout << ss.str();
      }
      thread_mutex_.unlock();
    }
    return;
  }
  // 尝试在大内存中寻找， 并释放
  if (thread_to_over_.count(pid) != 0) {
    auto overs = thread_to_over_[pid];
    for (auto &over: overs) {
      if (over.addr == p) {
        FreeToSys(p, over.size);
        thread_mutex_.unlock_shared();
        std::stringstream ss;
        ss << "thread " << pid << " free to sys success, ptr: " << p << "\n";
        std::cout << ss.str();
        return;
      }
    }
  }
  std::cout << "invalid mem addr !\n";
  thread_mutex_.unlock_shared();
}

void *CentralMemPool::AllocFromSys(size_t buff_size) {
  size_t sBufSize = buff_size;
  void *pBuf = mmap(NULL, buff_size, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (pBuf == MAP_FAILED) {
    std::cout << " AllocFromSys failed !\n";
    return nullptr;
  }
  return pBuf;
}

bool CentralMemPool::FreeToSys(void *pBuf, size_t buff_size) {
  if (munmap(pBuf, buff_size) == -1) {
    std::cout << " FreeToSys failed !\n";
    return false;
  }
  return true;
}

void CentralMemPool::FreeAllMem() {
  std::cout << "============== Free All Mem ==============" << std::endl;
  thread_mutex_.lock();
  for (auto &m: free_buff_) {
    FreeToSys(m, fix_size_);
  }

  for (auto &t: thread_to_over_) {
    for (auto &m: t.second) {
      FreeToSys(m.addr, m.size);
    }
  }

  for (auto &t: thread_to_mem_pool_) {
    for (auto &m: t.second) {
      FreeToSys((void *) m, fix_size_);
    }
  }
  thread_mutex_.unlock();
}

CentralMemPool::~CentralMemPool() {
  FreeAllMem();
}

size_t CentralMemPool::MaxAllocSizeInMemPool() {
  auto pid = std::this_thread::get_id();
  if (thread_to_mem_pool_.count(pid) == 0 || thread_to_mem_pool_[pid].empty()) {
    std::cout << "unexpected error !\n";
    thread_mutex_.unlock_shared();
    return -1;
  }
  return thread_to_mem_pool_[pid][0]->MaxAllocSize();
}