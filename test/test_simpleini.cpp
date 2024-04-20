
#include <cassert>
#include <exception>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>

#include <simpleini.h>

#define NAME simple_ini_test

const std::filesystem::path TESTCONFIG{ "./test.ini" };
const std::string FAULTYCONF{ ".faulty.ini" };

static void
create_faulty_ini()
{
    std::ofstream confstream(FAULTYCONF);
    const std::string data = ";hello\n"
                             "[abc]\n"
                             "val1 = hello with trailing    \n"
                             "val2 =    3 with leading\n\n\n"
                             "val3 = nice\n"
                             "      "
                             "[test_section]\n"
                             "testValue =    hey\n"
                             "with space = 123\n"
                             "normal = yep\n"
                             "[empty section]\n"
                             "; comment\n";
    confstream << data;
    confstream.close();
}

static void
create_test_ini()
{
    std::ofstream confstream(TESTCONFIG);
    const std::string data = ";hello\n"
                             "[abc]\n"
                             "val1 = hello with trailing    \n"
                             "val2 =    3 with leading\n\n\n"
                             "val3 = nice\n"
                             "      \n"
                             "[test section]\n"
                             "testValue =    hey\n"
                             "with space = 123\n"
                             "normal = yep\n"
                             "[empty section]\n"
                             "[with comment] # hello\n"
                             "hey = aloha\n"
                             "; comment\n";
    confstream << data;
    confstream.close();
}

TEST(NAME, trailing_ini)
{
    simpleini::SimpleINI test(TESTCONFIG);
    ASSERT_EQ(test.get_config_path().string(), TESTCONFIG);
    ASSERT_THROW(test["no key"], std::out_of_range);
    ASSERT_THROW(test["abc"]["no key"], std::out_of_range);

    ASSERT_FALSE(test["abc"].empty());
    ASSERT_TRUE(test["empty section"].empty());

    try {
        ASSERT_EQ(test["abc"]["val1"], "hello with trailing")
          << "Trailing spaces no removed correctly.";
        ASSERT_EQ(test["abc"]["val2"], "3 with leading")
          << "Leading spaces not removed correctly.";
        ASSERT_EQ(test["abc"]["val3"], "nice");
        ASSERT_EQ(test["test section"]["testValue"], "hey")
          << "section with space not working.";
        ASSERT_EQ(test["test section"]["with space"], "123")
          << "section key with space not working.";
        ASSERT_EQ(test["test section"]["normal"], "yep");
        ASSERT_EQ(test["with comment"]["hey"], "aloha");
    } catch (std::out_of_range& error) {
        FAIL() << "Error was thrown: " << error.what();
    }
}

TEST(NAME, faulty_ini)
{
    ASSERT_THROW(simpleini::SimpleINI test(FAULTYCONF),
                 simpleini::INIException);
}

TEST(NAME, file_not_found)
{
    ASSERT_THROW(simpleini::SimpleINI("/path/to/nowhere.ini"),
                 simpleini::INIException);
}

TEST(NAME, get_as_any)
{
    simpleini::SimpleINI test(TESTCONFIG);
    ASSERT_EQ(test["abc"].get_as<int>("val2"), 3);
    ASSERT_EQ(test["abc"].get_as<float>("val2"), 3.0f);
    ASSERT_EQ(test["abc"].get_as<std::string>("val2"), "3");
    ASSERT_THROW(test["abc"].get_as<int>("val1"), simpleini::INIException);
}

TEST(NAME, write_file)
{
    std::filesystem::path tmpconf{ "/tmp/tmpconf" };
    simpleini::SimpleINI test;
    test.set_config_file(tmpconf, false);
    simpleini::INISection test_section{ "test",
                                        { { "abc", "123" }, { "123", "50" } } };
    test.add_section("test", test_section);
    test.write();

    simpleini::SimpleINI read_test(tmpconf);
    ASSERT_TRUE(read_test["test"]["abc"] == "123");
}

int
main(int argc, char** argv)
{
    create_test_ini();
    create_faulty_ini();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
