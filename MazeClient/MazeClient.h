#ifndef MAZE_CLIENT_H
#define MAZE_CLIENT_H

#include <boost/asio.hpp>
#include "ClientManager.h"


class MazeClient
{
	boost::asio::io_service & io_service_;
	boost::asio::ip::tcp::socket socket_;
	GameMessage read_msg_;
	game_message_queue write_msgs_;
	ClientManager & client_mgr_;

public:
	MazeClient(boost::asio::io_service & io_service, boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
		ClientManager & client_mgr);

	void write(const GameMessage & msg);
	void close();

private:
	void handleConnect(const boost::system::error_code & error);
	void handleReadHeader(const boost::system::error_code & error);
	void handleReadBody(const boost::system::error_code & error);
	void doWrite(GameMessage msg);
	void handleWrite(const boost::system::error_code & error);
	void doClose();
};

#endif // MAZE_CLIENT_H
