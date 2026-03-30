#include <stdint.h>
#include <string>
#include <fstream>
#include <type_traits>

#include "util/Endian_Helper.hpp"

class NBT_DataType
{
	NBT_DataType(void) = delete;
	~NBT_DataType(void) = delete;

public:
	using BYTE = uint8_t;
	using SHORT = uint16_t;
	using SSHORT = int16_t;
	using INT = uint32_t;

	using BOOL = BYTE;
	using STRLEN = INT;

	constexpr const static inline INT CURRENT_NBS_VERSION = 5;

	template<typename T>
	static constexpr bool IsNumeric_Type =
		std::is_same_v<T, BYTE> ||
		std::is_same_v<T, SHORT> ||
		std::is_same_v<T, SSHORT> ||
		std::is_same_v<T, INT> ||
		std::is_same_v<T, BOOL> ||
		std::is_same_v<T, STRLEN>;
public:
	template<typename T>
	requires(IsNumeric_Type<T>)
	static bool ReadFromStream(T& tData, std::fstream &fileStream)
	{
		T tTmp{};
		if (!fileStream.read((char *)tTmp, sizeof(T)))
		{
			return false;
		}

		tData = Endian_Helper::LittleToNativeAny(tTmp);
		return true;
	}

	static bool ReadFromStream(std::string &strData, std::fstream &fileStream)
	{
		INT u32Length = 0;
		if (!ReadFromStream<INT>(u32Length, fileStream))
		{
			return false;
		}

		strData.resize(u32Length);
		return (bool)fileStream.read(strData.data(), u32Length * sizeof(*strData.data()));
	}


public:
	//备注，所有string都以cp1252编码
	struct Header
	{
		//版本信息
		INT version;

		//基础设置
		BYTE default_instruments;//version > 0
		SHORT song_length;//version >= 3
		SHORT song_layers;

		//歌曲元数据
		std::string song_name;
		std::string song_author;
		std::string original_author;
		std::string description;

		//播放设置
		SHORT tempo;//存储为整数，读取后float ftempo = tempo/100.0，存储时tempo = (SHORT)(ftempo*100.0)
		BOOL auto_save;
		BYTE auto_save_duration;
		BYTE time_signature;

		//统计信息
		INT minutes_spent;
		INT left_clicks;
		INT right_clicks;
		INT blocks_added;
		INT blocks_removed;

		//来源信息
		std::string song_origin;

		//循环设置
		BOOL loop;//version >= 4
		BYTE max_loop_count;//version >= 4
		SHORT loop_start;//version >= 4
	};

	struct Note
	{
		//位置信息
		SHORT tick;			//音符所在的 tick 位置
		SHORT layer;		//所属层索引

		//音符参数
		BYTE instrument;	//乐器索引 (0-255)
		BYTE key;			//音符键位 (0-87, 对应钢琴键)
		BYTE velocity;		//力度 (0-100, 默认100)
		BYTE panning;		//声像 (0-200, 0为最左, 200为最右, 100为中央, 默认100)
		SSHORT pitch;		//音高微调 (-32768 to 32767, 默认0)
	};

#define FAIL_RETURN(func)\
do\
{\
	if(!(func))\
	{\
		return false;\
	}\
}while(false)

#define COND_READ(field,cond,defval)\
do\
{\
	if((cond))\
	{\
		FAIL_RETURN(ReadFromStream((field), fileStream));\
	}\
	else\
	{\
		(field) = (defval);\
	}\
}while(false)

#define NORM_READ(field)\
FAIL_RETURN(ReadFromStream((field), fileStream));


	static bool ReadHeader(Header &header, std::fstream &fileStream)
	{
		SHORT song_length{};
		NORM_READ(song_length);

		INT version{};
		COND_READ(version, song_length == 0, (INT)0);//读入版本号，如果是新版本，则song_length长度为标记值0


		//初始化文件头
		header.version = version;

		COND_READ(header.default_instruments, version > 0, 10);
		COND_READ(header.song_length, version >= 3, song_length);
		NORM_READ(header.song_layers);

		NORM_READ(header.song_name);
		NORM_READ(header.song_author);
		NORM_READ(header.original_author);
		NORM_READ(header.description);

		NORM_READ(header.tempo);
		NORM_READ(header.auto_save);
		NORM_READ(header.auto_save_duration);
		NORM_READ(header.time_signature);

		NORM_READ(header.minutes_spent);
		NORM_READ(header.left_clicks);
		NORM_READ(header.right_clicks);
		NORM_READ(header.blocks_added);
		NORM_READ(header.blocks_removed);

		NORM_READ(header.song_origin);

		COND_READ(header.loop, version >= 4, (BYTE)0);
		COND_READ(header.max_loop_count, version >= 4, (BYTE)0);
		COND_READ(header.loop_start, version >= 4, (BYTE)0);
	}

	static bool ReadNotes()
	{

	}








};
























