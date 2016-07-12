#include "GameMessage.h"


bool GameMessage::decodeHeader()
{
	body_length_ = ntohl(*(reinterpret_cast<uint32_t *>(data_)));
	if (body_length_ > MAX_SIZE)
	{
		std::cerr << "ERROR: GameMessage::decodeHeader [Message exceeds maximum size]" << std::endl;
		return false;
	}

	game_code_ = static_cast<eGameCode>(ntohs(*(reinterpret_cast<uint16_t *>(data_ + LENGTH_SIZE))));
	if ( (game_code_ <= GC_NONE) || (game_code_ >= GC_MAX) )
	{
		std::cerr << "ERROR: GameMessage::decodeHeader [Invalid game message code " << game_code_ << "]" << std::endl;
		return false;
	}

	return true;
}

game_data_ptr GameMessage::decodeBody()
{
	switch (game_code_)
	{
	case GC_ID_NOTIFY:
		game_data_ = std::make_shared<PlayerID>();
		break;
	case GC_GAMES_NOTIFY:
		game_data_ = std::make_shared<GameSummary>();
		break;
	case GC_CREATE_REQ:
		game_data_ = std::make_shared<GameConfig>();
		break;
	case GC_SELECT_GAME_REQ:
		game_data_ = std::make_shared<GameSelect>();
		break;
	case GC_SELECT_GAME_RESP:
		game_data_ = std::make_shared<GameSelectResp>();
		break;
	case GC_START_NOTIFY:
		game_data_ = std::make_shared<matrix3d_u8>();
		break;
	case GC_UPDATE_NOTIFY:
		game_data_ = std::make_shared<Player>();
		break;
	case GC_MOVE_REQ:
		game_data_ = std::make_shared<MoveReq>();
		break;
	case GC_WINNER_NOTIFY:
		game_data_ = std::make_shared<Winner>();
		break;
	default:
		std::cerr << "ERROR: GameMessage::decodeBody [Unexpected game message code " << game_code_ << "]" << std::endl;
		game_data_ = nullptr;
		break;
	}
	
	if (game_data_)
	{
		if (!game_data_->deserializeData(body(), bodyLength()))
		{
			game_data_ = nullptr;
		}
	}

	return game_data_;
}

void GameMessage::processSerialData(const char * serial_data)
{
	if (body_length_ > MAX_SIZE)
		throw std::runtime_error("GameMessage::processSerialData: [Message exceeds maximum size]");

	char * ptr = data_;
	*(reinterpret_cast<uint32_t *>(ptr)) = htonl(static_cast<const uint32_t>(body_length_));
	ptr += LENGTH_SIZE;

	*(reinterpret_cast<uint16_t *>(ptr)) = htons(static_cast<const uint16_t>(game_code_));
	ptr += CODE_SIZE;

	memcpy(ptr, serial_data, body_length_);
}

std::ostream & operator<<(std::ostream & os, const GameMessage & game_msg)
{
	os << "GameMessage: Length=" << game_msg.length() << ", Code=" << game_msg.game_code_ << std::endl;
	if (game_msg.game_data_)
		os << *game_msg.game_data_;
	else
	{
		os << "Raw: ";
		os.write(game_msg.body(), game_msg.bodyLength());
		os << std::endl;
	}
	return os;
}
