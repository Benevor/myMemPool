//
// Created by 26473 on 2022/11/2.
//
#pragma once
#include "MemPool.h"
class Utils {
 public:
  static size_t check_align_addr(void *&pBuf) {
    size_t addr = *(int *) pBuf;
    size_t align = (ADDR_ALIGN - addr % ADDR_ALIGN) % ADDR_ALIGN;
    pBuf = (char *) pBuf + align;
    return align;
  }

  /**
   * 小于等于 size 的最大的 64 的倍数
   * @param size
   * @return
   */
  static size_t check_align_block(size_t size) {
    size_t align = size % MINUNITSIZE;
    return size - align;
  }

  /**
   * 大于等于 size 的最小的 64 的倍数
   * @param size
   * @return
   */
  static size_t check_align_size(size_t size) {
    size = (size + SIZE_ALIGN - 1) / SIZE_ALIGN * SIZE_ALIGN;
    return size;
  }

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
  static memory_chunk *front_pop(memory_chunk *&pool) {
    if (!pool) {
      return nullptr;
    }
    memory_chunk *tmp = pool;
    pool = tmp->next;
    pool->pre = nullptr;
    return tmp;
  }
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
  static void push_front(memory_chunk *&head, memory_chunk *element) {
    element->pre = nullptr;
    element->next = head;
    if (head != nullptr) {
      head->pre = element;
    }
    head = element;
  }
  static void delete_chunk(memory_chunk *&head, memory_chunk *element) {
    // 在双循环链表中删除元素
    if (element == nullptr) {
      return;
    }
      // element为链表头
    else if (element == head) {
      // 链表只有一个元素
      if (head->pre == head) {
        head = nullptr;
      } else {
        head = element->next;
        head->pre = element->pre;
        head->pre->next = head;
      }
    }
      // element为链表尾
    else if (element->next == head) {
      head->pre = element->pre;
      element->pre->next = head;
    } else {
      element->pre->next = element->next;
      element->next->pre = element->pre;
    }
    element->pre = nullptr;
    element->next = nullptr;
  }

  static void *index2addr(MemPool *mem_pool, size_t index) {
    char *p = (char *) (mem_pool->getMemory());
    void *ret = (void *) (p + index * MINUNITSIZE);
    return ret;
  }

  static size_t addr2index(MemPool *mem_pool, void *addr) {
    char *start = (char *) (mem_pool->getMemory());
    char *p = (char *) addr;
    size_t index = (p - start) / MINUNITSIZE;
    return index;
  }
};