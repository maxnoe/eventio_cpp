#include "eventio/Object.h"
#include "eventio/File.h"


namespace eventio {

inline uint32_t extract_bits(uint32_t value, uint8_t first, uint8_t n_bits) {
    uint32_t one = 1;
    return (value >> first) & ((one << n_bits) - one);
}

inline bool is_bit_set(uint32_t value, uint8_t bit) {
    return std::bitset<32>(value)[bit];
}



uint32_t ObjectHeader::type() const {
    // bits 0 to 15 (inclusive)
    return extract_bits(type_version_field, 0, 16);
}

bool ObjectHeader::is_user_bit_set() const {
    return is_bit_set(type_version_field, 16);
}

bool ObjectHeader::is_extended() const {
    return is_bit_set(type_version_field, 17);
}

bool ObjectHeader::is_container() const {
    return is_bit_set(length_field, 30);
}

uint32_t ObjectHeader::version() const {
    // bits 20 to 31 (inclusive)
    return extract_bits(type_version_field, 20, 12);
}

uint32_t ObjectHeader::id() const {
    return id_field;
}

uint64_t ObjectHeader::header_size() const {
    return is_extended() ? 16 : 12;
}

uint64_t ObjectHeader::size() const {
    uint64_t s = extract_bits(length_field, 0, 30);

    if (is_extended()) {
        uint64_t extension = extract_bits(extension_field, 0, 12);
        s += extension << 30;
    }
    return s;
}

Object::Object(uint64_t address, ObjectHeader header) :
    next_header(address),
    address(address),
    header(header) {
}

bytes_t Object::read(File& file) {
    file.file.seekg(address);
    return file.read(header.size());
}

bool Object::has_next() const {
    return header.is_container() ? (next_header - address) < header.size() : false;
}

std::unique_ptr<Object> Object::read_next_child(File& file) {
    if (!header.is_container()) {
        throw std::runtime_error("Current object is not a container");
    }
    if (!has_next()) {
        throw std::runtime_error("No objects left in container");
    }

    file.file.seekg(next_header);
    auto object = file.read_object(false);
    next_header += object->header.header_size() + object->header.size();

    return object;
}

std::string Object::getName() const {
    return "Object";
}

void Object::print(std::ostream& o) const {
o << getName()
    << "["
    << header.type() << ", " << header.version()
    << "]("
    << "address=" << address
    << ", size=" << header.size()
    << ", id=" << header.id()
    << ", is_container=" << header.is_container()
    << ")";
}

std::pair<int, std::string> TimestampedString::parse(File& file) {
    file.seekg(address);
    auto ts = file.read<int32_t>();
    return std::make_pair(ts, file.read_string());
}


} /* namespace eventio */
