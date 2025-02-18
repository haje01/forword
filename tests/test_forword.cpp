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

TEST_F(ForwordTest, MultilingualSupport) {
    // Create forbidden words file with multilingual words
    std::ofstream file(forbidden_words_file);
    // French
    file << u8"français\n"   // French
         << u8"garçon\n"     // boy
         << u8"café\n"       // coffee
    // Portuguese
         << u8"coração\n"    // heart
         << u8"não\n"        // no
         << u8"ação\n"       // action
    // Thai
         << u8"สวัสดี\n"      // hello
         << u8"ขอบคุณ\n"      // thank you
    // Existing test words
         << u8"坏话\n"      // Chinese
         << u8"ばか\n"      // Japanese
         << u8"плохой\n"    // Russian
         << u8"málaga\n"    // Spanish
         << u8"cattività\n"; // Italian
    file.close();

    Forword forword(forbidden_words_file);

    // Test French
    EXPECT_TRUE(forword.search(u8"Je parle français"));
    EXPECT_TRUE(forword.search(u8"Le garcon est la"));  // without ç
    EXPECT_TRUE(forword.search(u8"Un café noir"));
    EXPECT_EQ(forword.replace(u8"Je parle français bien"), 
              u8"Je parle *** bien");
    
    // Test Portuguese
    EXPECT_TRUE(forword.search(u8"Meu coração"));
    EXPECT_TRUE(forword.search(u8"Eu não sei"));
    EXPECT_TRUE(forword.search(u8"Uma ação importante"));
    EXPECT_EQ(forword.replace(u8"Meu coração bate"),
              u8"Meu *** bate");
    
    // Test Thai
    EXPECT_TRUE(forword.search(u8"พูดว่า สวัสดี ครับ"));
    EXPECT_TRUE(forword.search(u8"พูด ขอบคุณ ครับ"));
    EXPECT_EQ(forword.replace(u8"พูดว่า สวัสดี ครับ"),
              u8"พูดว่า *** ครับ");

    // Test Chinese
    EXPECT_TRUE(forword.search(u8"这是一个坏话的例子"));
}

TEST_F(ForwordTest, DuplicateWordWarning) {
    // Create a temporary file with duplicate words (after normalization)
    std::ofstream file(forbidden_words_file);
    file << "badword\n"
         << "b a d w o r d\n"    // Same as "badword" after normalization
         << "BAD-WORD\n"         // Same as "badword" after normalization
         << "málaga\n"
         << "malaga\n"           // Same as "málaga" after normalization
         << "scheiße\n"
         << "scheisse\n";        // Same as "scheiße" after normalization
    file.close();

    // Redirect cerr to capture warning messages
    std::stringstream warning_messages;
    std::streambuf* old_cerr = std::cerr.rdbuf(warning_messages.rdbuf());

    // Create forword instance (this should trigger warnings)
    Forword forword(forbidden_words_file);

    // Restore cerr
    std::cerr.rdbuf(old_cerr);

    // Get warning messages
    std::string warnings = warning_messages.str();

    // Check for expected warnings
    EXPECT_TRUE(warnings.find("'b a d w o r d' is equivalent to existing word 'badword'") != std::string::npos);
    EXPECT_TRUE(warnings.find("'BAD-WORD' is equivalent to existing word 'badword'") != std::string::npos);
    EXPECT_TRUE(warnings.find("'malaga' is equivalent to existing word 'málaga'") != std::string::npos);
    EXPECT_TRUE(warnings.find("'scheisse' is equivalent to existing word 'scheiße'") != std::string::npos);

    // Test that only unique words are actually loaded
    EXPECT_TRUE(forword.search("This is a badword"));     // Original word works
    EXPECT_TRUE(forword.search("This is a b a d w o r d")); // Spaced version also works
    EXPECT_TRUE(forword.search("This is málaga"));        // Accented version works
    EXPECT_TRUE(forword.search("This is malaga"));        // Non-accented version works
    EXPECT_TRUE(forword.search("This is scheiße"));       // German version works
    EXPECT_TRUE(forword.search("This is scheisse"));      // Normalized version works
}

