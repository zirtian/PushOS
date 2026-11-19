#pragma once

#include <cstddef>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace pushos
{
	inline std::string getCurrentTime()
	{
		std::time_t t = std::time(0);
		char tmp[32] = "";
		std::strftime(tmp, sizeof(tmp), "%Y/%m/%d %H:%M:%S", std::localtime(&t));
		return tmp;
	}
}

typedef void (*exe)(std::vector<std::string> args);

struct file
{
	std::string name;
	std::string lastWriteTime;
	std::vector<std::string> content;
	bool isExeFile;
	bool isProtected;
	exe fileApplicationPart;
};

struct dir
{
	std::string name;
	std::string lastWriteTime;
	dir* parentDir;
	std::vector<dir*> subDirs;
	std::vector<file*> subFiles;
	bool isRootDir;
	bool isProtected;
};

namespace fileSystemSign
{
	const int FILE_OR_DIRECTORY_NOT_FOUND = -1;
	const int FILE_OR_DIRECTORY_EXISTS = -2;
	const int TARGET_IS_A_DIRECTORY = -3;
	const int PERMISSION_DENIAL = -4;
	const int OPTION_COMPLETE = -5;
	const int FILE_TYPE_ERROR = -6;
}

class fileSystem
{
private:
	dir* currentDir = new dir;
	std::string currentPath;
	bool rootMode;

public:
	fileSystem()
	{
		currentDir->isProtected = true;
		currentDir->isRootDir = true;
		currentDir->lastWriteTime = "";
		currentDir->name = "/";
		currentDir->parentDir = nullptr;
		currentPath = "/";
		rootMode = false;
	}

	int openFile(std::string name)
	{
		for (std::size_t i = 0; i < currentDir->subFiles.size(); i++)
		{
			if (currentDir->subFiles[i]->name == name)
			{
				return i;
			}
		}
		return fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND;
	}

	int openDir(std::string name)
	{
		for (std::size_t i = 0; i < currentDir->subDirs.size(); i++)
		{
			if (currentDir->subDirs[i]->name == name)
			{
				return i;
			}
		}
		return fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND;
	}

	dir* changeDirOutside(dir* current, std::string name)
	{
		if (name == ".")
		{
			return current;
		}
		else if (name == "..")
		{
			if (!current->isRootDir)
			{
				for (std::size_t i = 0; i < current->parentDir->subDirs.size(); i++)
				{
					if (current->parentDir->subDirs[i]->name == current->name)
					{
						current->parentDir->subDirs[i] = current;
					}
				}
				current = current->parentDir;
			}
			return current;
		}
		else
		{
			for (std::size_t i = 0; i < current->subDirs.size(); i++)
			{
				if (current->subDirs[i]->name == name)
				{
					current->subDirs[i]->parentDir = current;
					current = current->subDirs[i];
					return current;
				}
			}
			return current;
		}
	}

	dir* getTargetDir(std::string fullPath)
	{
		dir* targetDir = currentDir;
		for (;;)
		{
			if (targetDir->isRootDir)
			{
				break;
			}
			targetDir = changeDirOutside(targetDir, "..");
		}
		std::istringstream path(fullPath);
		std::string component;
		while (std::getline(path, component, '/'))
		{
			if (!component.empty())
			{
				targetDir = changeDirOutside(targetDir, component);
			}
		}
		return targetDir;
	}

	std::string getCurrentPath()
	{
		return currentPath;
	}

