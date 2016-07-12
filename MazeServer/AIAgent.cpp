#include "AIAgent.h"
#include "Maze.h"


bool BranchNode::checkDeadEnd()
{
	size_t count = 0;
	for (auto it = adjacencies.begin(); it != adjacencies.end(); ++it)
		if ((*it)->flags & Maze::MAZE_DEADEND)
			++count;

	if ((adjacencies.size() - count) < 2)
	{
		flags |= Maze::MAZE_DEADEND;
		return true;
	}

	return false;
}


std::default_random_engine AIAgent::random_gen_;


AIAgent::AIAgent(uint32_t agent_num, Maze & maze) :
	maze_(maze), delay_ticks_(DEF_DELAY_TICKS), branch_point(true), prev_node_(nullptr), last_dir_(Maze::MAZE_NONE),
	halfway_(false), revert_(false)
{
	const matrix3d_u8 * maze_matrix = maze.getMazeMatrix();

	Vertex3DEx world_pos;

	switch (agent_num)
	{
	case 0:
		world_pos = Vertex3DEx(maze.getWorldMatrix()->getWidth() - 4, maze.getWorldMatrix()->getHeight() - 2, 0);
		maze_pos_ = Vertex3DEx(maze_matrix->getWidth() - 1, maze_matrix->getHeight() - 1, 0);
		break;
	default:
		throw std::runtime_error("AIAgent::AIAgent: [Invalid agent number]");
	}

	player_id_ = BASE_AGENT_ID + agent_num;
	agent_ = std::make_shared<Player>(player_id_, world_pos);

	target_node_ = root_ = std::make_shared<BranchNode>(maze_pos_, calcDistance(maze_pos_));
}

AIAgent::~AIAgent()
{
}

bool AIAgent::handleTick()
{
	if (!--delay_ticks_)
	{
		delay_ticks_ = DEF_DELAY_TICKS;
		return processMove();
	}
	return false;
}

bool AIAgent::processMove()
{	
	uint8_t room = maze_.getMazeMatrix()->at(maze_pos_);

	if (!halfway_ && (maze_pos_ == target_node_->maze_pos))
	{
		// Current node is a branch point (i.e.: there is more than one way to proceed).
		// Most branch points have 3 or more openings.  The following exceptions exist:
		//  1) Starting node
		//  2) Goal node
		//  3) Stairwells

		// Build out spanning tree.
		// For each branch, traverse rooms until either:
		//	1) another branch point is found, or
		//	2) the path ends.
		// In case 1, build and link another branch node.

		if (!(target_node_->flags & Maze::MAZE_EXPLORED))
			exploreBranchNode();

		int16_t * weights = new int16_t[target_node_->try_count.size()];
		uint32_t max = 0;
		size_t rev_index = NULL_INDEX;
		uint8_t min_try_count = std::numeric_limits<uint8_t>::max();
		branch_node_ptr node;
		for (size_t i = 0; i < target_node_->adjacencies.size(); ++i)
		{
			node = target_node_->adjacencies[i];
			if (node->flags & Maze::MAZE_DEADEND)
			{
				weights[i] = 0;
				continue;
			}
			else
				weights[i] = 1;

			if (node->distance > max)
				max = node->distance;
			else if (node->distance == 0)
			{
				for (size_t j = 0; j < target_node_->adjacencies.size(); ++j)
					weights[j] = 0;
				weights[i] = 1;
				break;
			}

			if (prev_node_ && (node->maze_pos == prev_node_->maze_pos))
				rev_index = i;

			if (target_node_->try_count[i] < min_try_count)
				min_try_count = target_node_->try_count[i];
		}

		int16_t total_weights = 0;
		for (size_t i = 0; i < target_node_->adjacencies.size(); ++i)
		{
			if (!weights[i])
				continue;

			weights[i] = 20;
			weights[i] += (max - target_node_->adjacencies[i]->distance) * 40;
			weights[i] -= (target_node_->try_count[i] - min_try_count) * 10;

			if (i == rev_index)
				weights[i] /= 4;

			if (weights[i] < 1)
				weights[i] = 1;

			total_weights += weights[i];
		}

		size_t index;
		std::uniform_int_distribution<int> distribution(0, total_weights - 1);
		int decision = distribution(random_gen_);
		for (index = 0; index < target_node_->adjacencies.size(); ++index)
		{
			if (!weights[index])
				continue;

			if (decision < weights[index])
				break;

			decision -= weights[index];
		}

		delete [] weights;

		/*
		uint32_t min = std::numeric_limits<uint32_t>::max();
		size_t index;
		bool reverse = false;
		branch_node_ptr node;
		for (size_t i = 0; i < target_node_->adjacencies.size(); ++i)
		{
			node = target_node_->adjacencies[i];
			if ( !(node->flags & Maze::MAZE_DEADEND) && ( (node->distance < min) || reverse) )
			{
				if (prev_node_)
				{
					if (node->maze_pos == prev_node_->maze_pos)
						reverse = true;
					else
						reverse = false;
				}
				if ( !reverse || (min == std::numeric_limits<uint32_t>::max()) )
				{
					min = node->distance;
					index = i;
				}
			}
		}
		*/

		target_node_->checkDeadEnd();

		target_node_->try_count[index]++;
		prev_node_ = target_node_;
		target_node_ = target_node_->adjacencies[index];
		last_dir_ = getDirFromIndex(index, prev_node_->flags);
	}
	else if (!halfway_)
	{
		last_dir_ = Maze::getOppositeWall(last_dir_);
		for (uint8_t i = 1; i <= (1 << 5); i <<= 1)
		{
			if ( (i != last_dir_) && !(room & i))
			{
				last_dir_ = i;
				break;
			}
		}
	}

	bool won = false;
	if ( !maze_.movePlayer(player_id_, std::make_shared<MoveReq>(Maze::mazeDirToMoveDir(last_dir_)), &won) )
	{
		last_dir_ = Maze::getOppositeWall(last_dir_);
		std::swap(target_node_, prev_node_);
		//if ( (last_dir_ != Maze::MAZE_BOTTOM) && (last_dir_ != Maze::MAZE_TOP) )
		if (halfway_)
			revert_ = true;
		return false;
	}

	if ( (last_dir_ != Maze::MAZE_BOTTOM) && (last_dir_ != Maze::MAZE_TOP) )
		halfway_ = !halfway_;

	if (!halfway_ && !revert_)
	{
		switch (last_dir_)
		{
		case Maze::MAZE_LEFT: --maze_pos_.x; break;
		case Maze::MAZE_RIGHT: ++maze_pos_.x; break;
		case Maze::MAZE_UP: --maze_pos_.y; break;
		case Maze::MAZE_DOWN: ++maze_pos_.y; break;
		case Maze::MAZE_BOTTOM: --maze_pos_.z; break;
		case Maze::MAZE_TOP: ++maze_pos_.z; break;
		}
	}
	revert_ = false;

	if (maze_pos_ != target_node_->maze_pos)
		delay_ticks_ = 3;

	return won;
}

