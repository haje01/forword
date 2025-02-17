#include <chrono>
#include <fstream>
#include <iostream>
#include "../forword.h"

int main() {
    // Create forbidden words file
    const char* forbidden_words_file = "forbidden_words.txt";
    std::ofstream file(forbidden_words_file);
    file << "bad\nbadword\n나쁜말\n욕설";
    file.close();

    // Initialize Forword
    Forword forword(forbidden_words_file);

    // Prepare test text
    const std::string text = "이것은 나쁜말 입니다. This is a bad word. 여기에 욕설이 있습니다.";

    // Benchmark replace operation
    const int iterations = 10000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        forword.replace(text);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    double elapsed_sec = elapsed.count() / 1000.0;
    double ops_per_sec = iterations / elapsed_sec;

    std::cout << "C++ Forword Benchmark\n";
    std::cout << "---------------------\n";
    std::cout << "Operations: " << iterations << "\n";
    std::cout << "Total time: " << elapsed_sec << " seconds\n";
    std::cout << "Ops/sec: " << ops_per_sec << "\n";

    // Cleanup
    std::remove(forbidden_words_file);

    return 0;
} 