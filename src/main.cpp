#define PROGRAM_NAME        "PushOS"
#define PROGRAM_LAST_UPDATE "2026/11/19"
#define PROGRAM_VERSION     "0.1.0"
#define PROGRAM_CREATOR     "ZirconiumTian"

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include <pushos/filesys.hpp>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::cin;

#if defined(_WIN32) || defined(_WIN64)

#define SYS_WINDOWS

#include <Windows.h>
#include <conio.h>

#define WINDOWS_TERMINAL_ANSI_COLOR_SUPPORT

#if defined(WINDOWS_TERMINAL_ANSI_COLOR_SUPPORT)

#define RED             "\033[31m"
#define LIGHT_RED       "\033[1;31m"
#define GREEN           "\033[32m"
#define LIGHT_GREEN     "\033[1;32m"
#define YELLOW          "\033[33m"
#define LIGHT_YELLOW    "\033[1;33m"
#define BLUE            "\033[34m"
#define LIGHT_BLUE      "\033[1;34m"
#define PURPLE          "\033[35m"
#define LIGHT_PURPLE    "\033[1;35m"
#define CYAN            "\033[36m"
#define LIGHT_CYAN      "\033[1;36m"
#define WHITE           "\033[37m"
#define LIGHT_WHITE     "\033[1;37m"
#define BG_RED          "\033[41m"
#define BG_LIGHT_RED    "\033[1;41m"
#define BG_GREEN        "\033[42m"
#define BG_LIGHT_GREEN  "\033[1;42m"
#define BG_YELLOW       "\033[43m"
#define BG_LIGHT_YELLOW "\033[1;43m"
#define BG_BLUE         "\033[44m"
#define BG_LIGHT_BLUE   "\033[1;44m"
#define BG_PURPLE       "\033[45m"
#define BG_LIGHT_PURPLE "\033[1;45m"
#define BG_CYAN         "\033[46m"
#define BG_LIGHT_CYAN   "\033[1;46m"
#define BG_WHITE        "\033[47m"
#define BG_LIGHT_WHITE  "\033[1;47m"
#define CLEAR           "\033[0m"

#else

#define RED             ""
#define LIGHT_RED       ""
#define GREEN           ""
#define LIGHT_GREEN     ""
#define YELLOW          ""
#define LIGHT_YELLOW    ""
#define BLUE            ""
#define LIGHT_BLUE      ""
#define PURPLE          ""
#define LIGHT_PURPLE    ""
#define CYAN            ""
#define LIGHT_CYAN      ""
#define WHITE           ""
#define LIGHT_WHITE     ""
#define BG_RED          ""
#define BG_LIGHT_RED    ""
#define BG_GREEN        ""
#define BG_LIGHT_GREEN  ""
#define BG_YELLOW       ""
#define BG_LIGHT_YELLOW ""
#define BG_BLUE         ""
#define BG_LIGHT_BLUE   ""
#define BG_PURPLE       ""
#define BG_LIGHT_PURPLE ""
#define BG_CYAN         ""
#define BG_LIGHT_CYAN   ""
#define BG_WHITE        ""
#define BG_LIGHT_WHITE  ""
#define CLEAR           ""

#endif

#elif defined(__linux__)

#define SYS_LINUX

#include <termio.h>
#include <unistd.h>

#define RED             "\033[31m"
#define LIGHT_RED       "\033[1;31m"
#define GREEN           "\033[32m"
#define LIGHT_GREEN     "\033[1;32m"
#define YELLOW          "\033[33m"
#define LIGHT_YELLOW    "\033[1;33m"
#define BLUE            "\033[34m"
#define LIGHT_BLUE      "\033[1;34m"
#define PURPLE          "\033[35m"
#define LIGHT_PURPLE    "\033[1;35m"
#define CYAN            "\033[36m"
#define LIGHT_CYAN      "\033[1;36m"
#define WHITE           "\033[37m"
#define LIGHT_WHITE     "\033[1;37m"
#define BG_RED          "\033[41m"
#define BG_LIGHT_RED    "\033[1;41m"
#define BG_GREEN        "\033[42m"
#define BG_LIGHT_GREEN  "\033[1;42m"
#define BG_YELLOW       "\033[43m"
#define BG_LIGHT_YELLOW "\033[1;43m"
#define BG_BLUE         "\033[44m"
#define BG_LIGHT_BLUE   "\033[1;44m"
#define BG_PURPLE       "\033[45m"
#define BG_LIGHT_PURPLE "\033[1;45m"
#define BG_CYAN         "\033[46m"
#define BG_LIGHT_CYAN   "\033[1;46m"
#define BG_WHITE        "\033[47m"
#define BG_LIGHT_WHITE  "\033[1;47m"
#define CLEAR           "\033[0m"

