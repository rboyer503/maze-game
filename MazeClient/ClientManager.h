#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include "OSTerminal.h"
#include "../MazeShared/GameMessage.h"


class MazeClient;


class PlayerState
{
	Player states_[2];
	size_t curr_index;
	bool updated_;
	bool changed_level_;

public:
	PlayerState() :
		curr_index(0), updated_(false), changed_level_(false)
	{}

	Player & getCurrState() { return states_[curr_index]; }
	const Player & getCurrState() const { return states_[curr_index]; }
	
	Player & getPrevState() { return states_[(curr_index + 1) % 2]; }
	const Player & getPrevState() const { return states_[(curr_index + 1) % 2]; }

	bool checkAndClearChangedLevel()
	{ 
		if (changed_level_)
		{
			changed_level_ = false;
			return true;
		}
		return false;
	}

	bool checkAndClearUpdated() 
	{
		if (updated_)
		{
			updated_ = false;
			return true;
		}
		return false; 
	}

	void updateState(const Player & new_state)
	{
		curr_index = (curr_index + 1) % 2;
		states_[curr_index] = new_state;
		updated_ = true;
		changed_level_ = (states_[0].getPosition().z != states_[1].getPosition().z);
	}
};


class ClientManager
{
public:
	enum eClientState
	{
		CS_INIT = 100,
		/* Insert new codes after CS_INIT */
		CS_INPUT,
		CS_WAIT,
		CS_WAIT_START,
		CS_ACTIVE,
		CS_GAME_OVER,
		/* Insert new codes before CS_MAX */
		CS_MAX
	};

	ClientManager();
	void processMessage(GameMessage & game_msg);
	void run(MazeClient * client);

private:
	void processInput();
	void processWaitInput();
	bool createMaze();
	bool getMazeConfiguration(MazeConfig & config);
	bool enterMaze(uint32_t num_players);
	void processGame();

	std::shared_ptr<ITerminal> terminal_;
	bool exiting_;
	eClientState state_;
	uint32_t player_id_;
	MazeClient * client_;
	GameSummary summary_data_;
	matrix3d_u8_ptr world_map_;
	bool redraw_;
	PlayerState player_states_[2];
	bool win_;
	bool game_over_display_;
};

#endif // CLIENT_MANAGER_H
