// Copyright (c) 2024 ZirconiumTian
// SPDX-License-Identifier: MIT

/**
 * @file       main.cpp
 * @brief      Entry point for PushOS.
 * @author     ZirconiumTian
 * @version    0.1.0
 */

#include "pushos/pushos.hpp"

#include <iostream>

int main() {
	using namespace pushos;
	Device device;
	device.ip_innet = "192.168.1.1";
	device.ip_public = "67.67.67.67";

	UserFsctl fsctl(&device.storage_directory);
	fsctl.user_name = "root";
	const auto status = executables::pushos(&fsctl, {"pushos", "--install"});
	if (status != 0) return static_cast<int>(status);

	auto *shell = device.fsctl_device.getFileByPath("/bin/push");
	if (shell == nullptr or shell->executable_part == nullptr) {
		std::cerr << "pushos: cannot start /bin/push.\n";
		return 1;
	}
	return static_cast<int>(shell->execute(&fsctl, {"push"}));
}
