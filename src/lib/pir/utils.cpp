#include <iostream>
#include <fstream>
#include "time.h"
#include <chrono>
#include "utils.h"

double currentDateTime()
{

    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

    time_t tnow = std::chrono::system_clock::to_time_t(now);
    tm *date = localtime(&tnow); // todo: dperecated use localtime_s
    date->tm_hour = 0;
    date->tm_min = 0;
    date->tm_sec = 0;

    auto midnight = std::chrono::system_clock::from_time_t(mktime(date));

    return std::chrono::duration<double, std::milli>(now - midnight).count();
}