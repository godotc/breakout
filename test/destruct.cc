#include <cstdio>
#include <gtest/gtest.h>
#include <unordered_map>


using namespace std;


class MyClass
{
  public:
    MyClass() = default;
    MyClass(int a) : m_a(a) { cout << "Construct....\n"; }
    ~MyClass() { cout << "Destruct....\n"; }

    int m_a;
};

TEST(DISABLED_TestDestruction, should_call_destructon_when_call_equation_constructor)
{
    {
        unordered_map<string, MyClass> mp;

        MyClass a('a');


        mp["a"] = a;


        MyClass b('b');

        mp["a"] = b;
    }

    getchar();
}
