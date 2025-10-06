#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <3ds.h>

inline std::string toLower(const std::string &str) {
    std::string result = str;
    std::transform(
        result.begin(), result.end(), result.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return result;
}

inline std::string getParentPath(std::string path) {
    path.pop_back();
    const size_t slash = path.find_last_of("/");
    return path.substr(0, slash) + "/";
}

std::vector<std::string> loadDirectory(const std::string& path, FS_Archive& sdmcArchive) {
    Handle dirHandle;
    FSUSER_OpenDirectory(&dirHandle, sdmcArchive, fsMakePath(PATH_ASCII, path.c_str()));
    u32 entriesRead = 1;
    int dirCount = 0;
    FS_DirectoryEntry entry;

    std::vector<std::string> files;
    files.reserve(40);
    while(entriesRead) {
        entriesRead = 0;
        FSDIR_Read(dirHandle, &entriesRead, 1, &entry);

        if(!entriesRead)
            break;
            
        if(entry.attributes & FS_ATTRIBUTE_HIDDEN)
            continue;

        const bool isDirectory = entry.attributes & FS_ATTRIBUTE_DIRECTORY;
        
        char name[262] = {0};
        utf16_to_utf8((uint8_t*)name, entry.name, 262);
        if(isDirectory) {
            std::string file = name;
            file += "/";
            files.insert(files.begin() + dirCount, file);
            dirCount++;
        } else {
            files.emplace_back(name);
        }
    }
    FSDIR_Close(dirHandle);
    svcCloseHandle(dirHandle);

    std::sort(files.begin(), files.end(), [](const std::string& a, const std::string& b) {
        const std::string aConverted = toLower(a);
        const std::string bConverted = toLower(b);
        if(aConverted.back() == '/' && bConverted.back() != '/')
            return true;
        if(bConverted.back() == '/' && aConverted.back() != '/')
            return false;

        return aConverted<bConverted;
    });

    return files;
}

std::deque<C2D_Text> loadDirList(std::vector<std::string>& files, C2D_Font& font, C2D_TextBuf& textBuf) {
    std::deque<C2D_Text> fileText;
    const size_t count = std::min<size_t>(18, files.size());
    for(size_t i = 0; i < count; i++) {
        C2D_Text text;
        C2D_TextFontParse(&text, font, textBuf, files.at(i).c_str());
        C2D_TextOptimize(&text);
        fileText.push_back(text);
    }
    return fileText;
}
