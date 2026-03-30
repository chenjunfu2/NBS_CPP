#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <fstream>
#include <type_traits>
#include <filesystem>
#include <algorithm>

#include "NBS_Endian.hpp"

class NBS_File
{
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

	//extern type
	using FLOAT = double;
	using SBYTE = int8_t;

public:
	//备注，所有string都以cp1252编码
	struct Header
	{
		//版本信息
		BYTE version;					//NBS文件版本号（0-5，当前最新为5），决定后续字段的可用性

		//基础设置
		BYTE default_instruments;		//version > 0：默认乐器数量（通常为16），仅在版本1及以上有效
		SHORT song_length;				//version >= 3：歌曲总长度（最大tick值），仅在版本3及以上有效
		SHORT song_layers;				//歌曲总层数

		//歌曲元数据
		std::string song_name;			//歌曲名称
		std::string song_author;		//歌曲作者
		std::string original_author;	//原曲作者（改编时使用）
		std::string description;		//歌曲描述/备注

		//播放设置
		SHORT tempo;					//速度值，存储为整数，实际速度 = tempo / 100.0，默认1000对应10.0，读取后float ftempo = tempo/100.0，存储时tempo = (SHORT)(ftempo*100.0)
		BOOL auto_save;					//是否开启自动保存
		BYTE auto_save_duration;		//自动保存间隔（分钟）
		BYTE time_signature;			//拍号分母（如4表示4/4拍）

		//统计信息
		INT minutes_spent;				//编辑总耗时（分钟）
		INT left_clicks;				//鼠标左键点击次数（用于统计）
		INT right_clicks;				//鼠标右键点击次数（用于统计）
		INT blocks_added;				//添加的方块总数
		INT blocks_removed;				//移除的方块总数

		//来源信息
		std::string song_origin;		//歌曲来源标识（如"Noteblock Studio"等）

		//循环设置
		BOOL loop;						//version >= 4：是否启用循环播放
		BYTE max_loop_count;			//version >= 4：最大循环次数（0表示无限循环）
		SHORT loop_start;				//version >= 4：循环起始位置（tick）

	public:
		FLOAT Get_tempo_ActualValue(void)
		{
			return (FLOAT)tempo / (FLOAT)100.0;
		}

		void Set_tempo_ActualValue(FLOAT dNewActualVal)
		{
			tempo = (SHORT)(dNewActualVal * (FLOAT)100.0);
		}
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

	public:
		SBYTE Get_panning_ActualValue(void)
		{
			return (SBYTE)((SSHORT)panning - (SSHORT)100);
		}

		void Set_panning_ActualValue(SBYTE sbNewActualVal)
		{
			panning = (BYTE)((SSHORT)sbNewActualVal + (SSHORT)100);
		}
	};

	struct Layer
	{
		SHORT id;			//层索引 (0-based)
		std::string name;	//层名称
		BOOL lock;			//是否锁定 (v4+)
		BYTE volume;		//音量 (0-100)
		BYTE panning;		//声像 (-100 到 100, 0为中央, 默认100)，使用的时候需-100获得实际值，存储时无符号需要+100

	public:
		SBYTE Get_panning_ActualValue(void)
		{
			return (SBYTE)((SSHORT)panning - (SSHORT)100);
		}

		void Set_panning_ActualValue(SBYTE sbNewActualVal)
		{
			panning = (BYTE)((SSHORT)sbNewActualVal + (SSHORT)100);
		}
	};

	struct Instrument
	{
		INT id;				//乐器ID（在乐器列表中的索引，从0开始）
		std::string name;	//乐器名称
		std::string file;	//自定义音效文件路径（空字符串表示使用默认音色）
		BYTE pitch;			//音高偏移（默认45，对应C#，范围0-255）
		BOOL press_key;		//是否按下按键（默认true）
	};

	using ListNote = std::vector<Note>;
	using ListLayer = std::vector<Layer>;
	using ListInstrument = std::vector<Instrument>;

public:
	Header header;
	ListNote listNote;
	ListLayer listLayer;
	ListInstrument listInstrument;
};

class NBS_IO
{
	NBS_IO(void) = delete;
	~NBS_IO(void) = delete;

public:
	template<typename T>
	requires(NBS_File::IsNumeric_Type<T>)
		static bool ReadFromStream(T &tData, std::fstream &fileStream)
	{
		T tTmp{};
		if (!fileStream.read((char *)&tTmp, sizeof(T)))
		{
			return false;
		}

		tData = Endian_Helper::LittleToNativeAny(tTmp);
		return true;
	}

	static bool ReadFromStream(std::string &strData, std::fstream &fileStream)
	{
		NBS_File::INT u32Length = 0;
		if (!ReadFromStream<NBS_File::INT>(u32Length, fileStream))
		{
			return false;
		}
		strData.resize(u32Length);

		return (bool)fileStream.read(strData.data(), u32Length * sizeof(*strData.data()));
	}

