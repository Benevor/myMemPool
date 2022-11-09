//
// Created by 26473 on 2022/11/2.
//

// TODO: 头文件互相引用怎么办？
//#include "MemPool.h"
#include "utils.h"
#include <iostream>

// alloc 找 free_chunk_
void *MemPool::AllocMemory(size_t sMemorySize) {
  sMemorySize = Utils::check_align_size(sMemorySize);
  size_t index = 0;
  memory_chunk *tmp = this->free_chunk_;
  // 遍历 free_chunk_ 寻找满足大小的 chunk
  for (; index < this->free_chunk_count_; index++) {
    if (tmp->map_addr->count * MINUNITSIZE >= sMemorySize) {
      break;
    }
    tmp = tmp->next;
  }
  if (index == this->free_chunk_count_) { // 分配失败
    return nullptr;
  }

  this->mem_used_size_ += sMemorySize;
  if (tmp->map_addr->count * MINUNITSIZE == sMemorySize) {
    // 当请求的内存大小与当前chunk中的内存大小相同时
    // 直接从 free_chunk_ 链表中删除此chunk
    size_t current_index = (tmp->map_addr - this->chunk_map_) / sizeof(map_unit);
    Utils::delete_chunk(this->free_chunk_, tmp);
    tmp->map_addr->chunk_addr = nullptr; // nullptr表示当前内存块已被分配

    Utils::push_front(this->chunk_pool_, tmp);
    this->free_chunk_count_--;
    this->chunk_pool_count_++;

    return Utils::index2addr(this, current_index);
  } else {
    // 当要分配的内存小于当前chunk中的内存时，更改 free_chunk_ 中相应 chunk 的 map_addr
    // 复制当前 map_unit
    map_unit *current_map = tmp->map_addr;
    map_unit copy;
    copy.start = current_map->start;
    copy.count = current_map->count;
    copy.chunk_addr = current_map->chunk_addr;  // 理应等于tmp

    // 记录该block的起始和结束索引
    current_map->count = sMemorySize / MINUNITSIZE;
    current_map->chunk_addr = nullptr; // nullptr表示当前内存块已被分配

    size_t current_index = (current_map - this->chunk_map_) / sizeof(map_unit);
    this->chunk_map_[current_index + current_map->count - 1].start = current_index;

    // 当前block被一分为二，更新第二个block中的内容
    this->chunk_map_[current_index + current_map->count].count = copy.count - current_map->count;
    this->chunk_map_[current_index + current_map->count].chunk_addr = copy.chunk_addr;
    this->chunk_map_[current_index + copy.count - 1].start = current_index + current_map->count;

    // 更新原来 chunk 的 map_addr
    tmp->map_addr = &(this->chunk_map_[current_index + current_map->count]);

    return Utils::index2addr(this, current_index);
  }
}

// free 找 chunk_map_
void MemPool::FreeMemory(void *ptrMemoryBlock) {
  size_t current_index = Utils::addr2index(this, ptrMemoryBlock);
  size_t size = this->chunk_map_[current_index].count * MINUNITSIZE;
  // 判断与当前释放的内存块相邻的内存块是否可以与当前释放的内存块合并
  map_unit *pre_block = nullptr;
  map_unit *next_block = nullptr;
  map_unit *current_map = &(this->chunk_map_[current_index]);
  // 第一个 chunk
  if (current_index == 0) {
    if (current_map->count < this->mem_block_count_) {
      next_block = &(this->chunk_map_[current_index + current_map->count]);
      // 如果后一个内存块是空闲的，合并
      if (next_block->chunk_addr != nullptr) {
        next_block->chunk_addr->map_addr = current_map;
        this->chunk_map_[current_index + current_map->count + next_block->count - 1].start = current_index;
        current_map->count += next_block->count;
        current_map->chunk_addr = next_block->chunk_addr;
        next_block->chunk_addr = nullptr;
      } else { // 如果后一块内存不是空闲的，在free_chunk_中增加一个chunk
        memory_chunk *new_chunk = Utils::front_pop(this->chunk_pool_);
        new_chunk->map_addr = current_map;
        current_map->chunk_addr = new_chunk;
        Utils::push_back(this->free_chunk_, new_chunk);
        this->chunk_pool_count_--;
        this->free_chunk_count_++;
      }
    } else { // current_map->count == this->mem_block_count_
      memory_chunk *new_chunk = Utils::front_pop(this->chunk_pool_);
      new_chunk->map_addr = current_map;
      current_map->chunk_addr = new_chunk;
      Utils::push_back(this->free_chunk_, new_chunk);
      this->chunk_pool_count_--;
      this->free_chunk_count_++;
    }
  } else if ((current_index + current_map->count - 1) == (this->mem_block_count_ - 1)) { //判断是否为最后1个chunk
    if (current_map->count < this->mem_block_count_) {
      pre_block = &(this->chunk_map_[this->chunk_map_[current_index - 1].start]);

      // 如果前一个内存块是空闲的，合并
      if (pre_block->chunk_addr != nullptr) {
        this->chunk_map_[current_index + current_map->count - 1].start = current_index - pre_block->count;
        pre_block->count += current_map->count;
        current_map->chunk_addr = nullptr;
      } else { // 如果前一块内存不是空闲的，在free_chunk_中增加一个chunk
        memory_chunk *new_chunk = Utils::front_pop(this->chunk_pool_);
        new_chunk->map_addr = current_map;
        current_map->chunk_addr = new_chunk;
        Utils::push_back(this->free_chunk_, new_chunk);
        this->chunk_pool_count_--;
        this->free_chunk_count_++;
      }
    } else { // current_map->count == this->mem_block_count_
      memory_chunk *new_chunk = Utils::front_pop(this->chunk_pool_);
      new_chunk->map_addr = current_map;
      current_map->chunk_addr = new_chunk;
      Utils::push_back(this->free_chunk_, new_chunk);
      this->chunk_pool_count_--;
      this->free_chunk_count_++;
    }
  } else {
    next_block = &(this->chunk_map_[current_index + current_map->count]);
    pre_block = &(this->chunk_map_[this->chunk_map_[current_index - 1].start]);
    bool has_merged = false;
    // 前后都已经被分配
    if (next_block->chunk_addr == nullptr && pre_block->chunk_addr == nullptr) {
      memory_chunk *new_chunk = Utils::front_pop(this->chunk_pool_);
      new_chunk->map_addr = current_map;
      current_map->chunk_addr = new_chunk;
      Utils::push_back(this->free_chunk_, new_chunk);
      this->chunk_pool_count_--;
      this->free_chunk_count_++;
    }
    // 后一个内存块空闲
    if (next_block->chunk_addr != nullptr) {
      next_block->chunk_addr->map_addr = current_map;
      this->chunk_map_[current_index + current_map->count + next_block->count - 1].start = current_index;
      current_map->count += next_block->count;
      current_map->chunk_addr = next_block->chunk_addr;
      next_block->chunk_addr = nullptr;
      has_merged = true;
    }
    // 前一个内存块空闲
    if (pre_block->chunk_addr != nullptr) {
      this->chunk_map_[current_index + current_map->count - 1].start = current_index - pre_block->count;
      pre_block->count += current_map->count;
      if (has_merged) {
        Utils::delete_chunk(this->free_chunk_, current_map->chunk_addr);
        Utils::push_front(this->chunk_pool_, current_map->chunk_addr);
        this->free_chunk_count_--;
        this->chunk_pool_count_++;
      }
      current_map->chunk_addr = nullptr;
    }
  }
  this->mem_used_size_ -= size;
}

