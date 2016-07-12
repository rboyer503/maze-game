#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <map>
#include <memory>
#include <boost/asio.hpp> // Included to ensure that ASIO gets to include WinSock first
#include "GameStructs.h"

#ifdef _WIN32_WINNT
#include <Windows.h>
#else
#include <arpa/inet.h>
#endif


class GameData
{
public:
	virtual ~GameData() {}
	virtual char * serializeData() = 0;
	virtual bool deserializeData(const char * data, size_t length) = 0;
	virtual size_t getLength() const = 0;
	friend std::ostream & operator<<(std::ostream & os, const GameData & game_data);

protected:
	virtual void print(std::ostream & os) const {};
};

typedef std::shared_ptr<GameData> game_data_ptr;


template <typename T>
class BasicSingle : public GameData
{
	static const size_t DATA_SIZE = 4;

	T data_;

	char serial_data_[DATA_SIZE];

public:
	// Constructor for message receiver.
	BasicSingle() :
		data_(0)
	{}

	// Constructor for message sender.
	BasicSingle(T data) :
		data_(data)
	{}

	T getData() const { return data_; }

	virtual char * serializeData()
	{
		*(reinterpret_cast<T *>(serial_data_)) = htonl(data_);
		return serial_data_;
	}

	virtual bool deserializeData(const char * data, size_t length)
	{
		if (length != DATA_SIZE)
			return false;

		data_ = ntohl(*(reinterpret_cast<const uint32_t *>(data)));
		return true;
	}

	virtual size_t getLength() const { return DATA_SIZE; }

protected:
	virtual void print(std::ostream & os) const
	{
		os << "BasicSingle: Data=" << data_ << std::endl;
	}
};


class PlayerID : public BasicSingle<uint32_t>
{
public:
	// Constructor for message receiver.
	PlayerID() :
		BasicSingle(0)
	{}

	// Constructor for message sender.
	PlayerID(uint32_t id) :
		BasicSingle(id)
	{}

	uint32_t getPlayerId() const { return getData(); }

protected:
	virtual void print(std::ostream & os) const
	{
		os << "PlayerID: ID=" << getData() << std::endl;
	}
};

typedef std::shared_ptr<PlayerID> player_id_ptr;


class Winner : public BasicSingle<uint32_t>
{
public:
	// Constructor for message receiver.
	Winner() :
		BasicSingle(0)
	{}

	// Constructor for message sender.
	Winner(uint32_t id) :
		BasicSingle(id)
	{}

	uint32_t getWinnerId() const { return getData(); }

protected:
	virtual void print(std::ostream & os) const
	{
		os << "Winner: ID=" << getData() << std::endl;
	}
};

typedef std::shared_ptr<Winner> winner_ptr;


class GameSelectResp : public BasicSingle<uint32_t>
{
public:
	enum eSelectResp
	{
		SR_NONE,
		SR_WAIT,
		SR_FAIL
	};

	// Constructor for message receiver.
	GameSelectResp() :
		BasicSingle(0)
	{}

	// Constructor for message sender.
	GameSelectResp(eSelectResp resp) :
		BasicSingle(resp)
	{}

	eSelectResp getSelectResp() const { return static_cast<eSelectResp>(getData()); }

protected:
	virtual void print(std::ostream & os) const
	{
		os << "GameSelectResp: Resp=" << getData() << std::endl;
	}
};

typedef std::shared_ptr<GameSelectResp> select_resp_ptr;


class GameConfig : public GameData
{
	static const size_t DATA_SIZE = 12;

	uint32_t width_, height_, levels_;

	char serial_data_[DATA_SIZE];

public:
	// Constructor for message receiver.
	GameConfig() :
		width_(0), height_(0), levels_(0)
	{}

	// Constructor for message sender.
	GameConfig(uint32_t width, uint32_t height, uint32_t levels) :
		width_(width), height_(height), levels_(levels)
	{}

	uint32_t getWidth() const { return width_; }
	uint32_t getHeight() const { return height_; }
	uint32_t getLevels() const { return levels_; }

	virtual char * serializeData()
	{
		*(reinterpret_cast<uint32_t *>(serial_data_)) = htonl(width_);
		*(reinterpret_cast<uint32_t *>(serial_data_ + 4)) = htonl(height_);
		*(reinterpret_cast<uint32_t *>(serial_data_ + 8)) = htonl(levels_);
		return serial_data_;
	}