	template<typename T>
	requires(NBS_File::IsNumeric_Type<T>)
	static bool WriteToStream(const T &tData, std::fstream &fileStream)
	{
		T tTmp = Endian_Helper::NativeToLittleAny(tData);
		return (bool)fileStream.write((char *)&tTmp, sizeof(T));
	}

	static bool WriteToStream(const std::string &strData, std::fstream &fileStream)
	{
		if (strData.size() > UINT32_MAX)
		{
			return false;
		}
		NBS_File::INT u32Length = strData.size();

		if (!WriteToStream<NBS_File::INT>(u32Length, fileStream))
		{
			return false;
		}

		return (bool)fileStream.write(strData.data(), u32Length * sizeof(*strData.data()));
	}

#define FAIL_RETURN(func)\
do\
{\
	if(!(func))\
	{\
		return false;\
	}\
}while(false)

public://READ
#define NORM_READ(field)\
FAIL_RETURN(ReadFromStream((field), fileStream))

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

	static bool ReadHeader(NBS_File::Header &header, std::fstream &fileStream)
	{
		NBS_File::SHORT song_length{};
		NORM_READ(song_length);

		NBS_File::BYTE version{};
		COND_READ(version, song_length == 0, (NBS_File::BYTE)0);//读入版本号，如果是新版本，则song_length长度为标记值0

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

		COND_READ(header.loop, version >= 4, (NBS_File::BOOL)false);
		COND_READ(header.max_loop_count, version >= 4, (NBS_File::BYTE)0);
		COND_READ(header.loop_start, version >= 4, (NBS_File::SHORT)0);
	
		return true;
	}

	static bool ReadNotes(const NBS_File::Header &header, NBS_File::ListNote &listNote, std::fstream &fileStream)
	{
		listNote.clear();

		NBS_File::INT cur_tick = -1;
		while (true)
		{
			NBS_File::SHORT tick{};
			NORM_READ(tick);
			if (tick == 0)
			{
				break;
			}
			cur_tick += tick;

			NBS_File::INT cur_layer = -1;
			while (true)
			{
				NBS_File::SHORT layer{};
				NORM_READ(layer);
				if (layer == 0)
				{
					break;
				}
				cur_layer += layer;

				NBS_File::Note note;
				note.tick = cur_tick;
				note.layer = cur_layer;
				NORM_READ(note.instrument);
				NORM_READ(note.key);
				COND_READ(note.velocity, header.version >= 4, (NBS_File::BYTE)100);
				COND_READ(note.panning, header.version >= 4, (NBS_File::BYTE)0);
				COND_READ(note.pitch, header.version >= 4, (NBS_File::SSHORT)0);

				listNote.push_back(std::move(note));
			}
		}

		return true;
	}

	static bool ReadLayers(const NBS_File::Header &header, NBS_File::ListLayer &listLayer, std::fstream &fileStream)
	{
		listLayer.clear();
		listLayer.reserve(header.song_layers);

		for (NBS_File::SHORT i = 0; i < header.song_layers; ++i)
		{
			NBS_File::Layer layer;
			layer.id = i;
			NORM_READ(layer.name);
			COND_READ(layer.lock, header.version >= 4, (NBS_File::BOOL)false);
			NORM_READ(layer.volume);
			COND_READ(layer.panning, header.version >= 2, (NBS_File::BYTE)0);

			listLayer.push_back(std::move(layer));
		}

		return true;
	}

	static bool ReadInstruments(const NBS_File::Header &header, NBS_File::ListInstrument &listInstrument, std::fstream &fileStream)
	{
		listInstrument.clear();

		NBS_File::BYTE u8InstrumentCount{};
		NORM_READ(u8InstrumentCount);

		for (NBS_File::BYTE i = 0; i < u8InstrumentCount; ++i)
		{
			NBS_File::Instrument instrument;
			instrument.id = i;
			NORM_READ(instrument.name);
			NORM_READ(instrument.file);
			NORM_READ(instrument.pitch);
			NORM_READ(instrument.press_key);

			listInstrument.push_back(std::move(instrument));
		}

		return true;
	}

	static bool ReadNBSFile(std::filesystem::path path, NBS_File &fileNBS)
	{
		std::fstream fRead(path, std::ios_base::in | std::ios_base::binary);
		if (!fRead.is_open())
		{
			return false;
		}

		return
			ReadHeader(fileNBS.header, fRead) &&
			ReadNotes(fileNBS.header, fileNBS.listNote, fRead) &&
			ReadLayers(fileNBS.header, fileNBS.listLayer, fRead) &&
			ReadInstruments(fileNBS.header, fileNBS.listInstrument, fRead);
	}

#undef NORM_READ
#undef COND_READ


public://WRITE
#define NORM_WRITE(field)\
FAIL_RETURN(WriteToStream((field), fileStream))

#define COND_WRITE(field,cond)\
do\
{\
	if((cond))\
	{\
		FAIL_RETURN(WriteToStream((field), fileStream));\
	}\
}while(false)

