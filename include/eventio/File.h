#ifndef EVENTIO_FILE_H
#define EVENTIO_FILE_H

#include <fstream>
#include <cstdint>
#include <bitset>
#include <memory>
#include <iostream>
#include <vector>
#include <cstddef>

#include "eventio/Object.h"



namespace eventio {

using bytes_t = std::vector<std::byte>;


class File {
    friend class Object;
    protected:
        std::fstream file;
        uint64_t next_header = 0;

    public:
        File(const std::string& filename);
        ~File();

        ObjectHeader read_header();
        bool has_next_toplevel();
        std::unique_ptr<Object> next_toplevel();
        std::unique_ptr<Object> read_object(bool toplevel);
        operator bool();
        bytes_t read(uint64_t bytes);

        void seekg(uint64_t pos);

        template<typename T> T read();
        template<typename T> void read(T& out);
        std::string read_string();
};


} /* namespace eventio */

#endif /* ifndef EVENTIO_FILE_H */
