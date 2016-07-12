#ifndef AIAGENT_H
#define AIAGENT_H

#include <random>
#include "../MazeShared/GameMessage.h"


struct BranchNode
{
	Vertex3DEx maze_pos;
	uint32_t distance; // # of rooms from goal - used as simple heuristic
	std::vector<std::shared_ptr<BranchNode> > adjacencies;
	std::vector<uint8_t> try_count;
	uint8_t flags;

	BranchNode(const Vertex3DEx & pos, uint32_t dist) :
		maze_pos(pos), distance(dist), flags(0)
	{
	}

	bool checkDeadEnd();
};

typedef std::shared_ptr<BranchNode> branch_node_ptr;
typedef std::vector<branch_node_ptr> branch_node_vec;


class Maze;


class AIAgent
{
	static const uint32_t BASE_AGENT_ID = 17; // 17 + 48 = 65 = ASCII 'A'
	static const uint8_t DEF_DELAY_TICKS = 6; // Default to moving every 6 ticks (0.6 second)
	static const size_t NULL_INDEX = 6;

	static std::default_random_engine random_gen_;
	Maze & maze_;
	player_ptr agent_;
	uint32_t player_id_;
	Vertex3DEx maze_pos_;
	uint8_t delay_ticks_;
	branch_node_ptr root_;
	bool branch_point;
	branch_node_ptr prev_node_;
	branch_node_ptr target_node_;
	uint8_t last_dir_;
	bool halfway_;
	bool revert_;

public:
	AIAgent(uint32_t agent_num, Maze & maze);
	~AIAgent();

	player_ptr & getPlayer() { return agent_; }
	const uint32_t getPlayerId() const { return player_id_; }

	bool handleTick();
	bool processMove();

private:
	uint32_t calcDistance(const Vertex3DEx & pos) const;
	void exploreBranchNode();
	uint8_t getDirFromIndex(size_t index, uint8_t room) const;
};

#endif // AIAGENT_H
