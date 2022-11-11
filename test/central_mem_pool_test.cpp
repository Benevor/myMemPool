//
// Created by 26473 on 2022/11/9.
//
#include "CentralMemPool.h"
#include <cmath>

// 单个 CentralMemPool 的测试
// 多线程、多 ThreadMemPool 的并发测试

// 简单内存申请与释放
void thread_alloc(CentralMemPool *central_pool) {
  central_pool->AddThread();
  auto ptr = central_pool->MyMalloc(BLOCK_SIZE);
  central_pool->MyFree(ptr);
  central_pool->DeleteThread();
}

// 涉及单个线程的多个ThreadMemPool申请与释放
void thread_cc(CentralMemPool *central_pool) {
  central_pool->AddThread();
  std::vector<void *> ptrs;
  for (int i = 0; i < 4; ++i) {
    if (i == 3) {
      auto ptr = central_pool->MyMalloc(central_pool->MaxAllocSizeInMemPool() * BLOCK_SIZE);
      ptrs.emplace_back(ptr);
    } else {
      auto ptr = central_pool->MyMalloc(BLOCK_SIZE * pow(10, i));
      ptrs.emplace_back(ptr);
    }
  }

  for (int i = 3; i >= 0; --i) {
    central_pool->MyFree(ptrs[i]);
  }

  central_pool->DeleteThread();
}

// 回收线程的空ThreadMemPool
void thread_gc(CentralMemPool *central_pool) {
  central_pool->AddThread();
  if (central_pool->MaxAllocSizeInMemPool() == -1) {
    std::cout << "thread gc fail !" << std::endl;
  }
  auto ptr1 = central_pool->MyMalloc(central_pool->MaxAllocSizeInMemPool() * BLOCK_SIZE);
  auto ptr2 = central_pool->MyMalloc(BLOCK_SIZE);
  central_pool->MyFree(ptr2);
  central_pool->MyFree(ptr1);
  central_pool->DeleteThread();
}

// 尝试重复释放同一个指针对应的地址空间
void thread_dup_free(CentralMemPool *central_pool) {
  central_pool->AddThread();
  auto ptr = central_pool->MyMalloc(BLOCK_SIZE);
  central_pool->MyFree(ptr);
  central_pool->MyFree(ptr);

  central_pool->DeleteThread();
}

void thread_over(CentralMemPool *central_pool) {
  central_pool->AddThread();
  if (central_pool->MaxAllocSizeInMemPool() == -1) {
    std::cout << "thread gc fail !" << std::endl;
  }
  auto ptr1 = central_pool->MyMalloc(central_pool->MaxAllocSizeInMemPool() * BLOCK_SIZE+BLOCK_SIZE);
  central_pool->MyFree(ptr1);
  central_pool->DeleteThread();
}

// 测试：单线程简单gc测试
void simple_gc_test() {
  auto central_pool = new CentralMemPool();
  std::thread t1(thread_gc, central_pool);
  t1.join();
  delete central_pool;
}

// 测试：简单并发测试
void simple_cc_test() {
  auto central_pool = new CentralMemPool();
  std::thread t1(thread_alloc, central_pool);
  std::thread t2(thread_alloc, central_pool);
  t1.join();
  t2.join();
  delete central_pool;
}

// 测试：并发gc测试
void cc_gc_test() {
  auto central_pool = new CentralMemPool();
  std::thread t1(thread_gc, central_pool);
  std::thread t2(thread_gc, central_pool);
  t1.join();
  t2.join();
  delete central_pool;
}

// 测试：并发测试
void cc_test() {
  auto central_pool = new CentralMemPool();
  std::thread t1(thread_cc, central_pool);
  std::thread t2(thread_cc, central_pool);
  std::thread t3(thread_cc, central_pool);
  t1.join();
  t2.join();
  t3.join();
  delete central_pool;
}

// 测试：测试重复内存释放
void duplicate_free_test() {
  auto central_pool = new CentralMemPool();
  std::thread t1(thread_dup_free, central_pool);
  t1.join();
  delete central_pool;
}

// 超大内存块并发测试
void cc_over_test() {
  auto central_pool = new CentralMemPool();
  std::thread t1(thread_over, central_pool);
  std::thread t2(thread_over, central_pool);
  t1.join();
  t2.join();
  delete central_pool;
}

int main(int argc, char *argv[]) {
  cc_over_test();
  return 0;
}