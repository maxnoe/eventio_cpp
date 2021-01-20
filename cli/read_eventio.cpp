#include <iostream>
#include <cstddef>

#include "eventio/File.h"
#include "eventio/Object.h"


void print_all_objects(eventio::Object& object, eventio::File& file, int level=0) {
    for (int i = 0; i < level; ++i) {
        std::cout << "  ";
    }

    int type = object.header.type();
    if (type == 71 ||  type == 72) {
        auto [timestamp, line] = dynamic_cast<eventio::TimestampedString&>(object).parse(file);
        std::cout << timestamp << " " << line << std::endl;
    } else {
        std::cout << object << std::endl;
    }

    if (object.header.is_container()) {
        while (object.has_next()) {
            auto sub_object = object.read_next_child(file);
            print_all_objects(*sub_object.get(), file, level + 1);
        }
    }
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: read_eventio <path>" << std::endl;
    }
    eventio::File file(argv[1]);

    std::cout << std::boolalpha;
    while (file.has_next_toplevel()) {
        auto object = file.next_toplevel();
        print_all_objects(*object.get(), file);
    }

    return 0;
}