TEST_F(ForwordTest, CustomIgnoredSymbols) {
    // 임시 금칙어 파일 생성
    std::string temp_file = temp_dir + "/custom_ignored_symbols.txt";
    std::ofstream ofs(temp_file);
    ofs << "badword\ntest" << std::endl;
    ofs.close();
    
    // 기본 무시 기호로 테스트
    Forword default_forword(temp_file);
    EXPECT_TRUE(default_forword.search("b-a-d-w-o-r-d"));
    EXPECT_TRUE(default_forword.search("t.e.s.t"));
    EXPECT_TRUE(default_forword.search("b a d w o r d"));
    
    // 사용자 정의 무시 기호로 테스트 (하이픈과 공백만)
    std::unordered_set<char> custom_symbols{'-', ' '};
    Forword custom_forword(temp_file, custom_symbols);
    EXPECT_TRUE(custom_forword.search("b-a-d-w-o-r-d"));
    EXPECT_TRUE(custom_forword.search("b a d w o r d"));
    EXPECT_FALSE(custom_forword.search("b.a.d.w.o.r.d"));
    EXPECT_FALSE(custom_forword.search("t.e.s.t"));
    
    // 무시 기호 없이 테스트
    Forword strict_forword(temp_file, std::unordered_set<char>{});
    EXPECT_FALSE(strict_forword.search("b-a-d-w-o-r-d"));
    EXPECT_FALSE(strict_forword.search("b a d w o r d"));
    EXPECT_FALSE(strict_forword.search("b.a.d.w.o.r.d"));
    EXPECT_TRUE(strict_forword.search("badword"));
    
    // 치환 기능 테스트
    EXPECT_EQ("This is ***", 
        custom_forword.replace("This is b-a-d-w-o-r-d"));
    EXPECT_EQ("This is ***", 
        custom_forword.replace("This is b a d w o r d"));
    EXPECT_EQ("This is b.a.d.w.o.r.d",
        custom_forword.replace("This is b.a.d.w.o.r.d"));
}

class NormalizeUtf8Test : public ::testing::Test {
protected:
    static std::string normalize_utf8_test(const std::string& input) {
        return Forword::normalize_utf8(input);
    }
};

TEST_F(NormalizeUtf8Test, BasicNormalization) {
    // Test basic ASCII
    EXPECT_EQ(NormalizeUtf8Test::normalize_utf8_test("hello"), "hello");
    
    // Test Italian characters
    EXPECT_EQ(NormalizeUtf8Test::normalize_utf8_test("cattività"), "cattivita");  // à -> a
    EXPECT_EQ(NormalizeUtf8Test::normalize_utf8_test("perché"), "perche");        // é -> e
    
    // Test Spanish characters
    EXPECT_EQ(NormalizeUtf8Test::normalize_utf8_test("málaga"), "malaga");        // á -> a
    EXPECT_EQ(NormalizeUtf8Test::normalize_utf8_test("niño"), "nino");           // ñ -> n
    
    // Test multiple combining marks
    EXPECT_EQ(NormalizeUtf8Test::normalize_utf8_test("a\u0300\u0301"), "a");     // a + combining grave + combining acute -> a
    
    // Test mixed normal and combining characters
    EXPECT_EQ(NormalizeUtf8Test::normalize_utf8_test("e\u0301"), "e");           // e + combining acute -> e
    
    // Test German characters
    EXPECT_EQ(NormalizeUtf8Test::normalize_utf8_test("schön"), "schon");     // ö -> o
    EXPECT_EQ(NormalizeUtf8Test::normalize_utf8_test("über"), "uber");       // ü -> u
    EXPECT_EQ(NormalizeUtf8Test::normalize_utf8_test("Mädchen"), "madchen"); // ä -> a
    EXPECT_EQ(NormalizeUtf8Test::normalize_utf8_test("groß"), "gross");      // ß -> ss
    
    // Print hex values for debugging
    std::string input = "cattività";
    std::string normalized = NormalizeUtf8Test::normalize_utf8_test(input);
    std::cout << "Input hex: ";
    for (unsigned char c : input) {
        printf("%02X ", c);
    }
    std::cout << "\nNormalized hex: ";
    for (unsigned char c : normalized) {
        printf("%02X ", c);
    }
    std::cout << std::endl;
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 