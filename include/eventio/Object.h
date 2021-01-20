#ifndef EVENTIO_OBJECT_H
#define EVENTIO_OBJECT_H

#include <cstdint>
#include <bitset>
#include <array>
#include <cstddef>
#include <vector>
#include <memory>


namespace eventio {

class File;

using bytes_t = std::vector<std::byte>;
const std::array<std::byte, 4> MARKER_BYTES_LE {
    std::byte{0xd4},
    std::byte{0x1f},
    std::byte{0x8a},
    std::byte{0x37},
};
const std::uint64_t SYNC_MARKER = UINT64_C(0xd41f8a37);


inline uint32_t extract_bits(uint32_t value, uint8_t first, uint8_t n_bits);
inline bool is_bit_set(uint32_t value, uint8_t bit);


struct ObjectHeader {
    uint32_t type_version_field = 0;
    uint32_t id_field = 0;
    uint32_t length_field = 0;
    uint32_t extension_field = 0;

    uint32_t type() const;
    bool is_user_bit_set() const;
    bool is_extended() const;
    bool is_container() const;
    uint32_t version() const;
    uint32_t id() const;
    uint64_t header_size() const;
    uint64_t size() const;
};

class Object {
    protected:
        uint64_t next_header = 0;

    public:
        const uint64_t address = 0;
        const ObjectHeader header;

        Object(uint64_t address, ObjectHeader header);
        virtual ~Object(){};

        bytes_t read(File& file);
        bool has_next() const;
        std::unique_ptr<Object> read_next_child(File& file);
        virtual std::string getName() const;
        virtual void print(std::ostream& o) const;
};

inline std::ostream& operator << (std::ostream& os, const Object& o) {
    o.print(os);
    return os;
}

class History : public Object {
    public:
        const static int type;
        using Object::Object;
        virtual std::string getName() const override {return "History";};
};


class TimestampedString : public Object {
    public:
        using Object::Object;

        std::pair<int, std::string> parse(File& file);
};

class CommandLine : public TimestampedString {
    public:
        using TimestampedString::TimestampedString;

        const static int type;
        virtual std::string getName() const override {return "CommandLine";};
};


class ConfigLine : public TimestampedString {
    public:
        using TimestampedString::TimestampedString;

        const static int type;
        virtual std::string getName() const override {return "ConfigLine";};
};

inline const int History::type = 70;
inline const int CommandLine::type = 71;
inline const int ConfigLine::type = 72;

}

#endif /* ifndef EVENTIO_OBJECT_H */
