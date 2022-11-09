//
// Created by 26473 on 2022/11/9.
//

#include "CentralMemPool.h"

void CentralMemPool::AddThread(int id) {
  auto pid = std::this_thread::get_id();
  //  std::cout << "add son " << id << ":" << pid << std::endl;
  if (thread_to_mem_pool_.count(pid) != 0) {
    std::cout << "this thread existed !" << std::endl;
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
}

void CentralMemPool::DeleteThread(int id) {
  auto pid = std::this_thread::get_id();
  // std::cout << "delete son " << id << ":" << pid << std::endl;
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
}

void *CentralMemPool::MyMalloc(size_t size) {
  auto pid = std::this_thread::get_id();
  if (thread_to_mem_pool_.count(pid) == 0 || thread_to_mem_pool_[pid].empty()) {
    std::cout << "unexpected error !" << std::endl;
    return nullptr;
  }
  // 申请的空间太大
  if (size > thread_to_mem_pool_[pid][0]->MaxAllocSize() * MINUNITSIZE) {
    auto ptr = AllocFromSys(size);
    over_mem tmp = {ptr, size};
    thread_to_over_[pid].emplace_back(tmp);
    return ptr;
  }
  // 尝试在该线程现有的mem_pool中进行申请
  auto mem_pools = thread_to_mem_pool_[pid];
  for (auto &mem_pool: mem_pools) {
    auto ptr = mem_pool->AllocMemory(size);
    if (ptr != nullptr) {
      ptr_to_mem_pool_.emplace(ptr, mem_pool);
      return ptr;
    }
  }
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
  return ptr;
}

void CentralMemPool::MyFree(void *p) {
  auto pid = std::this_thread::get_id();
  if (thread_to_mem_pool_.count(pid) == 0 || thread_to_mem_pool_[pid].empty()) {
    std::cout << "unexpected error !" << std::endl;
    return;
  }
  // 在该线程的meme pool中寻找
  if (ptr_to_mem_pool_.count(p) != 0) {
    auto mem_pool = ptr_to_mem_pool_[p];
    mem_pool->FreeMemory(p);
    return;
  }
  // 尝试在大内存中寻找
  if (thread_to_over_.count(pid) != 0) {
    auto overs = thread_to_over_[pid];
    for (auto &over: overs) {
      if (over.addr == p) {
        FreeToSys(p, over.size);
        return;
      }
    }
  }
  std::cout << "invalid mem addr !" << std::endl;
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
}