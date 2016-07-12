#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "MazeClient.h"
#include "Utility.h"


ClientManager::ClientManager() :
	terminal_(new OSTerminal()), exiting_(false), state_(CS_INIT), 
	player_id_(0), client_(nullptr), redraw_(true), win_(false), game_over_display_(false)
{
	if (!terminal_->initialize())
		throw std::runtime_error("ClientManager::ClientManager: [Terminal initialization failed]");
}

void ClientManager::processMessage(GameMessage & game_msg)
{
	game_data_ptr game_data = game_msg.decodeBody();
	if (!game_data)
	{
		std::cerr << "ERROR: ClientManager::processMessage [Message decode failed; code=" << game_msg.getGameCode() << "]" << std::endl;
		return;
	}

	switch (state_)
	{
	case CS_INIT:
		{
			player_id_ptr player_id_data = std::dynamic_pointer_cast<PlayerID>(game_data);
			if (!player_id_data)
			{
				std::cerr << "ERROR: ClientManager::processMessage [Unexpected message received]" << std::endl << *game_data;
				return;
			}
			
			player_id_ = player_id_data->getPlayerId();
			state_ = CS_INPUT;
		}
		break;

	case CS_INPUT:
	case CS_WAIT:
		{
			game_summary_ptr summary_data = std::dynamic_pointer_cast<GameSummary>(game_data);
			if (!summary_data)
			{
				std::cerr << "ERROR: ClientManager::processMessage [Unexpected message received]" << std::endl << *game_data;
				return;
			}

			summary_data_ = *summary_data;

			state_ = CS_INPUT;
		}
		break;

	case CS_WAIT_START:
		{
			switch (game_msg.getGameCode())
			{
			case GameMessage::GC_GAMES_NOTIFY:
				{
					game_summary_ptr summary_data = std::dynamic_pointer_cast<GameSummary>(game_data);
					if (!summary_data)
					{
						std::cerr << "ERROR: ClientManager::processMessage [Unexpected message received]" << std::endl << *game_data;
						return;
					}

					summary_data_ = *summary_data;
				}
				break;
			case GameMessage::GC_SELECT_GAME_RESP:
				{
					select_resp_ptr select_resp = std::dynamic_pointer_cast<GameSelectResp>(game_data);
					if (!select_resp)
					{
						std::cerr << "ERROR: ClientManager::processMessage [Unexpected message received]" << std::endl << *game_data;
						return;
					}

					if (select_resp->getSelectResp() == GameSelectResp::SR_FAIL)
					{
						std::cout << std::endl << "Cannot enter maze; currently occupied..." << std::endl;
						boost::this_thread::sleep(boost::posix_time::millisec(500));
						state_ = CS_INPUT;
					}
					else
					{
						std::cout << std::endl << "Waiting for other player..." << std::endl;

						if (!terminal_->setMode(TM_GAME))
							throw std::runtime_error("ClientManager::processMessage: [Terminal setMode failed]");
					}
				}
				break;
			case GameMessage::GC_START_NOTIFY:
				{
					world_map_ = std::dynamic_pointer_cast<matrix3d_u8>(game_data);
					if (!world_map_)
					{
						std::cerr << "ERROR: ClientManager::processMessage [Unexpected message received]" << std::endl << *game_data;
						return;
					}

					redraw_ = true;
					win_ = false;
					if (!terminal_->setMode(TM_GAME))
						throw std::runtime_error("ClientManager::processMessage: [Terminal setMode failed]");
					state_ = CS_ACTIVE;
				}
				break;
			default:
				{	
					std::cerr << "ERROR: ClientManager::processMessage [Unexpected message received]" << std::endl << *game_data;
					return;
				}
				break;
			}
		}
		break;

	case CS_ACTIVE:
		{
			switch (game_msg.getGameCode())
			{
			case GameMessage::GC_GAMES_NOTIFY:
				{
					game_summary_ptr summary_data = std::dynamic_pointer_cast<GameSummary>(game_data);
					if (!summary_data)
					{
						std::cerr << "ERROR: ClientManager::processMessage [Unexpected message received]" << std::endl << *game_data;
						return;
					}

					summary_data_ = *summary_data;
				}
				break;
			case GameMessage::GC_UPDATE_NOTIFY:
				{
					const player_ptr & player_data = std::dynamic_pointer_cast<Player>(game_data);
					if (!player_data)
					{
						std::cerr << "ERROR: ClientManager::processMessage [Unexpected message received]" << std::endl << *game_data;
						return;
					}

					uint32_t id = player_data->getPlayerId();
					size_t ps_index = (id == player_id_ ? 0 : 1);
					player_states_[ps_index].updateState(*player_data);

					if (player_states_[0].checkAndClearChangedLevel())
						redraw_ = true;
				}
				break;
			case GameMessage::GC_WINNER_NOTIFY:
				{
					const winner_ptr & winner_data = std::dynamic_pointer_cast<Winner>(game_data);
					if (!winner_data)
					{
						std::cerr << "ERROR: ClientManager::processMessage [Unexpected message received]" << std::endl << *game_data;
						return;
					}

					if (winner_data->getWinnerId() == player_id_)
						win_ = true;
					game_over_display_ = true;

					state_ = CS_GAME_OVER;
				}
				break;
			default:
				{	
					std::cerr << "ERROR: ClientManager::processMessage [Unexpected message received]" << std::endl << *game_data;
					return;
				}
				break;
			}
		}
		break;

	case CS_GAME_OVER:
		{
			switch (game_msg.getGameCode())
			{
			case GameMessage::GC_GAMES_NOTIFY:
				{
					game_summary_ptr summary_data = std::dynamic_pointer_cast<GameSummary>(game_data);
					if (!summary_data)
					{
						std::cerr << "ERROR: ClientManager::processMessage [Unexpected message received]" << std::endl << *game_data;
						return;
					}

					summary_data_ = *summary_data;
				}
				break;
			default:
				{	
					std::cerr << "ERROR: ClientManager::processMessage [Unexpected message received]" << std::endl << *game_data;
					return;
				}
				break;
			}
		}
		break;

	default:
		{	
			std::cerr << "ERROR: ClientManager::processMessage [Unexpected message received]" << std::endl << *game_data;
			return;
		}
		break;
	}
}

