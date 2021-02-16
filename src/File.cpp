#include "eventio/File.h"
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <array>

namespace eventio {

std::unique_ptr<io::filtering_istream> open_file(const std::string& path) {
    auto stream = std::make_unique<io::filtering_istream>();

    std::fstream file(path, std::fstream::in | std::fstream::binary);

    std::array<std::byte, 4> magic;
    file.read(reinterpret_cast<char*>(&magic.front()), magic.size());

    if (magic[0] == GZIP_MAGIC[0] && magic[1] == GZIP_MAGIC[1]) {
        std::cout << "Found gzip compressed file" << std::endl;
        stream->push(io::gzip_decompressor());
    } else if (magic == ZSTD_MAGIC) {
        std::cout << "Found zstd compressed file" << std::endl;
        stream->push(io::zstd_decompressor());
    } else {
        std::cout << "No known compression found, assuming uncompressed" << std::endl;
    }

    stream->push(io::file_source(path));
    return stream;
}


File::File(std::istream* stream) {
    stream_ = std::make_unique<io::filtering_istream>();
    stream_->push(*stream);
}
File::File(io::filtering_istream* stream) : stream_(stream) {}

File::File(const std::string& path) {
    stream_ = open_file(path);

    if (!stream_->good()) {
        throw std::invalid_argument("Could not open file '" + path + "'");
    }
}

File::~File() {
}


template<typename T>
void File::read(T& out) {
    stream_->read(reinterpret_cast<char*>(&out), sizeof(out));
    position_ += sizeof(out);
}

template<typename T>
T File::read() {
    T var = 0;
    read<T>(var);
    return var;
}

// explicitly compile all needed readers
// so they are guaranteed to be available
template uint8_t File::read<uint8_t>();
template uint16_t File::read<uint16_t>();
template uint32_t File::read<uint32_t>();
template uint64_t File::read<uint64_t>();
template int8_t File::read<int8_t>();
template int16_t File::read<int16_t>();
template int32_t File::read<int32_t>();
template int64_t File::read<int64_t>();
template float File::read<float>();
template double File::read<double>();

ObjectHeader File::read_header() {
    ObjectHeader header;

    size_t header_size = sizeof(header) - sizeof(header.extension_field);
    stream_->read(reinterpret_cast<char*>(&header), header_size);
    position_ += header_size;

    if (header.is_extended()) {
        read<uint32_t>(header.extension_field);
    }

    return header;
}

bool File::has_next_toplevel() {
    if (!stream_->good()) return false;

    // need to go to the next expected toplevel header position
    seekg(next_header_);
    // peek to see if we have bytes left, will change internal
    // state of file if not.
    stream_->peek();

    return stream_->good();
}

std::unique_ptr<Object> File::next_toplevel() {
    if (!has_next_toplevel()) {
        throw std::runtime_error("EOF");
    }
    return read_object(true);
}

std::unique_ptr<Object> File::read_object(bool toplevel) {
    if (toplevel) {
        auto marker = read<uint32_t>();
        if (marker != SYNC_MARKER) {
            throw std::runtime_error("Marker does not match: " + std::to_string(marker));
        }
    }

    ObjectHeader header = read_header();
    uint64_t address = tellg();

    if (toplevel) {
        next_header_ = address + header.size();
    }

    switch (header.type()) {
        case History::type: return std::make_unique<History>(address, header);
        case CommandLine::type: return std::make_unique<CommandLine>(address, header);
        case ConfigLine::type: return std::make_unique<ConfigLine>(address, header);
    }

    return std::make_unique<Object>(address, header);
}

File::operator bool() {
    return stream_->good();
}

bytes_t File::read(uint64_t bytes) {
    bytes_t s(bytes);
    stream_->read(reinterpret_cast<char*>(&s.front()), bytes);

    // check how many bytes we actually read
    uint64_t n_read = stream_->gcount();
    s.resize(n_read);

    // advance position
    position_ += n_read;

    return s;
}

void File::seekg(uint64_t pos) {
    if (pos < position_) {
        throw std::invalid_argument("Cannot seek back");
    }
    stream_->ignore(pos - position_);
    position_ = pos;
}

uint64_t File::tellg() const {
    return position_;
}


std::string File::read_string() {
    auto size = read<uint16_t>();

    std::string s(size, '\0');
    stream_->read(&s.front(), size);

    uint64_t n_read = stream_->gcount();
    position_ += n_read;
    s.resize(n_read);
    return s;
}


} /* namespace eventio */
