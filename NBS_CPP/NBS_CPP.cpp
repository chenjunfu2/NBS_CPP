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
	static bool ReadNumeric(T& tData, std::fstream &fileStream)
	{
		T tTmp{};
		if (!fileStream.read((char *)tTmp, sizeof(T)))
		{
			return false;
		}

		tData = Endian_Helper::LittleToNativeAny(tTmp);
		return true;
	}

	static bool ReadString(std::string &strData, std::fstream &fileStream)
	{
		INT u32Length = 0;
		if (!ReadNumeric(u32Length, fileStream))
		{
			return false;
		}

		strData.resize(u32Length);
		return (bool)fileStream.read(strData.data(), u32Length * sizeof(*strData.data()));
	}


public:
	struct Header
	{
		//版本信息
		INT version = CURRENT_NBS_VERSION;

		//基础设置
		BYTE default_instruments = 16;//version > 0
		SHORT song_length = 0;//version >= 3
		SHORT song_layers = 0;

		//歌曲元数据
		std::string song_name = {};
		std::string song_author = {};
		std::string original_author = {};
		std::string description = {};

		//播放设置
		SHORT tempo = 1000;//存储为整数，读取后float ftempo = tempo/100.0，存储时tempo = (SHORT)(ftempo*100.0)
		BOOL auto_save = false;
		BYTE auto_save_duration = 10;
		BYTE time_signature = 4;

		//统计信息
		INT minutes_spent = 0;
		INT left_clicks = 0;
		INT right_clicks = 0;
		INT blocks_added = 0;
		INT blocks_removed = 0;

		//来源信息
		std::string song_origin = {};

		//循环设置
		BOOL loop = false;//version >= 4
		BYTE max_loop_count = 0;//version >= 4
		SHORT loop_start = 0;//version >= 4
	};

	static bool ReadHeader(Header &header, std::fstream &fileStream)
	{
		SHORT song_length{};
		if (!ReadNumeric(song_length, fileStream))
		{
			return false;
		}

		if (song_length == 0)//新版本，长度为标记值
		{
			if (!ReadNumeric(header.version, fileStream))//读入版本号
			{
				return false;
			}
		}
		else
		{
			header.version = 0;//否则为0
		}


		ReadNumeric(header.default_instruments, fileStream);




	}










};
























