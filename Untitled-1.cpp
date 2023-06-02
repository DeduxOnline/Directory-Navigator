#include <iostream>
#include <filesystem>
#include <string>
#include <locale>
#include <conio.h>
#include <vector>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <algorithm>

struct FileInfo {
    int num;
    std::wstring name;
    std::wstring path;
    std::wstring lastWriteTime;
    std::wstring type;
    std::wstring size;
};

void showNavigationInfo() {
    std::wcout << L" \033[3;33mF1:\033[0m help\t";
    std::wcout << L" \033[3;33mF2:\033[0m filter\t";
    std::wcout << L" \033[3;33mF3:\033[0m actions\t";
    std::wcout << L" \033[3;33mF4:\033[0m sort\t";
    std::wcout << L" \033[3;33mF5:\033[0m new direction\t";
    std::wcout << L" \033[3;33mEsc:\033[0m exit\n";
}

std::wstring formatFileTime(const std::filesystem::file_time_type& fileTime) {
    auto timePoint = std::chrono::time_point_cast<std::chrono::system_clock::duration>(fileTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    std::time_t t = std::chrono::system_clock::to_time_t(timePoint);
    std::tm localTime;
    localtime_s(&localTime, &t);
    std::wostringstream oss;
    oss << std::put_time(&localTime, L"%d-%m-%Y %H:%M");
    return oss.str();
}

void printFiles(const std::wstring& currentDirectory, const std::vector<FileInfo>& file_list) {
    std::wcout << L"\033[4;32mCurrent Directory:\033[0m " << currentDirectory << L"\n";
    std::cout << std::setiosflags(std::ios::left) << std::setw(3) << "\033[3;33mNum: \033[0m";
    std::cout << std::setiosflags(std::ios::left) << std::setw(80) << "\033[3;33mName:\033[0m";
    std::cout << std::setiosflags(std::ios::left) << std::setw(35) << "\033[3;33m Last write time:\033[0m";
    std::cout << std::setiosflags(std::ios::left) << std::setw(30) << "\033[3;33mType:\033[0m";
    std::cout << std::setiosflags(std::ios::left) << std::setw(15) << "\033[3;33mSize(KB):\033[0m";
    std::wcout << L"\n";
    for (const auto& file : file_list) {
        std::wcout << std::setiosflags(std::ios::left) << std::setw(3) << file.num << "| ";
        std::wcout << std::setiosflags(std::ios::left) << std::setw(70) << file.name;
        std::wcout << std::setiosflags(std::ios::left) << std::setw(23) << file.lastWriteTime;
        std::wcout << std::setiosflags(std::ios::left) << std::setw(20) << file.type;
        std::wcout << std::setiosflags(std::ios::left) << std::setw(10) << file.size;
        std::wcout << L"\n";
    }
    std::wcout << L"\n";
    showNavigationInfo();
}

bool filterByExtension(const std::wstring& extension, const std::wstring& filePath) {
    if (extension.empty())
        return true;
    std::filesystem::path path(filePath);
    std::wstring fileExtension = path.extension().wstring();
    return fileExtension == extension;
}

bool filterByType(const std::wstring& fileType, const std::wstring& filePath) {
    if (fileType.empty())
        return true;
    std::filesystem::path path(filePath);
    bool isDirectory = std::filesystem::is_directory(path);
    if (fileType == L"Folder")
        return isDirectory;
    else if (fileType == L"File")
        return !isDirectory;
    return true;
}

bool filterByName(const std::wstring& searchString, const std::wstring& filePath) {
    if (searchString.empty())
        return true;
    std::filesystem::path path(filePath);
    std::wstring fileName = path.filename().wstring();
    return fileName.find(searchString) != std::wstring::npos;
}

bool filterBySize(const std::wstring& fileSizeFilter, const std::wstring& filePath) {
    if (fileSizeFilter.empty())
        return true;
    std::filesystem::path path(filePath);
    if (!std::filesystem::is_regular_file(path))
        return true;
    std::wstring sizeOperator = fileSizeFilter.substr(0, 1);
    std::wstring sizeValue = fileSizeFilter.substr(1);
    std::uintmax_t fileSize = std::filesystem::file_size(path) / 1024;
    if (sizeOperator == L"<")
        return fileSize < std::stoull(sizeValue);
    else if (sizeOperator == L">")
        return fileSize > std::stoull(sizeValue);
    return true;
}

bool filterByDate(const std::wstring& dateFilter, const std::wstring& filePath) {
    if (dateFilter.empty())
        return true;
    std::filesystem::path path(filePath);
    std::filesystem::file_time_type lastWriteTime = std::filesystem::last_write_time(path);
    std::wstring formattedTime = formatFileTime(lastWriteTime);
    return formattedTime == dateFilter;
}

void updateNumbers(std::vector<FileInfo>& file_list) {
    for (int i = 0; i < file_list.size(); i++) {
        file_list[i].num = i + 1;
    }
}

void directionInfoGet(std::wstring& path, std::vector<FileInfo>& file_list, bool update = false) {
    do {
        if (update) {
            file_list.clear(); 
        }
        else {
            std::wcout << L"\033[5;36m Enter directory path: \033[0m";
            std::getline(std::wcin, path);
        }
        if (!std::filesystem::is_directory(path)) {
            std::wcout << L"\033[5;31m Invalid directory path\033[0m\n";
        }
        else {
            int num = 1;
            file_list.clear();
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                FileInfo fileInfo;
                fileInfo.num = num;
                fileInfo.name = entry.path().filename().wstring();
                fileInfo.lastWriteTime = formatFileTime(std::filesystem::last_write_time(entry.path()));
                fileInfo.type = std::filesystem::is_directory(entry.path()) ? L"Folder" : entry.path().extension().wstring();
                fileInfo.size = std::filesystem::is_directory(entry.path()) ? L"<DIR>" : std::to_wstring(std::filesystem::file_size(entry.path()) / 1024);
                fileInfo.path = entry.path().wstring();
                file_list.push_back(fileInfo);
                num++;
            }
            updateNumbers(file_list);
            printFiles(path, file_list);
            break;
        }
    } while (true);
}

