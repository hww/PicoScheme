#ifndef PLATFORM_H_
#define PLATFORM_H_




#define PLATFORM_ESP32

#ifdef PLATFORM_ESP32

#include "esp_system.h"
#include "esp_timer.h"

#define PLATFORM_CHAR

#else
#define PLATFORM_WCHAR
#endif

#ifdef PLATFORM_CHAR
#define MYL(s) (s)
#define MYCHAR char
#define COUT std::cout
#define CERR std::cerr
#define CLOG std::clog
#define CIN std::cin
#define OSTREAM std::ostream
#define ISTRINGSTREAM std::istringstream
#define STD_STOD strtod
#else
#define MYL(s) L#s
#define MYCHAR wchar_t
#define COUT std::wcout
#define CERR std::wcerr
#define CLOG std::wclog
#define CIN std::wcin
#define OSTREAM std::wostream
#define ISTRINGSTREAM std::wistringstream
#define STD_STOD std::wcstod
#endif

char* esp32_read_line(char* prompt);

#ifdef PLATFORM_ESP32
#define READ_LINE read_line
#else
#endif

#endif // PLATFORM_H_
