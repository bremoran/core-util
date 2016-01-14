/*
 * Copyright (c) 2015 ARM Limited
 */

#include <core-util/v2/functional.hpp>
#include <mbed-drivers/app.h>


void testprnt() {
    printf("%s\r\n", __PRETTY_FUNCTION__);
}
void testprnt3(int i, int j, int k) {
    printf("%s(%i, %i, %i)\r\n", __PRETTY_FUNCTION__, i, j, k);
}

class foo {
public:
    void testprnt() {
        printf("%s\r\n", __PRETTY_FUNCTION__);
    }
};

void app_start(int , char **)
{
    functional::Function<void(void)> f(testprnt);
    f();
    foo o;
    functional::Function<void()> f2(&o,&foo::testprnt);
    f2();
    int i = 1;
    functional::Function<void()> f3(
        [i]() {
            printf("%s(%i)\r\n", __PRETTY_FUNCTION__, i);
        }
    );
    f3();
    functional::Function<void(int,int,int)> f4(testprnt3);
    f4(1,2,3);
    static_assert(std::is_same<functional::Function<void(char,int)>,
        functional::detail::RemoveArgs<void(double,char,int),double>::type >::value, "oops");
    functional::Function<void(int,int)> f5 = f4.bind(4);
    f5(1,2);
    functional::Function<void(int)> f6 = f4.bind(4,5);
    f6(1);
    functional::Function<void()> f7 = f4.bind(4,5,6);
    f7();
}
