/*
 * Copyright (C) 2015 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

// To compile on Linux, use: g++ -g -pthread -std=c++11 tst-tls.cc

#include <iostream>
#include <thread>

static int tests = 0, fails = 0;

// A thread-local variable used in a shared object and visible to other shared
// objects uses the "General Dynamic" TLS model (sometimes known as Global
// Dynamic).
__thread int v1 = 123;
__thread int v2 = 234;

// If the compiler knows the thread-local variable must be defined in the same
// shared object, it can use the "Local Dynamic" TLS model.
static __thread int v3 = 345;
static __thread int v4 = 456;

// The compiler and linker won't normally use the "Initial Exec" TLS model in
// a shared library, but some libraries are compiled with
// -ftls-model=initial-exec, and we can force this model on one variable
// with an attribute.
__thread int v5 __attribute__ ((tls_model ("initial-exec"))) = 567;

#ifndef __OSV__
// We can also try to force the "Local Exec" TLS model, but OSv's makefile
// builds all tests as shared objects (.so), and the linker will report an
// error, because local-exec is not allowed in shared libraries, just in
// executables (including PIE).
__thread int v6 __attribute__ ((tls_model ("local-exec"))) = 678;
#endif


static void report(bool ok, std::string msg)
{
    ++tests;
    fails += !ok;
    std::cout << (ok ? "PASS" : "FAIL") << ": " << msg << "\n";
}

int main(int argc, char** argv)
{
    report(v1 == 123, "v1");
    report(v2 == 234, "v2");
    report(v3 == 345, "v3");
    report(v4 == 456, "v4");
    report(v5 == 567, "v5");
#ifndef __OSV__
    report(v6 == 678, "v6");
#endif

    // Write on this thread's variables, and see a new thread gets
    // the original default values
    v1 = 0;
    v2 = 0;
    v3 = 0;
    v4 = 0;
    v5 = 0;
#ifndef __OSV__
    v6 = 0;
#endif

    // Try the same in a new thread
    std::thread t1([] {
            report(v1 == 123, "v1 in new thread");
            report(v2 == 234, "v2 in new thread");
            report(v3 == 345, "v3 in new thread");
            report(v4 == 456, "v4 in new thread");
            report(v5 == 567, "v5 in new thread");
#ifndef __OSV__
            report(v6 == 678, "v6 in new thread");
#endif
    });
    t1.join();

    std::cout << "SUMMARY: " << tests << " tests, " << fails << " failures\n";
}