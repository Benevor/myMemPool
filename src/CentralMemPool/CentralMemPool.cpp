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
    std::cout << "this thread existed !" << std::endl;
    thread_mutex_.unlock();
    return;
  }
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
  if (thread_to_mem_pool_.count(pid) != 0) {
    for (auto &t: thread_to_mem_pool_[pid]) {
      free_buff_.emplace_back((void *) t);
    }
    thread_to_mem_pool_.erase(pid);
  }
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
    std::cout << "unexpected error !" << std::endl;
    thread_mutex_.unlock_shared();
    return nullptr;
  }
  // 申请的空间太大
  if (size > thread_to_mem_pool_[pid][0]->MaxAllocSize() * MINUNITSIZE) {
    thread_mutex_.unlock_shared();
    thread_mutex_.lock();
    auto ptr = AllocFromSys(size);
    over_mem tmp = {ptr, size};
    thread_to_over_[pid].emplace_back(tmp);
    thread_mutex_.unlock();
    std::cout << "thread " << pid << " malloc " << size << "from sys success" << std::endl;
    return ptr;
  }
  // 尝试在该线程现有的mem_pool中进行申请
  auto mem_pools = thread_to_mem_pool_[pid];
  for (auto &mem_pool: mem_pools) {
    auto ptr = mem_pool->AllocMemory(size);
    if (ptr != nullptr) {
      ptr_mutex_.lock();
      ptr_to_mem_pool_.emplace(ptr, mem_pool);
      ptr_mutex_.unlock();
      thread_mutex_.unlock_shared();
      std::cout << "thread " << pid << " malloc " << size << " from mem pool success" << std::endl;
      return ptr;
    }
  }
  thread_mutex_.unlock_shared();
  thread_mutex_.lock();
  // 为该线程添加一个mem pool
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
  std::cout << "thread " << pid << " malloc " << size << " from mem pool success, after add mem pool" << std::endl;
  return ptr;
}

void CentralMemPool::MyFree(void *p) {
  auto pid = std::this_thread::get_id();
  thread_mutex_.lock_shared();
  if (thread_to_mem_pool_.count(pid) == 0 || thread_to_mem_pool_[pid].empty()) {
    std::cout << "unexpected error !" << std::endl;
    thread_mutex_.unlock_shared();
    return;
  }
  // 在该线程的meme pool中寻找
  if (ptr_to_mem_pool_.count(p) != 0) {
    ptr_mutex_.lock();
    auto mem_pool = ptr_to_mem_pool_[p];
    ptr_to_mem_pool_.erase(p);
    ptr_mutex_.unlock();
    mem_pool->FreeMemory(p);
    thread_mutex_.unlock_shared();
    std::cout << "thread " << pid << " free to mem pool success" << std::endl;
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
        // 每个线程至少留一个 mem pool
        thread_to_mem_pool_[pid].erase(thread_to_mem_pool_[pid].begin() + index);
        free_buff_.emplace_back((void *) mem_pool);
        std::cout << "thread " << pid << " gc index:" << index << " mem pool" << std::endl;
      }
      thread_mutex_.unlock();
    }
    return;
  }
  // 尝试在大内存中寻找
  if (thread_to_over_.count(pid) != 0) {
    auto overs = thread_to_over_[pid];
    for (auto &over: overs) {
      if (over.addr == p) {
        FreeToSys(p, over.size);
        thread_mutex_.unlock_shared();
        std::cout << "thread " << pid << " free to sys success" << std::endl;
        return;
      }
    }
  }
  std::cout << "invalid mem addr !" << std::endl;
  thread_mutex_.unlock_shared();
}

void *CentralMemPool::AllocFromSys(size_t buff_size) {
  size_t sBufSize = buff_size;
  void *pBuf = mmap(NULL, buff_size, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (pBuf == MAP_FAILED) {
    std::cout << " AllocFromSys failed " << std::endl;
    return nullptr;
  }
  return pBuf;
}

bool CentralMemPool::FreeToSys(void *pBuf, size_t buff_size) {
  if (munmap(pBuf, buff_size) == -1) {
    std::cout << " FreeToSys failed" << std::endl;
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
    std::cout << "unexpected error !" << std::endl;
    thread_mutex_.unlock_shared();
    return -1;
  }
  return thread_to_mem_pool_[pid][0]->MaxAllocSize();
}