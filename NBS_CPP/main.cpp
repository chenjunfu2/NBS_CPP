#include <nbs_cpp/NBS_CPP.hpp>

#include <iostream>
#include <iomanip>
#include <string>

void PrintHeaderInfo(const NBS_File::Header &header)
{
	std::cout << "========== Header Info ==========" << std::endl;
	std::cout << "Version: " << (int)header.version << std::endl;
	std::cout << "Default Instruments: " << (int)header.default_instruments << std::endl;
	std::cout << "Song Length (ticks): " << header.song_length << std::endl;
	std::cout << "Song Layers: " << header.song_layers << std::endl;
	std::cout << "Song Name: " << header.song_name << std::endl;
	std::cout << "Song Author: " << header.song_author << std::endl;
	std::cout << "Original Author: " << header.original_author << std::endl;
	std::cout << "Description: " << header.description << std::endl;
	std::cout << "Tempo: " << std::fixed << std::setprecision(2) << header.Get_tempo_ActualValue() << " (" << header.tempo << "/100)" << std::endl;
	std::cout << "Auto Save: " << (header.auto_save ? "Yes" : "No") << std::endl;
	std::cout << "Auto Save Duration: " << (int)header.auto_save_duration << " min" << std::endl;
	std::cout << "Time Signature: 4/" << (int)header.time_signature << std::endl;
	std::cout << "Minutes Spent: " << header.minutes_spent << std::endl;
	std::cout << "Left Clicks: " << header.left_clicks << std::endl;
	std::cout << "Right Clicks: " << header.right_clicks << std::endl;
	std::cout << "Blocks Added: " << header.blocks_added << std::endl;
	std::cout << "Blocks Removed: " << header.blocks_removed << std::endl;
	std::cout << "Song Origin: " << header.song_origin << std::endl;
	std::cout << "Loop: " << (header.loop ? "Yes" : "No") << std::endl;
	std::cout << "Max Loop Count: " << (int)header.max_loop_count << std::endl;
	std::cout << "Loop Start: " << header.loop_start << std::endl;
	std::cout << "=================================" << std::endl;
}

void PrintNotesInfo(const NBS_File::ListNote &notes)
{
	std::cout << "========== Notes Info ==========" << std::endl;
	std::cout << "Total Notes: " << notes.size() << std::endl;

	//显示前10个音符作为示例
	size_t displayCount = std::min(notes.size(), size_t(10));
	for (size_t i = 0; i < displayCount; ++i)
	{
		const auto &note = notes[i];
		std::cout << "  Note " << i + 1 << ": tick=" << note.tick
			<< ", layer=" << note.layer
			<< ", instrument=" << (int)note.instrument
			<< ", key=" << (int)note.key
			<< ", velocity=" << (int)note.velocity
			<< ", panning=" << (int)note.Get_panning_ActualValue()
			<< ", pitch=" << note.pitch << std::endl;
	}

	if (notes.size() > displayCount)
	{
		std::cout << "  ... and " << (notes.size() - displayCount) << " more notes" << std::endl;
	}
	std::cout << "=================================" << std::endl;
}

void PrintLayersInfo(const NBS_File::ListLayer &layers)
{
	std::cout << "========== Layers Info ==========" << std::endl;
	std::cout << "Total Layers: " << layers.size() << std::endl;

	//显示前10层作为示例
	size_t displayCount = std::min(layers.size(), size_t(10));
	for (size_t i = 0; i < displayCount; ++i)
	{
		const auto &layer = layers[i];
		std::cout << "  Layer " << layer.id
			<< ": name=\"" << layer.name << "\""
			<< ", lock=" << (layer.lock ? "Yes" : "No")
			<< ", volume=" << (int)layer.volume
			<< ", panning=" << (int)layer.Get_panning_ActualValue() << std::endl;
	}

	if (layers.size() > displayCount)
	{
		std::cout << "  ... and " << (layers.size() - displayCount) << " more layers" << std::endl;
	}
	std::cout << "=================================" << std::endl;
}

void PrintInstrumentsInfo(const NBS_File::ListInstrument &instruments)
{
	std::cout << "========== Instruments Info ==========" << std::endl;
	std::cout << "Total Instruments: " << instruments.size() << std::endl;

	//显示前10种作为示例
	size_t displayCount = std::min(instruments.size(), size_t(10));
	for (size_t i = 0; i < displayCount; ++i)
	{
		const auto &instr = instruments[i];
		std::cout << "  Instrument " << instr.id
			<< ": name=\"" << instr.name << "\""
			<< ", file=\"" << instr.file << "\""
			<< ", pitch=" << (int)instr.pitch
			<< ", press_key=" << (instr.press_key ? "Yes" : "No") << std::endl;
	}

	if (instruments.size() > displayCount)
	{
		std::cout << "  ... and " << (instruments.size() - displayCount) << " more instruments" << std::endl;
	}
	std::cout << "======================================" << std::endl;
}

int main(int argc, char *argv[])
{
	// 检查命令行参数
	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " <input.nbs>" << std::endl;
		return 1;
	}

	std::filesystem::path inputPath = argv[1];

	// 检查输入文件是否存在
	if (!std::filesystem::exists(inputPath))
	{
		std::cout << "Error: Input file does not exist: " << inputPath << std::endl;
		return 1;
	}

	// 读取 NBS 文件
	NBS_File nbsFile;
	std::cout << "Reading file: " << inputPath << std::endl;

	if (!NBS_IO::ReadNBS(nbsFile, inputPath))
	{
		std::cout << "Error: Failed to read NBS file!" << std::endl;
		return 1;
	}

	std::cout << "Successfully read NBS file!" << std::endl << std::endl;

	// 输出文件信息
	PrintHeaderInfo(nbsFile.header);
	PrintNotesInfo(nbsFile.listNote);
	PrintLayersInfo(nbsFile.listLayer);
	PrintInstrumentsInfo(nbsFile.listInstrument);

	return 0;

	// 构造输出文件路径
	std::filesystem::path outputPath = inputPath;
	std::string stem = outputPath.stem().string();
	outputPath.replace_filename(stem + "(2)" + outputPath.extension().string());

	std::cout << std::endl << "Writing file to: " << outputPath << std::endl;

	// 写回 NBS 文件
	if (!NBS_IO::WriteNBS(nbsFile, outputPath))
	{
		std::cout << "Error: Failed to write NBS file!" << std::endl;
		return 1;
	}

	std::cout << "Successfully wrote NBS file!" << std::endl;

	return 0;
}