#endif

class auxiliaryConsoleTools
{
public:
	vector<string> split(string srcstr, string delimeter)
	{
		vector<string> ret(0);
		if (srcstr.empty())
		{
			return ret;
		}
		string::size_type pos_begin = srcstr.find_first_not_of(delimeter);
		string::size_type dlm_pos;
		string temp;
		while (pos_begin != string::npos)
		{
			dlm_pos = srcstr.find(delimeter, pos_begin);
			if (dlm_pos != string::npos)
			{
				temp = srcstr.substr(pos_begin, dlm_pos - pos_begin);
				pos_begin = dlm_pos + delimeter.length();
			}
			else
			{
				temp = srcstr.substr(pos_begin);
				pos_begin = dlm_pos;
			}
			if (!temp.empty())
			{
				ret.push_back(temp);
			}
		}
		return ret;
	}

	void clearScreen()
	{
		#if defined(SYS_WINDOWS)
		system("cls");
		#elif defined(SYS_LINUX)
		cout << "\033c";
		#endif

		return;
	}

	int getch(void)
	{
		#if defined(SYS_WINDOWS)
		return _getch();
		#elif defined(SYS_LINUX)

		struct termios tm, tm_old;
		int fd = 0, ch;
		if (tcgetattr(fd, &tm) < 0)
		{
			return -1;
		}
		tm_old = tm;
		cfmakeraw(&tm);
		if (tcsetattr(fd, TCSANOW, &tm) < 0)
		{
			return -1;
		}
		ch = getchar();
		if (tcsetattr(fd, TCSANOW, &tm_old) < 0)
		{
			return -1;
		}
		return ch;
		#endif
	}

	void delay(int milliSeconds)
	{
		#if defined(SYS_WINDOWS)
		Sleep(milliSeconds);
		#elif defined(SYS_LINUX)
		usleep(milliSeconds * 1000);
		#endif
		return;
	}

	bool startswith(string str, string start)
	{
		int srclen = str.size();
		int startlen = start.size();
		if (srclen >= startlen)
		{
			string temp = str.substr(0, startlen);
			if (temp == start)
			{
				return true;
			}
		}
		return false;
	}

	bool endswith(string str, string end)
	{
		int srclen = str.size();
		int endlen = end.size();
		if (srclen >= endlen)
		{
			string temp = str.substr(srclen - endlen, endlen);
			if (temp == end)
			{
				return true;
			}
		}
		return false;
	}
};

auxiliaryConsoleTools consoleTools;

fileSystem localFileSystem;

namespace executableFileLibrary
{
	void mkdir(vector<string> args)
	{
		if (args.size() == 1)
		{
			cout << "mkdir: 参数太少" << endl;
			cout << "用法: mkdir <目录...>" << endl;
		}
		else
		{
			for (size_t i = 1; i < args.size(); i++)
			{
				int code = localFileSystem.makeDir(args[i], false);
				if (code != fileSystemSign::OPTION_COMPLETE)
				{
					if (code == fileSystemSign::FILE_OR_DIRECTORY_EXISTS)
					{
						cout << "mkdir: " << args[i] << ": 目录已存在" << endl;
					}
				}
			}
		}
	}

	void create(vector<string> args)
	{
		if (args.size() == 1)
		{
			cout << "create: 参数太少" << endl;
			cout << "用法: create <文件...>" << endl;
		}
		else
		{
			for (size_t i = 1; i < args.size(); i++)
			{
				int code = localFileSystem.createFile(args[i], {}, false, false, nullptr);
				if (code != fileSystemSign::OPTION_COMPLETE)
				{
					if (code == fileSystemSign::FILE_OR_DIRECTORY_EXISTS)
					{
						cout << "create: " << args[i] << ": 文件已存在" << endl;
					}
					else if (code == fileSystemSign::TARGET_IS_A_DIRECTORY)
					{
						cout << "create: " << args[i] << ": 是一个目录" << endl;
					}
				}
			}
		}
	}

