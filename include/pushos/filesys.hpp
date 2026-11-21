/**
 * @file       filesys.hpp
 * @brief      Declares the PushOS virtual file system types
 *             and controller.
 * @author     ZirconiumTian
 */

#pragma once

#include <stdint.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace pushos {
struct Device;
struct UserFsctl;

namespace filesys {

using args_t = std::vector<std::string>;
using exeptr_t = int64_t(*)(UserFsctl*, const args_t&);

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

	int64_t execute(UserFsctl*, const std::vector<std::string>&);
};

/**
 * @brief      Represents a directory.
 *             Stores child nodes and a pointer to the parent directory.
 *             Supports adding, removing, and finding child entries,
 *             including looking up subdirectories by name.
 */
struct Dir : Node, std::enable_shared_from_this<Dir> {
	Dir *parent_dir = nullptr;
	std::vector<Node*> sub_files;

	Dir() = default;
	Dir(std::string_view);
	~Dir();

	Dir& setOwner(std::string_view);
	Dir& setProtection(std::string_view);

	bool isDir() override {
		return true;
	}

	int64_t find(std::string_view);
	Dir* getSubDir(std::string_view);
	FsResult addDir(Dir*);
	FsResult addDir(std::shared_ptr<Dir>);
	FsResult addFile(File*);
	FsResult addFile(std::shared_ptr<File>);
	FsResult delDir(std::string_view);
	FsResult delFile(std::string_view);
	void clear();
	FsResult changeTo(std::string_view);

private:
	void releaseOwnedNode(const Node*);
	std::vector<std::shared_ptr<Node>> owned_nodes;
};

struct FileSystemCtl {
	Dir *working_dir;

	FileSystemCtl(Dir*);
	Dir* getWorkingDir() const;
	void setWorkingDir(Dir*);

	bool createDir(std::string_view, Dir*);
	bool createFile(std::string_view, File*);
	bool remove(std::string_view);
	File* getFileByPath(std::string_view);
	Dir* getDirByPath(std::string_view);

private:
	Dir *root_dir = nullptr;
	Dir *tracked_dir = nullptr;
	std::weak_ptr<Dir> working_dir_owner;
	bool managed_working_dir = false;
};

} // namespace filesys
} // namespace pushos
