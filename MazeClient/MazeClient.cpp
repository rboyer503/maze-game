#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include "MazeClient.h"

using boost::asio::ip::tcp;


MazeClient::MazeClient(boost::asio::io_service & io_service, tcp::resolver::iterator endpoint_iterator,
	ClientManager & client_mgr) : 
	io_service_(io_service), socket_(io_service), client_mgr_(client_mgr)
{
	boost::asio::async_connect(socket_, endpoint_iterator,
		boost::bind(&MazeClient::handleConnect, this,
			boost::asio::placeholders::error));
}

void MazeClient::write(const GameMessage & msg)
{
	io_service_.post(boost::bind(&MazeClient::doWrite, this, msg));
}

void MazeClient::close()
{
	io_service_.post(boost::bind(&MazeClient::doClose, this));
}

void MazeClient::handleConnect(const boost::system::error_code & error)
{
	if (error)
	{
		std::cout << "failed.\n\n";
		std::cerr << "ERROR: MazeClient::handleConnect [" << error.value() << ": " << error.message() << "]" << std::endl;
		return;
	}

	std::cout << "connected.\n\n";
	boost::asio::async_read(socket_,
		boost::asio::buffer(read_msg_.data(), GameMessage::HEADER_SIZE),
		boost::bind(&MazeClient::handleReadHeader, this,
			boost::asio::placeholders::error));
}

void MazeClient::handleReadHeader(const boost::system::error_code & error)
{
	if (error)
	{
		if ( (error.value() != 1236) && (error.value() != 125) )
			std::cerr << "ERROR: MazeClient::handleReadHeader [" << error.value() << ": " << error.message() << "]" << std::endl;
		doClose();
		return;
	}

	if (read_msg_.decodeHeader())
	{
		boost::asio::async_read(socket_,
			boost::asio::buffer(read_msg_.body(), read_msg_.bodyLength()),
			boost::bind(&MazeClient::handleReadBody, this,
				boost::asio::placeholders::error));
	}
	else
	{
		std::cerr << "ERROR: MazeClient::handleReadHeader [Decode header failed]" << std::endl;
		doClose();
	}
}

void MazeClient::handleReadBody(const boost::system::error_code & error)
{
	if (error)
	{
		std::cerr << "ERROR: MazeClient::handleReadBody [" << error.value() << ": " << error.message() << "]" << std::endl;
		doClose();
		return;
	}

	client_mgr_.processMessage(read_msg_);

	boost::asio::async_read(socket_,
		boost::asio::buffer(read_msg_.data(), GameMessage::HEADER_SIZE),
		boost::bind(&MazeClient::handleReadHeader, this,
			boost::asio::placeholders::error));
}

void MazeClient::doWrite(GameMessage msg)
{
	bool write_in_progress = !write_msgs_.empty();
	write_msgs_.push_back(msg);
	if (!write_in_progress)
	{
		boost::asio::async_write(socket_,
			boost::asio::buffer(write_msgs_.front().data(),	write_msgs_.front().length()),
			boost::bind(&MazeClient::handleWrite, this,
				boost::asio::placeholders::error));
	}
}

void MazeClient::handleWrite(const boost::system::error_code & error)
{
	if (error)
	{
		std::cerr << "ERROR: MazeClient::handleWrite [" << error.value() << ": " << error.message() << "]" << std::endl;
		doClose();
		return;
	}

	write_msgs_.pop_front();
	if (!write_msgs_.empty())
	{
		boost::asio::async_write(socket_,
			boost::asio::buffer(write_msgs_.front().data(),	write_msgs_.front().length()),
			boost::bind(&MazeClient::handleWrite, this,
				boost::asio::placeholders::error));
	}
}

void MazeClient::doClose()
{
	socket_.close();
}


int main(int argc, char* argv[])
{
	try
	{
		if (argc != 3)
		{
			std::cerr << "Usage: MazeClient <host> <port>" << std::endl;
			return 1;
		}

		std::cout << "Maze Game" << std::endl;
		std::cout << "---------" << std::endl << std::endl;
		std::cout << "Connecting to maze server... ";

		boost::asio::io_service io_service;
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(argv[1], argv[2]);
		tcp::resolver::iterator iterator = resolver.resolve(query);

		ClientManager client_mgr;
		MazeClient client(io_service, iterator, client_mgr);

		// Resolve ambiguity...
		std::size_t (boost::asio::io_service::*run) () = &boost::asio::io_service::run;
		boost::thread thread(boost::bind(run, &io_service));

		client_mgr.run(&client);

		client.close();
		thread.join();
	}
	catch (std::runtime_error & e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}
