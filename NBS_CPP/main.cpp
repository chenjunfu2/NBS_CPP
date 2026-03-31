#include <nbs_cpp/NBS_CPP.hpp>

#include <stdio.h>
#include <format>
#include <string>
#include <stdlib.h>
#include <cstdio>

#if defined(_MSC_VER) && _MSC_VER < 1935 //旧版本MSVC 1935-不支持
#define format_string _Fmt_string //使用MSVC库内部类型
#endif

template<typename... Args>
void print(std::format_string<Args...> fmt, Args&&... args)
{
	std::string output = std::format(fmt, std::forward<Args>(args)...);
	fwrite(output.data(), 1, output.size(), stdout);
}


void PrintHeaderInfo(const NBS_File::Header &header)
{
	print("========== Header Info ==========\n");
	print("Version: {}\n", header.version);
	print("Default Instruments: {}\n", header.default_instruments);
	print("Song Length (ticks): {}\n", header.song_length);
	print("Song Layers: {}\n", header.song_layers);
	print("Song Name: {}\n", header.song_name);
	print("Song Author: {}\n", header.song_author);
	print("Original Author: {}\n", header.original_author);
	print("Description: {}\n", header.description);
	print("Tempo: {:.2f} ({}/100)\n", header.Get_tempo_ActualValue(), header.tempo);
	print("Auto Save: {}\n", (header.auto_save ? "Yes" : "No"));
	print("Auto Save Duration: {} min\n", header.auto_save_duration);
	print("Time Signature: 4/{}\n", header.time_signature);
	print("Minutes Spent: {}\n", header.minutes_spent);
	print("Left Clicks: {}\n", header.left_clicks);
	print("Right Clicks: {}\n", header.right_clicks);
	print("Blocks Added: {}\n", header.blocks_added);
	print("Blocks Removed: {}\n", header.blocks_removed);
	print("Song Origin: {}\n", header.song_origin);
	print("Loop: {}\n", (header.loop ? "Yes" : "No"));
	print("Max Loop Count: {}\n", header.max_loop_count);
	print("Loop Start: {}\n", header.loop_start);
	print("=================================\n");
}

void PrintNotesInfo(const NBS_File::ListNote &notes)
{
	print("========== Notes Info ==========\n");
	print("Total Notes: {}\n", notes.size());

	//显示前10个音符作为示例
	size_t displayCount = std::min(notes.size(), size_t(10));
	for (size_t i = 0; i < displayCount; ++i)
	{
		const auto &note = notes[i];
		print("  Note {}: tick={}, layer={}, instrument={}, key={}, velocity={}, panning={}, pitch={}\n",
			i + 1, note.tick, note.layer, note.instrument, note.key,
			note.velocity, note.Get_panning_ActualValue(), note.pitch);
	}

	if (notes.size() > displayCount)
	{
		print("  ... and {} more notes\n", (notes.size() - displayCount));
	}
	print("=================================\n");
}

void PrintLayersInfo(const NBS_File::ListLayer &layers)
{
	print("========== Layers Info ==========\n");
	print("Total Layers: {}\n", layers.size());

	//显示前10层作为示例
	size_t displayCount = std::min(layers.size(), size_t(10));
	for (size_t i = 0; i < displayCount; ++i)
	{
		const auto &layer = layers[i];
		print("  Layer {}: name=\"{}\", lock={}, volume={}, panning={}\n",
			layer.id, layer.name, (layer.lock ? "Yes" : "No"),
			layer.volume, layer.Get_panning_ActualValue());
	}

	if (layers.size() > displayCount)
	{
		print("  ... and {} more layers\n", (layers.size() - displayCount));
	}
	print("=================================\n");
}

void PrintInstrumentsInfo(const NBS_File::ListInstrument &instruments)
{
	print("========== Instruments Info ==========\n");
	print("Total Instruments: {}\n", instruments.size());

	//显示前10种作为示例
	size_t displayCount = std::min(instruments.size(), size_t(10));
	for (size_t i = 0; i < displayCount; ++i)
	{
		const auto &instr = instruments[i];
		print("  Instrument {}: name=\"{}\", file=\"{}\", pitch={}, press_key={}\n",
			instr.id, instr.name, instr.file, instr.pitch, (instr.press_key ? "Yes" : "No"));
	}

	if (instruments.size() > displayCount)
	{
		print("  ... and {} more instruments\n", (instruments.size() - displayCount));
	}
	print("======================================\n");
}

int main(int argc, char *argv[])
{
	// 检查命令行参数
	if (argc < 2)
	{
		print("Usage: {} <input.nbs>\n", argv[0]);
		return 1;
	}

	std::filesystem::path inputPath = argv[1];

	// 检查输入文件是否存在
	if (!std::filesystem::exists(inputPath))
	{
		print("Error: Input file does not exist: {}\n", inputPath.string());
		return 1;
	}

	// 读取 NBS 文件
	NBS_File nbsFile;
	print("Reading file: {}\n", inputPath.string());

	if (!NBS_IO::ReadNBS(nbsFile, inputPath))
	{
		print("Error: Failed to read NBS file!\n");
		return 1;
	}

	print("Successfully read NBS file!\n\n");

	// 输出文件信息
	PrintHeaderInfo(nbsFile.header);
	PrintNotesInfo(nbsFile.listNote);
	PrintLayersInfo(nbsFile.listLayer);
	PrintInstrumentsInfo(nbsFile.listInstrument);

	system("pause");

	// 构造输出文件路径
	std::filesystem::path outputPath = inputPath;
	std::string stem = outputPath.stem().string();
	outputPath.replace_filename(stem + "(2)" + outputPath.extension().string());

	print("\nWriting file to: {}\n", outputPath.string());

	// 写回 NBS 文件
	if (!NBS_IO::WriteNBS(nbsFile, outputPath))
	{
		print("Error: Failed to write NBS file!\n");
		return 1;
	}

	print("Successfully wrote NBS file!\n");

	return 0;
}