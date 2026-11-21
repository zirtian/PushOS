#include "pushos/pushos.hpp"
#include "pushos/strextens.hpp"

#include <iostream>

using namespace pushos;

namespace {
using Result = UserFsctl::Result;

void printFsError(std::string_view command, std::string_view path, Result result) {
	const char *message;
	switch (result) {
	case Result::FileNotFound: message = "no such file or directory"; break;
	case Result::FileExists: message = "file exists"; break;
	case Result::FileTypeError: message = "invalid file type"; break;
	case Result::FileNameError: message = "invalid file name"; break;
	case Result::PermissionDenial: message = "permission denial"; break;
	case Result::DirectoryNotEmpty: message = "directory not empty"; break;
	default: message = "operation failed"; break;
	}
	std::cerr << command << ": " << path << ": " << message << ".\n";
}
} // namespace

int64_t executables::cat(UserFsctl *fsctl, const args_t &args) {
	size_t argc = args.size();
	if (argc < 2) {
		std::cerr << "cat: need more arguments.\n";
		return 1;
	}
	bool error = false;
	for (size_t i = 1; i < argc; i++) {
		const auto &path = args[i];
		Result result;
		filesys::Node *node = fsctl->find(path, result);
		auto *file = dynamic_cast<filesys::File*>(node);
		if (node == nullptr) {
			printFsError("cat", path, result);
		} else if (file == nullptr) {
			printFsError("cat", path, Result::FileTypeError);
		} else if (file->executable_part != nullptr) {
			printFsError("cat", path, Result::PermissionDenial);
		} else {
			std::cout << file->file_content;
			continue;
		}
		error = true;
	}
	return error ? 1 : 0;
}

int64_t executables::cp(UserFsctl *fsctl, const args_t &args) {
	size_t argc = args.size();
	struct Operand {
		std::string_view path;
		bool recursive;
	};
	std::vector<Operand> paths;
	bool recursive = false;
	for (size_t i = 1; i < argc; i++) {
		if (args[i] == "-d") {
			recursive = true;
			continue;
		}
		paths.push_back({args[i], recursive});
	}
	if (paths.size() < 2) {
		std::cerr << "cp: need a source and a destination.\n";
		return 1;
	}
	const auto destination = paths.back().path;
	paths.pop_back();
	if (paths.size() > 1) {
		auto *node = fsctl->find(destination);
		if (node == nullptr or !node->isDir()) {
			std::cerr << "cp: " << destination << ": destination must be a directory.\n";
			return 1;
		}
	}
	bool error = false;
	for (const auto &source : paths) {
		Result result = fsctl->copy(source.path, destination, source.recursive);
		if (result == Result::Success) continue;
		printFsError("cp", source.path, result);
		error = true;
	}
	return error ? 1 : 0;
}

int64_t executables::edit(UserFsctl *fsctl, const args_t &args) {
	size_t argc = args.size();
	return 0;
}

int64_t executables::email(UserFsctl *fsctl, const args_t &args) {
	size_t argc = args.size();
	return 0;
}

int64_t executables::find(UserFsctl *fsctl, const args_t &args) {
	size_t argc = args.size();
	return 0;
}

int64_t executables::ls(UserFsctl *fsctl, const args_t &args) {
	auto list = [fsctl](std::string_view path) {
		Result result;
		filesys::Node *node = fsctl->find(path, result);
		if (node == nullptr) {
			printFsError("ls", path, result);
			return false;
		}
		if (!node->isDir()) {
			std::cout << node->file_name << '\n';
			return true;
		}
		for (auto *child : fsctl->subfilesof(path)) {
			std::cout << child->file_name << (child->isDir() ? "/\n" : "\n");
		}
		return true;
	};
	if (args.size() < 2) return list(".") ? 0 : 1;
	bool error = false;
	for (size_t i = 1; i < args.size(); i++) {
		if (!list(args[i])) error = true;
	}
	return error ? 1 : 0;
}

int64_t executables::mkdir(UserFsctl *fsctl, const args_t &args) {
	size_t argc = args.size();
	if (argc == 1) {
		std::cerr << "mkdir: need more arguments.";
		return 1;
	}
	bool error = false;
	bool ignore_if_exists = false;
	for (size_t i = 1; i < argc; i++) {
		if (args[i] == "-p") {
			ignore_if_exists = true;
		}
		UserFsctl::Result res = fsctl->mkdir(args[i]);
		if (res == UserFsctl::Result::FileExists and !ignore_if_exists) {
			std::cerr << "mkdir: " << args[i] << ": file exists.\n";
			error = true;
		} else if (res == UserFsctl::Result::PermissionDenial) {
			std::cerr << "mkdir: permission denial.\n";
			error = true;
		}
	}
	return error ? 1 : 0;
}

