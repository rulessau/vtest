#include <iostream>

#include "vtest.h"

using namespace vtest;
using namespace std;

void init_env();

void test_manaul_add();

int main(int argc, char* argv[])
{
    // support arguments
    // -e set exit_on_failed = true, 
    //    (if set pause_on_exit, then will pause first) 
    // -p set pause_on_exit = true
    // -rx disable all region test, that means no test will run
    //     (combine with -ra, allow special tests run)
    // -ra=group1 allow the test in region group1 running
    // -rd=group1 disable the test in region group1 running
    VTEST_INIT(argc, argv);

    // manaul add test function
    VTEST_ADD(test_manaul_add);

    // the first run test
    VTEST_TOP_ADD(init_env);

    // disable all region test, that means no test will run
    //VTEST_DISABLE_ALL_REGION();

    // disable the test in region __root__ running
    // __root__ is the default region for all test that not decalre with special region 
    //VTEST_DISABLE_REGION("__root__");

    // disable the test in region k2 running
    //VTEST_DISABLE_REGION("k2");

    // allow the test in region group1 running
    //VTEST_ALLOW_REGION("k1");

    VASSERT(1 != 0);

    VTEST_RUN_ALL();

    return 0;
}

void init_env()
{
    printf("init unit test env here...\n");
}

void test_manaul_add()
{
    printf("manaul add test function...\n");
}

// auto add test function
VTEST(t_simple)
{
    TIPC("hello %d", 123);
    TIPC(" gaga %s ", "abc");
    TIP("this test function will auto run");
    EXPECT(1 == 1);
    EXPECT_EQ(1, 2);
    VEXPECT("test 1 == 3, false", 1 == 3);
    VEXPECT_EQ("test 1 == 3, false", "1", "3");
}

int count(int a, int b)
{
    return a + b;
}

VTEST(t_batch_test_with_return_value)
{
    TIP("batch test function count with multi test case.");

    // test cases format: expect output, input...
    ut_vars v = {
        { 2, 1, 1 },
        { 5, 1, 2 }
    };

    // function count has 2 params
    VBAT_CHECK_2(count, v);
}

VTEST(t_var)
{
    TIP("ut_var test...");
    {
        EXPECT(ut_var(true) == true);
        EXPECT((ut_var(true) == false) == false);
        EXPECT((ut_var(true) != true) == false);
        EXPECT(ut_var(true) != false);
    }
    {
        short v = 1, v2 = 2;
        EXPECT(ut_var(v) == 1);
        EXPECT((ut_var(v) == 0) == false);
        EXPECT((ut_var(v2) != 2) == false);
        EXPECT(ut_var(v2) != 0);
    }
    {
        int v = 1, v2 = 2;
        EXPECT(ut_var(v) == 1);
        EXPECT((ut_var(v) == 0) == false);
        EXPECT((ut_var(v2) != 2) == false);
        EXPECT(ut_var(v2) != 0);
    }
    {
        ut_i64 v = 1, v2 = 2;
        EXPECT(ut_var(v) == 1);
        EXPECT((ut_var(v) == 0) == false);
        EXPECT((ut_var(v2) != 2) == false);
        EXPECT(ut_var(v2) != 0);
    }
    {
        float v = 1, v2 = 2;
        EXPECT(ut_var(v) == (float)1);
        EXPECT((ut_var(v) == (float)0) == false);
        EXPECT((ut_var(v2) != (float)2) == false);
        EXPECT(ut_var(v2) != (float)0);
        EXPECT(ut_var(v) == 1);
        EXPECT((ut_var(v) == 0) == false);
        EXPECT((ut_var(v2) != 2) == false);
        EXPECT(ut_var(v2) != 0);
    }
    {
        double v = 1, v2 = 2;
        EXPECT(ut_var(v) == (double)1);
        EXPECT((ut_var(v) == (double)0) == false);
        EXPECT((ut_var(v2) != (double)2) == false);
        EXPECT(ut_var(v2) != (double)0);
        EXPECT(ut_var(v) == 1);
        EXPECT((ut_var(v) == 0) == false);
        EXPECT((ut_var(v2) != 2) == false);
        EXPECT(ut_var(v2) != 0);
    }
    {
        char* v = "1", *v2 = "2";
        EXPECT(ut_var(v) == "1");
        EXPECT((ut_var(v) == "0") == false);
        EXPECT((ut_var(v2) != "2") == false);
        EXPECT(ut_var(v2) != "0");
    }
    {
        string v = "1", v2 = "2";
        EXPECT(ut_var(v) == "1");
        EXPECT((ut_var(v) == "0") == false);
        EXPECT((ut_var(v2) != "2") == false);
        EXPECT(ut_var(v2) != "0");
    }
    {
        bool bv = true;
        ut_var v(bv);
        bool& bv2 = v;
        bv2 = false;
        EXPECT(v == false);
    }
    {
        char v = 'a';
        ut_var var(v);
        char& v2 = var;
        v2 = 'b';
        EXPECT(var == 'b');
    }
    {
        short v = 12;
        ut_var var(v);
        short& v2 = var;
        v2 = 11;
        EXPECT(var == 11);
    }
    {
        int v = 12;
        ut_var var(v);
        int& v2 = var;
        v2 = 11;
        EXPECT(var == 11);
    }
    {
        ut_i64 v = 12;
        ut_var var(v);
        ut_i64& v2 = var;
        v2 = 11;
        EXPECT(var == 11);
    }
    {
        float v = 12.3f;
        ut_var var(v);
        float& v2 = var;
        v2 = 11;
        EXPECT(var == (float)11);
        EXPECT(var == 11);
    }
    {
        double v = 12.3;
        ut_var var(v);
        double& v2 = var;
        v2 = 11;
        EXPECT(var == (double)11);
        EXPECT(var == 11);
    }
}

VTEST_REGION_PUSH(k1);

VTEST(t_haha)
{
    TIP("k1, from demo");
}

VTEST_REGION_POP(k1);


VTEST_REGION_PUSH(k2);

VTEST(t_haha2)
{
    TIP("k2, from demo");
}

VTEST_REGION_POP(k2);