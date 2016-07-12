#include <boost/bind.hpp>
#include "MazeServer.h"

using boost::asio::ip::tcp;


// Compiler warning can be ignored: ('this' : used in base member initializer list).
MazeServer::MazeServer(boost::asio::io_service & io_service, const tcp::endpoint & endpoint) :
	io_service_(io_service), acceptor_(io_service, endpoint), maze_mgr_(*this)
{
	startAccept();
}

void MazeServer::startAccept()
{
	int pos = findFreeSessionPos();
	size_t id = static_cast<size_t>(pos + 1);
	if (pos < 0)
		id = sessions_.size() + 1;

	maze_session_ptr newSession(new MazeSession(io_service_, static_cast<uint32_t>(id), maze_mgr_));
	if (pos < 0)
		sessions_.push_back(newSession);
	else
		sessions_[pos] = newSession;

	acceptor_.async_accept(newSession->socket(),
		boost::bind(&MazeServer::handleAccept, this, newSession,
			boost::asio::placeholders::error));
}

void MazeServer::handleAccept(maze_session_ptr session, const boost::system::error_code & error)
{
	if (!error)
	{
		session->start();
		std::cout << "Session established for Player " << session->getPlayerId() << "." << std::endl;
	}
	else
	{
		std::cerr << "ERROR: MazeServer::handleAccept [" << error.value() << ": " << error.message() << "]" << std::endl;
	}

	startAccept();
}

void MazeServer::broadcast(const GameMessage & msg)
{
	for (maze_session_weak_vec::iterator it = sessions_.begin(); it != sessions_.end(); ++it)
	{
		try
        {
            if (auto p = (*it).lock())
			{
				if (p->isStarted())
					p->write(msg);
			}
			else
			{
				std::cerr << "WARNING: MazeServer::broadcast [Weak pointer lock failure]" << std::endl;
			}
        }
        catch (const std::bad_weak_ptr & b)
        {
            std::cerr << "WARNING: MazeServer::broadcast [Ignoring bad weak pointer]" << std::endl;
        }
	}
}

int MazeServer::findFreeSessionPos() const
{
	for (size_t i = 0; i < sessions_.size(); ++i)
	{
		if (sessions_[i].expired())
			return static_cast<int>(i);
	}
	return -1;
}


int main(int argc, char * argv[])
{
	try
	{
		if (argc != 2)
		{
			std::cerr << "Usage: MazeServer <port>" << std::endl;
			return 1;
		}

		boost::asio::io_service io_service;
		tcp::endpoint endpoint(tcp::v4(), atoi(argv[1]));
		MazeServer server(io_service, endpoint);
		io_service.run();
	}
	catch (std::runtime_error & e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