void ClientManager::run(MazeClient * client)
{
	client_ = client;

	while (!exiting_)
	{
		switch (state_)
		{
		case CS_INPUT:
			processInput();
			break;
		case CS_WAIT_START:
			processWaitInput();
			break;
		case CS_ACTIVE:
		case CS_GAME_OVER:
			processGame();
			break;
		}
		boost::this_thread::sleep(boost::posix_time::millisec(10));
	}
}

void ClientManager::processInput()
{
	boost::this_thread::sleep(boost::posix_time::millisec(250));
	terminal_->clearScreen();
	std::cout << "Welcome, Player " << player_id_ << "!" << std::endl;

	bool done = false;
	while (!done)
	{
		std::cout << std::endl << "Maze Game Main Menu" << std::endl <<
			"\t1) Create maze" << std::endl <<
			"\t2) Enter maze (1 player game)" << std::endl <<
			"\t3) Enter maze (2 player game)" << std::endl <<
			"\t4) Quit" << std::endl;
	
		std::string input;
		int selection;
		do
		{
			std::cout << "Enter selection (1-4): ";
			std::getline(std::cin, input);
		} while (!validateIntRange(input, selection, 1, 4));

		switch (selection)
		{
		case 1:
			if (createMaze())
			{
				state_ = CS_WAIT;
			}
			done = true;
			break;
		case 2:
		case 3:
			if (enterMaze(selection - 1))
			{
				state_ = CS_WAIT_START;
			}
			done = true;
			break;
		case 4:
			exiting_ = done = true;
			break;
		}
	}
}

void ClientManager::processWaitInput()
{
	kb_codes_vec kb_codes;
	if (terminal_->pollKeys(kb_codes))
	{
		switch (kb_codes[0])
		{
		case KB_ESCAPE:
			{
				GameMessage msg(GameMessage::GC_CANCEL_REQ);
				client_->write(msg);

				if (!terminal_->setMode(TM_NORMAL))
					throw std::runtime_error("ClientManager::processWaitInput: [Terminal setMode failed]");
				state_ = CS_INPUT;
			}
			break;
		}
	}
}

