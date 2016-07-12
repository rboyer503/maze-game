#ifndef WINTERMINAL_H
#define WINTERMINAL_H

#include "ITerminal.h"
#include <Windows.h>


class WinTerminal : public ITerminal
{
	HANDLE stdout_handle_;
	HANDLE stdin_handle_;
	DWORD old_mode_;

public:
	WinTerminal();
	~WinTerminal();

	virtual bool initialize();
	virtual bool release();
	virtual bool setMode(eTermMode term_mode);
	virtual void clearScreen() const;
	virtual void setCursorPos(int x, int y) const;
	virtual void output(const std::ostringstream & oss) const;
	virtual bool pollKeys(kb_codes_vec & kb_codes);

private:
	bool setCursorVisibility(BOOL visible) const;
};

#endif // WINTERMINAL_H
