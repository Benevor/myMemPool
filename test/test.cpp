//
// Created by 26473 on 2022/11/2.
//

#include <sys/mman.h>　
#include "MemPool.h"
#include <iostream>
using namespace std;
#define MEMORY_POOL_BYTE_SIZE 5*1024*1024 //5M = 5M * 1024kb * 1024bytes
char g_memPool_Buffer[MEMORY_POOL_BYTE_SIZE]; //only for test_Func2()

void test_Func1() {
  size_t sBufSize = MEMORY_POOL_BYTE_SIZE;
  //  void *pBuf = malloc(sBufSize);
  void *pBuf = mmap(NULL, MEMORY_POOL_BYTE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  // void *pBuf = mmap(NULL, MEMORY_POOL_BYTE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, -1, 0);
  if (pBuf == MAP_FAILED) {
    cout << " test_Func1: mmap failed" << endl;
    return;
  }

  MemPool *mem_pool = nullptr;
  CreateMemoryPool(pBuf, sBufSize, mem_pool);
  //定义1个int型数组，10个元素
  void *p_void = mem_pool->AllocMemory(sizeof(int) * 10); //申请内存
  cout << "============Test-Func1+malloc+void=========" << endl;
  for (int i0 = 0; i0 < 10; i0++) {
    ((int *) p_void)[i0] = i0;
    cout << ((int *) p_void)[i0] << endl;
  }
  mem_pool->FreeMemory(p_void); //释放内存
  //
  //定义1个int型数组，10个元素
  int *p_int = (int *) mem_pool->AllocMemory(sizeof(int) * 10); //申请内存
  cout << "============Test-Func1+malloc+int=========" << endl;
  for (int i0 = 0; i0 < 10; i0++) {
    p_int[i0] = i0;
    cout << p_int[i0] << endl;
  }
  mem_pool->FreeMemory(p_int); //释放内存

  //    free(pBuf);
  munmap(pBuf, sBufSize);
}

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

int main(int argc, char *argv[]) {
  test_Func1(); //测试01：通过mmap函数开辟内存池所需的大内存
  test_Func2(); //测试02：通过静态全局变量开辟内存池所需的大内存
  return 0;
}
