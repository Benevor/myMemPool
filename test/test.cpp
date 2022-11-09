//
// Created by 26473 on 2022/11/2.
//

#include <sys/mman.h>　
#include "MemPool.h"
#include <vector>
#include <iostream>
using namespace std;
#define MEMORY_POOL_BYTE_SIZE 5*1024*1024 //5M = 5M * 1024kb * 1024bytes
char g_memPool_Buffer[MEMORY_POOL_BYTE_SIZE]; //only for test_Func2()

void test_Func2() {
  size_t sBufSize = MEMORY_POOL_BYTE_SIZE;
  void *pBuf = g_memPool_Buffer; //通过静态全局变量数据开辟memory pool所需的大内存
  MemPool *mem_pool = nullptr;
  CreateMemoryPool(pBuf, sBufSize, mem_pool);
  //定义1个int型数组，10个元素
  void *p_void = mem_pool->AllocMemory(sizeof(int) * 10); //申请内存
  cout << "============Test-Func2+static+void=========" << endl;
  for (int i0 = 0; i0 < 10; i0++) {
    ((int *) p_void)[i0] = i0;
    cout << ((int *) p_void)[i0] << endl;
  }
  mem_pool->FreeMemory(p_void); //释放内存
  //
  //定义1个int型数组，10个元素
  int *p_int = (int *) mem_pool->AllocMemory(sizeof(int) * 10); //申请内存
  cout << "============Test-Func2+static+int=========" << endl;
  for (int i0 = 0; i0 < 10; i0++) {
    p_int[i0] = i0;
    cout << p_int[i0] << endl;
  }
  mem_pool->FreeMemory(p_int); //释放内存
}

