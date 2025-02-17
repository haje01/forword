#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include "../forword.h"

class ForwordTest : public ::testing::Test {
protected:
    std::string temp_dir;
    std::string forbidden_words_file;
    std::unique_ptr<Forword> forword;

    void SetUp() override {
        // Create temporary directory and file
        temp_dir = std::filesystem::temp_directory_path().string() + "/forword_test";
        std::filesystem::create_directory(temp_dir);
        forbidden_words_file = temp_dir + "/forbidden_words.txt";

        // Create forbidden words file
        std::ofstream file(forbidden_words_file);
        file << "bad\nbadword\n나쁜말\n욕설";
        file.close();

        forword = std::make_unique<Forword>(forbidden_words_file);
    }

    void TearDown() override {
        // Clean up temporary files
        std::filesystem::remove_all(temp_dir);
    }
};

TEST_F(ForwordTest, BasicSearch) {
    EXPECT_TRUE(forword->search("This is a bad word"));
    EXPECT_TRUE(forword->search("This is a badword"));
    EXPECT_FALSE(forword->search("This is good"));
}

TEST_F(ForwordTest, SearchWithSpaces) {
    EXPECT_TRUE(forword->search("This is a b a d word"));
    EXPECT_TRUE(forword->search("b-a-d"));
    EXPECT_TRUE(forword->search("b.a.d"));
}

TEST_F(ForwordTest, SearchKorean) {
    EXPECT_TRUE(forword->search("이것은 나쁜말 입니다"));
    EXPECT_TRUE(forword->search("이것은 욕설 입니다"));
    EXPECT_FALSE(forword->search("이것은 좋은말 입니다"));
}

TEST_F(ForwordTest, BasicReplace) {
    EXPECT_EQ(forword->replace("This is a bad word"), "This is a ***");
    EXPECT_EQ(forword->replace("This is a badword"), "This is a ***");
}

TEST_F(ForwordTest, ReplaceWithSpaces) {
    EXPECT_EQ(forword->replace("This is a b a d word"), "This is a ***");
    EXPECT_EQ(forword->replace("b-a-d"), "***");
}

TEST_F(ForwordTest, ReplaceKorean) {
    EXPECT_EQ(forword->replace("이것은 나쁜말 입니다"), "이것은 *** 입니다");
    EXPECT_EQ(forword->replace("이것은 욕설 입니다"), "이것은 *** 입니다");
}

TEST_F(ForwordTest, EmptyInput) {
    EXPECT_FALSE(forword->search(""));
    EXPECT_EQ(forword->replace(""), "");
}

TEST_F(ForwordTest, WordsWithWhitespace) {
    // Create forbidden words file with whitespace
    std::ofstream file(forbidden_words_file);
    file << "bad \n  badword\n\t나쁜말\n  욕설  \n";
    file.close();

    Forword forword(forbidden_words_file);

    // Test search
    EXPECT_TRUE(forword.search("This is a bad word"));
    EXPECT_TRUE(forword.search("This is a badword"));
    EXPECT_TRUE(forword.search("이것은 나쁜말 입니다"));
    EXPECT_TRUE(forword.search("이것은 욕설 입니다"));

    // Test replace
    EXPECT_EQ(forword.replace("This is a bad word"), "This is a ***");
    EXPECT_EQ(forword.replace("이것은 욕설 입니다"), "이것은 *** 입니다");
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 