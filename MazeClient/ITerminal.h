#ifndef ITERMINAL_H
#define ITERMINAL_H

#include <sstream>
#include <vector>


enum eTermMode
{
	TM_NORMAL,
	TM_GAME
};

enum eKBCode
{
	KB_NONE,
	KB_ESCAPE,
	KB_ENTER,
	KB_UP,
	KB_LEFT,
	KB_RIGHT,
	KB_DOWN,
	KB_A,
	KB_Z,
	KB_MAX
};

typedef std::vector<eKBCode> kb_codes_vec;


class ITerminal
{
protected:
	eTermMode term_mode_;

public:
	ITerminal() :
		term_mode_(TM_NORMAL)
	{}
	virtual ~ITerminal() {}

	eTermMode getMode() const {return term_mode_; }

	virtual bool initialize() = 0;
	virtual bool release() = 0;
	virtual bool setMode(eTermMode term_mode) = 0;
	virtual void clearScreen() const = 0;
	virtual void setCursorPos(int x, int y) const = 0;
	virtual void output(const std::ostringstream & oss) const = 0;
	virtual bool pollKeys(kb_codes_vec & kb_codes) = 0;
};

#endif // ITERMINAL_H
