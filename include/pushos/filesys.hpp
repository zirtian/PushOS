/**
 * @file       filesys.hpp
 * @brief      Declares the PushOS virtual file system types
 *             and controller.
 * @author     ZirconiumTian
 */

#pragma once

#include <stdint.h>
#include <string>
#include <string_view>
#include <vector>

namespace pushos {
namespace filesys {

/** @brief  */
using exeptr_t = int64_t(*)(const std::vector<std::string>&);

enum FsResult : int8_t {
	Success,
	FileNotFound,
	FileExists,
	FileTypeError,
	PermissionDenial,
	Unknown
};

/**
 * @brief      Represents a basic file system node.
 *             Stores its name, owner, creation and modification times,
 *             and protection code.
 */
struct Node {
	std::string file_name;
	std::string owner;
	std::string last_write;
	std::string create_time;
	std::string protection;
	virtual bool isDir();
};

/**
 * @brief      Represents a file.
 *             Stores text content and an executable callback pointer.
 */
struct File : Node {
	std::string file_content;
	exeptr_t executable_part = nullptr;

	File() = default;
	File(std::string_view);
	File& setOwner(std::string_view);
	File& setContent(std::string_view);
	File& setExecutable(exeptr_t);
	File& setProtection(std::string_view);

	bool isDir() override {
		return false;
	}

	int64_t execute(const std::vector<std::string>&);
};

/**
 * @brief      Represents a directory.
 *             Stores child nodes and a pointer to the parent directory.
 *             Supports adding, removing, and finding child entries,
 *             including looking up subdirectories by name.
 */
struct Dir : Node {
	Dir *parent_dir = nullptr;
	std::vector<Node*> sub_files;

	Dir() = default;
	Dir(std::string_view);

	Dir& setOwner(std::string_view);
	Dir& setProtection(std::string_view);

	bool isDir() override {
		return true;
	}

	int64_t find(std::string_view);
	Dir* getSubDir(std::string_view);
	FsResult addDir(Dir*);
	FsResult addFile(File*);
	FsResult delDir(std::string_view);
	FsResult delFile(std::string_view);
	FsResult changeTo(std::string_view);
};

struct FileSystemCtl {
	Dir *working_dir;
	std::vector<std::string> current_path;

	FileSystemCtl(Dir *dir) : working_dir(dir) {}

	bool createDir(std::string_view, Dir&);
	bool createFile(std::string_view, File&);
	bool gotoDir(std::string_view);
	bool remove(std::string_view);
	File* getFileByPath(std::string_view);
	Dir* getDirByPath(std::string_view);
};

} // namespace filesys
} // namespace pushos
