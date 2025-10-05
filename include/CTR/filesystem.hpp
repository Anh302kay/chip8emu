#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <3ds.h>

struct file {
    char name[262];
    u8 flag;
};

std::vector<std::string> loadDirectory(const std::string& path, FS_Archive& sdmcArchive) {
    Handle dirHandle;
    FSUSER_OpenDirectory(&dirHandle, sdmcArchive, fsMakePath(PATH_ASCII, path.c_str()));
    u32 entriesRead = 1;
    FS_DirectoryEntry entry;

    std::vector<std::string> files;
    files.reserve(20);
    while(entriesRead) {
        entriesRead = 0;
        FSDIR_Read(dirHandle, &entriesRead, 1, &entry);

        if(!entriesRead)
            break;
            
        if(entry.attributes & FS_ATTRIBUTE_HIDDEN)
            continue;
        
        char name[262] = {0};
        utf16_to_utf8((uint8_t*)name, entry.name, 262);
        files.emplace_back(name);
    }
    FSDIR_Close(dirHandle);
    svcCloseHandle(dirHandle);
    return files;
}

std::string getParentPath(std::string path) {
    path.pop_back();
    const size_t slash = path.find_last_of("/");
    return path.substr(0, slash) + "/";
}
