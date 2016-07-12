#ifndef GAME_MESSAGE_H
#define GAME_MESSAGE_H

#include <deque>
#include "GameData.h"


class GameMessage
{
	static const size_t LENGTH_SIZE = 4;
	static const size_t CODE_SIZE = 2;
	static const size_t MAX_SIZE = 20000;

public:
	static const size_t HEADER_SIZE = LENGTH_SIZE + CODE_SIZE;

	enum eGameCode /* for C++11 add ": uint16_t" */
	{
		GC_NONE = 100,
		/* Insert new codes after GC_NONE */
		GC_ID_NOTIFY,
		GC_GAMES_NOTIFY,
		GC_CREATE_REQ,
		GC_SELECT_GAME_REQ,
		GC_SELECT_GAME_RESP,
		GC_START_NOTIFY,
		GC_MOVE_REQ,
		GC_CANCEL_REQ,
		GC_UPDATE_NOTIFY,
		GC_WINNER_NOTIFY,
		/* Insert new codes before GC_MAX */
		GC_MAX
	};

	// Constructor for message receiver.
	GameMessage() :
		body_length_(0), game_code_(GC_NONE)
	{}

	// Constructors for message sender.
	explicit GameMessage(const eGameCode game_code, const char * serial_data = "", size_t length = 0) :
		body_length_(length), game_code_(game_code)
	{
		processSerialData(serial_data);
	}

	GameMessage(const eGameCode game_code, GameData * game_data) :
		body_length_(game_data->getLength()), game_code_(game_code)
	{
		if (!game_data)
			throw std::runtime_error("GameMessage::GameMessage: [game_data is NULL]");

		processSerialData(game_data->serializeData());
	}

	const char * data() const { return data_; }
	char * data() { return data_; }

	size_t length() const { return HEADER_SIZE + body_length_; }

	const char * body() const { return data_ + HEADER_SIZE; }
	char * body() { return data_ + HEADER_SIZE; }

	size_t bodyLength() const { return body_length_; }

	eGameCode getGameCode() const { return game_code_; }

	bool decodeHeader();
	game_data_ptr decodeBody();

	friend std::ostream & operator<<(std::ostream & os, const GameMessage & game_message);

private:
	void processSerialData(const char * serial_data);

	char data_[HEADER_SIZE + MAX_SIZE];
	size_t body_length_;
	eGameCode game_code_;
	game_data_ptr game_data_; // Only set by decodeBody for message receiver.
};

typedef std::deque<GameMessage> game_message_queue;

#endif // GAME_MESSAGE_H
