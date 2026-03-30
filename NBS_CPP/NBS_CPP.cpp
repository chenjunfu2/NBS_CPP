#include <stdint.h>
#include <string>
#include <fstream>
#include <type_traits>

#include "util/Endian_Helper.hpp"
#include <vector>

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
		BYTE panning;		//声像 (0-200, 0为最左, 200为最右, 100为中央, 默认100)，使用的时候需-100获得实际值，存储时无符号需要+100
		SSHORT pitch;		//音高微调 (-32768 to 32767, 默认0)
	};

	struct Layer
	{
		//标识信息
		SHORT id;			//层索引 (0-based)

		//层属性
		std::string name;	//层名称
		BOOL lock;			//是否锁定 (v4+)
		BYTE volume;		//音量 (0-100)
		BYTE panning;		//声像 (-100 到 100, 0为中央, 默认100)，使用的时候需-100获得实际值，存储时无符号需要+100
	};

	struct Instrument
	{
		INT id;				//乐器ID（在乐器列表中的索引，从0开始）
		std::string name;	//乐器名称
		std::string file;	//自定义音效文件路径（空字符串表示使用默认音色）
		BYTE pitch;			//音高偏移（默认45，对应C#，范围0-255）
		BOOL press_key;		//是否按下按键（默认true）
	};

public:
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

		COND_READ(header.loop, version >= 4, (BOOL)false);
		COND_READ(header.max_loop_count, version >= 4, (BYTE)0);
		COND_READ(header.loop_start, version >= 4, (SHORT)0);
	
		return true;
	}

	static bool ReadNotes(const Header &header, std::vector<Note> &listNote, std::fstream &fileStream)
	{
		listNote.clear();

		INT cur_tick = -1;
		while (true)
		{
			SHORT jump{};
			NORM_READ(jump);
			if (jump == 0)
			{
				break;
			}
			cur_tick += jump;

			INT cur_layer = -1;
			while (true)
			{
				SHORT jump{};
				NORM_READ(jump);
				if (jump == 0)
				{
					break;
				}
				cur_layer += jump;

				Note note;
				note.tick = cur_tick;
				note.layer = cur_layer;
				NORM_READ(note.instrument);
				NORM_READ(note.key);
				COND_READ(note.velocity, header.version >= 4, (BYTE)100);
				COND_READ(note.panning, header.version >= 4, (BYTE)0);
				COND_READ(note.pitch, header.version >= 4, (SSHORT)0);

				listNote.push_back(std::move(note));
			}
		}

		return true;
	}

	static bool ReadLayers(const Header &header, std::vector<Layer> &listLayer, std::fstream &fileStream)
	{
		listLayer.clear();
		listLayer.reserve(header.song_layers);

		for (SHORT i = 0; i < header.song_layers; ++i)
		{
			Layer layer;
			layer.id = i;
			NORM_READ(layer.name);
			COND_READ(layer.lock, header.version >= 4, (BOOL)false);
			NORM_READ(layer.volume);
			COND_READ(layer.panning, header.version >= 2, (BYTE)0);

			listLayer.push_back(std::move(layer));
		}

		return true;
	}

	static bool ReadInstruments(const Header &header, std::vector<Instruments> &listInstruments, std::fstream &fileStream)
	{
		listInstruments.clear();


		return true;
	}




};
























