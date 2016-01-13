/*
 * Copyright (c) 2015 ARM Limited
 */

#include <core-util/v2/functional.hpp>
#include <mbed-drivers/app.h>


void testprnt() {
    printf("%s\r\n", __PRETTY_FUNCTION__);
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
}