void performAction(const std::wstring& action, const std::vector<int>& fileNumbers, const std::vector<FileInfo>& file_list) {
    if (action == L"delete") {
        std::wcout << L"\033[4;32mStar\033[0m\n";
        for (int number : fileNumbers) {
            if (number >= 1 && number <= file_list.size()) {
                const FileInfo& file = file_list[number - 1];
                std::filesystem::remove(file.path);
               std::wcout << L" File \033[3;33m\"" << file.name << L"\"\033[0m deleted \033[3;32msuccessfully.\033[0m\n";
            }
        }
    }
    else if (action == L"rename") {
        std::wstring newName;
        std::wstring extensionChoice;
        std::wcout << L"\033[5;36m Add file extension to the new name? (Y/N): \033[0m";
        std::getline(std::wcin, extensionChoice);
        bool addExtension = (extensionChoice == L"Y" || extensionChoice == L"y");
        std::wcout << L"\033[5;36m Enter new name: \033[0m";
        std::getline(std::wcin, newName);
        std::wcout << L"\033[4;32mStar\033[0m\n";
        if (fileNumbers.size() > 1) {
            int counter = 1;
            for (int number : fileNumbers) {
                if (number >= 1 && number <= file_list.size()) {
                    const FileInfo& file = file_list[number - 1];
                    std::filesystem::path filePath(file.path);
                    if (std::filesystem::exists(filePath) && filePath.has_parent_path()) {
                        std::wstring finalName = std::to_wstring(counter) + L"_" + newName;
                        if (addExtension) {
                            std::wstring extension = filePath.extension();
                            finalName += extension;
                        }
                        std::filesystem::rename(filePath, filePath.parent_path() / finalName);
                        std::wcout << L" File \033[3;33m\"" << filePath.filename().wstring() << L"\"\033[0m renamed \033[3;32msuccessfully.\033[0m\n";
                        counter++;
                    }
                    else {
                        std::wcout << L"\033[2;31m Invalid file path.\033[0m\n";
                    }
                }
            }
        }
        else if (fileNumbers.size() == 1) {
            int number = fileNumbers[0];
            if (number >= 1 && number <= file_list.size()) {
                const FileInfo& file = file_list[number - 1];
                std::filesystem::path filePath(file.path);
                if (std::filesystem::exists(filePath) && filePath.has_parent_path()) {
                    std::wstring finalName = newName;
                    if (addExtension) {
                        std::wstring extension = filePath.extension();
                        finalName += extension;
                    }
                    std::filesystem::rename(filePath, filePath.parent_path() / finalName);
                    std::wcout << L" File \033[3;33m\"" << filePath.filename().wstring() << L"\"\033[0m renamed \033[3;32msuccessfully.\033[0m\n";
                }
                else {
                    std::wcout << L"\033[2;31m Invalid file path.\033[0m\n";
                }
            }
        }
    }
    else if (action == L"copy") {
        std::wstring destination;
        std::wcout << L"\033[5;36mEnter destination path: \033[0m";
        std::getline(std::wcin, destination);
        std::wcout << L"\033[4;32mStar\033[0m\n";
        for (int number : fileNumbers) {
            if (number >= 1 && number <= file_list.size()) {
                const FileInfo& file = file_list[number - 1];
                std::filesystem::path sourcePath(file.path);
                std::filesystem::path destinationPath(destination);
                std::wstring baseName = sourcePath.stem().wstring();
                std::wstring extension = sourcePath.extension().wstring();
                int copyNumber = 1;
                std::wstring copySuffix = L"-(copy)";

                while (std::filesystem::exists(destinationPath / (baseName + copySuffix + extension))) {
                    copyNumber++;
                    copySuffix = L"-(copy" + std::to_wstring(copyNumber) + L")";
                }

                baseName += copySuffix;
                std::filesystem::path finalPath = destinationPath / (baseName + extension);
                std::filesystem::copy(sourcePath, finalPath);
                std::wcout << L"File \033[3;33m\"" << sourcePath.filename().wstring() << L"\"\033[0m copied \033[3;32msuccessfully.\033[0m\n";
            }
        }

    }
    else if (action == L"move") {
        std::wstring destination;
        std::wcout << L"\033[5;36m Enter destination path: \033[0m";
        std::getline(std::wcin, destination);
        std::wcout << L"\033[4;32mStar\033[0m\n";
        for (int number : fileNumbers) {
            if (number >= 1 && number <= file_list.size()) {
                const FileInfo& file = file_list[number - 1];
                std::filesystem::path sourcePath(file.path);
                std::filesystem::path destinationPath(destination);
                std::wstring baseName = sourcePath.stem().wstring();
                std::wstring extension = sourcePath.extension().wstring();
                if (std::filesystem::exists(destinationPath / (baseName + extension))) {
                    baseName += L"-(copy)";
                }
                std::filesystem::path finalPath = destinationPath / (baseName + extension);
                std::filesystem::rename(sourcePath, finalPath);
                std::wcout << L" File \033[3;33m\"" << sourcePath.filename().wstring() << L"\"\033[0m moved \033[3;32msuccessfully.\033[0m\n";
            }
        }
    }
    std::wcout << L"\033[4;32mFinish\033[0m\n\n";
}

