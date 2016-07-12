#ifndef MAZE_SESSION_H
#define MAZE_SESSION_H

#include <boost/asio.hpp>
#include "MazeManager.h"


class MazeSession : public std::enable_shared_from_this<MazeSession>
{
	boost::asio::ip::tcp::socket socket_;
	GameMessage read_msg_;
	uint32_t player_id_;
	game_message_queue write_msgs_;
	MazeManager & maze_mgr_;
	volatile bool started_;
	uint32_t curr_maze_;
	boost::mutex write_mutex_;

public:
	MazeSession(boost::asio::io_service & io_service, uint32_t player_id, MazeManager & maze_mgr);
	~MazeSession();

	boost::asio::ip::tcp::socket & socket() { return socket_; }
	uint32_t getPlayerId() const { return player_id_; }
	bool isStarted() const { return started_; }

	void start();
	void write(const GameMessage & msg);

	void handleReadHeader(const boost::system::error_code & error);
	void handleReadBody(const boost::system::error_code & error);
	void handleWrite(const boost::system::error_code & error);

private:
	void processMessage(GameMessage & game_msg);
	void leaveMaze();

};

typedef std::shared_ptr<MazeSession> maze_session_ptr;
typedef std::weak_ptr<MazeSession> maze_session_weak_ptr;
typedef std::vector<maze_session_ptr> maze_session_vec;
typedef std::vector<maze_session_weak_ptr> maze_session_weak_vec;

#endif // MAZE_SESSION_H
