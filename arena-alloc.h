#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
/// A fixed-capacity, non-owning arena that allocates raw storage.
/// Individual allocations cannot be freed; reset_arena() reclaims all storage.
/// Concurrent access requires external synchronization.
class ArenaAllocator{
  private:
    char* _arena_start;
    std::size_t _capacity;
    std::size_t _size;
  public:
    /// Creates an empty arena over caller-provided storage.
    /// @param buffer Pointer to contiguous writable storage.
    /// @param capacity Number of bytes available in the storage.
    /// @pre buffer points to at least capacity writable bytes and remains valid
    /// while the arena or any of its allocations are in use.
    /// @note The caller owns the backing storage and manages its lifetime.
    ArenaAllocator(char* buffer, std::size_t capacity) :  _arena_start(buffer),_capacity(capacity),_size(0) {};

    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

    ArenaAllocator(ArenaAllocator&&) = delete;
    ArenaAllocator& operator=(ArenaAllocator&&) = delete;

    /// Allocates raw storage with byte alignment (equivalent to alignment 1).
    /// @param size_req Number of bytes requested.
    /// @return Pointer to storage, or nullptr for zero size or insufficient space.
    /// @note Use the explicit-alignment overload for stronger alignment needs.
    /// Failed requests leave the arena unchanged. No objects are constructed.
    void* alloc(std::size_t size_req){
      return alloc(size_req,1);
    }
    /// Allocates contiguous raw storage with the requested alignment.
    /// @param size_req Number of usable bytes requested.
    /// @param alignment Required starting-address alignment in bytes;
    /// must be a nonzero power of two.
    /// @return Pointer to aligned storage, or nullptr for zero size, invalid
    /// alignment, or insufficient space including alignment padding.
    /// @note Failed requests leave the arena unchanged. Consumed space includes
    /// padding. No objects are constructed. Allocations remain reserved until
    /// reset_arena(); the backing storage must remain alive throughout use.
    void* alloc(std::size_t size_req,std::size_t alignment ){
      if(size_req == 0 || alignment == 0 || (alignment & (alignment - 1)) != 0) return nullptr;
      auto boundery_dis = (reinterpret_cast<std::uintptr_t>(this->_arena_start) + this->size()) % alignment;
      std::size_t padding = 0;
      if(boundery_dis != 0){
        padding = alignment - boundery_dis;
      }
      if(this->remaining() < padding){
        return nullptr;
      }
      if(this->remaining() - padding < size_req){
        return nullptr;
      }
      void *ptr = &this->_arena_start[this->_size + padding];
      this->_size += padding + size_req;

      return ptr;
    }
    /// Makes all arena storage available for reuse and resets used space to zero.
    /// @note Callers must stop using previous allocations. This does not run
    /// destructors or clear the backing storage.
    void reset_arena(){
      this->_size = 0;
    }
    /// @return Number of consumed bytes, including alignment padding.
    std::size_t size() const {
      return this->_size;
    }
    /// @return Number of unused bytes; future alignment padding may consume some.
    std::size_t remaining() const {
      return this->_capacity - this->size();
    }
    ~ArenaAllocator(){}
};

