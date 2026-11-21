#include "pushos/pushos.hpp"
#include "pushos/filesys.hpp"
#include "pushos/strextens.hpp"

#include <algorithm>

using namespace pushos;

UserFsctl::UserFsctl(filesys::Dir *dir) : fsctl_user(nullptr) {
	setWorkingDir(dir);
}

void UserFsctl::setWorkingDir(filesys::Dir *dir) {
	std::vector<std::string> path;
	if (dir != nullptr) {
		for (auto *current = dir; current->parent_dir != nullptr; current = current->parent_dir) {
			path.push_back(current->file_name);
		}
		path.emplace_back("/");
		std::reverse(path.begin(), path.end());
	}
	fsctl_user.setWorkingDir(dir);
	user_working_path.swap(path);
}

const std::vector<std::string>& UserFsctl::getWorkingPath() {
	auto *dir = fsctl_user.getWorkingDir();
	if (dir != fsctl_user.working_dir) setWorkingDir(dir);
	return user_working_path;
}

UserFsctl::Result UserFsctl::chdir(std::string_view path) {
	getWorkingPath();
	const auto path_args = strextens::splitPath(path);
	if (path_args.empty() or strextens::stringHas(path, '\0') or
	    strextens::stringHas(path, '\\')) {
		return Result::FileNameError;
	}
	filesys::Dir *dir = fsctl_user.getWorkingDir();
	if (dir == nullptr) return Result::FileNotFound;
	for (const auto &part : path_args) {
		if (part == "/") {
			dir = dir->getSubDir("/");
			continue;
		}
		if (!askFilePermission(*dir, 'x')) return Result::PermissionDenial;
		auto *next = dir->getSubDir(part);
		if (next == nullptr) {
			if (dir->find(part) != -1) return Result::FileTypeError;
			return Result::FileNotFound;
		}
		dir = next;
	}
	if (!askFilePermission(*dir, 'x')) return Result::PermissionDenial;
	setWorkingDir(dir);
	return Result::Success;
}

UserFsctl::Result UserFsctl::mkdir(std::string_view path) {
	std::vector<std::string> path_args = strextens::splitPath(path);
	if (path_args.empty()) {
		return Result::FileNameError;
	}
	std::string opt_name = path_args.back();
	if (opt_name == "/" or opt_name == "." or opt_name == "..") {
		return Result::FileExists;
	}
	path_args.pop_back();
	std::string opt_path = strextens::makePath(path_args);
	filesys::Dir *opt_dir;
	if (path_args.empty()) opt_dir = fsctl_user.getWorkingDir();
	else opt_dir = fsctl_user.getDirByPath(opt_path);
	if (!askFilePermission(opt_path, 'w')) {
		return Result::PermissionDenial;
	}
	int64_t index = opt_dir->find(opt_name);
	if (index == -1) {
		auto new_dir = std::make_shared<filesys::Dir>(opt_name);
		new_dir->owner = user_name;
		new_dir->create_time = helper::getCurrentTimeF("%Y-%m-%d %H:%M:%S");
		new_dir->last_write = new_dir->create_time;
		new_dir->protection = "drwxr-x";
		if (opt_dir->addDir(new_dir) != filesys::FsResult::Success) {
			return Result::FileExists;
		}
		return Result::Success;
	}
	filesys::Node *same_file = opt_dir->sub_files[index];
	if (same_file->isDir()) return Result::FileExists;
	return Result::FileTypeError;
}

