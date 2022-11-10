//
// Created by 26473 on 2022/11/2.
//

#include "ThreadMemPool.h"
#include <iostream>
using namespace std;

void int_alloc_test() {
  size_t sBufSize = MEMORY_POOL_BYTE_SIZE;
  void *pBuf = mmap(NULL, MEMORY_POOL_BYTE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (pBuf == MAP_FAILED) {
    cout << " int_alloc_test: mmap failed" << endl;
    return;
  }

  ThreadMemPool *mem_pool = nullptr;
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

void print_info_test() {
  size_t sBufSize = MEMORY_POOL_BYTE_SIZE;
  void *pBuf = mmap(NULL, MEMORY_POOL_BYTE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (pBuf == MAP_FAILED) {
    cout << " print_info_test: mmap failed" << endl;
    return;
  }

  ThreadMemPool *mem_pool = nullptr;
  CreateMemoryPool(pBuf, sBufSize, mem_pool);
  mem_pool->PrintInfo();
  munmap(pBuf, sBufSize);
}

void seq_alloc_test() {
  size_t sBufSize = MEMORY_POOL_BYTE_SIZE;
  void *pBuf = mmap(NULL, MEMORY_POOL_BYTE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (pBuf == MAP_FAILED) {
    cout << " seq_alloc_test: mmap failed" << endl;
    return;
  }

  ThreadMemPool *mem_pool = nullptr;
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

void seq_alloc_align_test() {
  size_t sBufSize = MEMORY_POOL_BYTE_SIZE;
  void *pBuf = mmap(NULL, MEMORY_POOL_BYTE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (pBuf == MAP_FAILED) {
    cout << " seq_alloc_align_test: mmap failed" << endl;
    return;
  }

  ThreadMemPool *mem_pool = nullptr;
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

void not_merge_test() {
  size_t sBufSize = MEMORY_POOL_BYTE_SIZE;
  void *pBuf = mmap(NULL, MEMORY_POOL_BYTE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (pBuf == MAP_FAILED) {
    cout << " not_merge_test: mmap failed" << endl;
    return;
  }

  ThreadMemPool *mem_pool = nullptr;
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

void merge_test() {
  size_t sBufSize = MEMORY_POOL_BYTE_SIZE;
  void *pBuf = mmap(NULL, MEMORY_POOL_BYTE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (pBuf == MAP_FAILED) {
    cout << " merge_test: mmap failed" << endl;
    return;
  }

  ThreadMemPool *mem_pool = nullptr;
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

void max_size_test() {
  size_t sBufSize = MEMORY_POOL_BYTE_SIZE;
  void *pBuf = mmap(NULL, MEMORY_POOL_BYTE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (pBuf == MAP_FAILED) {
    cout << " max_size_test: mmap failed" << endl;
    return;
  }

  ThreadMemPool *mem_pool = nullptr;
  CreateMemoryPool(pBuf, sBufSize, mem_pool);
  cout << "============ test_max =========" << endl;
  void *ptr = mem_pool->AllocMemory(mem_pool->MaxAllocSize() * MINUNITSIZE);
  cout << "ptr:" << ptr << endl;
  mem_pool->PrintChunkInfo();
  mem_pool->FreeMemory(ptr);
  mem_pool->PrintChunkInfo();
  munmap(pBuf, sBufSize);
}

void overflow_test() {
  size_t sBufSize = MEMORY_POOL_BYTE_SIZE;
  void *pBuf = mmap(NULL, MEMORY_POOL_BYTE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (pBuf == MAP_FAILED) {
    cout << " overflow_test: mmap failed" << endl;
    return;
  }

  ThreadMemPool *mem_pool = nullptr;
  CreateMemoryPool(pBuf, sBufSize, mem_pool);
  cout << "============ test_max =========" << endl;
  void *ptr = mem_pool->AllocMemory(mem_pool->MaxAllocSize() * MINUNITSIZE + 1);
  cout << "ptr:" << ptr << endl;
  if (ptr != nullptr) {
    mem_pool->PrintChunkInfo();
    mem_pool->FreeMemory(ptr);
    mem_pool->PrintChunkInfo();
  } else {
    cout << "alloc failed !" << endl;
  }
  munmap(pBuf, sBufSize);
}

void max_num_test() {
  size_t sBufSize = MEMORY_POOL_BYTE_SIZE;
  void *pBuf = mmap(NULL, MEMORY_POOL_BYTE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (pBuf == MAP_FAILED) {
    cout << " max_num_test: mmap failed" << endl;
    return;
  }

  ThreadMemPool *mem_pool = nullptr;
  CreateMemoryPool(pBuf, sBufSize, mem_pool);
  cout << "============ max_num_test =========" << endl;
  std::vector<void *> ptrs;
  std::cout << "max valid num: " << mem_pool->MaxAllocSize() << std::endl;
  for (int i = 0; i <= mem_pool->MaxAllocSize(); ++i) {
    void *ptr = mem_pool->AllocMemory(MINUNITSIZE - 1);
    if (ptr == nullptr) {
      std::cout << "alloc failed !" << std::endl;
      break;
    }
    ptrs.emplace_back(ptr);
  }
  std::cout << "success num: " << ptrs.size() << std::endl;
  mem_pool->PrintChunkInfo();
  for (auto &p: ptrs) {
    mem_pool->FreeMemory(p);
  }
  mem_pool->PrintChunkInfo();
  munmap(pBuf, sBufSize);
}

int main(int argc, char *argv[]) {
  max_num_test();
  return 0;
}
