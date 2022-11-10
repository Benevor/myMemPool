//
// Created by 26473 on 2022/11/2.
//
#pragma once
#include "ThreadMemPool.h"
class Utils {
  // 辅助类，用于封装工具函数
 public:
  /**
   * 小于等于 size 的最大的 BLOCK_SIZE 的倍数
   * @param size
   * @return
   */
  static size_t check_align_block(size_t size) {
    size_t align = size % BLOCK_SIZE;
    return size - align;
  }

  /**
   * 大于等于 size 的最小的 BLOCK_SIZE 的倍数
   * @param size
   * @return
   */
  static size_t check_align_size(size_t size) {
    size = (size + SIZE_ALIGN - 1) / SIZE_ALIGN * SIZE_ALIGN;
    return size;
  }

  /**
  * ADDR_ALIGN 对齐
  * @param pBuf 待对齐的空间
  * @return 对齐移位
  */
  static size_t check_align_addr(void *&pBuf) {
    size_t addr = *(int *) pBuf;
    size_t align = (ADDR_ALIGN - addr % ADDR_ALIGN) % ADDR_ALIGN;
    pBuf = (char *) pBuf + align;
    return align;
  }

  /**
   * 双向链表初始化
   * @param pool 空间地址
   * @param count 成员数量
   * @return 链表首地址
   */
  static memory_chunk *create_list(memory_chunk *pool, size_t count) {
    if (!pool) {
      return nullptr;
    }
    memory_chunk *head = nullptr;
    for (size_t i = 0; i < count; i++) {
      pool->pre = nullptr;
      pool->next = head;
      if (head != nullptr) {
        head->pre = pool;
      }
      head = pool;
      pool++;
    }
    return head;
  }

  /**
   * 取出链表首元素
   * @param pool 链表首地址
   * @return 首元素地址
   */
  static memory_chunk *front_pop(memory_chunk *&pool) {
    if (!pool) {
      return nullptr;
    }
    memory_chunk *tmp = pool;
    pool = tmp->next;
    pool->pre = nullptr;
    return tmp;
  }

  /**
   * 添加元素到链表尾部。一般用于 free_chunk_
   * @param head 链表头指针
   * @param element 待添加元素
   */
  static void push_back(memory_chunk *&head, memory_chunk *element) {
    if (head == nullptr) {
      head = element;
      head->pre = element;
      head->next = element;
      return;
    }
    head->pre->next = element;
    element->pre = head->pre;
    head->pre = element;
    element->next = head;
  }

  /**
   * 添加元素到链表头部。一般用于 chunk_pool_
   * @param head 链表头指针
   * @param element 待添加元素
   */
  static void push_front(memory_chunk *&head, memory_chunk *element) {
    element->pre = nullptr;
    element->next = head;
    if (head != nullptr) {
      head->pre = element;
    }
    head = element;
  }

  /**
   * 删除链表上的指定元素
   * @param head 链表头指针
   * @param element 待删除元素
   */
  static void delete_chunk(memory_chunk *&head, memory_chunk *element) {
    if (element == nullptr) {
      return;
    } else if (element == head) {
      // 链表只有一个元素
      if (head->pre == head) {
        head = nullptr;
      } else {
        head = element->next;
        head->pre = element->pre;
        head->pre->next = head;
      }
    } else if (element->next == head) {
      // element为链表尾
      head->pre = element->pre;
      element->pre->next = head;
    } else {
      element->pre->next = element->next;
      element->next->pre = element->pre;
    }
    element->pre = nullptr;
    element->next = nullptr;
  }

  /**
   * 根据 map_unit 索引，找到对应的 block 地址
   * @param mem_pool ThreadMemPool
   * @param index map_unit 索引
   * @return 指针 / block 地址
   */
  static void *index2addr(ThreadMemPool *mem_pool, size_t index) {
    char *p = (char *) (mem_pool->getMemory());
    void *ret = (void *) (p + index * BLOCK_SIZE);
    return ret;
  }

  /**
   * 根据 block 地址，找到对应的 map_unit 索引
   * @param mem_pool ThreadMemPool
   * @param addr 指针 / block 地址
   * @return map_unit 索引
   */
  static size_t addr2index(ThreadMemPool *mem_pool, void *addr) {
    char *start = (char *) (mem_pool->getMemory());
    char *p = (char *) addr;
    size_t index = (p - start) / BLOCK_SIZE;
    return index;
  }
};