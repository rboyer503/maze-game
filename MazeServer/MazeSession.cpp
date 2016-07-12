#include <boost/bind.hpp>
#include "MazeSession.h"

using boost::asio::ip::tcp;


MazeSession::MazeSession(boost::asio::io_service & io_service, uint32_t player_id, MazeManager & maze_mgr) :
	socket_(io_service), player_id_(player_id), maze_mgr_(maze_mgr), started_(false), curr_maze_(0)
{}

MazeSession::~MazeSession()
{
	std::cout << "Session terminated for Player " << player_id_ << "." << std::endl;
}

void MazeSession::start()
{
	boost::asio::async_read(socket_,
		boost::asio::buffer(read_msg_.data(), GameMessage::HEADER_SIZE),
		boost::bind(
			&MazeSession::handleReadHeader, shared_from_this(),
			boost::asio::placeholders::error));
	started_ = true;

	PlayerID game_data(player_id_);
	GameMessage msg(GameMessage::GC_ID_NOTIFY, &game_data);
	write(msg);

	maze_mgr_.broadcastSummaryData();
}

void MazeSession::write(const GameMessage & msg)
{
	boost::mutex::scoped_lock lock(write_mutex_);
	bool write_in_progress = !write_msgs_.empty();
	write_msgs_.push_back(msg);
	if (!write_in_progress)
	{
		boost::asio::async_write(socket_,
			boost::asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
			boost::bind(&MazeSession::handleWrite, shared_from_this(),
				boost::asio::placeholders::error));
	}
}

void MazeSession::handleReadHeader(const boost::system::error_code & error)
{
	if (error)
	{
		std::cerr << "ERROR: MazeSession::handleReadHeader [" << error.value() << ": " << error.message() << "]" << std::endl;
		leaveMaze();
		return;
	}

	if (read_msg_.decodeHeader())
	{
		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.body(), read_msg_.bodyLength()),
			boost::bind(&MazeSession::handleReadBody, shared_from_this(),
				boost::asio::placeholders::error));
	}
	else
	{
		std::cerr << "ERROR: MazeSession::handleReadHeader [Decode header failed]" << std::endl;
		leaveMaze();
	}
}

void MazeSession::handleReadBody(const boost::system::error_code & error)
{
	if (error)
	{
		std::cerr << "ERROR: MazeSession::handleReadBody [" << error.value() << ": " << error.message() << "]" << std::endl;
		leaveMaze();
		return;
	}

	processMessage(read_msg_);

	boost::asio::async_read(socket_,
		boost::asio::buffer(read_msg_.data(), GameMessage::HEADER_SIZE),
		boost::bind(&MazeSession::handleReadHeader, shared_from_this(),
			boost::asio::placeholders::error));
}

void MazeSession::handleWrite(const boost::system::error_code & error)
{
	if (error)
	{
		std::cerr << "ERROR: MazeSession::handleWrite [" << error.value() << ": " << error.message() << "]" << std::endl;
		leaveMaze();
		return;
	}

	boost::mutex::scoped_lock lock(write_mutex_);
	write_msgs_.pop_front();
	if (!write_msgs_.empty())
	{
		boost::asio::async_write(socket_,
			boost::asio::buffer(write_msgs_.front().data(),	write_msgs_.front().length()),
			boost::bind(&MazeSession::handleWrite, shared_from_this(),
				boost::asio::placeholders::error));
	}
}

void MazeSession::processMessage(GameMessage & game_msg)
{
	switch (game_msg.getGameCode())
	{
	case GameMessage::GC_CREATE_REQ:
		{
			game_config_ptr game_data = std::dynamic_pointer_cast<GameConfig>(game_msg.decodeBody());
			if (!game_data)
			{
				std::cerr << "ERROR: MazeSession::processMessage [Unexpected message received]" << std::endl << *game_data;
				return;
			}

			maze_mgr_.loadNewMaze(MazeConfig(game_data->getWidth(), game_data->getHeight(), game_data->getLevels()));
		}
		break;
	case GameMessage::GC_SELECT_GAME_REQ:
		{
			game_select_ptr game_data = std::dynamic_pointer_cast<GameSelect>(game_msg.decodeBody());
			if (!game_data)
			{
				std::cerr << "ERROR: MazeSession::processMessage [Unexpected message received]" << std::endl << *game_data;
				return;
			}

			if (!maze_mgr_.joinMaze(shared_from_this(), game_data->getSelection(), game_data->getNumPlayers()))
			{
				GameSelectResp select_resp(GameSelectResp::SR_FAIL);
				GameMessage msg(GameMessage::GC_SELECT_GAME_RESP, &select_resp);
				write(msg);

				std::cerr << "ERROR: MazeSession::processMessage [Failed to join maze]" << std::endl << *game_data;
				return;
			}

			curr_maze_ = game_data->getSelection();
		}
		break;
	case GameMessage::GC_MOVE_REQ:
		{
			move_req_ptr game_data = std::dynamic_pointer_cast<MoveReq>(game_msg.decodeBody());
			if (!game_data)
			{
				std::cerr << "ERROR: MazeSession::processMessage [Unexpected message received]" << std::endl << *game_data;
				return;
			}

			maze_mgr_.movePlayer(curr_maze_ - 1, player_id_, game_data);
		}
		break;
	case GameMessage::GC_CANCEL_REQ:
		{
			leaveMaze();
		}
		break;
	default:
		{
			std::cerr << "ERROR: MazeSession::processMessage [Unexpected message received]" << std::endl;
			game_data_ptr game_data = game_msg.decodeBody();
			if (game_data)
				std::cerr << *game_data;
			else
				std::cerr << "[Code=" << game_msg.getGameCode() << "]" << std::endl;
		}
		break;
	}
}

void MazeSession::leaveMaze()
{
	if (curr_maze_)
	{
		maze_mgr_.leaveMaze(shared_from_this(), (curr_maze_ - 1));
		curr_maze_ = 0;
	}
}