void CreateMemoryPool(void *pBuf, size_t sBufSize, MemPool *&mem_pool) {
  // 确保分配的内存已经清空
  memset(pBuf, 0, sBufSize);
  // 将该内存空间进行类型转换
  mem_pool = reinterpret_cast<MemPool *>(pBuf);
  // 拿到指针等元数据的空间
  size_t meta_size = sizeof(MemPool);

  // 初始化 chunk_map_ 与 chunk_pool_ （保证 chunk 数量大于真实数据 block 数量）
  mem_pool->chunk_pool_count_ = (sBufSize - meta_size + MINUNITSIZE - 1) / MINUNITSIZE;
  mem_pool->chunk_map_count_ = (sBufSize - meta_size + MINUNITSIZE - 1) / MINUNITSIZE;
  mem_pool->chunk_map_ = (map_unit *) ((char *) pBuf + meta_size);
  mem_pool->chunk_pool_ =
      (memory_chunk *) ((char *) pBuf + meta_size + sizeof(map_unit) * mem_pool->chunk_map_count_);

  // 初始化 memory
  mem_pool->memory = (char *) pBuf + meta_size + sizeof(map_unit) * mem_pool->chunk_map_count_
      + sizeof(memory_chunk) * mem_pool->chunk_pool_count_;
  mem_pool->size_ = sBufSize - meta_size - sizeof(map_unit) * mem_pool->chunk_map_count_
      - sizeof(memory_chunk) * mem_pool->chunk_pool_count_;
  size_t align = Utils::check_align_addr(mem_pool->memory);
  mem_pool->size_ -= align;
  mem_pool->size_ = Utils::check_align_block(mem_pool->size_);
  mem_pool->mem_block_count_ = mem_pool->size_ / MINUNITSIZE;

  // 链表化chunk_pool_ （双向循环链表）
  mem_pool->chunk_pool_ = Utils::create_list(mem_pool->chunk_pool_, mem_pool->chunk_pool_count_);

  // 初始化 free_chunk_ （从 chunk_pool_ 取出一个 chunk）
  memory_chunk *tmp = Utils::front_pop(mem_pool->chunk_pool_);
  tmp->pre = tmp;
  tmp->next = tmp;
  tmp->map_addr = mem_pool->chunk_map_;
  mem_pool->chunk_pool_count_--;
  Utils::push_back(mem_pool->free_chunk_, tmp);
  mem_pool->free_chunk_count_ = 1;
  mem_pool->mem_used_size_ = 0;

  // 初始化 chunk_map_
  mem_pool->chunk_map_[0].count = mem_pool->mem_block_count_;
  mem_pool->chunk_map_[0].chunk_addr = tmp;
  mem_pool->chunk_map_[mem_pool->mem_block_count_ - 1].start = 0;  // 理解了

  std::cout << mem_pool->chunk_map_ << std::endl;
  std::cout << &mem_pool->chunk_map_[0] << std::endl;
  std::cout << &mem_pool->chunk_map_[1] << std::endl;
  std::cout << &mem_pool->chunk_map_[2] << std::endl;

}