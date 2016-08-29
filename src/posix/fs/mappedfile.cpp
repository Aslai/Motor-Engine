#include "platform.hpp"
#include "mappedfile.hpp"
#include <string>
#include <vector>
#include "util/exceptions.hpp"
extern "C"{
    #include <stdlib.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <fcntl.h>
    #include <sys/mman.h>
    #include <stdlib.h>
    #include <limits.h>
    #include <unistd.h>
}

namespace Motor{
    void MappedFile::MapView(int access){
        desiredaccess = access;
        view = mmap(0, file_size, access, MAP_SHARED, file, 0 );
        if (view == (void*) -1 ){
            throw Exception::ErrNo("Couldn't map file");
        }
    }


    MappedFile::MappedFile(){
        file = -1;
        view = nullptr;
        desiredaccess = 0;
        file_size = 0;
    }

    MappedFile::MappedFile( const std::string & fileName, const std::vector<std::string> & include_paths, MappedFile::Access access ){
        int faccess = 0;
        desiredaccess = 0;
        if (access == Access::Read){
            faccess = O_RDONLY;
            desiredaccess = PROT_READ;
        }
        else if (access == Access::ReadWrite){
            faccess = O_RDWR;
            desiredaccess = PROT_READ | PROT_WRITE;
        }
        std::string testFileName;
        file = -1;
        for (const auto & path : include_paths){
            testFileName = path;
            if (testFileName.back() != '\\' && testFileName.back() != '/'){
                testFileName += '/';
            }
            testFileName += fileName;
            file = open( testFileName.c_str(), faccess, 0 );
            if ( file != -1 ){
                break;
            }
        }
        if (file == -1){
            testFileName = fileName;
            file = open(testFileName.c_str(), faccess, 0 );
        }
        if (file == -1){
            throw Exception::ErrNo("Could not open " + fileName);
        }

        struct stat st;
        fstat(file, &st);
        file_size = st.st_size;

        char buffer[PATH_MAX];
        if( realpath(testFileName.c_str(), buffer) == nullptr ){
            throw Exception::ErrNo();
        }

        full_name = buffer;

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
        file = dup( other.file );
        if( file == -1 ){
            throw Exception::ErrNo("Couldn't dup file descriptor");
        }
        file_size = other.file_size;
        full_name = other.full_name;
        MapView(other.desiredaccess);
        return *this;
    }

    MappedFile& MappedFile::operator=(MappedFile && other){
        this->desiredaccess = other.desiredaccess;
        this->file = other.file;
        this->view = other.view;
        this->file_size = other.file_size;
        other.file = -1;
        other.view = nullptr;
        full_name = std::move(other.full_name);
        return *this;
    }

    MappedFile::~MappedFile(){
        Release();
    }

    void MappedFile::Release(){
        if (view){
            munmap(view, file_size);
        }
        if (file != -1){
            close(file);
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