UserFsctl::Result UserFsctl::touch(std::string_view path) {
	std::vector<std::string> path_args = strextens::splitPath(path);
	if (path_args.empty() or path.find('\0') != std::string_view::npos) {
		return Result::FileNameError;
	}
	std::string opt_name = path_args.back();
	strextens::trim(opt_name);
	if (opt_name == "/" or opt_name == "." or opt_name == "..") {
		return Result::FileTypeError;
	}
	if (opt_name.empty() or strextens::stringHas(opt_name, '\\')) {
		return Result::FileNameError;
	}
	path_args.pop_back();
	std::string opt_path = strextens::makePath(path_args);
	filesys::Dir *opt_dir;
	if (path_args.empty()) opt_dir = fsctl_user.getWorkingDir();
	else opt_dir = fsctl_user.getDirByPath(opt_path);
	if (opt_dir == nullptr) return Result::FileNotFound;
	if (!askFilePermission(*opt_dir, 'w')) {
		return Result::PermissionDenial;
	}
	int64_t index = opt_dir->find(opt_name);
	if (index == -1) {
		auto new_file = std::make_shared<filesys::File>(opt_name);
		new_file->owner = user_name;
		new_file->create_time = helper::getCurrentTimeF("%Y-%m-%d %H:%M:%S");
		new_file->last_write = new_file->create_time;
		new_file->protection = "-rw-r--";
		if (opt_dir->addFile(new_file) != filesys::FsResult::Success) {
			return Result::FileExists;
		}
		return Result::Success;
	}
	filesys::Node *same_file = opt_dir->sub_files[index];
	if (same_file->isDir()) return Result::FileTypeError;
	if (!askFilePermission(*same_file, 'w')) {
		return Result::PermissionDenial;
	}
	same_file->last_write = helper::getCurrentTimeF("%Y-%m-%d %H:%M:%S");
	return Result::Success;
}

UserFsctl::Result UserFsctl::exegen(std::string_view path, filesys::exeptr_t exe) {
	std::vector<std::string> path_args = strextens::splitPath(path);
	if (path_args.empty() or path.find('\0') != std::string_view::npos) {
		return Result::FileNameError;
	}
	std::string opt_name = path_args.back();
	strextens::trim(opt_name);
	if (opt_name == "/" or opt_name == "." or opt_name == "..") {
		return Result::FileTypeError;
	}
	if (opt_name.empty() or strextens::stringHas(opt_name, '\\')) {
		return Result::FileNameError;
	}
	path_args.pop_back();
	std::string opt_path = strextens::makePath(path_args);
	filesys::Dir *opt_dir;
	if (path_args.empty()) opt_dir = fsctl_user.getWorkingDir();
	else opt_dir = fsctl_user.getDirByPath(opt_path);
	if (opt_dir == nullptr) return Result::FileNotFound;
	if (!askFilePermission(*opt_dir, 'w')) {
		return Result::PermissionDenial;
	}
	int64_t index = opt_dir->find(opt_name);
	if (index != -1) {
		filesys::Node *same_file = opt_dir->sub_files[index];
		if (same_file->isDir()) return Result::FileTypeError;
		return Result::FileExists;
	}
	if (exe == nullptr) return Result::UnknownError;
	auto new_file = std::make_shared<filesys::File>(opt_name);
	new_file->owner = user_name;
	new_file->create_time = helper::getCurrentTimeF("%Y-%m-%d %H:%M:%S");
	new_file->last_write = new_file->create_time;
	new_file->protection = "-rwxr-x";
	new_file->setExecutable(exe);
	if (opt_dir->addFile(new_file) != filesys::FsResult::Success) {
		return Result::FileExists;
	}
	return Result::Success;
}

