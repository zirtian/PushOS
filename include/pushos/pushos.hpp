#pragma once

#include "filesys.hpp"

#include <string>
#include <string_view>
#include <time.h>
#include <vector>

namespace pushos {
using args_t = std::vector<std::string>;

namespace helper {
inline std::string getCurrentTimeF(const char *fmt) {
	const time_t now = time(nullptr);
	if (now == static_cast<time_t>(-1) or fmt == nullptr) return {};
	const tm* curr_tm = localtime(&now);
	char buffer[64];
	if (curr_tm == nullptr or
	    strftime(buffer, sizeof(buffer), fmt, curr_tm) == 0) {
		return {};
	}
	return buffer;
}

} // namespace helper

struct User {
	std::string name;
	std::string full_name;
	std::string login_passwd_base64;
	User(std::string_view);
};

struct UserFsctl {
	enum class Result : uint8_t {
		Success,
		FileNotFound,
		FileExists,
		FileTypeError,
		FileNameError,
		PermissionDenial,
		UnknownError,
		DirectoryNotEmpty
	};

	std::string user_name;
	std::vector<std::string> user_working_path;
	UserFsctl(filesys::Dir*);
	Result chdir(std::string_view);
	const std::vector<std::string>& getWorkingPath();
	Result mkdir(std::string_view);
	Result touch(std::string_view);
	Result exegen(std::string_view, filesys::exeptr_t);
	Result copy(std::string_view source, std::string_view destination, bool recursive);
	Result rm(std::string_view);
	Result rmdir(std::string_view, bool recursive = true);
	filesys::Node* find(std::string_view);
	filesys::Node* find(std::string_view, Result&);
	const std::vector<filesys::Node*>& subfilesof(std::string_view);
	bool askFilePermission(std::string_view, char);

private:
	void setWorkingDir(filesys::Dir*);
	Result copyNode(filesys::Node&, filesys::Dir&, std::string_view name);
	bool askFilePermission(filesys::Node&, char);
	filesys::FileSystemCtl fsctl_user;
};

struct Device {
	std::string device_name;
	std::string ip_innet;
	std::string ip_public;
	std::vector<bool> ports;
	filesys::Dir storage_directory = filesys::Dir("/").setOwner("root").setProtection("drwxr-x");
	filesys::FileSystemCtl fsctl_device;
	Device();
};

namespace executables {
int64_t cat(UserFsctl*, const args_t&);
int64_t cp(UserFsctl*, const args_t&);
int64_t edit(UserFsctl*, const args_t&);
int64_t email(UserFsctl*, const args_t&);
int64_t find(UserFsctl*, const args_t&);
int64_t ls(UserFsctl*, const args_t&);
int64_t mkdir(UserFsctl*, const args_t&);
int64_t push(UserFsctl*, const args_t&);
int64_t pushos(UserFsctl*, const args_t&);
int64_t rm(UserFsctl*, const args_t&);
int64_t rmdir(UserFsctl*, const args_t&);
int64_t scp(UserFsctl*, const args_t&);
int64_t ssh(UserFsctl*, const args_t&);
int64_t web(UserFsctl*, const args_t&);
int64_t wget(UserFsctl*, const args_t&);
int64_t portscan(UserFsctl*, const args_t&);
int64_t passguess(UserFsctl*, const args_t&);
int64_t track(UserFsctl*, const args_t&);
} // namespace executables

} // namesapce pushos
