#include <type_traits>



#define PROPERTY(TYPE, NAME, DEFAULT)                   \
  private:                                              \
    TYPE NAME##__Generate = {DEFAULT};                  \
                                                        \
  public:                                               \
    TYPE Get##NAME() const { return this->NAME##__Generate; } \
    void Set##NAME(TYPE value) { this->NAME##__Generate = value; }