	virtual bool deserializeData(const char * data, size_t length)
	{
		if (length != DATA_SIZE)
			return false;

		width_ = ntohl(*(reinterpret_cast<const uint32_t *>(data)));
		height_ = ntohl(*(reinterpret_cast<const uint32_t *>(data + 4)));
		levels_ = ntohl(*(reinterpret_cast<const uint32_t *>(data + 8)));
		return true;
	}

	virtual size_t getLength() const { return DATA_SIZE; }

protected:
	virtual void print(std::ostream & os) const
	{
		os << "GameConfig: Width=" << width_ << ", Height=" << height_ <<
			", Levels=" << levels_ << std::endl;
	}
};

typedef std::shared_ptr<GameConfig> game_config_ptr;


class GameSummary : public GameData
{
	static const size_t ENTRY_SIZE = 12;

	maze_config_vec configs_;

	size_t data_len_;
	char * serial_data_;

public:
	// Constructor for message receiver.
	GameSummary() :
		data_len_(0), serial_data_(nullptr)
	{}

	// Constructor for message sender.
	GameSummary(size_t num_games)
	{ 
		data_len_ = ENTRY_SIZE * num_games;
		serial_data_ = new char[data_len_];
	}

	// Rule of three:
	GameSummary(const GameSummary & other)
	{
		configs_ = other.configs_;
		data_len_ = other.data_len_;
		if (other.serial_data_)
		{
			serial_data_ = new char[data_len_];
			memcpy(serial_data_, other.serial_data_, data_len_);
		}
		else
			serial_data_ = nullptr;
	}

	// Copy-and-swap idiom.
	friend void swap(GameSummary & first, GameSummary & second)
	{
		// Enable ADL (best practice).
		using std::swap;

        swap(first.configs_, second.configs_);
        swap(first.data_len_, second.data_len_);
		swap(first.serial_data_, second.serial_data_);
	}

	GameSummary & operator=(GameSummary other)
	{
		swap(*this, other);
		return *this;
	}

	~GameSummary()
	{
		delete [] serial_data_;
	}

	void addMazeConfig(MazeConfig & config)
	{
		configs_.push_back(config);
	}

	const maze_config_vec & getConfigs() { return configs_; }

	virtual char * serializeData()
	{
		char * ptr = serial_data_;

		for (maze_config_vec::iterator it = configs_.begin(); it != configs_.end(); ++it)
		{
			*(reinterpret_cast<uint32_t *>(ptr)) = htonl((*it).width);
			*(reinterpret_cast<uint32_t *>(ptr + 4)) = htonl((*it).height);
			*(reinterpret_cast<uint32_t *>(ptr + 8)) = htonl((*it).levels);
			ptr += ENTRY_SIZE;
		}

		return serial_data_;
	}

	virtual bool deserializeData(const char * data, size_t length)
	{
		if ( (length % ENTRY_SIZE) != 0 )
			return false;

		data_len_ = length;

		size_t num_games = length / ENTRY_SIZE;
		for (size_t i = 0; i < num_games; ++i)
		{
			uint32_t width = ntohl(*(reinterpret_cast<const uint32_t *>(data)));
			uint32_t height = ntohl(*(reinterpret_cast<const uint32_t *>(data + 4)));
			uint32_t levels = ntohl(*(reinterpret_cast<const uint32_t *>(data + 8)));
			configs_.push_back(MazeConfig(width, height, levels));
			data += ENTRY_SIZE;
		}
		return true;
	}

	virtual size_t getLength() const { return data_len_; }

protected:
	virtual void print(std::ostream & os) const
	{
		os << "GameSummary:" << std::endl;
		for (maze_config_vec::const_iterator it = configs_.begin(); it != configs_.end(); ++it)
		{
			os << "\tWidth=" << (*it).width << ", Height=" << (*it).height <<
				", Levels=" << (*it).levels << std::endl;
		}
	}
};

typedef std::shared_ptr<GameSummary> game_summary_ptr;


class GameSelect : public GameData
{
	static const size_t DATA_SIZE = 8;

	uint32_t selection_;
	uint32_t num_players_;

	char serial_data_[DATA_SIZE];

public:
	// Constructor for message receiver.
	GameSelect() :
		selection_(0), num_players_(0)
	{}

