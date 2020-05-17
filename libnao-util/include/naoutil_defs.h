/*  This file is part of libnao-util.

    libnao-util is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libnao-util is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libnao-util.  If not, see <https://www.gnu.org/licenses/>.   */
#pragma once

#ifdef LIBNAOUTIL_EXPORTS
#define NAOUTIL_API __declspec(dllexport)
#else
#define NAOUTIL_API __declspec(dllimport)
#endif

/*
#define LINE_STRINGIFY(x) LINE_STRINGIFY2(x)
#define LINE_STRINGIFY2(x) #x
#define FILE_AT __FILE__ ":" LINE_STRINGIFY(__LINE__)

#ifdef NDEBUG
#include "logging.h"

#define ASSERT(cond) do { if (!(cond)) { \
    constexpr const char* str = "assertion failed in " FILE_AT " -> " #cond; \
    logging::cout(str); \
    throw std::runtime_error(str); }} while (0)
#else
#include <cassert>
#define ASSERT(cond) assert(cond);}
#endif
*/