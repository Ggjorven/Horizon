#include "hzpch.h"

#if defined(HZ_PLATFORM_ANDROID)
#include "Horizon/Core/Logging.hpp"

#include "Horizon/IO/FileSystem.hpp"

namespace Hz::IO
{

	///////////////////////////////////////////////////////////
	// General functions
	///////////////////////////////////////////////////////////
	bool FileSystem::Exists(const std::filesystem::path& path)
	{
		JNIEnv* env = GetJNIEnv();

		jclass fileClass = env->FindClass("java/io/File");
		jmethodID existsMethod = env->GetMethodID(fileClass, "exists", "()Z");

		jstring jPath = env->NewStringUTF(path.string().c_str());
		jobject fileObject = env->NewObject(fileClass, env->GetMethodID(fileClass, "<init>", "(Ljava/lang/String;)V"), jPath);

		jboolean exists = env->CallBooleanMethod(fileObject, existsMethod);
		env->DeleteLocalRef(fileClass);
		env->DeleteLocalRef(jPath);
		env->DeleteLocalRef(fileObject);

		return exists;
	}

	bool FileSystem::CreateDirectory(const std::filesystem::path& path)
	{
		JNIEnv* env = GetJNIEnv();

		jclass fileClass = env->FindClass("java/io/File");
		jmethodID mkdirMethod = env->GetMethodID(fileClass, "mkdir", "()Z");

		jstring jPath = env->NewStringUTF(path.string().c_str());
		jobject fileObject = env->NewObject(fileClass, env->GetMethodID(fileClass, "<init>", "(Ljava/lang/String;)V"), jPath);

		jboolean created = env->CallBooleanMethod(fileObject, mkdirMethod);
		env->DeleteLocalRef(fileClass);
		env->DeleteLocalRef(jPath);
		env->DeleteLocalRef(fileObject);

		return created;
	}

	bool FileSystem::Remove(const std::filesystem::path& path)
	{
		JNIEnv* env = GetJNIEnv();

		jclass fileClass = env->FindClass("java/io/File");
		jmethodID deleteMethod = env->GetMethodID(fileClass, "delete", "()Z");

		jstring jPath = env->NewStringUTF(path.string().c_str());
		jobject fileObject = env->NewObject(fileClass, env->GetMethodID(fileClass, "<init>", "(Ljava/lang/String;)V"), jPath);

		jboolean deleted = env->CallBooleanMethod(fileObject, deleteMethod);
		env->DeleteLocalRef(fileClass);
		env->DeleteLocalRef(jPath);
		env->DeleteLocalRef(fileObject);

		return deleted;
	}

	void FileSystem::Rename(const std::filesystem::path& oldName, const std::filesystem::path& newName)
	{
		JNIEnv* env = GetJNIEnv();

		jclass fileClass = env->FindClass("java/io/File");
		jmethodID renameToMethod = env->GetMethodID(fileClass, "renameTo", "(Ljava/io/File;)Z");

		jstring jOldName = env->NewStringUTF(oldName.string().c_str());
		jobject oldFile = env->NewObject(fileClass, env->GetMethodID(fileClass, "<init>", "(Ljava/lang/String;)V"), jOldName);

		jstring jNewName = env->NewStringUTF(newName.string().c_str());
		jobject newFile = env->NewObject(fileClass, env->GetMethodID(fileClass, "<init>", "(Ljava/lang/String;)V"), jNewName);

		env->CallBooleanMethod(oldFile, renameToMethod, newFile);
		env->DeleteLocalRef(fileClass);
		env->DeleteLocalRef(jOldName);
		env->DeleteLocalRef(jNewName);
		env->DeleteLocalRef(oldFile);
		env->DeleteLocalRef(newFile);
	}

	std::filesystem::path FileSystem::Absolute(const std::filesystem::path& path)
	{
		// On Android, this is often handled natively. If needed, convert paths to absolute using Java methods.
		return path; // Placeholder
	}

	std::filesystem::path FileSystem::Relative(const std::filesystem::path& path, const std::filesystem::path& base)
	{
		// On Android, this is often handled natively. If needed, calculate relative paths using Java methods.
		return path; // Placeholder
	}

