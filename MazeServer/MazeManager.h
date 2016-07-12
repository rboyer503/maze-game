#ifndef MAZE_MANAGER_H
#define MAZE_MANAGER_H

#include "Maze.h"


// Forward declaration to avoid circular dependency
class MazeServer;


class MazeManager
{
	maze_vector mazes_;
	MazeServer & server_;

public:
	MazeManager(MazeServer & server);

	void loadNewMaze(const MazeConfig & config);
	
	bool joinMaze(const std::shared_ptr<MazeSession> & session, uint32_t selection, uint32_t num_players);
	void leaveMaze(const std::shared_ptr<MazeSession> & session, uint32_t selection);

	void movePlayer(uint32_t maze, uint32_t player_id, move_req_ptr & req);

	void broadcastSummaryData() const;
};

#endif
