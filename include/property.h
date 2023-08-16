
#define PROPERTY(TYPE, NAME)                            \
  private:                                              \
    TYPE NAME##_Generate;                               \
                                                        \
  public:                                               \
    TYPE Get##NAME() const { return NAME##__Generate; } \
    void Set##NAME(TYPE value) { NAME##__Generate = value; }
