#include "pushos/filesys.hpp"
#include "pushos/strextens.hpp"

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

int64_t File::execute(UserFsctl* dev, const std::vector<std::string> &args) {
	return executable_part(dev, args);
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

Dir::~Dir() {
	for (const auto &node : owned_nodes) {
		auto *dir = dynamic_cast<Dir*>(node.get());
		if (dir != nullptr and dir->parent_dir == this) dir->parent_dir = nullptr;
	}
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
	for (Dir *parent = this; parent != nullptr; parent = parent->parent_dir) {
		if (parent == dir) return FsResult::Unknown;
	}
	if (dir->parent_dir != nullptr and dir->parent_dir != this) {
		return FsResult::Unknown;
	}
	sub_files.push_back(dir);
	dir->parent_dir = this;
	return FsResult::Success;
}

FsResult Dir::addDir(std::shared_ptr<Dir> dir) {
	owned_nodes.push_back(dir);
	FsResult result;
	try {
		result = addDir(dir.get());
	} catch (...) {
		owned_nodes.pop_back();
		throw;
	}
	if (result != FsResult::Success) owned_nodes.pop_back();
	return result;
}

FsResult Dir::addFile(File *file) {
	if (find(file->file_name) != -1) {
		return FsResult::FileExists;
	}
	sub_files.push_back(file);
	return FsResult::Success;
}

FsResult Dir::addFile(std::shared_ptr<File> file) {
	owned_nodes.push_back(file);
	FsResult result;
	try {
		result = addFile(file.get());
	} catch (...) {
		owned_nodes.pop_back();
		throw;
	}
	if (result != FsResult::Success) owned_nodes.pop_back();
	return result;
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
	releaseOwnedNode(directory);
	return FsResult::Success;
}

FsResult Dir::delFile(std::string_view name) {
	const auto index = find(name);
	if (index == -1) {
		return FsResult::FileNotFound;
	}
	const auto entry = sub_files.begin() + index;
	auto *file = dynamic_cast<File*>(*entry);
	if (file == nullptr) {
		return FsResult::FileTypeError;
	}
	sub_files.erase(entry);
	releaseOwnedNode(file);
	return FsResult::Success;
}

void Dir::releaseOwnedNode(const Node *node) {
	for (auto owned = owned_nodes.begin(); owned != owned_nodes.end(); ++owned) {
		if (owned->get() == node) {
			owned_nodes.erase(owned);
			return;
		}
	}
}

void Dir::clear() {
	for (auto *node : sub_files) {
		auto *dir = dynamic_cast<Dir*>(node);
		if (dir != nullptr and dir->parent_dir == this) {
			dir->clear();
			dir->parent_dir = nullptr;
		}
	}
	sub_files.clear();
	owned_nodes.clear();
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
		if (parent_dir) {
			Dir contents = *parent_dir;
			contents.releaseOwnedNode(this);
			*this = contents;
		}
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
		const Dir contents = *target;
		*this = contents;
		return FsResult::Success;
	}
	return FsResult::FileNotFound;
}

FileSystemCtl::FileSystemCtl(Dir *dir) {
	setWorkingDir(dir);
}

void FileSystemCtl::setWorkingDir(Dir *dir) {
	working_dir = dir;
	tracked_dir = dir;
	root_dir = dir != nullptr ? dir->getSubDir("/") : nullptr;
	working_dir_owner = dir != nullptr ? dir->weak_from_this() : std::weak_ptr<Dir>{};
	managed_working_dir = !working_dir_owner.expired();
}

Dir* FileSystemCtl::getWorkingDir() const {
	if (working_dir != tracked_dir or working_dir == nullptr) return working_dir;
	if (managed_working_dir and working_dir_owner.expired()) return root_dir;
	if (working_dir != root_dir and working_dir->parent_dir == nullptr) return root_dir;
	return working_dir;
}

bool FileSystemCtl::createDir(std::string_view location, Dir *dir) {
	auto* parent = getDirByPath(location);
	return parent && parent->addDir(dir) == FsResult::Success;
}

bool FileSystemCtl::createFile(std::string_view location, File *file) {
	auto* parent = getDirByPath(location);
	return parent && parent->addFile(file) == FsResult::Success;
}

bool FileSystemCtl::remove(std::string_view full_path) {
	if (getFileByPath(full_path) or
		getDirByPath(full_path)) {
		return true;
	}
	return false;
}

File* FileSystemCtl::getFileByPath(std::string_view full_path) {
	Dir* dir = getWorkingDir();
	if (dir == nullptr or full_path.find('\0') != std::string_view::npos) {
		return nullptr;
	}
	const auto path_args = strextens::splitPath(std::string{full_path});
	if (path_args.empty()) return nullptr;
	const auto& file_name = path_args.back();
	if (file_name == "/" or file_name == "." or file_name == "..") {
		return nullptr;
	}
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
	Dir* dir = getWorkingDir();
	if (dir == nullptr or full_path.find('\0') != std::string_view::npos) {
		return nullptr;
	}
	const auto path_args = strextens::splitPath(std::string{full_path});
	if (path_args.empty()) return nullptr;
	for (const auto& arg : path_args) {
		dir = dir->getSubDir(arg);
		if (dir == nullptr) {
			return nullptr;
		}
	}
	return dir;
}