	// Constructor for message sender.
	GameSelect(uint32_t selection, uint32_t num_players) :
		selection_(selection), num_players_(num_players)
	{}

	uint32_t getSelection() const { return selection_; }
	uint32_t getNumPlayers() const { return num_players_; }

	virtual char * serializeData()
	{
		*(reinterpret_cast<uint32_t *>(serial_data_)) = htonl(selection_);
		*(reinterpret_cast<uint32_t *>(serial_data_ + 4)) = htonl(num_players_);
		return serial_data_;
	}

	virtual bool deserializeData(const char * data, size_t length)
	{
		if (length != DATA_SIZE)
			return false;

		selection_ = ntohl(*(reinterpret_cast<const uint32_t *>(data)));
		num_players_ = ntohl(*(reinterpret_cast<const uint32_t *>(data + 4)));
		return true;
	}

	virtual size_t getLength() const { return DATA_SIZE; }

protected:
	virtual void print(std::ostream & os) const
	{
		os << "GameSelect: Selection=" << selection_ << ", NumPlayers=" << num_players_ << std::endl;
	}
};

typedef std::shared_ptr<GameSelect> game_select_ptr;


template <typename T>
class Matrix3D : public GameData
{
	static const size_t HEADER_SIZE = 12;

	uint32_t width_, height_, depth_;
	uint32_t depth_offset_;
	T * buffer_;

	size_t data_len_;
	char * serial_data_;

public:
	// Constructor for message receiver (client).
	Matrix3D() :
		width_(0), height_(0), depth_(0), depth_offset_(0), buffer_(nullptr),
		data_len_(0), serial_data_(nullptr)
	{}

	// Constructor for message sender (server).
	Matrix3D(uint32_t width, uint32_t height, uint32_t depth, T init_value) :
		width_(width), height_(height), depth_(depth), depth_offset_(width * height),
		buffer_(new T[width * height * depth])
	{
		data_len_ = HEADER_SIZE + (depth_offset_ * depth);
		serial_data_ = new char[data_len_];

		T * ptr = buffer_;
		for (uint32_t i = 0; i < depth_offset_ * depth; ++i)
			*ptr++ = init_value;
	}

	~Matrix3D()
	{
		delete [] buffer_;
		delete [] serial_data_;
	}

	uint32_t getWidth() const { return width_; }
	uint32_t getHeight() const { return height_; }
	uint32_t getDepth() const { return depth_; }

	T & at(uint32_t x, uint32_t y, uint32_t z) { return buffer_[index(x, y, z)]; }
	const T & at(uint32_t x, uint32_t y, uint32_t z) const { return buffer_[index(x, y, z)]; }

	T * ptr(uint32_t x, uint32_t y, uint32_t z) { return &buffer_[index(x, y, z)]; }
	const T * ptr(uint32_t x, uint32_t y, uint32_t z) const { return &buffer_[index(x, y, z)]; }

	T & at(const Vertex3DEx & vert) { return at(vert.x, vert.y, vert.z); }
	const T & at(const Vertex3DEx & vert) const { return at(vert.x, vert.y, vert.z); }

	T * ptr(const Vertex3DEx & vert) { return ptr(vert.x, vert.y, vert.z); }
	const T * ptr(const Vertex3DEx & vert) const { return ptr(vert.x, vert.y, vert.z); }

	virtual char * serializeData()
	{
		*(reinterpret_cast<uint32_t *>(serial_data_)) = htonl(width_);
		*(reinterpret_cast<uint32_t *>(serial_data_ + 4)) = htonl(height_);
		*(reinterpret_cast<uint32_t *>(serial_data_ + 8)) = htonl(depth_);

		memcpy((serial_data_ + HEADER_SIZE), buffer_, (depth_offset_ * depth_));

		return serial_data_;
	}

	virtual bool deserializeData(const char * data, size_t length)
	{
		if (length < HEADER_SIZE)
			return false;

		width_ = ntohl(*(reinterpret_cast<const uint32_t *>(data)));
		height_ = ntohl(*(reinterpret_cast<const uint32_t *>(data + 4)));
		depth_ = ntohl(*(reinterpret_cast<const uint32_t *>(data + 8)));

		depth_offset_ = width_ * height_;

		data_len_ = HEADER_SIZE + (depth_offset_ * depth_);
		if (length != data_len_)
			return false;

		buffer_ = new T[depth_offset_ * depth_];
		memcpy(buffer_, (data + HEADER_SIZE), (depth_offset_ * depth_));

		return true;
	}

