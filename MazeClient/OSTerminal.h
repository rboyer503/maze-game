#ifndef OSTERMINAL_H
#define OSTERMINAL_H

#include <boost/asio.hpp> // Included to ensure that ASIO gets to include WinSock first

#ifdef _WIN32_WINNT
#define OSTerminal WinTerminal
#include "WinTerminal.h"
#elif _LINUX
#define OSTerminal LinuxTerminal
#include "LinuxTerminal.h"
#else 
#error Unsupported OS
#endif

#endif // OSTERMINAL_H
