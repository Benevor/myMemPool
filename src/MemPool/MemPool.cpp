//
// Created by 26473 on 2022/11/2.
//

// TODO: 头文件互相引用怎么办？
//#include "MemPool.h"
#include "utils.h"

void *MemPool::AllocMemory(size_t sMemorySize) {
  sMemorySize = Utils::check_align_size(sMemorySize);
  size_t index = 0;
  memory_chunk *tmp = this->pfree_mem_chunk;
  for (; index < this->free_mem_chunk_count; index++) {
    if (tmp->pfree_mem_addr->count * MINUNITSIZE >= sMemorySize) {
      break;
    }

    tmp = tmp->next;
  }

  if (index == this->free_mem_chunk_count) {
    return nullptr;
  }
  this->mem_used_size += sMemorySize;
  if (tmp->pfree_mem_addr->count * MINUNITSIZE == sMemorySize) {
    // 当要分配的内存大小与当前chunk中的内存大小相同时，从pfree_mem_chunk链表中删除此chunk
    size_t current_index = (tmp->pfree_mem_addr - this->pmem_map);
    Utils::delete_chunk(this->pfree_mem_chunk, tmp);
    tmp->pfree_mem_addr->pmem_chunk = nullptr;

    Utils::push_front(this->pfree_mem_chunk_pool, tmp);
    this->free_mem_chunk_count--;
    this->mem_map_pool_count++;

    return Utils::index2addr(this, current_index);
  } else {
    // 当要分配的内存小于当前chunk中的内存时，更改pfree_mem_chunk中相应chunk的pfree_mem_addr

    // 复制当前mem_map_unit
    memory_block copy;
    copy.count = tmp->pfree_mem_addr->count;
    copy.pmem_chunk = tmp;
    // 记录该block的起始和结束索引
    memory_block *current_block = tmp->pfree_mem_addr;
    current_block->count = sMemorySize / MINUNITSIZE;
    size_t current_index = (current_block - this->pmem_map);
    this->pmem_map[current_index + current_block->count - 1].start = current_index;
    current_block->pmem_chunk = nullptr; // nullptr表示当前内存块已被分配
    // 当前block被一分为二，更新第二个block中的内容
    this->pmem_map[current_index + current_block->count].count = copy.count - current_block->count;
    this->pmem_map[current_index + current_block->count].pmem_chunk = copy.pmem_chunk;
    // 更新原来的pfree_mem_addr
    tmp->pfree_mem_addr = &(this->pmem_map[current_index + current_block->count]);

    size_t end_index = current_index + copy.count - 1;
    this->pmem_map[end_index].start = current_index + current_block->count;
    return Utils::index2addr(this, current_index);
  }
}

