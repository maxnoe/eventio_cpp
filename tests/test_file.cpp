#include <gtest/gtest.h>
#include "eventio/File.h"
#include <cstdio>



TEST(TestFile, TestInvalid) {
    std::remove("test.txt");
    EXPECT_THROW({
        try {
            eventio::File("test.txt");
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("Could not open file 'test.txt'", e.what());
            throw;
        }
    }, std::invalid_argument);

    std::fstream file("test.txt", std::fstream::out | std::fstream::trunc | std::fstream::binary);
    file.close();

    EXPECT_THROW({
        try {
            eventio::File("test.txt");
        } catch (const std::invalid_argument& e) {
            EXPECT_STREQ("File 'test.txt' is not an eventio file", e.what());
            throw;
        }
    }, std::invalid_argument);

    std::remove("test.txt");
}


TEST(TestFile, TestValid) {
    eventio::File("../../lst_muons_with_true_photons.simtel");
}
