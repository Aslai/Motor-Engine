#include "platform.h"
#include "mappedfile.hpp"
#include <string>
#include <vector>
#include "util/exceptions.hpp"

namespace Motor{
    void MappedFile::MapView(DWORD access){
        desiredaccess = access;
        view = MapViewOfFile(mapping, desiredaccess, 0, 0, 0);
        if (!view){
            throw Exception::Error("Failed to map view of file");
        }
    }


    MappedFile::MappedFile(){
        file = NULL;
        mapping = NULL;
        view = nullptr;
        desiredaccess = NULL;
        file_size = 0;
    }

    MappedFile::MappedFile( const std::string & fileName, const std::vector<std::string> & include_paths, MappedFile::Access access ){
        DWORD waccess = NULL, wprotect = NULL;
        desiredaccess = NULL;
        if (access == Access::Read){
            waccess = GENERIC_READ;
            wprotect = PAGE_READONLY;
            desiredaccess = FILE_MAP_READ;
        }
        else if (access == Access::ReadWrite){
            waccess = GENERIC_WRITE | GENERIC_READ;
            wprotect = PAGE_READWRITE;
            desiredaccess = FILE_MAP_ALL_ACCESS;
        }
        std::wstring wfileName;
        file = INVALID_HANDLE_VALUE;
        for (const auto & path : include_paths){
            wfileName = TStrFromUTF8(path);
            if (wfileName.back() != '\\' && wfileName.back() != '/'){
                wfileName += '\\';
            }
            wfileName += TStrFromUTF8(fileName);
            file = CreateFile(wfileName.c_str(), waccess, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (file != INVALID_HANDLE_VALUE){
                break;
            }
        }
        if (file == INVALID_HANDLE_VALUE){
            wfileName = TStrFromUTF8(fileName);
            file = CreateFile(wfileName.c_str(), waccess, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        }
        if (file == INVALID_HANDLE_VALUE){
            throw Exception::NotFound(fileName);
        }
        mapping = CreateFileMapping(file, NULL, wprotect, 0, 0, NULL);
        if (!mapping){
            DWORD err = GetLastError();
            throw Exception::Error("Failed to create file mapping");
        }
        DWORD size_high;
        DWORD size_low = GetFileSize(file, &size_high);
        file_size = size_high;
        file_size <<= sizeof(DWORD);
        file_size += size_low;

        TCHAR buffer[1000];
        GetFinalPathNameByHandle(file, buffer, 999, VOLUME_NAME_DOS);
        full_name = TStrToUTF8(buffer);

        MapView(desiredaccess);

    }

    MappedFile::MappedFile( const std::string & fileName, MappedFile::Access access) :
        MappedFile(fileName, std::vector<std::string>(), access){

    }

    MappedFile::MappedFile(const MappedFile & other){
        operator=(other);
    }

    MappedFile::MappedFile(MappedFile && other){
        operator=(other);
    }

    MappedFile& MappedFile::operator=(const MappedFile & other){
        if (!DuplicateHandle(GetCurrentProcess(), other.file, GetCurrentProcess(), &file, 0, FALSE, DUPLICATE_SAME_ACCESS)){
            throw Exception::Error("Failed to dup file handle");
        }
        if (!DuplicateHandle(GetCurrentProcess(), other.mapping, GetCurrentProcess(), &mapping, 0, FALSE, DUPLICATE_SAME_ACCESS)){
            throw Exception::Error("Failed to dup file mapping");
        }
        MapView(other.desiredaccess);
        file_size = other.file_size;
        full_name = other.full_name;
        return *this;
    }

    MappedFile& MappedFile::operator=(MappedFile && other){
        this->desiredaccess = other.desiredaccess;
        this->file = other.file;
        this->mapping = other.mapping;
        this->view = other.view;
        this->file_size = other.file_size;
        other.file = INVALID_HANDLE_VALUE;
        other.mapping = NULL;
        other.view = nullptr;
        full_name = std::move(other.full_name);
        return *this;
    }

    MappedFile::~MappedFile(){
        if (view){
            UnmapViewOfFile(view);
        }
        if (mapping){
            CloseHandle(mapping);
        }
        if (file != INVALID_HANDLE_VALUE){
            CloseHandle(file);
        }
    }

    size_t MappedFile::size() const{
        return file_size;
    }

    char& MappedFile::operator[](size_t idx){
        return ((char*)view)[idx];
    }

    char& MappedFile::at(size_t idx) const{
        return ((char*)view)[idx];
    }

    const std::string & MappedFile::name() const{
        return full_name;
    }
}
