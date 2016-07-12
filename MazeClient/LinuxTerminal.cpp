#include <iostream>
#include <ncurses.h>
#include "LinuxTerminal.h"


LinuxTerminal::LinuxTerminal() :
	ITerminal()
{
}

LinuxTerminal::~LinuxTerminal()
{
	release();
}

bool LinuxTerminal::initialize()
{
	return true;
}

bool LinuxTerminal::release()
{
	if (term_mode_ == TM_GAME)
		endwin();

	return true;
}

bool LinuxTerminal::setMode(eTermMode term_mode)
{
	if (term_mode_ == term_mode)
		return true;

	term_mode_ = term_mode;

	switch (term_mode)
	{
	case TM_NORMAL:
		endwin();
		break;
	case TM_GAME:
		initscr();
		cbreak();
		noecho();
		keypad(stdscr, TRUE);
		nodelay(stdscr, TRUE);
		curs_set(0);
		break;
	}

	return true;
}

void LinuxTerminal::clearScreen() const
{
	switch (term_mode_)
	{
	case TM_NORMAL:
		std::system("clear");
		break;
	case TM_GAME:
		clear();
		break;
	}
}

void LinuxTerminal::setCursorPos(int x, int y) const
{
	if (term_mode_ == TM_NORMAL)
		return;

	move(y, x);
}

void LinuxTerminal::output(const std::ostringstream & oss) const
{
	switch (term_mode_)
	{
	case TM_NORMAL:
		std::cout << oss.str();
		break;
	case TM_GAME:
		printw(oss.str().c_str());
		refresh();
		break;
	}
}

bool LinuxTerminal::pollKeys(kb_codes_vec & kb_codes)
{
	if (term_mode_ == TM_NORMAL)
		return false;

	kb_codes.clear();

	int c;
	while ( (c = getch()) != ERR )
	{
		switch (c)
		{
		case 27:		kb_codes.push_back(KB_ESCAPE); break;
		case '\n':		kb_codes.push_back(KB_ENTER); break;
		case KEY_UP:	kb_codes.push_back(KB_UP); break;
		case KEY_LEFT:	kb_codes.push_back(KB_LEFT); break;
		case KEY_RIGHT:	kb_codes.push_back(KB_RIGHT); break;
		case KEY_DOWN:	kb_codes.push_back(KB_DOWN); break;
		case 'A':
		case 'a':		kb_codes.push_back(KB_A); break;
		case 'Z':
		case 'z':		kb_codes.push_back(KB_Z); break;
		}
	}

	return (kb_codes.size() > 0);
}
