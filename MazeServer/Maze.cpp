#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <limits>
#include "Maze.h"
#include "MazeSession.h"
#include "AIAgent.h"


Maze::Maze() :
  maze_matrix_(nullptr), world_matrix_(nullptr), game_in_progress_(false)
{
}

Maze::~Maze()
{
	delete maze_matrix_;
	delete world_matrix_;
}

void Maze::displayWorldMatrix(int level /* = -1 */) const
{
	if (level == -1)
	{
		for (uint32_t z = 0; z < world_matrix_->getDepth(); ++z)
			std::cout << world_matrix_->ptr(0, 0, z) << std::endl;
	}
	else
	{
		std::cout << world_matrix_->ptr(0, 0, level) << std::endl;
	}
}

void Maze::buildMaze(const MazeConfig & config)
{
	config_ = config;

	// Build 3D matrix with all "rooms" having all 6 walls and flagged as not explored.
	uint8_t init_value = MAZE_LEFT | MAZE_RIGHT | MAZE_UP | MAZE_DOWN | MAZE_BOTTOM | MAZE_TOP;
	maze_matrix_ = new matrix3d_u8(config_.width, config_.height, config_.levels, init_value);

	// Load initial room and initiate modified Prim's algorithm.
	rooms_.push_back(Vertex3DEx(config_.width / 2, config_.height / 2, config_.levels / 2));

	while (rooms_.size() > 0)
	{
		// Randomly select next room and mark it explored.
		int index = (rand() % rooms_.size());
		Vertex3DEx curr_room = rooms_[index];
		maze_matrix_->at(curr_room) |= MAZE_EXPLORED;

		// Load adjacent, unexplored rooms for future processing.
		// Also locally track adjacent, explored rooms.
		vertex3d_vec explored_verts;
		if (curr_room.x > 0)
		{
			processProspectiveRoom(Vertex3DEx(curr_room.x - 1, curr_room.y, curr_room.z, MAZE_LEFT), explored_verts);	
		}
		if (curr_room.x < (config_.width - 1))
		{
			processProspectiveRoom(Vertex3DEx(curr_room.x + 1, curr_room.y, curr_room.z, MAZE_RIGHT), explored_verts);
		}
		if (curr_room.y > 0)
		{
			processProspectiveRoom(Vertex3DEx(curr_room.x, curr_room.y - 1, curr_room.z, MAZE_UP), explored_verts);
		}
		if (curr_room.y < (config_.height - 1))
		{
			processProspectiveRoom(Vertex3DEx(curr_room.x, curr_room.y + 1, curr_room.z, MAZE_DOWN), explored_verts);
		}
		if (curr_room.z > 0)
		{
			processProspectiveRoom(Vertex3DEx(curr_room.x, curr_room.y, curr_room.z - 1, MAZE_BOTTOM), explored_verts);
		}
		if (curr_room.z < (config_.levels - 1))
		{
			processProspectiveRoom(Vertex3DEx(curr_room.x, curr_room.y, curr_room.z + 1, MAZE_TOP), explored_verts);
		}
		
		if (explored_verts.size() > 0)
		{
			// Randomly select adjacent, explored room to connect.
			int adj_index = (rand() % explored_verts.size());
			Vertex3DEx & curr_adj = explored_verts[adj_index];

			// Extract direction from extra field and remove the appropriate walls from both rooms.
			uint8_t extra = static_cast<uint8_t>(curr_adj.extra);
			uint8_t opposite_wall = getOppositeWall(extra);
			maze_matrix_->at(curr_room) &= ~extra;
			maze_matrix_->at(curr_adj) &= ~opposite_wall;
		}

		// Remove the room we just processed.
		rooms_.erase(rooms_.begin() + index);
	}

	world_matrix_ = new matrix3d_u8( (config_.width * 4) + 2, (config_.height * 2) + 1, config_.levels, ' ' );

	// Build edges.
	uint8_t * ptr = nullptr;
	for (uint32_t z = 0; z < config_.levels; ++z)
	{
		for (uint32_t y = 0; y < config_.height; ++y)
		{
			for (uint32_t x = 0; x < config_.width; ++x)
			{
				uint8_t curr_room = maze_matrix_->at(x, y, z);

				// Top edge?
				if ( (y == 0) || (curr_room & MAZE_UP) )
				{
					ptr = world_matrix_->ptr((x * 4) + 1, (y * 2), z);
					for (int i = 0; i < 3; ++i)
						*ptr++ = 205;
				}

				// Left edge.
				if ( (x == 0) || (curr_room & MAZE_LEFT) )
					world_matrix_->at((x * 4), (y * 2) + 1, z) = 186;

				if (x == (config_.width - 1))
				{
					// Right edge.
					world_matrix_->at(((x + 1) * 4), (y * 2) + 1, z) = 186;
				}

				if (y == (config_.height - 1))
				{
					// Bottom edge.
					ptr = world_matrix_->ptr((x * 4) + 1, ((y + 1) * 2), z);
					for (int i = 0; i < 3; ++i)
						*ptr++ = 205;
				}

				// Staircases.
				bool down = false;
				bool up = false;
				if ( (curr_room & MAZE_BOTTOM) == 0 )
					down = true;
				if ( (curr_room & MAZE_TOP) == 0 )
					up = true;

				if (up && down)
					world_matrix_->at((x * 4) + 2, (y * 2) + 1, z) = 'X';
				else if (up)
					world_matrix_->at((x * 4) + 2, (y * 2) + 1, z) = '/';
				else if (down)
					world_matrix_->at((x * 4) + 2, (y * 2) + 1, z) = '\\';
			}
		}
	}

	// Build corners.
	bool up, down, left, right;
	for (uint32_t z = 0; z < world_matrix_->getDepth(); ++z)
	{
		for (uint32_t y = 0; y < world_matrix_->getHeight(); y += 2)
		{
			for (uint32_t x = 0; x < world_matrix_->getWidth(); x += 4)
			{
				// Collect surrounding edges.
				if ( (x == 0) || (world_matrix_->at(x - 1, y, z) == ' ') )
					left = false;
				else
					left = true;

				if ( (x == (world_matrix_->getWidth() - 1)) || (world_matrix_->at(x + 1, y, z) == ' ') )
					right = false;
				else
					right = true;

				if ( (y == 0) || (world_matrix_->at(x, y - 1, z) == ' ') )
					up = false;
				else
					up = true;

				if ( (y == (world_matrix_->getHeight() - 1)) || (world_matrix_->at(x, y + 1, z) == ' ') )
					down = false;
				else
					down = true;

				// Select appropriate corner.
				uint8_t corner_value = 0;
				if (left && right && up && down)
					corner_value = 206;
				else if (left && right && up)
					corner_value = 202;
				else if (left && right && down)
					corner_value = 203;
				else if (left && up && down)
					corner_value = 185;
				else if (right && up && down)
					corner_value = 204;
				else if (left && up)
					corner_value = 188;
				else if (left && down)
					corner_value = 187;
				else if (right && up)
					corner_value = 200;
				else if (right && down)
					corner_value = 201;
				else if (left || right)
					corner_value = 205;
				else
					corner_value = 186;

				world_matrix_->at(x, y, z) = corner_value;
			}

			if (y < (world_matrix_->getHeight() - 1))
			{
				world_matrix_->at((config_.width * 4) + 1, y, z) = '\n';
				world_matrix_->at((config_.width * 4) + 1, y + 1, z) = '\n';
			}
			else
				world_matrix_->at((config_.width * 4) + 1, y, z) = '\0';
		}
	}

	// Set goal.
	// Spiral out from center until we find an empty room to avoid wiping out stairways.
	Vertex3DEx goal(((config_.width / 2) * 4) + 2,
					((config_.height / 2) * 2) + 1,
					config_.levels - 1);
	uint8_t dir = MAZE_DOWN;
	uint8_t steps = 0;
	uint8_t curr_step = 0;
	while (world_matrix_->at(goal) != ' ')
	{
		if (!curr_step)
		{
			switch (dir)
			{
			case (MAZE_DOWN):
				dir = MAZE_LEFT;
				curr_step = ++steps;
				break;
			case (MAZE_LEFT):
				dir = MAZE_UP;
				curr_step = steps;
				break;
			case (MAZE_UP):
				dir = MAZE_RIGHT;
				curr_step = ++steps;
				break;
			case (MAZE_RIGHT):
				dir = MAZE_DOWN;
				curr_step = steps;
				break;
			}
		}

		--curr_step;

		switch (dir)
		{
		case (MAZE_LEFT):
			goal.x -= 4;
			break;
		case (MAZE_UP):
			goal.y -= 2;
			break;
		case (MAZE_RIGHT):
			goal.x += 4;
			break;
		case (MAZE_DOWN):
			goal.y += 2;
			break;
		}
	}
	world_matrix_->at(goal) = 234;

	goal_.x = (goal.x - 2) / 4;
	goal_.y = (goal.y - 1) / 2;
	goal_.z = goal.z;
}

