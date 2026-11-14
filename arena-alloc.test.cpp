#include "./arena-alloc.h"
#include <cstddef>
#include <iostream>
#include <limits>
#include <cassert>

void initial_state_test(){
  alignas(64) char buffer[1024]{};
  ArenaAllocator alloc(buffer,1024);
  assert(alloc.size() == 0 && "Size of a freshly initialized allocator to be 0");
  assert(alloc.remaining() == 1024 && "Size of the array was expected to be 1024 but got something else");

  return;
}
void basic_allocation_test(){
  alignas(64) char buffer[1024]{};
  ArenaAllocator alloc(buffer,1024);
  char *requested_buffer_from_arena = static_cast<char*>(alloc.alloc(10));
  assert(requested_buffer_from_arena);
  assert(alloc.size() == 10 && "Excpected size was 10 bytes");

  return;
}

void multile_allocation_test(){
  alignas(64) char buffer[1024]{};
  ArenaAllocator alloc(buffer,1024);
  char *requested_buffer_from_arena = static_cast<char*>(alloc.alloc(2));
  assert(requested_buffer_from_arena != nullptr && "Returned pointer shouldn't be null");
  assert(alloc.size() == 2 && "Excpected size was 2 bytes");
  requested_buffer_from_arena[0] = 'a';
  requested_buffer_from_arena[1] = 'b';
  char *requested_buffer_from_arena_2 = static_cast<char*>(alloc.alloc(2));
  assert(requested_buffer_from_arena_2 != nullptr && "Returned pointer shouldn't be null");
  assert(alloc.size() == 4 && "Excpected size was 4 bytes");
  requested_buffer_from_arena_2[0] = 'c';
  requested_buffer_from_arena_2[1] = 'd';
  char *requested_buffer_from_arena_3 = static_cast<char*>(alloc.alloc(2));
  assert(requested_buffer_from_arena_3 != nullptr && "Returned pointer shouldn't be null");
  assert(alloc.size() == 6 && "Excpected size was 6 bytes");
  requested_buffer_from_arena_3[0] = 'e';
  requested_buffer_from_arena_3[1] = 'f';
  assert((
        requested_buffer_from_arena[0] == 'a' &&
        requested_buffer_from_arena[1] == 'b' &&
        requested_buffer_from_arena_2[0] == 'c' &&
        requested_buffer_from_arena_2[1] == 'd' &&
        requested_buffer_from_arena_3[0] == 'e' &&
        requested_buffer_from_arena_3[1] == 'f'
        ) && "Requesting more buffers shouldn't overlap the prev ones.");
  return;
}


void exact_allocation_test(){
  alignas(64) char buffer[1024]{};
  ArenaAllocator alloc(buffer,1024);
  char *exact_buffer = static_cast<char*>(alloc.alloc(1024));
  assert(exact_buffer != nullptr);
  assert(alloc.size() == 1024 && "buffer size was expected to be 1024");
  assert(alloc.remaining() == 0 && "remaining space was expected to be 0");
  void *p = alloc.alloc(1);
  assert(p == nullptr && "should return nullptr on requesting from an entirely consumed buffer");
  assert(alloc.size() == 1024 && alloc.remaining() == 0 && "size and capacity should remain unchanged after failing allocation");
  return;
}


void exhaustion_allocation_test(){
  alignas(64) char buffer[10]{};
  ArenaAllocator alloc(buffer,sizeof buffer);
  auto p = alloc.alloc(7);
  assert(p != nullptr);
  auto size_before = alloc.size();
  p = alloc.alloc(3,8);
  assert(
      p == nullptr
      && "Requesting 3 bytes with 8 alignment should fail when 7 bytes are already consumed from a 64 aligned 10 byte buffer"
      );
  assert(
      alloc.size() == size_before && "After failing the allocation the size should be the same as before"
      );
  alloc.reset_arena();
  p = alloc.alloc(9);
  assert(p != nullptr);
  size_before = alloc.size();
  p = alloc.alloc(1,8);
  assert(p == nullptr && "padding exceeds remaining space");
  assert(
      alloc.size() == size_before && "After failing the allocation the size should be the same as before"
      );


  return;
}

void reset_test(){
  alignas(64) char buffer[1024]{};
  ArenaAllocator alloc(buffer,1024);
  char * test_buffer = static_cast<char*>(alloc.alloc(alloc.remaining()));
  assert(test_buffer != nullptr);
  assert(alloc.remaining() == 0 && alloc.size() == 1024 && "After requesting the entire buffer, remaining should be 0");
  alloc.reset_arena();
  assert((alloc.remaining() == 1024 && alloc.size() == 0) && "After reseting remaining should be 1024 and size 0 ");
  test_buffer = static_cast<char*>(alloc.alloc(alloc.remaining()));
  assert(test_buffer != nullptr && "After reseting, should be able to request memory again.");
  assert(alloc.remaining() == 0 && "After requesting the entire buffer for the second time, remaining should be 0");
  return;
}



void huge_alloc_request_test(){
  alignas(64) char buffer[1]{};
  ArenaAllocator alloc(buffer,1);
  auto p = alloc.alloc(1);
  assert(p!=nullptr);
  auto size_before = alloc.size();
  p = alloc.alloc(std::numeric_limits<std::size_t>::max());
  assert(  p == nullptr
      && "Requesting SIZE_MAX should return nullptrt");
  assert(size_before == alloc.size());
  return;
}

void invalid_request_test(){
  alignas(64) char buffer[1024]{};
  ArenaAllocator alloc(buffer,1024);
  void * p = alloc.alloc(0);
  assert(  p == nullptr && "Requesting buffer of 0 size should return nullptr");
  assert(alloc.size() == 0);
  p = alloc.alloc(10,0);
  assert( p == nullptr && "Requesting buffer with alignment of 0 size should return nullptr");
  assert(alloc.size() == 0);
  p = alloc.alloc(10,6);
  assert( p == nullptr && "Requesting buffer with alignment that is not a pow 2 should return nullptr");
  assert(alloc.size() == 0);

  return;
}

void alignment_test(){
  alignas(64) char buffer[1024]{};
  ArenaAllocator alloc(buffer,1024);
  auto p = alloc.alloc(8,8);
  assert(p != nullptr);
  assert(reinterpret_cast<std::uintptr_t>(p) % 8 == 0 && "The address returned should be mod 0 with the requested alignment");

  return;
}
void misalignment_test(){
  alignas(64) char buffer[1024]{};
  ArenaAllocator alloc(buffer + 1,sizeof buffer - 1);
  auto p = alloc.alloc(8,8);
  assert(p != nullptr);
  assert(reinterpret_cast<std::uintptr_t>(p) % 8 ==0 && "The address returned should be mod 0 with the requested alignment");

  return;
}

int main () {
  initial_state_test();
  basic_allocation_test();
  multile_allocation_test();
  exact_allocation_test();
  exhaustion_allocation_test();
  alignment_test();
  misalignment_test();
  invalid_request_test();
  huge_alloc_request_test();
  reset_test();
  std::cout<<"All Test cases passed successfully\n";
  return 0;
}


