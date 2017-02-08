#include <iostream>

#include "vtest.h"

using namespace vtest;
using namespace std;

int main(int argc, char* argv[])
{
    return VTEST_RUN_ALL();
}

VTEST(t_simple)
{
    // print a text with color, use like printf
    TIP("vTest t_simple");

    // expect the two argument's value is equal
    EXPECT_EQ("1", "3");

    // print a tip text, and then call EXPECT_EQ
    VEXPECT_EQ("compare str 1 == 3", "1", "3");

    // expect the value is true
    EXPECT(1 == 1);

    // print a tip text, and then call EXPECT
    VEXPECT("compare int 1 == 3", 1 == 3);

    // assert the value is true
    VASSERT(1 != 0);
}