void test_int_alloc() {
  size_t sBufSize = MEMORY_POOL_BYTE_SIZE;
  void *pBuf = mmap(NULL, MEMORY_POOL_BYTE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (pBuf == MAP_FAILED) {
    cout << " test_Func1: mmap failed" << endl;
    return;
  }

  MemPool *mem_pool = nullptr;
  CreateMemoryPool(pBuf, sBufSize, mem_pool);
  //定义1个int型数组，10个元素
  void *p_void = mem_pool->AllocMemory(sizeof(int) * 10); //申请内存
  cout << "============ test_int_alloc 1 =========" << endl;
  for (int i0 = 0; i0 < 10; i0++) {
    ((int *) p_void)[i0] = i0;
    cout << ((int *) p_void)[i0] << endl;
  }
  mem_pool->FreeMemory(p_void); //释放内存

  int *p_int = (int *) mem_pool->AllocMemory(sizeof(int) * 10); //申请内存
  cout << "============ test_int_alloc 2 =========" << endl;
  for (int i0 = 0; i0 < 10; i0++) {
    p_int[i0] = i0;
    cout << p_int[i0] << endl;
  }
  mem_pool->FreeMemory(p_int);
  munmap(pBuf, sBufSize);
}

void test_print_info() {
  size_t sBufSize = MEMORY_POOL_BYTE_SIZE;
  void *pBuf = mmap(NULL, MEMORY_POOL_BYTE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (pBuf == MAP_FAILED) {
    cout << " test_Func1: mmap failed" << endl;
    return;
  }

  MemPool *mem_pool = nullptr;
  CreateMemoryPool(pBuf, sBufSize, mem_pool);
  mem_pool->PrintInfo();
  munmap(pBuf, sBufSize);
}

void test_seq_alloc() {
  size_t sBufSize = MEMORY_POOL_BYTE_SIZE;
  void *pBuf = mmap(NULL, MEMORY_POOL_BYTE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (pBuf == MAP_FAILED) {
    cout << " test_Func1: mmap failed" << endl;
    return;
  }

  MemPool *mem_pool = nullptr;
  CreateMemoryPool(pBuf, sBufSize, mem_pool);
  mem_pool->PrintChunkInfo();

  vector<void *> ptrs;
  cout << "============ test_seq_print_info =========" << endl;
  for (int i = 0; i < 5; ++i) {
    void *ptr = mem_pool->AllocMemory(MINUNITSIZE);
    ptrs.emplace_back(ptr);
    cout << ptr << endl;
  }
  mem_pool->PrintChunkInfo();
  for (int i = 4; i >= 0; --i) {
    mem_pool->FreeMemory(ptrs[i]);
  }
  mem_pool->PrintChunkInfo();
  munmap(pBuf, sBufSize);
}

void test_seq_alloc_align() {
  size_t sBufSize = MEMORY_POOL_BYTE_SIZE;
  void *pBuf = mmap(NULL, MEMORY_POOL_BYTE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (pBuf == MAP_FAILED) {
    cout << " test_Func1: mmap failed" << endl;
    return;
  }

  MemPool *mem_pool = nullptr;
  CreateMemoryPool(pBuf, sBufSize, mem_pool);
  mem_pool->PrintChunkInfo();

  vector<void *> ptrs;
  cout << "============ test_seq_print_info =========" << endl;
  for (int i = 0; i < 5; ++i) {
    void *ptr = mem_pool->AllocMemory(MINUNITSIZE - 1);
    ptrs.emplace_back(ptr);
    cout << ptr << endl;
  }
  mem_pool->PrintChunkInfo();
  for (int i = 4; i >= 0; --i) {
    mem_pool->FreeMemory(ptrs[i]);
  }
  mem_pool->PrintChunkInfo();
  munmap(pBuf, sBufSize);
}

void test_not_merge() {
  size_t sBufSize = MEMORY_POOL_BYTE_SIZE;
  void *pBuf = mmap(NULL, MEMORY_POOL_BYTE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (pBuf == MAP_FAILED) {
    cout << " test_Func1: mmap failed" << endl;
    return;
  }

  MemPool *mem_pool = nullptr;
  CreateMemoryPool(pBuf, sBufSize, mem_pool);

  vector<void *> ptrs;
  cout << "============ test_seq_print_info =========" << endl;
  for (int i = 0; i < 5; ++i) {
    void *ptr = mem_pool->AllocMemory(MINUNITSIZE - 1);
    ptrs.emplace_back(ptr);
    cout << ptr << endl;
  }
  mem_pool->FreeMemory(ptrs[0]);
  mem_pool->FreeMemory(ptrs[2]);
  mem_pool->PrintChunkInfo();
  munmap(pBuf, sBufSize);
}

void test_merge() {
  size_t sBufSize = MEMORY_POOL_BYTE_SIZE;
  void *pBuf = mmap(NULL, MEMORY_POOL_BYTE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (pBuf == MAP_FAILED) {
    cout << " test_Func1: mmap failed" << endl;
    return;
  }

  MemPool *mem_pool = nullptr;
  CreateMemoryPool(pBuf, sBufSize, mem_pool);

  vector<void *> ptrs;
  cout << "============ test_seq_print_info =========" << endl;
  for (int i = 0; i < 5; ++i) {
    void *ptr = mem_pool->AllocMemory(MINUNITSIZE - 1);
    ptrs.emplace_back(ptr);
    cout << ptr << endl;
  }
  mem_pool->FreeMemory(ptrs[0]);
  mem_pool->FreeMemory(ptrs[2]);
  mem_pool->PrintChunkInfo();
  mem_pool->FreeMemory(ptrs[1]);
  mem_pool->FreeMemory(ptrs[3]);
  mem_pool->FreeMemory(ptrs[4]);
  mem_pool->PrintChunkInfo();
  munmap(pBuf, sBufSize);
}

void test_max() {}

void test_overflow() {}

int main(int argc, char *argv[]) {
//  test_Func1(); //测试01：通过mmap函数开辟内存池所需的大内存
//  test_Func2(); //测试02：通过静态全局变量开辟内存池所需的大内存
  test_merge();
  return 0;
}