void MemPool::FreeMemory(void *ptrMemoryBlock) {
  size_t current_index = Utils::addr2index(this, ptrMemoryBlock);
  size_t size = this->pmem_map[current_index].count * MINUNITSIZE;
  // 判断与当前释放的内存块相邻的内存块是否可以与当前释放的内存块合并
  memory_block *pre_block = nullptr;
  memory_block *next_block = nullptr;
  memory_block *current_block = &(this->pmem_map[current_index]);
  // 第一个
  if (current_index == 0) {
    if (current_block->count < this->mem_block_count) {
      next_block = &(this->pmem_map[current_index + current_block->count]);
      // 如果后一个内存块是空闲的，合并
      if (next_block->pmem_chunk != nullptr) {
        next_block->pmem_chunk->pfree_mem_addr = current_block;
        this->pmem_map[current_index + current_block->count + next_block->count - 1].start = current_index;
        current_block->count += next_block->count;
        current_block->pmem_chunk = next_block->pmem_chunk;
        next_block->pmem_chunk = nullptr;
      }
        // 如果后一块内存不是空闲的，在pfree_mem_chunk中增加一个chunk
      else {
        memory_chunk *new_chunk = Utils::front_pop(this->pfree_mem_chunk_pool);
        new_chunk->pfree_mem_addr = current_block;
        current_block->pmem_chunk = new_chunk;
        Utils::push_back(this->pfree_mem_chunk, new_chunk);
        this->mem_map_pool_count--;
        this->free_mem_chunk_count++;
      }
    } else {
      memory_chunk *new_chunk = Utils::front_pop(this->pfree_mem_chunk_pool);
      new_chunk->pfree_mem_addr = current_block;
      current_block->pmem_chunk = new_chunk;
      Utils::push_back(this->pfree_mem_chunk, new_chunk);
      this->mem_map_pool_count--;
      this->free_mem_chunk_count++;
    }
  }

    // 最后一个
  else if (current_index == this->mem_block_count - 1) //可能错误01：判断是否为最后1个的if条件有错误!
  {
    if (current_block->count < this->mem_block_count) {
      pre_block = &(this->pmem_map[current_index - 1]);
      size_t index = pre_block->start;
      pre_block = &(this->pmem_map[index]);

      // 如果前一个内存块是空闲的，合并
      if (pre_block->pmem_chunk != nullptr) {
        this->pmem_map[current_index + current_block->count - 1].start = current_index - pre_block->count;
        pre_block->count += current_block->count;
        current_block->pmem_chunk = nullptr;
      }
        // 如果前一块内存不是空闲的，在pfree_mem_chunk中增加一个chunk
      else {
        memory_chunk *new_chunk = Utils::front_pop(this->pfree_mem_chunk_pool);
        new_chunk->pfree_mem_addr = current_block;
        current_block->pmem_chunk = new_chunk;
        Utils::push_back(this->pfree_mem_chunk, new_chunk);
        this->mem_map_pool_count--;
        this->free_mem_chunk_count++;
      }
    } else {
      memory_chunk *new_chunk = Utils::front_pop(this->pfree_mem_chunk_pool);
      new_chunk->pfree_mem_addr = current_block;
      current_block->pmem_chunk = new_chunk;
      Utils::push_back(this->pfree_mem_chunk, new_chunk);
      this->mem_map_pool_count--;
      this->free_mem_chunk_count++;
    }
  } else {
    next_block = &(this->pmem_map[current_index + current_block->count]);
    pre_block = &(this->pmem_map[current_index - 1]);
    size_t index = pre_block->start;
    pre_block = &(this->pmem_map[index]);
    bool is_back_merge = false;
    if (next_block->pmem_chunk == nullptr && pre_block->pmem_chunk == nullptr) {
      memory_chunk *new_chunk = Utils::front_pop(this->pfree_mem_chunk_pool);
      new_chunk->pfree_mem_addr = current_block;
      current_block->pmem_chunk = new_chunk;
      Utils::push_back(this->pfree_mem_chunk, new_chunk);
      this->mem_map_pool_count--;
      this->free_mem_chunk_count++;
    }
    // 后一个内存块
    if (next_block->pmem_chunk != nullptr) {
      next_block->pmem_chunk->pfree_mem_addr = current_block;
      this->pmem_map[current_index + current_block->count + next_block->count - 1].start = current_index;
      current_block->count += next_block->count;
      current_block->pmem_chunk = next_block->pmem_chunk;
      next_block->pmem_chunk = nullptr;
      is_back_merge = true;
    }
    // 前一个内存块
    if (pre_block->pmem_chunk != nullptr) {
      this->pmem_map[current_index + current_block->count - 1].start = current_index - pre_block->count;
      pre_block->count += current_block->count;
      if (is_back_merge) {
        Utils::delete_chunk(this->pfree_mem_chunk, current_block->pmem_chunk);
        Utils::push_front(this->pfree_mem_chunk_pool, current_block->pmem_chunk);
        this->free_mem_chunk_count--;
        this->mem_map_pool_count++;
      }
      current_block->pmem_chunk = nullptr;
    }
  }
  this->mem_used_size -= size;
}

void CreateMemoryPool(void *pBuf, size_t sBufSize, MemPool *&mem_pool) {
  memset(pBuf, 0, sBufSize);
  mem_pool = reinterpret_cast<MemPool *>(pBuf);
  // 计算需要多少memory map单元格
  size_t mem_pool_struct_size = sizeof(MemPool);
  mem_pool->mem_map_pool_count = (sBufSize - mem_pool_struct_size + MINUNITSIZE - 1) / MINUNITSIZE;
  mem_pool->mem_map_unit_count = (sBufSize - mem_pool_struct_size + MINUNITSIZE - 1) / MINUNITSIZE;
  mem_pool->pmem_map = (memory_block *) ((char *) pBuf + mem_pool_struct_size);
  mem_pool->pfree_mem_chunk_pool =
      (memory_chunk *) ((char *) pBuf + mem_pool_struct_size + sizeof(memory_block) * mem_pool->mem_map_unit_count);

  mem_pool->memory = (char *) pBuf + mem_pool_struct_size + sizeof(memory_block) * mem_pool->mem_map_unit_count
      + sizeof(memory_chunk) * mem_pool->mem_map_pool_count;
  mem_pool->size = sBufSize - mem_pool_struct_size - sizeof(memory_block) * mem_pool->mem_map_unit_count
      - sizeof(memory_chunk) * mem_pool->mem_map_pool_count;
  size_t align = Utils::check_align_addr(mem_pool->memory);
  mem_pool->size -= align;
  mem_pool->size = Utils::check_align_block(mem_pool->size);
  mem_pool->mem_block_count = mem_pool->size / MINUNITSIZE;
  // 链表化
  mem_pool->pfree_mem_chunk_pool = Utils::create_list(mem_pool->pfree_mem_chunk_pool, mem_pool->mem_map_pool_count);
  // 初始化 pfree_mem_chunk，双向循环链表
  memory_chunk *tmp = Utils::front_pop(mem_pool->pfree_mem_chunk_pool);
  tmp->pre = tmp;
  tmp->next = tmp;
  tmp->pfree_mem_addr = nullptr;
  mem_pool->mem_map_pool_count--;

  // 初始化 pmem_map
  mem_pool->pmem_map[0].count = mem_pool->mem_block_count;
  mem_pool->pmem_map[0].pmem_chunk = tmp;
  mem_pool->pmem_map[mem_pool->mem_block_count - 1].start = 0;

  tmp->pfree_mem_addr = mem_pool->pmem_map;
  Utils::push_back(mem_pool->pfree_mem_chunk, tmp);
  mem_pool->free_mem_chunk_count = 1;
  mem_pool->mem_used_size = 0;
}