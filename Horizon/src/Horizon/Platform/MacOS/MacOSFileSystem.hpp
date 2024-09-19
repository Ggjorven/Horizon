#pragma once

#if defined(HZ_PLATFORM_MACOS)
#include "Horizon/IO/FileSystem.hpp"

#include <fstream>

namespace Hz::IO
{

	class IFile
	{
	public:
		IFile(const std::filesystem::path& file, FileMode mode = FileMode::In);
		~IFile();

		void Read(std::vector<char>& data, size_t size);
		std::vector<uint8_t> ReadBytes(size_t count);
		std::string ReadAll();

		bool Seek(size_t position);
		size_t Tell();

		size_t Size();

		bool Open() const;
		bool Good() const;
		bool EndOfFile() const;

	private:
		std::ifstream m_File;
	};

	class OFile
	{
	public:
		OFile(const std::filesystem::path& file, FileMode mode = FileMode::Out);
		~OFile();

		bool Write(const std::string& data);
		bool WriteBytes(const std::vector<char>& bytes);
		bool WriteBytes(const std::vector<uint8_t>& bytes);

		bool Seek(size_t position);
		size_t Tell();

		void Flush();

		bool Open() const;
		bool Good() const;
		bool EndOfFile() const;

		OFile& operator << (const std::string& data);

	private:
		std::ofstream m_File;
	};

}
#endif
