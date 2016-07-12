#include <ctime>
#include "MazeServer.h"


MazeManager::MazeManager(MazeServer & server) :
	server_(server)
{
	srand(static_cast<unsigned int>(time(0)));
}

void MazeManager::loadNewMaze(const MazeConfig & config)
{
	maze_ptr maze = std::make_shared<Maze>();
	maze->buildMaze(config);
	maze->displayWorldMatrix();
	mazes_.push_back(maze);	

	broadcastSummaryData();
}

bool MazeManager::joinMaze(const maze_session_ptr & session, uint32_t selection, uint32_t num_players)
{
	if (--selection >= mazes_.size())
	{
		std::cerr << "ERROR: MazeManager::joinMaze [Game selection invalid: " << selection << "]" << std::endl;
		return false;
	}
	
	if (!mazes_[selection]->joinMaze(session, num_players))
	{
		std::cerr << "ERROR: MazeManager::joinMaze [Cannot join game]" << std::endl;
		return false;
	}

	return true;
}

void MazeManager::leaveMaze(const maze_session_ptr & session, uint32_t selection)
{
	mazes_[selection]->leaveMaze(session);
}

void MazeManager::movePlayer(uint32_t maze, uint32_t player_id, move_req_ptr & req)
{
	bool won = false;
	mazes_[maze]->movePlayer(player_id, req, &won);
	if (won)
	{
		mazes_[maze]->joinAIThread();
		mazes_[maze]->clearSessions();
	}
}

void MazeManager::broadcastSummaryData() const
{
	GameSummary summary_data(mazes_.size());
	for (maze_vector::const_iterator it = mazes_.begin(); it != mazes_.end(); ++it)
		summary_data.addMazeConfig((*it)->getMazeConfig());
	GameMessage msg(GameMessage::GC_GAMES_NOTIFY, &summary_data);
	server_.broadcast(msg);
}