	virtual size_t getLength() const { return data_len_; }

protected:
	uint32_t index(uint32_t x, uint32_t y, uint32_t z) const { return x + (width_ * y) + (depth_offset_ * z); }

	virtual void print(std::ostream & os) const
	{
		os << "Matrix3D:" << std::endl << 
			"\tWidth=" << width_ << ", Height=" << height_ << ", Depth=" << depth_ << std::endl;
	}

private:
	// Non-copyable.
	Matrix3D(const Matrix3D &);
	void operator=(const Matrix3D &);
};

typedef Matrix3D<uint8_t> matrix3d_u8;
typedef std::shared_ptr<matrix3d_u8> matrix3d_u8_ptr;


class Player : public GameData
{
	static const size_t DATA_SIZE = 16;

	uint32_t player_id_;
	Vertex3DEx pos_;

	char serial_data_[DATA_SIZE];

public:
	Player() :
		player_id_(0), pos_(0, 0, 0)
	{}

	Player(uint32_t player_id, const Vertex3DEx pos) :
		player_id_(player_id), pos_(pos)
	{}

	uint32_t getPlayerId() const { return player_id_; }

	Vertex3DEx & getPosition() { return pos_; }
	const Vertex3DEx & getPosition() const { return pos_; }

	void clear() { player_id_ = 0; }
	bool isClear() const { return (player_id_ == 0); }

	virtual char * serializeData()
	{
		*(reinterpret_cast<uint32_t *>(serial_data_)) = htonl(player_id_);
		*(reinterpret_cast<uint32_t *>(serial_data_ + 4)) = htonl(pos_.x);
		*(reinterpret_cast<uint32_t *>(serial_data_ + 8)) = htonl(pos_.y);
		*(reinterpret_cast<uint32_t *>(serial_data_ + 12)) = htonl(pos_.z);
		return serial_data_;
	}

	virtual bool deserializeData(const char * data, size_t length)
	{
		if (length != DATA_SIZE)
			return false;

		player_id_ = ntohl(*(reinterpret_cast<const uint32_t *>(data)));
		pos_.x = ntohl(*(reinterpret_cast<const uint32_t *>(data + 4)));
		pos_.y = ntohl(*(reinterpret_cast<const uint32_t *>(data + 8)));
		pos_.z = ntohl(*(reinterpret_cast<const uint32_t *>(data + 12)));
		return true;
	}

	virtual size_t getLength() const { return DATA_SIZE; }

protected:
	virtual void print(std::ostream & os) const
	{
		os << "Player: ID=" << player_id_ << ", Pos=(" << pos_.x << ", " << pos_.y << ", " << pos_.z << ")" << std::endl;
	}
};

typedef std::shared_ptr<Player> player_ptr;
typedef std::map<uint32_t, player_ptr> player_map;


class MoveReq : public GameData
{
	static const size_t DATA_SIZE = 4;

	char serial_data_[DATA_SIZE];

public:
	enum eMoveDir
	{
		MD_NONE,
		MD_LEFT,
		MD_RIGHT,
		MD_UP,
		MD_DOWN,
		MD_BOTTOM,
		MD_TOP
	};

	// Constructor for message receiver.
	MoveReq() :
		move_dir_(MD_NONE)
	{}

	// Constructor for message sender.
	MoveReq(eMoveDir dir) :
		move_dir_(dir)
	{}

	eMoveDir getMoveDir() const { return move_dir_; }

	virtual char * serializeData()
	{
		*(reinterpret_cast<uint32_t *>(serial_data_)) = htonl(move_dir_);
		return serial_data_;
	}

	virtual bool deserializeData(const char * data, size_t length)
	{
		if (length != DATA_SIZE)
			return false;

		move_dir_ = static_cast<eMoveDir>(ntohl(*(reinterpret_cast<const uint32_t *>(data))));
		return true;
	}

	virtual size_t getLength() const { return DATA_SIZE; }

protected:
	virtual void print(std::ostream & os) const
	{
		os << "MoveReq: Dir=" << move_dir_ << std::endl;
	}

private:
	eMoveDir move_dir_;
};

typedef std::shared_ptr<MoveReq> move_req_ptr;

#endif // GAME_DATA_H
