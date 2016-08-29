#ifndef MOTOR_H_FS_MAPPEDFILE_HPP
#define MOTOR_H_FS_MAPPEDFILE_HPP

#include "platform.h"
#include <string>
#include <vector>
#include "util/exceptions.hpp"

namespace Motor{
    class MappedFile{
        void MapView(DWORD access);

    public:
        enum class Access{
            Read,
            ReadWrite
        };

        MappedFile();
        MappedFile( const std::string & fileName, const std::vector<std::string> & include_paths, MappedFile::Access access = Access::Read);
        MappedFile( const std::string & fileName, MappedFile::Access access = Access::Read);
        MappedFile(const MappedFile & other);
        MappedFile(MappedFile && other);
        MappedFile& operator=(const MappedFile & other);
        MappedFile& operator=(MappedFile && other);
        ~MappedFile();
        void Release();
        size_t size() const;
        char& operator[](size_t idx);
        char& at(size_t idx) const;
        const std::string & name() const;

    private:
        HANDLE file;
        HANDLE mapping;
        void * view;
        size_t file_size;
        DWORD desiredaccess;
        std::string full_name;
    };
}

#endif
