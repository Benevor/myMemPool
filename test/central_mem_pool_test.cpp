//
// Created by 26473 on 2022/11/9.
//
#include "CentralMemPool.h"

void thread_alloc(CentralMemPool *central_pool) {
  central_pool->AddThread();
  auto ptr = central_pool->MyMalloc(MINUNITSIZE);
  central_pool->MyFree(ptr);
  central_pool->DeleteThread();
}

void thread_gc(CentralMemPool *central_pool) {
  central_pool->AddThread();
  if (central_pool->MaxAllocSizeInMemPool() == -1) {
    std::cout << "thread gc fail !" << std::endl;
  }
  auto ptr1 = central_pool->MyMalloc(central_pool->MaxAllocSizeInMemPool() * MINUNITSIZE);
  auto ptr2 = central_pool->MyMalloc(MINUNITSIZE);
  central_pool->MyFree(ptr2);
  central_pool->MyFree(ptr1);
  central_pool->DeleteThread();
}

void simple_gc_test() {
  CentralMemPool *central_pool = new CentralMemPool();
  std::thread t1(thread_gc, central_pool);
  t1.join();
  delete central_pool;
}

void simple_cc_test() {
  CentralMemPool *central_pool = new CentralMemPool();
  std::thread t1(thread_alloc, central_pool);
  std::thread t2(thread_alloc, central_pool);
  t1.join();
  t2.join();
  delete central_pool;
}

int main(int argc, char *argv[]) {
  simple_gc_test();
  return 0;
}