int64_t executables::push(UserFsctl *fsctl, const args_t &args) {
	for (; ; ) {
		if (!fsctl->user_name.empty()) std::cout << fsctl->user_name << ':';
		std::cout << strextens::makePath(fsctl->getWorkingPath()) << " -> ";
		std::string read_line;
		if (!std::getline(std::cin, read_line)) break;
		strextens::trim(read_line);
		if (read_line.empty()) continue;
		std::vector<std::string> args = strextens::split(read_line, " ");
		if (args[0] == "cd") {
			if (args.size() == 1) {
				std::cerr << "push: cd: need more arguments.\n";
				continue;
			}
			if (args.size() > 2) {
				std::cerr << "push: cd: need less arguments.\n";
				continue;
			}
			Result result = fsctl->chdir(args[1]);
			if (result != Result::Success) printFsError("cd", args[1], result);
			continue;
		}
		if (args[0] == "exit") {
			break;
		}
		const auto &file_exec = args.front();
		std::string exec_path = file_exec;
		Result result;
		filesys::Node *exe = fsctl->find(exec_path, result);
		if (result == Result::FileNotFound and !strextens::stringHas(file_exec, '/')) {
			exec_path = "/bin/" + file_exec;
			exe = fsctl->find(exec_path, result);
		}
		if (!exe) {
			printFsError("push", file_exec, result);
			continue;
		}
		if (exe->isDir()) {
			std::cerr << "push: " << file_exec << ": is a directory.\n";
			continue;
		}
		auto *file = dynamic_cast<filesys::File*>(exe);
		if (file == nullptr or file->executable_part == nullptr or !fsctl->askFilePermission(exec_path, 'x')) {
			printFsError("push", file_exec, Result::PermissionDenial);
			continue;
		}
		file->execute(fsctl, args);
	}
	return 0;
}

int64_t executables::pushos(UserFsctl *fsctl, const args_t &args) {
	if (args.size() != 2 or args[1] != "--install") {
		std::cerr << "usage: pushos --install\n";
		return 1;
	}

	for (const auto *path : {"/bin", "/sys", "/etc", "/home", "/root"}) {
		Result result = fsctl->mkdir(path);
		if (result == Result::Success or result == Result::FileExists) continue;
		printFsError("pushos", path, result);
		return 1;
	}

	const struct {
		const char *path;
		filesys::exeptr_t callback;
	} programs[] = {
		{"/bin/push", executables::push},
		{"/bin/pushos", executables::pushos},
		{"/bin/ls", executables::ls},
		{"/bin/cat", executables::cat},
		{"/bin/cp", executables::cp},
		{"/bin/mkdir", executables::mkdir},
		{"/bin/rm", executables::rm},
		{"/bin/rmdir", executables::rmdir},
	};
	for (const auto &program : programs) {
		Result result = fsctl->exegen(program.path, program.callback);
		if (result == Result::Success) continue;
		if (result == Result::FileExists) {
			auto *file = dynamic_cast<filesys::File*>(fsctl->find(program.path));
			if (file != nullptr and file->executable_part == program.callback and
			    fsctl->askFilePermission(program.path, 'x')) continue;
		}
		printFsError("pushos", program.path, result);
		return 1;
	}
	return 0;
}

int64_t executables::rm(UserFsctl *fsctl, const args_t &args) {
	size_t argc = args.size();
	bool error = false;
	bool ignore_if_missing = false;
	bool has_path = false;
	for (size_t i = 1; i < argc; i++) {
		if (args[i] == "-f") {
			ignore_if_missing = true;
			continue;
		}
		has_path = true;
		Result result = fsctl->rm(args[i]);
		if (result == Result::Success or (result == Result::FileNotFound and ignore_if_missing)) continue;
		printFsError("rm", args[i], result);
		error = true;
	}
	if (!has_path) {
		std::cerr << "rm: need more arguments.\n";
		return 1;
	}
	return error ? 1 : 0;
}

int64_t executables::rmdir(UserFsctl *fsctl, const args_t &args) {
	size_t argc = args.size();
	bool error = false;
	bool ignore_if_missing = false;
	bool recursive = false;
	bool has_path = false;
	for (size_t i = 1; i < argc; i++) {
		if (args[i] == "-f") {
			ignore_if_missing = true;
			continue;
		}
		if (args[i] == "-r") {
			recursive = true;
			continue;
		}
		has_path = true;
		Result result = fsctl->rmdir(args[i], recursive);
		if (result == Result::Success or (result == Result::FileNotFound and ignore_if_missing)) continue;
		printFsError("rmdir", args[i], result);
		error = true;
	}
	if (!has_path) {
		std::cerr << "rmdir: need more arguments.\n";
		return 1;
	}
	return error ? 1 : 0;
}

int64_t executables::scp(UserFsctl *fsctl, const args_t &args) {
	size_t argc = args.size();
	return 0;
}

int64_t executables::ssh(UserFsctl *fsctl, const args_t &args) {
	size_t argc = args.size();
	return 0;
}

int64_t executables::web(UserFsctl *fsctl, const args_t &args) {
	size_t argc = args.size();
	return 0;
}

int64_t executables::wget(UserFsctl *fsctl, const args_t &args) {
	size_t argc = args.size();
	return 0;
}

int64_t executables::portscan(UserFsctl *fsctl, const args_t &args) {
	size_t argc = args.size();
	return 0;
}

int64_t executables::passguess(UserFsctl *fsctl, const args_t &args) {
	size_t argc = args.size();
	return 0;
}

int64_t executables::track(UserFsctl *fsctl, const args_t &args) {
	size_t argc = args.size();
	return 0;
}
