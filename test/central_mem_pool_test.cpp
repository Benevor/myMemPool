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

void thread_alloc_2(CentralMemPool *central_pool) {
  central_pool->AddThread();
  central_pool->DeleteThread();
}

void add_thread_test() {
  CentralMemPool *central_pool = new CentralMemPool();
  std::thread t1(thread_alloc, central_pool);
  std::thread t2(thread_alloc, central_pool);
  t1.join();
  t2.join();
  delete central_pool;
}

int main(int argc, char *argv[]) {
  add_thread_test();
  return 0;
}