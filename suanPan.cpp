////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017-2021 Theodore Chang
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////

#include <suanPan>

#ifdef SUANPAN_WIN
#include <Windows.h>

BOOL WIN_EVENT(DWORD) { return TRUE; }
#endif

bool SUANPAN_PRINT = true;
const char* SUANPAN_EXE = "";
int SUANPAN_NUM_THREADS = 4;

// ReSharper disable once CppParameterMayBeConst
int main(int argc, char** argv) {
#ifdef SUANPAN_WIN
	if(!SetConsoleCtrlHandler(WIN_EVENT, TRUE)) return 0;
	const auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(handle, &info);
	const auto current_attribute = info.wAttributes;
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_GREEN);
#else
    std::cout << "\033[1;32m";
#endif

	if(check_debugger()) return 0;

#ifdef SUANPAN_MAGMA
	magma_init();
#endif

	argument_parser(argc, argv);

#ifdef SUANPAN_MAGMA
	magma_finalize();
#endif

#ifdef SUANPAN_WIN
	SetConsoleTextAttribute(handle, current_attribute);
#else
    std::cout << "\033[0m";
#endif

	return 0;
}
