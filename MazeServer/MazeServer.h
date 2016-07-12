#ifndef MAZE_SERVER_H
#define MAZE_SERVER_H

#include "MazeSession.h"


class MazeServer
{
	boost::asio::io_service & io_service_;
	boost::asio::ip::tcp::acceptor acceptor_;
	maze_session_weak_vec sessions_;
	MazeManager maze_mgr_;

public:
	MazeServer(boost::asio::io_service & io_service, const boost::asio::ip::tcp::endpoint & endpoint);

	void startAccept();
	void handleAccept(maze_session_ptr session, const boost::system::error_code & error);
	void broadcast(const GameMessage & msg);

private:
	int findFreeSessionPos() const;
};

#endif // MAZE_SERVER_H
