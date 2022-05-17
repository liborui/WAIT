#include "benchmark.h"


void __attribute__((noinline)) rtcbenchmark_measure_native_performance() {
    rtc_startBenchmarkMeasurement_Native();

    printStr("Hello World from WAIT!");

    rtc_stopBenchmarkMeasurement();
}
void javax_rtcbench_RTCBenchmark_void_test_native() {
    rtcbenchmark_measure_native_performance();
}