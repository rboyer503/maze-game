#include "GameData.h"


std::ostream & operator<<(std::ostream & os, const GameData & game_data)
{
	game_data.print(os);
	return os;
}
