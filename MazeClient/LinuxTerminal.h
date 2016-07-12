#ifndef LINUXTERMINAL_H
#define LINUXTERMINAL_H

#include <mutex>
#include "ITerminal.h"


class LinuxTerminal : public ITerminal
{
public:
	LinuxTerminal();
	~LinuxTerminal();

	virtual bool initialize();
	virtual bool release();
	virtual bool setMode(eTermMode term_mode);
	virtual void clearScreen() const;
	virtual void setCursorPos(int x, int y) const;
	virtual void output(const std::ostringstream & oss) const;
	virtual bool pollKeys(kb_codes_vec & kb_codes);
};

#endif // LINUXTERMINAL_H