bool Maze::joinMaze(const maze_session_ptr & session, uint32_t num_players)
{
	if (game_in_progress_)
		return false;

	if (sessions_.size() >= num_players)
		return false;

	sessions_.push_back(session);

	Vertex3DEx pos(2, 1, 0);
	if (sessions_.size() == MAX_PLAYERS)
	{
		pos.x = world_matrix_->getWidth() - 4;
		pos.y = world_matrix_->getHeight() - 2;
	}

	uint32_t id = session->getPlayerId();
	{
		boost::mutex::scoped_lock lock(players_mutex_);
		players_[id] = std::make_shared<Player>(id, pos);
	}

	if (sessions_.size() == num_players)
	{
		game_in_progress_ = true;

		if (num_players == 1)
		{
			ai_thread_ = boost::thread(boost::bind(&Maze::processAI, this));
		}

		GameMessage msg(GameMessage::GC_START_NOTIFY, world_matrix_);
		broadcast(msg);

		{
			boost::mutex::scoped_lock lock(players_mutex_);
			for (player_map::iterator it = players_.begin(); it != players_.end(); ++it)
			{
				GameMessage msg(GameMessage::GC_UPDATE_NOTIFY, ((*it).second).get());
				broadcast(msg);
			}
		}
	}
	else
	{
		GameSelectResp select_resp(GameSelectResp::SR_WAIT);
		GameMessage msg(GameMessage::GC_SELECT_GAME_RESP, &select_resp);
		session->write(msg);
	}

	return true;
}

