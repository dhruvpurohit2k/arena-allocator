
#include "arena-alloc.h"
#include <new>
#include <cassert>

class Widget{
  private:
    inline static int _constructor_counter = 0;
    inline static int _destructor_counter = 0;
    int int_member;
  public:
    Widget(int int_member_value) : int_member(int_member_value){
      ++_constructor_counter;
    }
    int get_int_member() const {return this->int_member;}
    void set_int_member(int new_value) {this->int_member = new_value;}
    static int get_constructor_counter(){return _constructor_counter;}
    static int get_destructor_counter(){return _destructor_counter;}
    ~Widget(){
      ++_destructor_counter;
    }
};

int main(){
  alignas(64) char buffer[1024]{};
  ArenaAllocator alloc(buffer,1024);

  void* storage = alloc.alloc(sizeof(Widget), alignof(Widget));
  assert(storage != nullptr);
  Widget *w1 = new (storage) Widget(10);
  assert(w1->get_int_member() == 10);
  w1->~Widget();
  alloc.reset_arena();
  assert(Widget::get_constructor_counter() == 1 && Widget::get_destructor_counter() == 1);
  assert(alloc.size() == 0);

  storage = alloc.alloc(sizeof(Widget), alignof(Widget));
  assert(storage != nullptr);
  Widget *w2 = new (storage) Widget(11);
  assert(w2->get_int_member() == 11);
  w2->~Widget();
  assert(Widget::get_constructor_counter() == 2 && Widget::get_destructor_counter() == 2);
}