	static bool WriteHeader(const NBS_File::Header &header, std::fstream &fileStream)
	{
		// 写入歌曲长度标记
		if (header.version > 0)
		{
			NORM_WRITE((NBS_File::SHORT)0);//新版本格式标记为0
			NORM_WRITE(header.version);
			NORM_WRITE(header.default_instruments);
			COND_WRITE(header.song_length, header.version >= 3);//如果还大于3，那么存在
		}
		else// header.version == 0
		{
			NORM_WRITE(header.song_length);
		}

		//写入信息字段
		NORM_WRITE(header.song_layers);

		//写入字符串字段
		NORM_WRITE(header.song_name);
		NORM_WRITE(header.song_author);
		NORM_WRITE(header.original_author);
		NORM_WRITE(header.description);

		//写入数值字段
		NORM_WRITE(header.tempo);
		NORM_WRITE(header.auto_save);
		NORM_WRITE(header.auto_save_duration);
		NORM_WRITE(header.time_signature);

		NORM_WRITE(header.minutes_spent);
		NORM_WRITE(header.left_clicks);
		NORM_WRITE(header.right_clicks);
		NORM_WRITE(header.blocks_added);
		NORM_WRITE(header.blocks_removed);
		NORM_WRITE(header.song_origin);

		//写入v4及以上字段
		if (header.version >= 4)
		{
			NORM_WRITE(header.loop);
			NORM_WRITE(header.max_loop_count);
			NORM_WRITE(header.loop_start);
		}

		return true;
	}


	static bool WriteNotes(const NBS_File::Header &header, const NBS_File::ListNote &listNote, std::fstream &fileStream)
	{
		// 按tick排序音符，同tick按照layer排序
		std::vector<NBS_File::Note> sortedNoteList = listNote;
		size_t szSortedNoteSize = sortedNoteList.size();
		std::sort(sortedNoteList.begin(), sortedNoteList.end(),
			[](const NBS_File::Note &a, const NBS_File::Note &b)
			{
				return a.tick != b.tick ? a.tick < b.tick : a.layer < b.layer;
			}
		);
		
		//遍历tick
		size_t noteIndex = 0;
		NBS_File::INT cur_tick = -1;
		while (noteIndex < szSortedNoteSize)
		{
			//获取当前音符的tick
			NBS_File::INT tick = sortedNoteList[noteIndex].tick;

			//写入tick偏移
			NORM_WRITE((NBS_File::SHORT)(tick - cur_tick));
			cur_tick = tick;

			//写入当前tick的所有音符作为一个layer层
			NBS_File::INT cur_layer = -1;
			do
			{
				const auto &note = sortedNoteList[noteIndex];

				//写入layer偏移
				NORM_WRITE((NBS_File::SHORT)(note.layer - cur_layer));
				cur_layer = note.layer;

				//写入音符数据
				NORM_WRITE(note.instrument);
				NORM_WRITE(note.key);

				//v4数据
				if (header.version >= 4)
				{
					NORM_WRITE(note.velocity);
					NORM_WRITE(note.panning);
					NORM_WRITE(note.pitch);
				}

				++noteIndex;
			} while (noteIndex < szSortedNoteSize && sortedNoteList[noteIndex].tick == tick);//第一次肯定不会失败，使用do while

			//写入layer结束标记
			NORM_WRITE((NBS_File::SHORT)0);
		}
		//写入tick结束标记
		NORM_WRITE((NBS_File::SHORT)0);

		return true;
	}

	static bool WriteLayers(const NBS_File::Header &header, const NBS_File::ListLayer &listLayer, std::fstream &fileStream)
	{
		//写入层
		for (const auto &layer : listLayer)
		{
			NORM_WRITE(layer.name);
			COND_WRITE(layer.lock, header.version >= 4);
			NORM_WRITE(layer.volume);
			COND_WRITE(layer.panning, header.version >= 2);
		}

		return true;
	}

	static bool WriteInstruments(const NBS_File::Header &header, const NBS_File::ListInstrument &listInstrument, std::fstream &fileStream)
	{
		//写入乐器数量
		if (listInstrument.size() > UINT8_MAX)
		{
			return false;
		}
		NORM_WRITE((NBS_File::BYTE)listInstrument.size());

		//写入每个乐器
		for (const auto &instrument : listInstrument)
		{
			NORM_WRITE(instrument.name);
			NORM_WRITE(instrument.file);
			NORM_WRITE(instrument.pitch);
			NORM_WRITE(instrument.press_key);
		}

		return true;
	}

	static bool WriteNBSFile(std::filesystem::path path, const NBS_File &fileNBS)
	{
		std::fstream fWrite(path, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
		if (!fWrite.is_open())
		{
			return false;
		}

		return
			WriteHeader(fileNBS.header, fWrite) &&
			WriteNotes(fileNBS.header, fileNBS.listNote, fWrite) &&
			WriteLayers(fileNBS.header, fileNBS.listLayer, fWrite) &&
			WriteInstruments(fileNBS.header, fileNBS.listInstrument, fWrite);
	}

#undef NORM_WRITE
#undef COND_WRITE

public:
#undef FAIL_RETURN

};