UserFsctl::Result UserFsctl::copy(std::string_view source, std::string_view destination, bool recursive) {
	filesys::Node *node = fsctl_user.getFileByPath(source);
	if (node == nullptr) node = fsctl_user.getDirByPath(source);
	if (node == nullptr) return Result::FileNotFound;
	if (!askFilePermission(*node, 'r')) return Result::PermissionDenial;
	if (node->isDir() and !recursive) return Result::FileTypeError;

	auto path_args = strextens::splitPath(destination);
	if (path_args.empty() or strextens::stringHas(destination, '\0') or
	    strextens::stringHas(destination, '\\')) {
		return Result::FileNameError;
	}
	std::string name = node->file_name;
	filesys::Dir *parent = fsctl_user.getDirByPath(destination);
	if (parent == nullptr) {
		name = path_args.back();
		strextens::trim(name);
		if (name.empty() or name == "." or name == "..") return Result::FileNameError;
		path_args.pop_back();
		if (path_args.empty()) parent = fsctl_user.getWorkingDir();
		else parent = fsctl_user.getDirByPath(strextens::makePath(path_args));
	}
	if (parent == nullptr) return Result::FileNotFound;
	if (!askFilePermission(*parent, 'w')) return Result::PermissionDenial;
	for (auto *dir = parent; dir != nullptr; dir = dir->parent_dir) {
		if (dir == node) return Result::FileExists;
	}

	const auto index = parent->find(name);
	if (index == -1) return copyNode(*node, *parent, name);
	filesys::Node *target = parent->sub_files[index];
	if (target == node) return Result::FileExists;
	auto *old_file = dynamic_cast<filesys::File*>(target);
	const auto *new_file = dynamic_cast<const filesys::File*>(node);
	if (old_file == nullptr or new_file == nullptr) {
		return target->isDir() and node->isDir() ? Result::FileExists : Result::FileTypeError;
	}
	if (!askFilePermission(*old_file, 'w')) return Result::PermissionDenial;

	auto content = new_file->file_content;
	auto last_write = helper::getCurrentTimeF("%Y-%m-%d %H:%M:%S");
	old_file->file_content.swap(content);
	old_file->executable_part = new_file->executable_part;
	old_file->last_write.swap(last_write);
	return Result::Success;
}

UserFsctl::Result UserFsctl::copyNode(filesys::Node &source, filesys::Dir &parent, std::string_view name) {
	if (!askFilePermission(source, 'r')) return Result::PermissionDenial;
	if (auto *dir = dynamic_cast<filesys::Dir*>(&source)) {
		auto new_dir = std::make_shared<filesys::Dir>(name);
		new_dir->owner = user_name;
		new_dir->protection = dir->protection;
		new_dir->create_time = helper::getCurrentTimeF("%Y-%m-%d %H:%M:%S");
		new_dir->last_write = new_dir->create_time;
		for (auto *child : dir->sub_files) {
			Result result = copyNode(*child, *new_dir, child->file_name);
			if (result != Result::Success) return result;
		}
		if (parent.addDir(new_dir) != filesys::FsResult::Success) return Result::FileExists;
		return Result::Success;
	}
	auto *file = dynamic_cast<filesys::File*>(&source);
	if (file == nullptr) return Result::FileTypeError;
	auto new_file = std::make_shared<filesys::File>(*file);
	new_file->file_name = name;
	new_file->owner = user_name;
	new_file->create_time = helper::getCurrentTimeF("%Y-%m-%d %H:%M:%S");
	new_file->last_write = new_file->create_time;
	if (parent.addFile(new_file) != filesys::FsResult::Success) return Result::FileExists;
	return Result::Success;
}

UserFsctl::Result UserFsctl::rm(std::string_view path) {
	std::vector<std::string> path_args = strextens::splitPath(path);
	if (path_args.empty() or path.find('\0') != std::string_view::npos) {
		return Result::FileNameError;
	}
	std::string opt_name = path_args.back();
	strextens::trim(opt_name);
	if (opt_name == "/" or opt_name == "." or opt_name == "..") {
		return Result::FileTypeError;
	}
	if (opt_name.empty() or strextens::stringHas(opt_name, '\\')) {
		return Result::FileNameError;
	}
	path_args.pop_back();
	std::string opt_path = strextens::makePath(path_args);
	filesys::Dir *opt_dir;
	if (path_args.empty()) opt_dir = fsctl_user.getWorkingDir();
	else opt_dir = fsctl_user.getDirByPath(opt_path);
	if (opt_dir == nullptr) return Result::FileNotFound;
	if (!askFilePermission(*opt_dir, 'w')) {
		return Result::PermissionDenial;
	}
	int64_t index = opt_dir->find(opt_name);
	if (index == -1) return Result::FileNotFound;
	filesys::Node *opt_file = opt_dir->sub_files[index];
	if (opt_file->isDir()) return Result::FileTypeError;
	if (!askFilePermission(*opt_file, 'w')) {
		return Result::PermissionDenial;
	}
	if (opt_dir->delFile(opt_name) != filesys::FsResult::Success) {
		return Result::FileTypeError;
	}
	return Result::Success;
}