std::vector<int> getFileNumbers(const std::wstring& fileNumbersStr, int fileListSize) {
    std::vector<int> fileNumbers;
    std::vector<int> invalidNumbers;
    if (fileNumbersStr.empty()) {
        for (int i = 1; i <= fileListSize; ++i) {
            fileNumbers.push_back(i);
        }
    }
    else {
        std::wstringstream ss(fileNumbersStr);
        int num;
        while (ss >> num) {
            if (ss.peek() == L'-') {
                ss.ignore();
                int rangeEnd;
                ss >> rangeEnd;
                for (int i = num + 1; i <= rangeEnd; ++i) {
                    if (i < 1 || i > fileListSize)
                        invalidNumbers.push_back(i);
                    else
                        fileNumbers.push_back(i);
                }
            }
            if (num < 1 || num > fileListSize) 
                invalidNumbers.push_back(num);
            else
                fileNumbers.push_back(num);
            if (ss.peek() == L' ')
                ss.ignore();
        }
    }
    if (!invalidNumbers.empty()) {
        std::cout << "\033[2;31m Skip files with the following numbers that do not exist:\033[0m ";
        for (const auto& num : invalidNumbers) {
            std::cout << num << " ";
        }
        std::cout << std::endl;
    }
    return fileNumbers;
}