	void rm(vector<string> args)
	{
		if (args.size() == 1)
		{
			cout << "rm: 参数太少" << endl;
			cout << "用法: rm <文件...>" << endl;
		}
		else
		{
			for (size_t i = 1; i < args.size(); i++)
			{
				int code = localFileSystem.removeFile(args[i]);
				if (code != fileSystemSign::OPTION_COMPLETE)
				{
					if (code == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
					{
						cout << "rm: " << args[i] << ": 没有那个文件或目录" << endl;
					}
					else if (code == fileSystemSign::PERMISSION_DENIAL)
					{
						cout << "rm: " << args[i] << ": 权限拒绝" << endl;
					}
				}
			}
		}
	}

	void rmdir(vector<string> args)
	{
		if (args.size() == 1)
		{
			cout << "rmdir: 参数太少" << endl;
			cout << "用法: rmdir <目录...>" << endl;
		}
		else
		{
			for (size_t i = 1; i < args.size(); i++)
			{
				int code = localFileSystem.removeDir(args[i]);
				if (code != fileSystemSign::OPTION_COMPLETE)
				{
					if (code == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
					{
						cout << "rmdir: " << args[i] << ": 没有那个文件或目录" << endl;
					}
					else if (code == fileSystemSign::PERMISSION_DENIAL)
					{
						cout << "rmdir: " << args[i] << ": 权限拒绝" << endl;
					}
				}
			}
		}
	}

	void type(vector<string> args)
	{
		if (args.size() == 1)
		{
			cout << "type: 参数太少" << endl;
			cout << "用法: type <文件...>" << endl;
		}
		else
		{
			for (size_t i = 1; i < args.size(); i++)
			{
				int code = localFileSystem.readFile(args[i]);
				if (code != fileSystemSign::OPTION_COMPLETE)
				{
					if (code == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
					{
						cout << "type: " << args[i] << ": 没有那个文件或目录" << endl;
					}
					else if (code == fileSystemSign::FILE_TYPE_ERROR)
					{
						cout << "type: " << args[i] << ": 不是一个有效的文本文件" << endl;
					}
					else if (code == fileSystemSign::PERMISSION_DENIAL)
					{
						cout << "type: " << args[i] << ": 权限拒绝" << endl;
					}
				}
			}
		}
	}

	void ls(vector<string> args)
	{
		if (args.size() == 1)
		{
			localFileSystem.listFiles(CLEAR, LIGHT_BLUE, LIGHT_WHITE, LIGHT_GREEN);
		}
		else
		{
			cout << "ls: 参数太多" << endl;
			cout << "用法: ls" << endl;
		}
	}

	void exec(vector<string> args)
	{
		if (args.size() == 1)
		{
			cout << "exec: 参数太少" << endl;
			cout << "用法: exec <文件> <参数...>" << endl;
		}
		else
		{
			string fileName = args[1];
			args.erase(args.begin());
			int code = localFileSystem.runFile(fileName, args);
			if (code != fileSystemSign::OPTION_COMPLETE)
			{
				if (code == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
				{
					cout << "exec: " << fileName << ": 没有那个文件或目录" << endl;
				}
				else if (code == fileSystemSign::FILE_TYPE_ERROR)
				{
					cout << "exec: " << fileName << ": 不是一个有效的可执行文件" << endl;
				}
			}
		}
	}

	void clear(vector<string> args)
	{
		if (args.size() == 1)
		{
			consoleTools.clearScreen();
		}
		else
		{
			cout << "clear: 参数太多" << endl;
			cout << "用法: clear" << endl;
		}
	}

	void cd(vector<string> args)
	{
		if (args.size() == 1)
		{
			cout << "cd: 参数太少" << endl;
			cout << "用法: cd <目录>" << endl;
		}
		else if (args.size() == 2)
		{
			if (args[1] == "/")
			{
				localFileSystem.moveToRootDir();
				return;
			}
			int code = localFileSystem.changeDir(args[1]);
			if (code != fileSystemSign::OPTION_COMPLETE)
			{
				cout << "cd: 没有那个文件或目录" << endl;
			}
		}
		else
		{
			cout << "cd: 参数太多" << endl;
			cout << "用法: cd <目录>" << endl;
		}
	}

	string runCommand(vector<string> args)
	{
		if (args.size() != 0)
		{
			if (args[0] == "logout")
			{
				dir* directoryBin = localFileSystem.getTargetDir("/bin");
				int code = localFileSystem.runFileOutside(directoryBin, "logout", args);
				if (code != fileSystemSign::OPTION_COMPLETE)
				{
					if (code == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
					{
						cout << "不能执行: " << args[0] << ": 没有那个文件或目录" << endl;
						return "continue";
					}
					else if (code == fileSystemSign::FILE_TYPE_ERROR)
					{
						cout << "不能执行: " << args[0] << ": 不是一个有效的可执行文件" << endl;
						return "continue";
					}
				}
				else
				{
					if (args.size() == 1)
					{
						return "logout";
					}
					else
					{
						cout << "logout: 参数太多" << endl;
						cout << "用法: logout" << endl;
					}
				}
				return "continue";
			}
			else if (args[0] == "shutdown")
			{
				dir* directoryBin = localFileSystem.getTargetDir("/PushOS/bin");
				int code = localFileSystem.runFileOutside(directoryBin, "shutdown", args);
				if (code != fileSystemSign::OPTION_COMPLETE)
				{
					if (code == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
					{
						cout << "不能执行: " << args[0] << ": 没有那个文件或目录" << endl;
						return "continue";
					}
					else if (code == fileSystemSign::FILE_TYPE_ERROR)
					{
						cout << "不能执行: " << args[0] << ": 不是一个有效的可执行文件" << endl;
						return "continue";
					}
				}
				return "continue";
			}
			else
			{
				dir* directoryBin = localFileSystem.getTargetDir("/PushOS/bin");
				int code = localFileSystem.runFileOutside(directoryBin, args[0], args);
				if (code != fileSystemSign::OPTION_COMPLETE)
				{
					if (code == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
					{
						cout << "不能执行: " << args[0] << ": 没有那个文件或目录" << endl;
					}
					else if (code == fileSystemSign::FILE_TYPE_ERROR)
					{
						cout << "不能执行: " << args[0] << ": 不是一个有效的可执行文件" << endl;
					}
				}
				return "continue";
			}
		}
		return "continue";
	}

	void overw(vector<string> args)
	{
		if (args.size() == 1)
		{
			cout << "overw: 参数太少" << endl;
			cout << "用法: overw <文件>" << endl;
		}
		else if (args.size() == 2)
		{
			int code = localFileSystem.writeFile(args[1]);
			if (code != fileSystemSign::OPTION_COMPLETE)
			{
				if (code == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
				{
					cout << "overw: " << args[1] << ": 没有那个文件或目录" << endl;
				}
				else if (code == fileSystemSign::FILE_TYPE_ERROR)
				{
					cout << "overw: " << args[1] << ": 不是一个有效的文本文件" << endl;
				}
				else if (code == fileSystemSign::PERMISSION_DENIAL)
				{
					cout << "overw: " << args[1] << ": 权限拒绝" << endl;
				}
			}
		}
		else
		{
			cout << "overw: 参数太多" << endl;
			cout << "用法: overw <文件>" << endl;
		}
	}

	void echo(vector<string> args)
	{
		if (args.size() == 1)
		{
			cout << endl;
		}
		else
		{
			for (size_t i = 1; i < args.size(); i++)
			{
				cout << args[i] << " ";
			}
			cout << endl;
		}
	}

	void shutdown(vector<string> args)
	{
		if (args.size() == 1)
		{
			consoleTools.clearScreen();
			exit(0);
		}
		else
		{
			cout << "shutdown: 参数太多" << endl;
			cout << "用法: shutdown" << endl;
		}
	}

	void helloWorld(vector<string> args)
	{
		cout << "Hello, World!\n";
	}

	void null(vector<string> args)
	{
		return;
	}

	void help(vector<string> args)
	{
		if (args.size() == 1)
		{
			cout << "cd       切换目录" << endl;
			cout << "clear    清空屏幕" << endl;
			cout << "create   创建空的文本文件" << endl;
			cout << "echo     显示一条文本消息" << endl;
			cout << "execute  运行一个可执行文件" << endl;
			cout << "help     显示此消息" << endl;
			cout << "logout   退出登录" << endl;
			cout << "ls       列出当前目录下的文件和子目录" << endl;
			cout << "mkdir    创建空的目录" << endl;
			cout << "overw    覆盖写入到一个文本文件" << endl;
			cout << "         将放弃原内容, Ctrl + X 结束写入" << endl;
			cout << "rm       删除文件" << endl;
			cout << "rmdir    删除目录" << endl;
			cout << "shutdown 关闭 PushOS, 并退出" << endl;
			cout << "type     显示文本文件的内容" << endl;
			cout << endl;
		}
		else
		{
			cout << "help: 参数太多" << endl;
			cout << "用法: help" << endl;
		}
	}
}

void logoPushOS()
{
	cout << LIGHT_CYAN << "  ____                  _      " << LIGHT_WHITE << "  ___    ____" << CLEAR << endl;
	cout << LIGHT_CYAN << " |  _ \\   _   _   ___  | |__   " << LIGHT_WHITE << " / _ \\  / ___| " << CLEAR << endl;
	cout << LIGHT_CYAN << " | |_) | | | | | / __| | '_ \\  " << LIGHT_WHITE << "| | | | \\___ \\ " << CLEAR << endl;
	cout << LIGHT_CYAN << " |  __/  | |_| | \\__ \\ | | | | " << LIGHT_WHITE << "| |_| |  ___) |" << CLEAR << endl;
	cout << LIGHT_CYAN << " |_|      \\__,_| |___/ |_| |_| " << LIGHT_WHITE << " \\___/  |____/ " << CLEAR << endl;
	cout << endl;
	cout << "PushOS 版本: " << PROGRAM_VERSION << ", 上次更新: " << PROGRAM_LAST_UPDATE << endl;
}

void installPushOS()
{
	localFileSystem.moveToRootDir();
	localFileSystem.makeDir("bin", true);
	localFileSystem.makeDir("logs", true);
	localFileSystem.makeDir("system", true);
	localFileSystem.makeDir("users", true);
	localFileSystem.changeDir("bin");
	localFileSystem.createFile("cd", {}, true, true, executableFileLibrary::cd);
	localFileSystem.createFile("clear", {}, true, true, executableFileLibrary::clear);
	localFileSystem.createFile("create", {}, true, true, executableFileLibrary::create);
	localFileSystem.createFile("echo", {}, true, true, executableFileLibrary::echo);
	localFileSystem.createFile("exec", {}, true, true, executableFileLibrary::exec);
	localFileSystem.createFile("help", {}, true, true, executableFileLibrary::help);
	localFileSystem.createFile("logout", {}, true, true, executableFileLibrary::null);
	localFileSystem.createFile("ls", {}, true, true, executableFileLibrary::ls);
	localFileSystem.createFile("mkdir", {}, true, true, executableFileLibrary::mkdir);
	localFileSystem.createFile("overw", {}, true, true, executableFileLibrary::overw);
	localFileSystem.createFile("rm", {}, true, true, executableFileLibrary::rm);
	localFileSystem.createFile("rmdir", {}, true, true, executableFileLibrary::rmdir);
	localFileSystem.createFile("shutdown", {}, true, true, executableFileLibrary::shutdown);
	localFileSystem.createFile("type", {}, true, true, executableFileLibrary::type);
	localFileSystem.moveToRootDir();
	localFileSystem.createFile("hello.asm",
		{
			"section .data",
			"    msg db \"Hello, World!\", 0xa",
			"    len equ $ - msg",
			"",
			"section .text",
			"    global _start",
			"",
			"_start:",
			"    mov eax, 4",
			"    mov ebx, 1",
			"    mov ecx, msg",
			"    mov edx, len",
			"    int 0x80",
			"",
			"    mov eax, 1",
			"    int 0x80",
		},
		false, false, nullptr);
	localFileSystem.createFile("hello", {}, true, false, executableFileLibrary::helloWorld);
}

struct user
{
	string name;
	string password;
	bool isRootUser;
	bool needPassword;
};

class userSystem
{
private:
	vector<user*> localUsers;

	string getUserNameInput()
	{
		string userName;
		for (;;)
		{
			char keyInput = consoleTools.getch();
			if (keyInput == '\r')
			{
				cout << endl;
				return userName;
			}
			else if (keyInput == '\b' || keyInput == 127)
			{
				if (userName.length() != 0)
				{
					cout << "\b \b";
					userName.pop_back();
				}
			}
			else
			{
				if (keyInput == ' ')
				{
					continue;
				}
				else if (!((keyInput < '0' || keyInput > '9') && (keyInput < 'a' || keyInput > 'z') && (keyInput != '-')))
				{
					userName += keyInput;
					cout << keyInput;
				}
			}
		}
	}

	string getPasswordInput()
	{
		string password;
		for (;;)
		{

			char keyInput = consoleTools.getch();
			if (keyInput == '\r')
			{
				cout << endl;
				return password;
			}
			else if (keyInput == '\b' || keyInput == 127)
			{
				if (password.length() != 0)
				{
					cout << "\b \b";
					password.pop_back();
				}
			}
			else
			{
				if (!(keyInput < ' ' || keyInput > 'z'))
				{
					password += keyInput;
					cout << "*";
				}
			}
		}
	}

public:
	string currentUserName;

	userSystem()
	{
		localUsers = {};
		user* rootUser = new user;
		rootUser->isRootUser = true;
		rootUser->name = "root";
		rootUser->needPassword = false;
		rootUser->password = "";
		localUsers.push_back(rootUser);
	}

	void putRootUserDir()
	{
		localFileSystem.moveToRootDir();
		localFileSystem.changeDir("users");
		localFileSystem.makeDir("root", true);
		localFileSystem.moveToRootDir();
	}

	void addUser(string name, string password, bool needPassword)
	{
		user* newUser = new user;
		newUser->isRootUser = false;
		newUser->name = name;
		newUser->needPassword = needPassword;
		newUser->password = password;
		for (size_t i = 0; i < localUsers.size(); i++)
		{
			if (localUsers[i]->name == name)
			{
				cout << "用户已存在" << endl;
				return;
			}
		}
		localFileSystem.moveToRootDir();
		int code = localFileSystem.openDir("users");
		if (code == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
		{
			cout << "users: 没有那个文件或目录" << endl;
			cout << "用户未创建" << endl;
			cout << endl;
			return;
		}
		localUsers.push_back(newUser);
		localFileSystem.moveToRootDir();
		localFileSystem.changeDir("users");
		localFileSystem.makeDir(name, true);
		localFileSystem.moveToRootDir();
	}

	bool loginShell()
	{
		cout << "登录到 PushOS" << endl;
		cout << endl;
		cout << "newuser   新建用户" << endl;
		cout << "userlist  查看用户列表" << endl;
		cout << endl;
		for (;;)
		{
			string input;
			cout << "login: ";
			getline(cin, input);
			vector<string> args = consoleTools.split(input, " ");
			if (input == "" || args.size() == 0)
			{
				continue;
			}
			else if (args[0] == "userlist")
			{
				cout << "本地用户列表: " << endl;
				for (size_t i = 0; i < localUsers.size(); i++)
				{
					if (i == (localUsers.size() - 1))
					{
						cout << "   └─ ";
					}
					else
					{
						cout << "   ├─ ";
					}
					cout << localUsers[i]->name << endl;
				}
				cout << endl;
			}
			else if (args[0] == "newuser")
			{
				if (args.size() == 1)
				{
					string newUserName = "";
					string newPassword = "";
					cout << "新用户名: ";
					newUserName = getUserNameInput();
					if (newUserName == "")
					{
						cout << "输入不能为空" << endl;
					}
					else if (newUserName == "newuser")
					{
						cout << "newuser: 是一个命令" << endl;
					}
					else if (newUserName == "userlist")
					{
						cout << "userlist: 是一个命令" << endl;
					}
					else
					{
						cout << "使用密码? (按 Y/N)" << endl;
						bool usePassword = false;
						for (;;)
						{
							char keyInput = consoleTools.getch();
							if (toupper(keyInput) == 'Y')
							{
								usePassword = true;
								break;
							}
							else if (toupper(keyInput) == 'N')
							{
								usePassword = false;
								break;
							}
						}
						if (usePassword)
						{
							cout << "键入新密码: ";
							newPassword = getPasswordInput();
						}
						bool stopped = false;
						for (size_t i = 0; i < localUsers.size(); i++)
						{
							if (localUsers[i]->name == newUserName)
							{
								cout << "用户已存在" << endl;
								stopped = true;
								break;
							}
						}
						if (!stopped)
						{
							cout << newUserName << " 已创建" << endl;
							cout << endl;
							addUser(newUserName, newPassword, usePassword);
						}
					}
				}
				else
				{
					cout << "newuser: 参数太多" << endl;
					cout << "用法: newuser" << endl;
				}
			}
			else
			{
				localFileSystem.moveToRootDir();
				bool stopped = false;
				for (size_t i = 0; i < localUsers.size(); i++)
				{
					if (localUsers[i]->name == input)
					{
						int code = localFileSystem.openDir("users");
						if (code == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
						{
							break;
						}
						else
						{
							dir* userDirectory = localFileSystem.getTargetDir("/users");
							for (size_t n = 0; n < userDirectory->subDirs.size(); n++)
							{
								if (userDirectory->subDirs[n]->name == input)
								{
									stopped = true;
								}
							}
							if (!stopped)
							{
								break;
							}
						}
						if (localUsers[i]->needPassword)
						{
							cout << localUsers[i]->name << "'s password: ";
							string tempPassword;
							tempPassword = getPasswordInput();
							if (localUsers[i]->password != tempPassword)
							{
								cout << "密码不正确" << endl;
								stopped = true;
							}
							else
							{
								currentUserName = input;
								logoPushOS();
								cout << pushos::getCurrentTime() << ": 以身份 " << input << " 登录到 PushOS" << endl;
								cout << endl;
								localFileSystem.moveToRootDir();
								int code = localFileSystem.changeDir("logs");
								if (code == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
								{
									cout << "无法写日志: logs: 没有那个文件或目录" << endl;
								}
								else
								{
									localFileSystem.addStrToFile("login.log", ("[ " + pushos::getCurrentTime() + " ] " + input + " 登入 PushOS"));
									localFileSystem.moveToRootDir();
								}
								localFileSystem.changeDir("users");
								localFileSystem.changeDir(input);
								return localUsers[i]->isRootUser;
							}
						}
						else
						{
							currentUserName = input;
							logoPushOS();
							cout << pushos::getCurrentTime() << ": 以身份 " << input << " 登录到 PushOS" << endl;
							cout << endl;
							localFileSystem.moveToRootDir();
							int code = localFileSystem.changeDir("logs");
							if (code == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
							{
								cout << "无法写日志: logs: 没有那个文件或目录" << endl;
							}
							else
							{
								localFileSystem.addStrToFile("login.log", ("[ " + pushos::getCurrentTime() + " ] " + input + " 登入 PushOS"));
								localFileSystem.moveToRootDir();
							}
							localFileSystem.changeDir("users");
							localFileSystem.changeDir(input);
							return localUsers[i]->isRootUser;
						}
					}
				}
				if (!stopped)
				{
					cout << "用户未找到" << endl;
				}
			}
		}
	}
};

userSystem localUserSystem;

void shell()
{
	for (;;)
	{
		string command;
		cout << LIGHT_GREEN;
		cout << localUserSystem.currentUserName;
		cout << "@"
			<< "localhost" << CLEAR
			<< " [ ";
		cout << LIGHT_BLUE << localFileSystem.getCurrentPath() << CLEAR << " ] → ";
		getline(cin, command);
		vector<string> args = consoleTools.split(command, " ");
		if (command == "" || args.size() == 0)
		{
			continue;
		}
		else
		{
			string shellSign = executableFileLibrary::runCommand(args);
			if (shellSign == "logout")
			{
				localFileSystem.moveToRootDir();
				cout << endl;
				cout << pushos::getCurrentTime() << ": 用户 " << localUserSystem.currentUserName << " 退出登录" << endl;
				cout << endl;
				int code = localFileSystem.changeDir("PushOS");
				code = localFileSystem.changeDir("logs");
				if (code == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
				{
					cout << "PushOS: 无法写日志: logs: 没有那个文件或目录" << endl;
				}
				else
				{
					localFileSystem.addStrToFile("login.log", ("[ " + pushos::getCurrentTime() + " ] " + localUserSystem.currentUserName + " 离开了 PushOS"));
					localFileSystem.moveToRootDir();
				}
				return;
			}
		}
	}
}

int main()
{
	cout << CLEAR;
	installPushOS();
	localUserSystem.putRootUserDir();
	consoleTools.clearScreen();
	for (;;)
	{
		localFileSystem.setRootMode(localUserSystem.loginShell());
		shell();
	}
	return 0;
}