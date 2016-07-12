#ifndef MAZE_H
#define MAZE_H

#include <boost/thread/thread.hpp>
#include "../MazeShared/GameMessage.h"


// Forward declaration to avoid circular dependency
class MazeSession;


class Maze
{
	MazeConfig config_;
	matrix3d_u8 * maze_matrix_;
	matrix3d_u8 * world_matrix_;
	vertex3d_vec rooms_;
	Vertex3DEx goal_;
	std::vector<std::shared_ptr<MazeSession> > sessions_;
	player_map players_;
	boost::thread ai_thread_;
	bool game_in_progress_;
	boost::mutex players_mutex_;

public:
	static const uint8_t MAZE_NONE =		0;
	static const uint8_t MAZE_LEFT =		1 << 0;
	static const uint8_t MAZE_RIGHT =		1 << 1;
	static const uint8_t MAZE_UP =			1 << 2;
	static const uint8_t MAZE_DOWN =		1 << 3;
	static const uint8_t MAZE_BOTTOM =		1 << 4;
	static const uint8_t MAZE_TOP =			1 << 5;
	static const uint8_t MAZE_LOADED =		1 << 6;
	static const uint8_t MAZE_DEADEND =		1 << 6;
	static const uint8_t MAZE_EXPLORED =	1 << 7;

	static const uint8_t MAX_PLAYERS = 2;

	Maze();
	~Maze();

	static size_t countBranches(uint8_t room)
	{
		size_t count = 0;
		for (size_t i = 1; i <= (1 << 5); i <<= 1)
			if (!(room & i))
				++count;
		return count;
	}

	static bool hasStairwell(uint8_t room)
	{
		return ( !((room & (Maze::MAZE_BOTTOM | Maze::MAZE_TOP)) == (Maze::MAZE_BOTTOM | Maze::MAZE_TOP)) );
	}

	static MoveReq::eMoveDir mazeDirToMoveDir(uint8_t dir)
	{
		switch (dir)
		{
		case Maze::MAZE_LEFT: return MoveReq::MD_LEFT; break;
		case Maze::MAZE_RIGHT: return MoveReq::MD_RIGHT; break;
		case Maze::MAZE_UP: return MoveReq::MD_UP; break;
		case Maze::MAZE_DOWN: return MoveReq::MD_DOWN; break;
		case Maze::MAZE_BOTTOM: return MoveReq::MD_BOTTOM; break;
		case Maze::MAZE_TOP: return MoveReq::MD_TOP; break;
		default: return MoveReq::MD_NONE;
		}
	}

	MazeConfig & getMazeConfig() { return config_; }
	const MazeConfig & getMazeConfig() const { return config_; }
	
	const matrix3d_u8 * getMazeMatrix() const { return maze_matrix_; }

	const matrix3d_u8 * getWorldMatrix() const { return world_matrix_; }
	void displayWorldMatrix(int level = -1) const;

	const Vertex3DEx & getGoal() const { return goal_; }

	bool isReady() const { return sessions_.size() == MAX_PLAYERS; }

	void buildMaze(const MazeConfig & config);
	bool joinMaze(const std::shared_ptr<MazeSession> & session, uint32_t num_players);
	void leaveMaze(const std::shared_ptr<MazeSession> & session);
	void clearSessions();

	bool movePlayer(uint32_t player_id, const move_req_ptr & req, bool * won = nullptr);

	void processAI();
	void joinAIThread();
	
	static uint8_t getOppositeWall(const uint8_t dir);

private:
	void processProspectiveRoom(const Vertex3DEx & prospect_vert, vertex3d_vec & explored_verts);

	void broadcast(const GameMessage & msg) const;

	// Non-copyable.
	Maze(const Maze &);
	void operator=(const Maze &);
};

typedef std::shared_ptr<Maze> maze_ptr;
typedef std::vector<maze_ptr> maze_vector;

#endif
