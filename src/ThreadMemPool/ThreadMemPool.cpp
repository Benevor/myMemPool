//
// Created by 26473 on 2022/11/2.
//

// TODO: 头文件互相引用怎么办？
//#include "ThreadMemPool.h"
#include "utils.h"
#include <cmath>

// 申请内存，找 free_chunk_
void *ThreadMemPool::AllocMemory(size_t sMemorySize) {
  sMemorySize = Utils::check_align_size(sMemorySize);
  size_t index = 0;
  memory_chunk *tmp = this->free_chunk_;
  // 遍历 free_chunk_ 寻找满足大小的 chunk
  for (; index < this->free_chunk_count_; index++) {
    if (tmp->map_addr->count * BLOCK_SIZE >= sMemorySize) {
      break;
    }
    tmp = tmp->next;
  }
  if (index == this->free_chunk_count_) { // 分配失败
    return nullptr;
  }

  this->mem_used_size_ += sMemorySize;
  if (tmp->map_addr->count * BLOCK_SIZE == sMemorySize) {
    // 当请求的内存大小与当前chunk中的内存大小相同时
    // 直接从 free_chunk_ 链表中删除此chunk，不需要对chunk进行截断
    size_t current_index = tmp->map_addr - this->chunk_map_;
    Utils::delete_chunk(this->free_chunk_, tmp);
    tmp->map_addr->chunk_addr = nullptr; // nullptr表示当前内存块已被分配

    Utils::push_front(this->chunk_pool_, tmp);
    this->free_chunk_count_--;
    this->chunk_pool_count_++;

    return Utils::index2addr(this, current_index);
  } else {
    // 当要分配的内存小于当前chunk中的内存时
    // 需要对chunk进行截断和分裂。更改 free_chunk_ 中相应 chunk 的 map_addr
    // 复制当前 map_unit
    map_unit *current_map = tmp->map_addr;
    map_unit copy;
    copy.start = current_map->start;
    copy.count = current_map->count;
    copy.chunk_addr = current_map->chunk_addr;  // 理应等于tmp

    // 更新当前 map_unit，进行截断
    current_map->count = sMemorySize / BLOCK_SIZE;
    current_map->chunk_addr = nullptr; // nullptr表示当前内存块已被分配
    // 更新当前map_unit对应的结束map_unit
    size_t current_index = current_map - this->chunk_map_;
    this->chunk_map_[current_index + current_map->count - 1].start = current_index;

    // 更新第二个map_unit，指向原来的free_chunk
    this->chunk_map_[current_index + current_map->count].count = copy.count - current_map->count;
    this->chunk_map_[current_index + current_map->count].chunk_addr = copy.chunk_addr;
    // 更新第二个map_unit对应的结束map_unit
    this->chunk_map_[current_index + copy.count - 1].start = current_index + current_map->count;

    // 更新原来 free_chunk 的 map_addr，指向第二个map_unit
    tmp->map_addr = &(this->chunk_map_[current_index + current_map->count]);

    // 根据map_unit索引，获得数据区对应 block 的地址
    return Utils::index2addr(this, current_index);
  }
}

