#include <mpi.h>
#include "timing.h"

timing_info_t timing_info = {0.0, 0};
timing_info_t pread_timing_info = {0.0, 0};
timing_info_t md_timing_info = {0.0, 0};

void record_timing(double start, double end) {
    double duration = end - start;
    timing_info.total_time += duration;
    timing_info.count++;
}

void pread_record_timing(double start, double end) {
    double duration = end - start;
    pread_timing_info.total_time += duration;
    pread_timing_info.count++;
}

void md_record_timing(double start, double end) {
    double duration = end - start;
    md_timing_info.total_time += duration;
    md_timing_info.count++;
}

void catalog_load_timing(double start, double end) {
    double duration = end - start;
    catalog_load_timing_info.total_time += duration;
    catalog_load_timing_info.count++;
}