	int changeDir(std::string name)
	{
		if (name == ".")
		{
			return fileSystemSign::OPTION_COMPLETE;
		}
		else if (name == "..")
		{
			if (!currentDir->isRootDir)
			{

				for (std::size_t i = 0; i < currentDir->parentDir->subDirs.size(); i++)
				{
					if (currentDir->parentDir->subDirs[i]->name == currentDir->name)
					{
						currentDir->parentDir->subDirs[i] = currentDir;
					}
				}
				currentDir = currentDir->parentDir;
				for (;;)
				{
					if (currentPath.back() == '/')
					{
						break;
					}
					currentPath.pop_back();
				}
				if (!currentDir->isRootDir)
				{
					currentPath.pop_back();
				}
			}
			return fileSystemSign::OPTION_COMPLETE;
		}
		else
		{
			int index = openDir(name);
			if (index == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
			{
				return fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND;
			}
			else
			{
				currentDir->subDirs[index]->parentDir = currentDir;
				currentDir = currentDir->subDirs[index];

				if (currentPath == "/")
				{
					currentPath += name;
				}
				else
				{
					currentPath += "/" + name;
				}
				return fileSystemSign::OPTION_COMPLETE;
			}
		}
	}

	void moveToRootDir()
	{
		for (;;)
		{
			if (currentDir->isRootDir)
			{
				break;
			}
			changeDir("..");
		}
		return;
	}

	int makeDir(std::string name, bool isProtected)
	{
		int index = openDir(name);
		if (index == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
		{
			dir* newDir = new dir;
			newDir->isProtected = isProtected;
			newDir->isRootDir = false;
			newDir->lastWriteTime = pushos::getCurrentTime();
			newDir->name = name;
			newDir->parentDir = currentDir;
			currentDir->subDirs.push_back(newDir);
			return fileSystemSign::OPTION_COMPLETE;
		}
		else
		{
			return fileSystemSign::FILE_OR_DIRECTORY_EXISTS;
		}
	}

	int removeDir(std::string name)
	{
		int index = openDir(name);
		if (index == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
		{
			return fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND;
		}
		else
		{
			if (currentDir->subDirs[index]->isProtected)
			{
				if (rootMode)
				{
					currentDir->subDirs.erase(currentDir->subDirs.begin() + index);
					return fileSystemSign::OPTION_COMPLETE;
				}
				else
				{
					return fileSystemSign::PERMISSION_DENIAL;
				}
			}
			else
			{
				currentDir->subDirs.erase(currentDir->subDirs.begin() + index);
				return fileSystemSign::OPTION_COMPLETE;
			}
		}
	}

	int createFile(std::string name, std::vector<std::string> content, bool isExeFile, bool isProtected, exe fileApplicationPart)
	{
		if (name == "." || name == ".." || name == "/" || name == "\\")
		{
			return fileSystemSign::TARGET_IS_A_DIRECTORY;
		}
		int index = openFile(name);
		if (index == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
		{
			file* newFile = new file;
			newFile->content = content;
			newFile->fileApplicationPart = fileApplicationPart;
			newFile->isExeFile = isExeFile;
			newFile->isProtected = isProtected;
			newFile->lastWriteTime = pushos::getCurrentTime();
			newFile->name = name;
			currentDir->subFiles.push_back(newFile);
			return fileSystemSign::OPTION_COMPLETE;
		}
		else
		{
			return fileSystemSign::FILE_OR_DIRECTORY_EXISTS;
		}
	}

	int removeFile(std::string name)
	{
		int index = openFile(name);
		if (index == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
		{
			return fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND;
		}
		else
		{
			if (currentDir->subFiles[index]->isProtected)
			{
				if (rootMode)
				{
					currentDir->subFiles.erase(currentDir->subFiles.begin() + index);
					return fileSystemSign::OPTION_COMPLETE;
				}
				else
				{
					return fileSystemSign::PERMISSION_DENIAL;
				}
			}
			else
			{
				currentDir->subFiles.erase(currentDir->subFiles.begin() + index);
				return fileSystemSign::OPTION_COMPLETE;
			}
		}
	}

	int readFile(std::string name)
	{
		int index = openFile(name);
		if (index == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
		{
			return fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND;
		}
		else
		{
			if (currentDir->subFiles[index]->isExeFile)
			{
				return fileSystemSign::FILE_TYPE_ERROR;
			}
			else
			{
				for (std::size_t i = 0; i < currentDir->subFiles[index]->content.size(); i++)
				{
					std::cout << currentDir->subFiles[index]->content[i] << std::endl;
				}
				return fileSystemSign::OPTION_COMPLETE;
			}
		}
	}

	int writeFile(std::string name)
	{
		int index = openFile(name);
		if (index == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
		{
			return fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND;
		}
		else
		{
			if (currentDir->subFiles[index]->isExeFile)
			{
				return fileSystemSign::FILE_TYPE_ERROR;
			}
			else if (currentDir->subFiles[index]->isProtected)
			{
				if (rootMode)
				{
					std::vector<std::string> newContent;
					for (;;)
					{
						std::string input;
						std::getline(std::cin, input);
						if (input.find('\x18') != std::string::npos)
						{
							std::string newLine;
							for (int i = 0; i < input.length(); i++)
							{
								if (input[i] == '\x18')
								{
									break;
								}
								newLine += input[i];
							}
							newContent.push_back(newLine);
							currentDir->subFiles[index]->content = newContent;
							currentDir->subFiles[index]->lastWriteTime = pushos::getCurrentTime();
							return fileSystemSign::OPTION_COMPLETE;
						}
						newContent.push_back(input);
					}
				}
				else
				{
					return fileSystemSign::PERMISSION_DENIAL;
				}
			}
			else
			{
				std::vector<std::string> newContent;
				for (;;)
				{
					std::string input;
					std::getline(std::cin, input);
					if (input.find('\x18') != std::string::npos)
					{
						std::string newLine;
						for (int i = 0; i < input.length(); i++)
						{
							if (input[i] == '\x18')
							{
								break;
							}
							newLine += input[i];
						}
						newContent.push_back(newLine);
						currentDir->subFiles[index]->content = newContent;
						currentDir->subFiles[index]->lastWriteTime = pushos::getCurrentTime();
						return fileSystemSign::OPTION_COMPLETE;
					}
					newContent.push_back(input);
				}
			}
		}
	}

	int runFile(std::string name, std::vector<std::string> args)
	{
		int index = openFile(name);
		if (index == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
		{
			return fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND;
		}
		else
		{
			if (currentDir->subFiles[index]->isExeFile)
			{
				currentDir->subFiles[index]->fileApplicationPart(args);
				return fileSystemSign::OPTION_COMPLETE;
			}
			else
			{
				return fileSystemSign::FILE_TYPE_ERROR;
			}
		}
	}

	int runFileOutside(dir* targetDir, std::string name, std::vector<std::string> args)
	{
		for (std::size_t i = 0; i < targetDir->subFiles.size(); i++)
		{
			if (targetDir->subFiles[i]->name == name)
			{
				if (targetDir->subFiles[i]->isExeFile)
				{
					targetDir->subFiles[i]->fileApplicationPart(args);
					return fileSystemSign::OPTION_COMPLETE;
				}
				else
				{
					return fileSystemSign::FILE_TYPE_ERROR;
				}
			}
		}
		return fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND;
	}

	void addStrToFile(std::string name, std::string content)
	{
		int index = openFile(name);
		if (index == fileSystemSign::FILE_OR_DIRECTORY_NOT_FOUND)
		{
			createFile(name, { content }, false, false, nullptr);
		}
		else
		{
			if (currentDir->subFiles[index]->isExeFile)
			{
				return;
			}
			else
			{
				currentDir->subFiles[index]->content.push_back(content);
				currentDir->subFiles[index]->lastWriteTime = pushos::getCurrentTime();
				return;
			}
		}
	}

	void listFiles(const char* resetColor = "", const char* protectedDirColor = "",
		const char* directoryColor = "", const char* executableColor = "")
	{
		std::cout << resetColor;
		for (std::size_t i = 0; i < currentDir->subDirs.size(); i++)
		{
			std::cout << currentDir->subDirs[i]->lastWriteTime << "    ";
			if (currentDir->subDirs[i]->isProtected)
			{
				std::cout << protectedDirColor << currentDir->subDirs[i]->name << " <DIR>" << resetColor;
				std::cout << std::endl;
			}
			else
			{
				std::cout << directoryColor << currentDir->subDirs[i]->name << " <DIR>" << resetColor;
				std::cout << std::endl;
			}
		}
		for (std::size_t i = 0; i < currentDir->subFiles.size(); i++)
		{
			std::cout << currentDir->subFiles[i]->lastWriteTime << "    ";
			if (currentDir->subFiles[i]->isExeFile)
			{
				std::cout << executableColor << currentDir->subFiles[i]->name << resetColor;
				std::cout << std::endl;
			}
			else
			{
				std::cout << resetColor << currentDir->subFiles[i]->name << resetColor;
				std::cout << std::endl;
			}
		}
	}

	void setRootMode(bool mode)
	{
		rootMode = mode;
	}
};