// 释放内存，先找 chunk_map_，对地址相邻的空闲 block 序列进行合并
void ThreadMemPool::FreeMemory(void *ptrMemoryBlock) {
  size_t current_index = Utils::addr2index(this, ptrMemoryBlock);
  size_t size = this->chunk_map_[current_index].count * BLOCK_SIZE;
  // 判断与当前释放的内存块相邻的内存块是否可以与当前释放的内存块合并
  map_unit *pre_block = nullptr;
  map_unit *next_block = nullptr;
  map_unit *current_map = &(this->chunk_map_[current_index]);
  // 当前 chunk 对应数据区的第一个 block 序列
  if (current_index == 0) {
    if (current_map->count < this->mem_block_count_) {
      next_block = &(this->chunk_map_[current_index + current_map->count]);
      // 如果后一个 block 序列是空闲的，合并
      if (next_block->chunk_addr != nullptr) {
        next_block->chunk_addr->map_addr = current_map;
        this->chunk_map_[current_index + current_map->count + next_block->count - 1].start = current_index;
        current_map->count += next_block->count;
        current_map->chunk_addr = next_block->chunk_addr;
        next_block->chunk_addr = nullptr;
      } else { // 如果后一个 block 序列不是空闲的，在free_chunk_中增加一个chunk
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
  } else if ((current_index + current_map->count - 1) == (this->mem_block_count_ - 1)) { //判断是否为最后1个 block 序列
    if (current_map->count < this->mem_block_count_) {
      pre_block = &(this->chunk_map_[this->chunk_map_[current_index - 1].start]);

      // 如果前一个 block 序列是空闲的，合并
      if (pre_block->chunk_addr != nullptr) {
        this->chunk_map_[current_index + current_map->count - 1].start = current_index - pre_block->count;
        pre_block->count += current_map->count;
        current_map->chunk_addr = nullptr;
      } else { // 如果前一个 block 序列不是空闲的，在 free_chunk_ 中增加一个 chunk
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
  } else { // 该 chunk 对应的 block 序列处于中间位置
    next_block = &(this->chunk_map_[current_index + current_map->count]);
    pre_block = &(this->chunk_map_[this->chunk_map_[current_index - 1].start]);
    bool has_merged = false;
    // 前后 block 序列都已经被占用，无法合并，则申请一个 chunk 作为 free_chunk
    if (next_block->chunk_addr == nullptr && pre_block->chunk_addr == nullptr) {
      memory_chunk *new_chunk = Utils::front_pop(this->chunk_pool_);
      new_chunk->map_addr = current_map;
      current_map->chunk_addr = new_chunk;
      Utils::push_back(this->free_chunk_, new_chunk);
      this->chunk_pool_count_--;
      this->free_chunk_count_++;
    }
    // 后一个 block 序列空闲， 可以合并，修改free_chunk
    if (next_block->chunk_addr != nullptr) {
      next_block->chunk_addr->map_addr = current_map;
      this->chunk_map_[current_index + current_map->count + next_block->count - 1].start = current_index;
      current_map->count += next_block->count;
      current_map->chunk_addr = next_block->chunk_addr;
      next_block->chunk_addr = nullptr;
      has_merged = true;
    }
    // 前一个 block 序列空闲，可以合并
    if (pre_block->chunk_addr != nullptr) {
      this->chunk_map_[current_index + current_map->count - 1].start = current_index - pre_block->count;
      pre_block->count += current_map->count;
      if (has_merged) { // 如果前后都可以合并，则需要删除一个free_chunk
        Utils::delete_chunk(this->free_chunk_, current_map->chunk_addr);
        Utils::push_front(this->chunk_pool_, current_map->chunk_addr);
        this->free_chunk_count_--;
        this->chunk_pool_count_++;
      }
      current_map->chunk_addr = nullptr;
    }
  }
  // 更新 ThreadMemPool 已分配空间的大小
  this->mem_used_size_ -= size;
}

// 根据内存空间创建 ThreadMemPool 的实例
void CreateMemoryPool(void *pBuf, size_t sBufSize, ThreadMemPool *&mem_pool) {
  // 确保分配的内存已经清空（后面实现多线程的时候，这里传入的空间可能是不干净的）
  memset(pBuf, 0, sBufSize);
  // 将该内存空间进行类型转换
  mem_pool = reinterpret_cast<ThreadMemPool *>(pBuf);
  // 拿到指针等元数据的空间
  size_t meta_size = sizeof(ThreadMemPool);

  // 初始化 chunk_map_ 与 chunk_pool_ （保证 chunk 数量大于真实数据 block 数量）
  size_t ond_size = BLOCK_SIZE + sizeof(memory_chunk) + sizeof(map_unit);
  mem_pool->chunk_pool_count_ = (sBufSize - meta_size) / ond_size;
  mem_pool->chunk_map_count_ = (sBufSize - meta_size) / ond_size;
  mem_pool->chunk_map_ = (map_unit *) ((char *) pBuf + meta_size);
  mem_pool->chunk_pool_ =
      (memory_chunk *) ((char *) pBuf + meta_size + sizeof(map_unit) * mem_pool->chunk_map_count_);

  // 初始化 memory
  mem_pool->memory_ = (char *) pBuf + meta_size + sizeof(map_unit) * mem_pool->chunk_map_count_
      + sizeof(memory_chunk) * mem_pool->chunk_pool_count_;
  mem_pool->size_ = sBufSize - meta_size - sizeof(map_unit) * mem_pool->chunk_map_count_
      - sizeof(memory_chunk) * mem_pool->chunk_pool_count_;
  size_t align = Utils::check_align_addr(mem_pool->memory_);
  mem_pool->size_ -= align;
  mem_pool->size_ = Utils::check_align_block(mem_pool->size_);
  mem_pool->mem_block_count_ = std::min(mem_pool->size_ / BLOCK_SIZE, mem_pool->chunk_map_count_);

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

  // 初始化 chunk_map_，现在只有一个最大号的chunk
  mem_pool->chunk_map_[0].count = mem_pool->mem_block_count_;
  mem_pool->chunk_map_[0].chunk_addr = tmp;
  mem_pool->chunk_map_[mem_pool->mem_block_count_ - 1].start = 0;

}

size_t ThreadMemPool::MaxAllocSize() {
  return this->mem_block_count_;
}

void ThreadMemPool::PrintInfo() {
  std::cout << "======================== Memory Pool Info ========================" << std::endl;
  std::cout << "meta_size: " << sizeof(ThreadMemPool) << std::endl;
  std::cout << "chunk_pool_count_: " << this->chunk_pool_count_ << std::endl;
  std::cout << "chunk_map_count_: " << this->chunk_map_count_ << std::endl;
  printf("chunk_map_: %p\n", this->chunk_map_);
  printf("chunk_pool_: %p\n", this->chunk_pool_);
  printf("memory: %p\n", this->memory_);
  std::cout << "memory size: " << this->size_ << std::endl;
  std::cout << "mem_block_count_: " << this->mem_block_count_ << std::endl;

  printf("free_chunk_: %p\n", this->free_chunk_);
  std::cout << "free_chunk_count_: " << this->free_chunk_count_ << std::endl;

  std::cout << "mem_used_size_: " << this->mem_used_size_ << std::endl;
}

void ThreadMemPool::PrintChunkInfo() {
  std::cout << "======================== Free Chunk Info ========================" << std::endl;
  printf("free_chunk_: %p\n", this->free_chunk_);
  std::cout << "free_chunk_count_: " << this->free_chunk_count_ << std::endl;
  auto tmp = this->free_chunk_;
  for (int i = 0; i < free_chunk_count_; ++i) {
    std::cout << "\n============ Free Chunk " << i << " ============" << std::endl;
    std::cout << "chunk addr:" << tmp << std::endl;
    std::cout << "map addr:" << tmp->map_addr << std::endl;
    std::cout << "map index:" << tmp->map_addr - this->chunk_map_ << std::endl;
    std::cout << "unit count:" << tmp->map_addr->count << std::endl;
    std::cout << "unit chunk addr:" << tmp->map_addr->chunk_addr << std::endl;
    tmp = tmp->next;
  }
  std::cout << "======================== Chunk Info ========================" << std::endl;
  std::cout << "chunk_pool_count_:" << this->chunk_pool_count_ << std::endl;
}