	std::filesystem::path FileSystem::CurrentPath()
	{
		JNIEnv* env = GetJNIEnv();

		jclass contextClass = env->FindClass("android/content/Context");
		jmethodID getFilesDirMethod = env->GetMethodID(contextClass, "getFilesDir", "()Ljava/io/File;");
		jobject filesDir = env->CallObjectMethod(env->GetApplicationContext(), getFilesDirMethod);

		jclass fileClass = env->FindClass("java/io/File");
		jmethodID getAbsolutePathMethod = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
		jstring jPath = (jstring)env->CallObjectMethod(filesDir, getAbsolutePathMethod);

		const char* cPath = env->GetStringUTFChars(jPath, nullptr);
		std::filesystem::path currentPath(cPath);
		env->ReleaseStringUTFChars(jPath, cPath);
		env->DeleteLocalRef(fileClass);
		env->DeleteLocalRef(contextClass);
		env->DeleteLocalRef(filesDir);
		env->DeleteLocalRef(jPath);

		return currentPath;
	}

	///////////////////////////////////////////////////////////
	// Input file
	///////////////////////////////////////////////////////////
	IFile::IFile(const std::filesystem::path& file, FileMode mode)
		: m_File(file, (std::ios_base::openmode)mode)
	{
		HZ_ASSERT((Open() && Good()), "Failed to open file: '{0}'", file.string());
	}

	IFile::~IFile()
	{
		m_File.close();
	}

	void IFile::Read(std::vector<char>& data, size_t size)
	{
		Seek(0);
		m_File.read(data.data(), size);
	}

	std::vector<uint8_t> IFile::ReadBytes(size_t count)
	{
		std::vector<uint8_t> buffer(count);

		if (Open() && count > 0)
		{
			m_File.read(reinterpret_cast<char*>(buffer.data()), count);
			buffer.resize(m_File.gcount()); // Resize to the actual number of bytes read
		}

		return buffer;
	}

	std::string IFile::ReadAll()
	{
		HZ_ASSERT((Open()), "File is not open.");
		std::string content((std::istreambuf_iterator<char>(m_File)), std::istreambuf_iterator<char>());

		return content;
	}

	bool IFile::Seek(size_t position)
	{
		if (!Open())
			return false;

		m_File.seekg(position);
		return Good();
	}

	size_t IFile::Tell()
	{
		HZ_ASSERT((Open()), "File is not open.");
		return static_cast<size_t>(m_File.tellg());
	}

	size_t IFile::Size()
	{
		HZ_ASSERT((Open()), "File is not open.");

		auto currentPos = m_File.tellg();
		m_File.seekg(0, std::ios::end);
		size_t fileSize = static_cast<size_t>(m_File.tellg());
		m_File.seekg(currentPos); // Return to the original position

		return fileSize;
	}

	bool IFile::Open() const
	{
		return m_File.is_open();
	}

	bool IFile::Good() const
	{
		return m_File.good();
	}

	bool IFile::EndOfFile() const
	{
		return m_File.eof();
	}

	///////////////////////////////////////////////////////////
	// Output file
	///////////////////////////////////////////////////////////
	OFile::OFile(const std::filesystem::path& file, FileMode mode)
		: m_File(file, (std::ios_base::openmode)mode)
	{
		HZ_ASSERT((Open() && Good()), "Failed to open file: '{0}'", file.string());
	}

	OFile::~OFile()
	{
		m_File.close();
	}

	bool OFile::Write(const std::string& data)
	{
		HZ_ASSERT((Open()), "File is not open.");

		m_File.write(data.c_str(), data.size());
		return Good();
	}

	bool OFile::WriteBytes(const std::vector<char>& bytes)
	{
		HZ_ASSERT((Open()), "File is not open.");

		m_File.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
		return Good();
	}

	bool OFile::WriteBytes(const std::vector<uint8_t>& bytes)
	{
		HZ_ASSERT((Open()), "File is not open.");

		m_File.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
		return Good();
	}

	bool OFile::Seek(size_t position)
	{
		HZ_ASSERT((Open()), "File is not open.");

		m_File.seekp(position);
		return Good();
	}

	size_t OFile::Tell()
	{
		HZ_ASSERT((Open()), "File is not open.");

		return static_cast<size_t>(m_File.tellp());
	}

	void OFile::Flush()
	{
		if (Open())
			m_File.flush();
	}

	bool OFile::Open() const
	{
		return m_File.is_open();
	}

	bool OFile::Good() const
	{
		return m_File.good();
	}

	bool OFile::EndOfFile() const
	{
		return m_File.eof();
	}

	OFile& OFile::operator << (const std::string& data)
	{
		Write(data);
		return *this;
	}

}
#endif
