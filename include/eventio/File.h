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

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zstd.hpp>
#include <boost/iostreams/device/file.hpp>


namespace eventio {

namespace io = boost::iostreams;

using bytes_t = std::vector<std::byte>;


const std::array<std::byte, 2> GZIP_MAGIC = {
    std::byte{0x1f},
    std::byte{0x8b}
};

const std::array<std::byte, 4> ZSTD_MAGIC = {
    std::byte{0x28},
    std::byte{0xb5},
    std::byte{0x2f},
    std::byte{0xfd}
};


std::unique_ptr<io::filtering_istream> open_file(const std::string& path);


class File {
    friend class Object;
    protected:
        std::unique_ptr<io::filtering_istream> stream_;
        uint64_t next_header_ = 0;
        uint64_t position_ = 0;

    public:
        File(std::istream* stream);
        File(io::filtering_istream* stream);
        File(const std::string& filename);
        ~File();

        ObjectHeader read_header();
        bool has_next_toplevel();
        std::unique_ptr<Object> next_toplevel();
        std::unique_ptr<Object> read_object(bool toplevel);
        operator bool();
        bytes_t read(uint64_t bytes);

        void seekg(uint64_t pos);
        uint64_t tellg() const;

        // readers for numerical types
        template<typename T> void read(T& out);
        template<typename T> T read();

        // eventio string, integer length + string data
        std::string read_string();
};


} /* namespace eventio */

#endif /* ifndef EVENTIO_FILE_H */
