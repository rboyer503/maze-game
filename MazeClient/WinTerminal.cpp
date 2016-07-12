#include <iostream>
#include "WinTerminal.h"


WinTerminal::WinTerminal() :
	ITerminal()
{
}

WinTerminal::~WinTerminal()
{
	release();
}

bool WinTerminal::initialize()
{
	stdout_handle_ = GetStdHandle(STD_OUTPUT_HANDLE);
	if (stdout_handle_ == INVALID_HANDLE_VALUE)
		return false;

	stdin_handle_ = GetStdHandle(STD_INPUT_HANDLE);
	if (stdin_handle_ == INVALID_HANDLE_VALUE)
		return false;

	if (!GetConsoleMode(stdin_handle_, &old_mode_))
		return false;

	return true;
}

bool WinTerminal::release()
{
	if (!SetConsoleMode(stdin_handle_, old_mode_))
		return false;

	if (!setCursorVisibility(TRUE))
		return false;

	return true;
}

bool WinTerminal::setMode(eTermMode term_mode)
{
	if (term_mode_ == term_mode)
		return true;

	term_mode_ = term_mode;

	switch (term_mode)
	{
	case TM_NORMAL:
		return release();
		break;
	case TM_GAME:
		if (!SetConsoleMode(stdin_handle_, old_mode_ & (~ENABLE_ECHO_INPUT)))
			return false;
		if (!setCursorVisibility(FALSE))
			return false;
		break;
	}

	return true;
}

void WinTerminal::clearScreen() const
{
	COORD pos = {0,0};
    CONSOLE_SCREEN_BUFFER_INFO s;  
    GetConsoleScreenBufferInfo(stdout_handle_, &s);
    DWORD written, cells = s.dwSize.X * s.dwSize.Y;
    FillConsoleOutputCharacter(stdout_handle_, ' ', cells, pos, &written);
    FillConsoleOutputAttribute(stdout_handle_, s.wAttributes, cells, pos, &written);
    SetConsoleCursorPosition(stdout_handle_, pos);
}

void WinTerminal::setCursorPos(int x, int y) const
{
	if (term_mode_ == TM_NORMAL)
		return;

	COORD pos = {x, y};
	SetConsoleCursorPosition(stdout_handle_, pos);
}

void WinTerminal::output(const std::ostringstream & oss) const
{
	std::cout << oss.str();
}

bool WinTerminal::pollKeys(kb_codes_vec & kb_codes)
{
	if (term_mode_ == TM_NORMAL)
		return false;

	DWORD num_events = 0;
	GetNumberOfConsoleInputEvents(stdin_handle_, &num_events);
	if (num_events == 0)
		return false;

    PINPUT_RECORD event_buffer = new INPUT_RECORD[num_events];
    DWORD num_read = 0;
    ReadConsoleInput(stdin_handle_, event_buffer, num_events, &num_read);

	kb_codes.clear();
    for (DWORD i = 0; i < num_read; ++i)
	{
        if ( (event_buffer[i].EventType == KEY_EVENT) && (event_buffer[i].Event.KeyEvent.bKeyDown) )
		{
            switch (event_buffer[i].Event.KeyEvent.wVirtualKeyCode)
			{
			case VK_ESCAPE: kb_codes.push_back(KB_ESCAPE); break;
			case VK_RETURN:	kb_codes.push_back(KB_ENTER); break;
			case VK_UP:		kb_codes.push_back(KB_UP); break;
			case VK_LEFT:	kb_codes.push_back(KB_LEFT); break;
			case VK_RIGHT:	kb_codes.push_back(KB_RIGHT); break;
			case VK_DOWN:	kb_codes.push_back(KB_DOWN); break;
			case 'A':		kb_codes.push_back(KB_A); break;
			case 'Z':		kb_codes.push_back(KB_Z); break;
            }
        }
    }

	delete [] event_buffer;

	return (kb_codes.size() > 0);
}

bool WinTerminal::setCursorVisibility(BOOL visible) const
{
	CONSOLE_CURSOR_INFO cursor_info;
	if (!GetConsoleCursorInfo(stdout_handle_, &cursor_info))
		return false;

	cursor_info.bVisible = visible;
	if (!SetConsoleCursorInfo(stdout_handle_, &cursor_info))
		return false;

	return true;
}