uint32_t AIAgent::calcDistance(const Vertex3DEx & pos) const
{
	const Vertex3DEx & goal = maze_.getGoal();
	return static_cast<uint32_t>( abs(static_cast<int>(pos.x) - static_cast<int>(goal.x)) +
		abs(static_cast<int>(pos.y) - static_cast<int>(goal.y)) +
		abs(static_cast<int>(pos.z) - static_cast<int>(goal.z)) );
}

void AIAgent::exploreBranchNode()
{
	target_node_->flags |= Maze::MAZE_EXPLORED;

	const matrix3d_u8 * maze_matrix = maze_.getMazeMatrix();
	const Vertex3DEx & goal = maze_.getGoal();

	uint8_t room = maze_matrix->at(target_node_->maze_pos);
	size_t num_branches = Maze::countBranches(room);
	target_node_->adjacencies.reserve(num_branches);
	target_node_->try_count.reserve(num_branches);

	uint8_t from_dir = Maze::getOppositeWall(last_dir_);
	for (uint8_t i = 1; i <= (1 << 5); i <<= 1)
	{
		if (i == from_dir)
		{
			target_node_->flags |= i;
			target_node_->adjacencies.push_back(prev_node_);
			target_node_->try_count.push_back(0);
		}
		else if (!(room & i))
		{
			Vertex3DEx temp_pos = target_node_->maze_pos;
			uint8_t temp_room;
			uint8_t dir = i;
			size_t num_branches;
			bool found = false;

			while (true)
			{
				switch (dir)
				{
				case Maze::MAZE_LEFT: --temp_pos.x; break;
				case Maze::MAZE_RIGHT: ++temp_pos.x; break;
				case Maze::MAZE_UP: --temp_pos.y; break;
				case Maze::MAZE_DOWN: ++temp_pos.y; break;
				case Maze::MAZE_BOTTOM: --temp_pos.z; break;
				case Maze::MAZE_TOP: ++temp_pos.z; break;
				}

				if (temp_pos == goal)
				{
					found = true;
					break;
				}

				temp_room = maze_matrix->at(temp_pos);

				num_branches = Maze::countBranches(temp_room);
				if (num_branches == 1)
					break;

				if ( (num_branches > 2) || (Maze::hasStairwell(temp_room)) )
				{
					found = true;
					break;
				}

				dir = Maze::getOppositeWall(dir);
				for (uint8_t j = 1; j <= (1 << 5); j <<= 1)
				{
					if ( (dir != j) && !(temp_room & j))
					{
						dir = j;
						break;
					}
				}
			} 

			if (found)
			{
				target_node_->flags |= i;
				target_node_->adjacencies.push_back(std::make_shared<BranchNode>(temp_pos, calcDistance(temp_pos)));
				target_node_->try_count.push_back(0);
			}
		}
	}
}

uint8_t AIAgent::getDirFromIndex(size_t index, uint8_t flags) const
{
	for (uint8_t i = 1; i <= (1 << 5); i <<= 1)
	{
		if (flags & i)
		{
			if (!index)
				return i;
			else
				--index;
		}
	}

	throw new std::runtime_error("AIAgent::getDirFromIndex: [Direction not found]");
}
