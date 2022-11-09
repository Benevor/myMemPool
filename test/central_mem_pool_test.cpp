//
// Created by 26473 on 2022/11/9.
//
#include "CentralMemPool.h"

void Allocate1(int id) {
  CentralMemPool cen;
  cen.AddThread(id);
}

void Allocate2(int id) {
  CentralMemPool cen;
  cen.AddThread(id);
}

void add_thread_test() {
  std::cout << "fa:" << std::this_thread::get_id() << std::endl;
  std::thread t1(Allocate1, 1);
  std::thread t2(Allocate1, 2);
  t1.join();
  t2.join();
}

int main(int argc, char *argv[]) {
  add_thread_test();
  return 0;
}