bool ClientManager::createMaze()
{
	std::cout << std::endl << "Create New Maze" << std::endl;
	
	// Retrieve configuration from user.
	MazeConfig config;
	if (!getMazeConfiguration(config))
		return false;

	// Transmit message to server.
	GameConfig game_data(config.width, config.height, config.levels);
	GameMessage msg(GameMessage::GC_CREATE_REQ, &game_data);
	client_->write(msg);

	return true;
}

bool ClientManager::getMazeConfiguration(MazeConfig & config)
{
	// Retrieve maze parameters from user.
	std::string input;
	do
	{
		std::cout << "\tEnter width (" << MazeConfig::MIN_WIDTH << "-" << MazeConfig::MAX_WIDTH << "): ";
		std::getline(std::cin, input);
	} while (!validateIntRange<uint32_t>(input, config.width, MazeConfig::MIN_WIDTH, MazeConfig::MAX_WIDTH));

	do
	{
		std::cout << "\tEnter height (" << MazeConfig::MIN_HEIGHT << "-" << MazeConfig::MAX_HEIGHT << "): ";
		std::getline(std::cin, input);
	} while (!validateIntRange<uint32_t>(input, config.height, MazeConfig::MIN_HEIGHT, MazeConfig::MAX_HEIGHT));

	do
	{
		std::cout << "\tEnter number of levels (" << MazeConfig::MIN_LEVELS << "-" << MazeConfig::MAX_LEVELS << "): ";
		std::getline(std::cin, input);
	} while (!validateIntRange<uint32_t>(input, config.levels, MazeConfig::MIN_LEVELS, MazeConfig::MAX_LEVELS));

	// Confirm with user.
	std::cout << std::endl;
	config.print();
	std::cout << std::endl;

	char inputChar = '\0';
	do
	{
		std::cout << "Is this configuration OK?" << std::endl;
		std::cout << "  [y]es" << std::endl;
		std::cout << "  [n]o" << std::endl;
		std::cout << "Enter key for selection: ";
		std::getline(std::cin, input);
		if (input.length() == 1)
			inputChar = tolower(input[0]);
	} while ( (inputChar != 'n') && (inputChar != 'y') );
	std::cout << std::endl;

	return (inputChar == 'y');
}

bool ClientManager::enterMaze(uint32_t num_players)
{
	std::cout << std::endl << "Select Maze" << std::endl;
	
	const maze_config_vec & configs = summary_data_.getConfigs();
	uint32_t i;
	for (i = 0; i < configs.size(); ++i)
	{
		std::cout << "\t" << (i + 1) << ") ";
		configs[i].print();
	}
	std::cout << "\t" << (i + 1) << ") Return to Main Menu" << std::endl;

	std::string input;
	uint32_t selection;
	do
	{
		std::cout << "Enter selection (1-" << (i + 1) << "): ";
		std::getline(std::cin, input);
	} while (!validateIntRange<uint32_t>(input, selection, 1, (i + 1)));

	if (selection == (i + 1))
		return false;

	// Transmit message to server.
	GameSelect game_data(selection, num_players);
	GameMessage msg(GameMessage::GC_SELECT_GAME_REQ, &game_data);
	client_->write(msg);

	return true;
}

