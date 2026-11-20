/**
 * @file       filesys.cpp
 * @brief      Implements virtual file and directory operations for PushOS.
 * @author     ZirconiumTian
 */

#include "pushos/filesys.hpp"
#include "pushos/strextens.hpp"

#include <algorithm>
#include <stdexcept>

using namespace pushos::filesys;

bool Node::isDir() {
	return false;
}

File::File(std::string_view name) {
	if (strextens::stringHas(name, '/') or
		strextens::stringHas(name, '\\')) {
		throw std::runtime_error("Invalid file name!");
	}
	std::string fname = name.data();
	strextens::trim(fname);
	file_name = fname;
}

File& File::setOwner(std::string_view owner) {
	this->owner = owner;
	return *this;
}

File& File::setContent(std::string_view content) {
	file_content = content;
	return *this;
}

File& File::setExecutable(exeptr_t exe) {
	executable_part = exe;
	return *this;
}

File& File::setProtection(std::string_view procode) {
	protection = procode;
	return *this;
}

int64_t File::execute(const std::vector<std::string> &args) {
	return executable_part(args);
}

Dir::Dir(std::string_view name) {
	std::string fname = name.data();
	strextens::trim(fname);
	if (fname != "/") if (strextens::stringHas(name, '/') or
		strextens::stringHas(name, '\\')) {
		throw std::runtime_error("Invalid file name!");
	}
	file_name = fname;
}

Dir& Dir::setOwner(std::string_view owner) {
	this->owner = owner;
	return *this;
}

Dir& Dir::setProtection(std::string_view procode) {
	protection = procode;
	return *this;
}

int64_t Dir::find(std::string_view name) {
	for (size_t i = 0; i < sub_files.size(); i++) {
		if (sub_files[i]->file_name == name) {
			return static_cast<int64_t>(i);
		}
	}
	return -1;
}

Dir* Dir::getSubDir(std::string_view name) {
	if (name == "/") {
		Dir* root = this;
		while (root->parent_dir != nullptr) {
			root = root->parent_dir;
		}
		return root;
	}
	if (name == ".") {
		return this;
	}
	if (name == "..") {
		return parent_dir != nullptr ? parent_dir : this;
	}

	const auto index = find(name);
	return index == -1 ? nullptr : dynamic_cast<Dir*>(sub_files[static_cast<size_t>(index)]);
}

FsResult Dir::addDir(Dir *dir) {
	if (find(dir->file_name) != -1) {
		return FsResult::FileExists;
	}
	dir->parent_dir = this;
	sub_files.push_back(dir);
	return FsResult::Success;
}

FsResult Dir::addFile(File *file) {
	if (find(file->file_name) != -1) {
		return FsResult::FileExists;
	}
	sub_files.push_back(file);
	return FsResult::Success;
}

FsResult Dir::delDir(std::string_view name) {
	const auto index = find(name);
	if (index == -1) {
		return FsResult::FileNotFound;
	}
	const auto entry = sub_files.begin() + index;
	auto* directory = dynamic_cast<Dir*>(*entry);
	if (directory == nullptr) {
		return FsResult::FileTypeError;
	}
	sub_files.erase(entry);
	if (directory->parent_dir == this) {
		directory->parent_dir = nullptr;
	}
	return FsResult::Success;
}

FsResult Dir::delFile(std::string_view name) {
	const auto index = find(name);
	if (index == -1) {
		return FsResult::FileNotFound;
	}
	const auto entry = sub_files.begin() + index;
	if (dynamic_cast<File*>(*entry) == nullptr) {
		return FsResult::FileTypeError;
	}
	sub_files.erase(entry);
	return FsResult::Success;
}

FsResult Dir::changeTo(std::string_view single_path) {
	if (strextens::stringHas(single_path, '/') or
	    strextens::stringHas(single_path, '\\')) {
		throw std::runtime_error("Invalid file name!");
	}
	std::string path = single_path.data();
	strextens::trim(path);
	if (path == ".") {
		return FsResult::Success;
	}
	if (path == "..") {
		if (parent_dir) *this = *this->parent_dir;
		return FsResult::Success;
	}
	if (path == "/") {
		while (parent_dir) *this = *this->parent_dir;
		return FsResult::Success;
	}
	int64_t index = find(path);
	if (index != -1) {
		auto *target = dynamic_cast<Dir*>(sub_files[static_cast<size_t>(index)]);
		if (target == nullptr) {
			return FsResult::FileTypeError;
		}
		*this = *target;
		return FsResult::Success;
	}
	return FsResult::FileNotFound;
}

bool FileSystemCtl::createDir(std::string_view location, Dir &dir) {
	auto* parent = getDirByPath(location);
	return parent && parent->addDir(&dir) == FsResult::Success;
}

bool FileSystemCtl::createFile(std::string_view location, File &file) {
	auto* parent = getDirByPath(location);
	return parent && parent->addFile(&file) == FsResult::Success;
}

bool FileSystemCtl::gotoDir(std::string_view full_path) {
	auto* target = getDirByPath(full_path);
	if (target == nullptr) {
		return false;
	}
	std::vector<std::string> path;
	for (auto* dir = target; dir->parent_dir != nullptr; dir = dir->parent_dir) {
		path.push_back(dir->file_name);
	}
	path.emplace_back("/");
	std::reverse(path.begin(), path.end());
	current_path.swap(path);
	working_dir = target;
	return true;
}

bool FileSystemCtl::remove(std::string_view full_path) {
	if (getFileByPath(full_path) or
		getDirByPath(full_path)) {
		return true;
	}
	return false;
}

File* FileSystemCtl::getFileByPath(std::string_view full_path) {
	if (working_dir == nullptr or full_path.find('\0') != std::string_view::npos) {
		return nullptr;
	}
	const auto path_args = strextens::splitPath(std::string{full_path});
	if (path_args.empty()) return nullptr;
	const auto& file_name = path_args.back();
	if (file_name == "/" or file_name == "." or file_name == "..") {
		return nullptr;
	}
	Dir* dir = working_dir;
	for (size_t i = 0; i + 1 < path_args.size(); ++i) {
		dir = dir->getSubDir(path_args[i]);
		if (dir == nullptr) {
			return nullptr;
		}
	}
	const auto index = dir->find(file_name);
	return (index == -1) ?
	 nullptr : dynamic_cast<File*>(
	  dir->sub_files[static_cast<size_t>(index)]
	 );
}

Dir* FileSystemCtl::getDirByPath(std::string_view full_path) {
	if (working_dir == nullptr or full_path.find('\0') != std::string_view::npos) {
		return nullptr;
	}
	const auto path_args = strextens::splitPath(std::string{full_path});
	if (path_args.empty()) return nullptr;
	Dir* dir = working_dir;
	for (const auto& arg : path_args) {
		dir = dir->getSubDir(arg);
		if (dir == nullptr) {
			return nullptr;
		}
	}
	return dir;
}
