#include "eventio/File.h"
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <array>

namespace eventio {

File::File(const std::string& filename) {
    file.open(filename, std::fstream::in | std::fstream::binary);
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.unsetf(std::ios::skipws);

    if (!file) {
        throw std::invalid_argument("Could not open file '" + filename + "'");
    }
}

File::~File() {
    file.close();
}


template<typename T>
void File::read(T& out) {
    file.read(reinterpret_cast<char*>(&out), sizeof(out));
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
    file.read(reinterpret_cast<char*>(&header), header_size);

    if (header.is_extended()) {
        read<uint32_t>(header.extension_field);
    }

    return header;
}

bool File::has_next_toplevel() {
    if (!file.good()) return false;

    // need to go to the next expected toplevel header position
    file.seekg(next_header);
    // peek to see if we have bytes left, will change internal
    // state of file if not.
    file.peek();

    return file.good();
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
    uint64_t address = file.tellg();

    if (toplevel) {
        next_header = address + header.size();
    }

    switch (header.type()) {
        case History::type: return std::make_unique<History>(address, header);
        case CommandLine::type: return std::make_unique<CommandLine>(address, header);
        case ConfigLine::type: return std::make_unique<ConfigLine>(address, header);
    }

    return std::make_unique<Object>(address, header);
}

File::operator bool() {
    return file.is_open() && !file.eof() && !file.fail();
}

bytes_t File::read(uint64_t bytes) {
    bytes_t s(bytes);
    file.read(reinterpret_cast<char*>(&s.front()), bytes);
    s.resize(file.gcount());
    return s;
}

void File::seekg(uint64_t pos) {
    file.seekg(pos);
}

std::string File::read_string() {
    auto size = read<uint16_t>();
    std::string s(size, '\0');
    file.read(reinterpret_cast<char*>(&s.front()), size);
    return s;
}


} /* namespace eventio */