void ClientManager::processGame()
{
	bool force_redraw_players = false;

	if (redraw_)
	{
		redraw_ = false;
		force_redraw_players = true;

		terminal_->clearScreen();
		
		std::ostringstream oss;
		oss << world_map_->ptr(0, 0, player_states_[0].getCurrState().getPosition().z) << std::endl;
		terminal_->output(oss);

		terminal_->setCursorPos(0, world_map_->getHeight());
		oss.str("");
		oss << "Player " << player_id_ << ", GO!" << std::endl;
		terminal_->output(oss);
	}

	for (size_t i = 0; i < 2; ++i)
	{
		PlayerState & ps = player_states_[i];
		if (ps.checkAndClearUpdated() || force_redraw_players)
		{
			if (!force_redraw_players)
			{
				const Player & prev_state = ps.getPrevState();
				if (!prev_state.isClear())
				{
					if ( (i == 0) || 
						 (prev_state.getPosition().z == player_states_[0].getCurrState().getPosition().z) )
					{
						const Vertex3DEx & prev_pos = prev_state.getPosition();
						uint8_t local_char = world_map_->at(prev_pos);
						terminal_->setCursorPos(prev_pos.x, prev_pos.y);

						std::ostringstream oss;
						oss << local_char;
						terminal_->output(oss);
					}
				}
			}

			if ( (i == 0) || 
				 (player_states_[i].getCurrState().getPosition().z == player_states_[0].getCurrState().getPosition().z) )
			{
				const Player & curr_state = ps.getCurrState();
				if (!curr_state.isClear())
				{
					const Vertex3DEx & curr_pos = curr_state.getPosition();
					terminal_->setCursorPos(curr_pos.x, curr_pos.y);

					std::ostringstream oss;
					oss << static_cast<char>(curr_state.getPlayerId() + 48);
					terminal_->output(oss);
				}
			}
		}
	}
	terminal_->setCursorPos(0, world_map_->getHeight());

	if (game_over_display_)
	{
		game_over_display_ = false;

		std::ostringstream oss;
		//terminal_->setCursorPos(0, world_map_->getHeight());
		if (win_)
			oss << "You win!  Press enter to return to menu..." << std::endl;
		else
			oss << "You lost.  Press enter to return to menu..." << std::endl;
		terminal_->output(oss);
	}

	std::unique_ptr<MoveReq> req;
	kb_codes_vec kb_codes;
	bool quit = false;
	if (terminal_->pollKeys(kb_codes))
	{
		if (state_ == CS_ACTIVE)
		{
			const Vertex3DEx & curr_pos = player_states_[0].getCurrState().getPosition();
			uint8_t local_char = world_map_->at(curr_pos);
			switch (kb_codes[0])
			{
			case KB_ESCAPE:
				quit = true;
				break;
			case KB_A:
				if ( (local_char == '/') || (local_char == 'X') )
				{
					req.reset(new MoveReq(MoveReq::MD_TOP));
				}
				break;
			case KB_Z:
				if ( (local_char == '\\') || (local_char == 'X') )
				{
					req.reset(new MoveReq(MoveReq::MD_BOTTOM));
				}
				break;
			case KB_UP:
				{
					uint8_t target_char = world_map_->at(curr_pos.x, curr_pos.y - 1, curr_pos.z);
					if ( (target_char < 127) || (target_char == 234) )
					{
						req.reset(new MoveReq(MoveReq::MD_UP));
					}
				}
				break;
			case KB_DOWN:
				{
					uint8_t target_char = world_map_->at(curr_pos.x, curr_pos.y + 1, curr_pos.z);
					if ( (target_char < 127) || (target_char == 234) )
					{
						req.reset(new MoveReq(MoveReq::MD_DOWN));
					}
				}
				break;
			case KB_LEFT:
				{
					uint8_t target_char = world_map_->at(curr_pos.x - 2, curr_pos.y, curr_pos.z);
					if ( (target_char < 127) || (target_char == 234) )
					{
						req.reset(new MoveReq(MoveReq::MD_LEFT));
					}
				}
				break;
			case KB_RIGHT:
				{
					uint8_t target_char = world_map_->at(curr_pos.x + 2, curr_pos.y, curr_pos.z);
					if ( (target_char < 127) || (target_char == 234) )
					{
						req.reset(new MoveReq(MoveReq::MD_RIGHT));
					}
				}
				break;
			}
		}
		else if (state_ == CS_GAME_OVER)
		{
			if (kb_codes[0] == KB_ENTER)
			{
				if (!terminal_->setMode(TM_NORMAL))
					throw std::runtime_error("ClientManager::processGame: [Terminal setMode failed]");
				state_ = CS_INPUT;
			}
		}
	}

	if (req)
	{
		GameMessage msg(GameMessage::GC_MOVE_REQ, req.get());
		client_->write(msg);
	}
	else if (quit)
	{
		GameMessage msg(GameMessage::GC_CANCEL_REQ);
		client_->write(msg);

		if (!terminal_->setMode(TM_NORMAL))
			throw std::runtime_error("ClientManager::processGame: [Terminal setMode failed]");
		state_ = CS_INPUT;
	}
}