void sortFiles(std::vector<FileInfo>& file_list, const std::wstring& sortOption, bool descending) {
    std::sort(file_list.begin(), file_list.end(), [&](const FileInfo& a, const FileInfo& b) {
        if (sortOption == L"Name") {
            if (descending)
                return a.name > b.name;
            else
                return a.name < b.name;
        }
        else if (sortOption == L"Creation Date") {
            if (descending)
                return a.lastWriteTime > b.lastWriteTime;
            else
                return a.lastWriteTime < b.lastWriteTime;
        }
        else if (sortOption == L"File Type") {
            if (descending)
                return a.type > b.type;
            else
                return a.type < b.type;
        }
        else if (sortOption == L"File Size") {
            if (descending)
                return a.size > b.size;
            else
                return a.size < b.size;
        }
        return false;
        });
}

void printProgramDescription() {
    std::wcout << L"\033[4;33mDirectory Navigator\033[0m\n";
    std::wcout << L"Version: 1.5.0\n";
    std::wcout << L"Author GitHub: https://github.com/DeduxOnline\n";
    std::wcout << L"Last Updated: 21.05.2023\n\n";
    std::wcout << L"Welcome! This program provides information about files and folders in a specified directory.\n";
    std::wcout << L"It has the following functionality:\n";
    std::wcout << L"1. View the contents of a directory and display information about files and folders.\n";
    std::wcout << L"2. Filter the list of files and folders based on certain criteria (extension, type, name, size, creation date).\n";
    std::wcout << L"3. Perform actions on selected files and folders (delete, rename, copy, move).\n";
    std::wcout << L"4. Sort the list of files and folders by name, creation date, type, and size in ascending or descending order.\n";
    std::wcout << L"5. Provide help and display instructions on navigation and usage of the program.\n";
    std::wcout << L"6. Update the directory content to refresh the list of files and folders.\n";
    std::wcout << L"7. Exit the program.\n\n";
    std::wcout << L"To interact with the program, use the function keys:\n";
    std::wcout << L"\033[3;33mF1\033[0m - Display help and instructions on navigation.\n";
    std::wcout << L"\033[3;33mF2\033[0m - Filter the list of files and folders.\n";
    std::wcout << L"\033[3;33mF3\033[0m - Perform actions on selected files and folders.\n";
    std::wcout << L"\033[3;33mF4\033[0m - Sort the list of files and folders.\n";
    std::wcout << L"\033[3;33mF5\033[0m - Update the directory content.\n";
    std::wcout << L"\033[3;33mEsc\033[0m - Exit the program.\n\n";
}