UserFsctl::Result UserFsctl::rmdir(std::string_view path, bool recursive) {
	std::string opt_path{path};
	strextens::trim(opt_path);
	if (opt_path.empty() or strextens::stringHas(opt_path, '\0') or
	    strextens::stringHas(opt_path, '\\')) {
		return Result::FileNameError;
	}
	filesys::Dir *target = fsctl_user.getDirByPath(opt_path);
	if (target == nullptr) {
		if (fsctl_user.getFileByPath(opt_path) != nullptr) return Result::FileTypeError;
		return Result::FileNotFound;
	}
	filesys::Dir *parent = target->parent_dir;
	if (!askFilePermission(*target, 'w') or
	    (parent != nullptr and !askFilePermission(*parent, 'w'))) {
		return Result::PermissionDenial;
	}
	if (parent != nullptr and parent->getSubDir(target->file_name) != target) {
		return Result::UnknownError;
	}
	if (!recursive and !target->sub_files.empty()) return Result::DirectoryNotEmpty;
	for (auto *dir = fsctl_user.getWorkingDir(); dir != nullptr; dir = dir->parent_dir) {
		if (dir == target) {
			setWorkingDir(parent != nullptr ? parent : target);
			break;
		}
	}
	target->clear();
	if (parent != nullptr and parent->delDir(target->file_name) != filesys::FsResult::Success) {
		return Result::UnknownError;
	}
	return Result::Success;
}

filesys::Node* UserFsctl::find(std::string_view path) {
	Result result;
	return find(path, result);
}

filesys::Node* UserFsctl::find(std::string_view path, Result &result) {
	filesys::Node *node;
	if (path.empty()) {
		node = fsctl_user.getWorkingDir();
	} else {
		node = fsctl_user.getFileByPath(path);
		if (node == nullptr) node = fsctl_user.getDirByPath(path);
	}
	if (node == nullptr) {
		result = Result::FileNotFound;
		return nullptr;
	}
	if (!askFilePermission(*node, 'r')) {
		result = Result::PermissionDenial;
		return nullptr;
	}
	result = Result::Success;
	return node;
}

const std::vector<filesys::Node*>& UserFsctl::subfilesof(std::string_view path) {
	static const std::vector<filesys::Node*> empty;
	filesys::Dir *opt_dir;
	if (path.empty()) opt_dir = fsctl_user.getWorkingDir();
	else opt_dir = fsctl_user.getDirByPath(path);
	if (opt_dir == nullptr or !askFilePermission(*opt_dir, 'r')) {
		return empty;
	}
	return opt_dir->sub_files;
}

bool UserFsctl::askFilePermission(std::string_view path, char permission) {
	filesys::Node *node;
	if (path.empty()) {
		node = fsctl_user.getWorkingDir();
	} else {
		node = fsctl_user.getFileByPath(path);
		if (node == nullptr) node = fsctl_user.getDirByPath(path);
	}
	return node != nullptr and askFilePermission(*node, permission);
}

bool UserFsctl::askFilePermission(filesys::Node& node, char permission) {
	const auto bit = std::string_view("rwx").find(permission);
	if (bit == std::string_view::npos) return false;
	if (node.owner == "root" and permission != 'x') return true;
	if (node.protection.size() != 7 or
	    node.protection.front() != (node.isDir() ? 'd' : '-')) {
		return false;
	}
	const auto offset = node.owner == user_name ? 1 : 4;
	return node.protection[offset + bit] == permission;
}

Device::Device() : fsctl_device(&storage_directory) {}