void Maze::leaveMaze(const maze_session_ptr & session)
{
	{
		boost::mutex::scoped_lock lock(players_mutex_);
		players_.erase(session->getPlayerId());
	}

	maze_session_vec::iterator pos = std::find(sessions_.begin(), sessions_.end(), session);
	if (pos != sessions_.end())
		sessions_.erase(pos);

	if (sessions_.size() == 0) 
	{
		joinAIThread();

		{
			boost::mutex::scoped_lock lock(players_mutex_);
			players_.clear();
		}

		game_in_progress_ = false;
	}
}

bool Maze::movePlayer(uint32_t player_id, const move_req_ptr & req, bool * won /* = nullptr */)
{
	Vertex3DEx pos;
	{
		boost::mutex::scoped_lock lock(players_mutex_);
		if (!players_[player_id])
			return false;
		pos = players_[player_id]->getPosition();
	}
	uint8_t local_char = world_matrix_->at(pos);

	switch (req->getMoveDir())
	{
	case MoveReq::MD_LEFT:
		pos.x -= 2;
		break;
	case MoveReq::MD_RIGHT:
		pos.x += 2;
		break;
	case MoveReq::MD_UP:
		pos.y--;
		break;
	case MoveReq::MD_DOWN:
		pos.y++;
		break;
	case MoveReq::MD_BOTTOM:
		pos.z--;
		if ( (local_char != '\\') && (local_char != 'X') )
			return false;
		break;
	case MoveReq::MD_TOP:
		pos.z++;
		if ( (local_char != '/') && (local_char != 'X') )
			return false;
		break;
	}

	bool win_ = false;
	if (world_matrix_->at(pos) == 234)
		win_ = true;
	else if (world_matrix_->at(pos) >= 127)
		return false;

	{
		boost::mutex::scoped_lock lock(players_mutex_);

		for (player_map::iterator it = players_.begin(); it != players_.end(); ++it)
			if ( ((*it).first != player_id) && (((*it).second)->getPosition() == pos) )
				return false;

		players_[player_id]->getPosition() = pos;

		GameMessage msg(GameMessage::GC_UPDATE_NOTIFY, players_[player_id].get());
		broadcast(msg);
	}

	if (win_)
	{
		Winner winner(player_id);
		GameMessage msg(GameMessage::GC_WINNER_NOTIFY, &winner);
		broadcast(msg);

		if (won)
			*won = true;
	}

	return true;
}

void Maze::processAI()
{
	std::cout << "DEBUG: Starting Maze::processAI" << std::endl;

	AIAgent agent(0, *this);

	{
		boost::mutex::scoped_lock lock(players_mutex_);
		players_[agent.getPlayerId()] = agent.getPlayer();
	}

	GameMessage msg(GameMessage::GC_UPDATE_NOTIFY, agent.getPlayer().get());
	broadcast(msg);

	while (true)
    {
        try
        {
			if (agent.handleTick())
				break;

            boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        }
        catch(boost::thread_interrupted &)
        {
			std::cout << "DEBUG: Ending Maze::processAI" << std::endl;
            return;
        }
    }

	clearSessions();
}

void Maze::joinAIThread()
{
	if (ai_thread_.joinable())
	{
		ai_thread_.interrupt();
		ai_thread_.join();
	}
}

uint8_t Maze::getOppositeWall(const uint8_t dir)
{
	switch (dir)
	{
	case MAZE_NONE: return MAZE_NONE;
	case MAZE_LEFT:	return MAZE_RIGHT;
	case MAZE_RIGHT: return MAZE_LEFT;
	case MAZE_UP: return MAZE_DOWN;
	case MAZE_DOWN: return MAZE_UP;
	case MAZE_BOTTOM: return MAZE_TOP;
	case MAZE_TOP: return MAZE_BOTTOM;
	default:
		throw std::runtime_error("Maze::getOppositeWall: [Unexpected input]");
	}
}

void Maze::processProspectiveRoom(const Vertex3DEx & prospect_vert, vertex3d_vec & explored_verts)
{
	uint8_t & room = maze_matrix_->at(prospect_vert);
	if (room & MAZE_EXPLORED)
	{
		explored_verts.push_back(prospect_vert);
	}
	else if ( (room & MAZE_LOADED) == 0 )
	{
		room |= MAZE_LOADED;
		rooms_.push_back(prospect_vert);
	}	
}

void Maze::broadcast(const GameMessage & msg) const
{
	for (size_t i = 0; i < sessions_.size(); ++i)
	{
		sessions_[i]->write(msg);
	}
}

void Maze::clearSessions()
{
	{
		boost::mutex::scoped_lock lock(players_mutex_);
		players_.clear();
	}
	sessions_.clear();

	game_in_progress_ = false;
}