int main() {
    std::locale::global(std::locale("uk_UA.UTF-8"));
    std::wstring path;
    std::vector<FileInfo> file_list;
    char ch;
    std::wcout << L"\033[4;33mDirectory Navigator\033[0m\n";
    directionInfoGet(path, file_list);
    do {
        ch = _getch();
        if (ch == 0x3B) { // F1 key
            std::wcout << L"\n\033[4;33mHelp\033[0m\n";
            printProgramDescription();
        }
        if (ch == 0x3C) { // F2 key
            std::wcout << L"\n\033[4;33mFilters\033[0m\n";
            std::wstring extension;
            std::wstring fileType;
            std::wstring searchString;
            std::wstring fileSizeFilter;
            std::wstring dateFilter;
            std::wcout << L"\033[5;36m Enter extension to filter by (leave blank for no filter):\033[0m ";
            std::getline(std::wcin, extension);
            std::wcout << L"\033[5;36m Enter file type to filter by (Folder / File, leave blank for no filter):\033[0m ";
            std::getline(std::wcin, fileType);
            std::wcout << L"\033[5;36m Enter search string to filter by (leave blank for no filter):\033[0m ";
            std::getline(std::wcin, searchString);
            std::wcout << L"\033[5;36m Enter file size filter (<size, >size, leave blank for no filter):\033[0m ";
            std::getline(std::wcin, fileSizeFilter);
            std::wcout << L"\033[5;36m Enter last write time filter (dd-mm-yyyy HH:MM, leave blank for no filter):\033[0m ";
            std::getline(std::wcin, dateFilter);
            file_list.erase(std::remove_if(file_list.begin(), file_list.end(), [&](const FileInfo& fileInfo) {
                return !filterByExtension(extension, fileInfo.path) ||
                    !filterByType(fileType, fileInfo.path) ||
                    !filterByName(searchString, fileInfo.path) ||
                    !filterBySize(fileSizeFilter, fileInfo.path) ||
                    !filterByDate(dateFilter, fileInfo.path);
                }), file_list.end());
            updateNumbers(file_list);
            std::wcout << L"\033[4;32mFilters applied successfully\033[0m \n\n";
            printFiles(path, file_list);
        }
        if (ch == 0x3D) { // F3 key
            std::wcout << L"\n\033[4;33mActions\033[0m\n";
            char ch;
            std::wstring action;
            do {
                std::wcout << L"\033[5;36m Press action button\t\033[3;33m 1:\033[0m delete\t\033[3;33m 2:\033[0m rename\t\033[3;33m 3:\033[0m copy\t\033[3;33m 3:\033[0m move\n";
                ch = _getch();
                switch (ch) {
                case '1':
                    action = L"delete";
                    std::wcout << L"\033[4;33mAction: Delete\033[0m\n";
                    break;
                case '2':
                    action = L"rename";
                    std::wcout << L"\033[4;33mAction: Rename\033[0m\n";
                    break;
                case '3':
                    action = L"copy";
                    std::wcout << L"\033[4;33mAction: Copy\033[0m\n";
                    break;
                case '4':
                    action = L"move";
                    std::wcout << L"\033[4;33mAction: Copy\033[0m\n";
                    break;
                default:
                    std::wcout << L"\033[2;31m Invalid action.\033[0m\n";
                    continue;
                }
                break;
            } while (true);
            std::wstring fileNumbersStr;
            std::wcout << L"\033[5;36m Enter file number(s) (space-separated), or leave empty for all files, or specify ranges using hyphen (-): \033[0m";
            std::getline(std::wcin, fileNumbersStr);
            std::vector<int> fileNumbers = getFileNumbers(fileNumbersStr, file_list.size());
            performAction(action, fileNumbers, file_list);
            directionInfoGet(path, file_list, true);
        }
        if (ch == 0x3E) { // F4 key 
            std::wcout << L"\n\033[4;33mSort\033[0m\n";
            std::wstring sortOption;
            do {
                std::wcout << L"\033[5;36m Press action button (1 Name / 2 Creation Date / 3 File Type / 4 File Size): \033[0m";
                ch = _getch();
                switch (ch) {
                case '1':
                    sortOption = L"Name";
                    std::wcout << L"Name\n";
                    break;
                case '2':
                    sortOption = L"Creation Date";
                    std::wcout << L"Creation Date\n";
                    break;
                case '3':
                    sortOption = L"File Type";
                    std::wcout << L"File Type\n";
                    break;
                case '4':
                    sortOption = L"File Size";
                    std::wcout << L"File Size\n";
                    break;
                default:
                    std::wcout << L"\033[2;31m Invalid sort option.\033[0m\n";
                    continue;
                }
                break;
            } while (true);
            bool descending;
            std::wstring sortOrder;
            do {
                std::wcout << L"\033[5;36m Press action button (1 Ascending / 2 Descending): \033[0m";
                ch = _getch();
                switch (ch) {
                case '1':
                    descending = false;
                    sortOrder = L"Ascending";
                    std::wcout << L"Ascending\n";
                    break;
                case '2':
                    descending = true;
                    sortOrder = L"Descending";
                    std::wcout << L"Descending\n";
                    break;
                default:
                    std::wcout << L"\033[2;31m Invalid sort order.\033[0m\n";
                    continue;
                }
                break;
            } while (true);
            if (sortOrder == L"Descending")
                descending = true;
            else
                descending = false;
            sortFiles(file_list, sortOption, descending);
            updateNumbers(file_list);
            std::wcout << L"\033[4;32mFiles sorted successfully\033[0m \n\n";
            printFiles(path, file_list);
        }
        if (ch == 0x3F) { // F5 key
            directionInfoGet(path, file_list);
        }
    } while (ch != 0x1B); // Esc key
    return 0;
}
