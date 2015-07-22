#include <csignal>
#include <thread>
#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <atomic>

#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#endif

#include "IBonDriver2.h"
#include "IB25Decoder.h"