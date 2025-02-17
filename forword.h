#ifndef FORWORD_H
#define FORWORD_H

#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <memory>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <regex>
#include <locale>
#include <codecvt>
#include <set>
#include <map>

class Forword {
friend class NormalizeUtf8Test;
private:
    struct TrieNode {
        std::unordered_map<char32_t, std::unique_ptr<TrieNode>> children;
        TrieNode* fail{nullptr};
        std::vector<std::u32string> output;
        bool is_root{false};

        TrieNode() = default;
    };

    std::unique_ptr<TrieNode> root;
    std::vector<std::u32string> forbidden_words;

    // UTF-8 conversion utilities
    static std::u32string to_utf32(const std::string& s) {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
        return conv.from_bytes(s);
    }

    static std::string to_utf8(const std::u32string& s) {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
        return conv.to_bytes(s);
    }

    // Normalize UTF-8 string with expansion and return mapping vector.
    // For each code point appended to the normalized output, mapping[i] holds
    // the index in the original UTF-32 string where that character came from.
    static std::pair<std::string, std::vector<size_t>> normalize_utf8_with_mapping(const std::string & input) {
        std::u32string utf32 = to_utf32(input);
        std::u32string normalized;
        normalized.reserve(utf32.size());
        std::vector<size_t> mapping;
        mapping.reserve(utf32.size());
        
        for (size_t i = 0; i < utf32.size(); i++) {
            char32_t ch = utf32[i];
            
            // Convert to lowercase first
            if (ch >= U'A' && ch <= U'Z') {
                ch = ch - U'A' + U'a';
            }
            
            // Map accented characters to their base form
            switch(ch) {
                case U'à': ch = U'a'; break;
                case U'á': ch = U'a'; break;
                case U'ä': ch = U'a'; break;  // German umlaut
                case U'è': ch = U'e'; break;
                case U'é': ch = U'e'; break;
                case U'ì': ch = U'i'; break;
                case U'í': ch = U'i'; break;
                case U'ò': ch = U'o'; break;
                case U'ó': ch = U'o'; break;
                case U'ö': ch = U'o'; break;  // German umlaut
                case U'ù': ch = U'u'; break;
                case U'ú': ch = U'u'; break;
                case U'ü': ch = U'u'; break;  // German umlaut
                case U'ñ': ch = U'n'; break;  // Spanish 'ñ' -> 'n'
                // For ß, expand to two 's' characters.
                case U'ß': {
                    normalized.push_back(U's');
                    mapping.push_back(i);
                    normalized.push_back(U's');
                    mapping.push_back(i);
                    continue;
                }
                default:
                    if(ch >= 0x0300 && ch <= 0x036F) continue;
            }
            
            normalized.push_back(ch);
            mapping.push_back(i);
        }
        
        return { to_utf8(normalized), mapping };
    }

    std::vector<std::u32string> load_forbidden_words(const std::string& file_path) {
        std::vector<std::u32string> words;
        std::ifstream file(file_path);
        
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open forbidden words file: " + file_path);
        }

        std::string line;
        while (std::getline(file, line)) {
            // Trim whitespace from both ends
            line.erase(0, line.find_first_not_of(" \t\n\r"));
            line.erase(line.find_last_not_of(" \t\n\r") + 1);
            
            if (!line.empty()) {  // Skip empty lines after trimming
                // First normalize UTF-8 string to handle accents
                auto normalized_utf8 = Forword::normalize_utf8(line);
                // Then convert to UTF-32 and normalize spaces/symbols
                auto utf32_word = to_utf32(normalized_utf8);
                auto normalized_word = normalize_text(utf32_word);
                words.push_back(normalized_word);
            }
        }

        return words;
    }

    void build_trie() {
        root = std::make_unique<TrieNode>();
        root->is_root = true;

        for (const auto& word : forbidden_words) {
            auto node = root.get();
            for (char32_t ch : word) {
                if (!node->children[ch]) {
                    node->children[ch] = std::make_unique<TrieNode>();
                }
                node = node->children[ch].get();
            }
            node->output.push_back(word);
        }
    }

    void build_failure_links() {
        std::queue<TrieNode*> q;

        // Set failure links for depth 1 nodes
        for (auto& pair : root->children) {
            pair.second->fail = root.get();
            q.push(pair.second.get());
        }

        // Build failure links for deeper nodes
        while (!q.empty()) {
            auto current = q.front();
            q.pop();

            for (auto& pair : current->children) {
                auto child = pair.second.get();
                q.push(child);
                
                auto failure = current->fail;
                while (failure && !failure->children.count(pair.first)) {
                    failure = failure->fail;
                }

                child->fail = failure ? failure->children[pair.first].get() : root.get();
                
                // Add outputs from failure node
                if (child->fail->output.size() > 0) {
                    child->output.insert(
                        child->output.end(),
                        child->fail->output.begin(),
                        child->fail->output.end()
                    );
                }
            }
        }
    }

    bool is_word_char(char32_t ch) const {
        // Basic Latin letters and numbers
        if (std::isalnum(ch)) return true;
        
        // Latin-1 Supplement letters (e.g., accented letters like á, é, etc.)
        if (ch >= 0x00C0 && ch <= 0x00FF) return true;
        
        // Skip combining diacritical marks
        if (ch >= 0x0300 && ch <= 0x036F) return false;
        
        // Hangul Syllables (AC00-D7AF)
        if (ch >= 0xAC00 && ch <= 0xD7AF) return true;
        
        // Hangul Jamo (1100-11FF)
        if (ch >= 0x1100 && ch <= 0x11FF) return true;
        
        // Hangul Compatibility Jamo (3130-318F)
        if (ch >= 0x3130 && ch <= 0x318F) return true;
        
        // CJK Unified Ideographs (4E00-9FFF)
        if (ch >= 0x4E00 && ch <= 0x9FFF) return true;
        
        // Hiragana (3040-309F)
        if (ch >= 0x3040 && ch <= 0x309F) return true;
        
        // Katakana (30A0-30FF)
        if (ch >= 0x30A0 && ch <= 0x30FF) return true;
        
        // Cyrillic (0400-04FF)
        if (ch >= 0x0400 && ch <= 0x04FF) return true;
        
        // Latin Extended Additional (1E00-1EFF) - for Spanish/Italian
        if (ch >= 0x1E00 && ch <= 0x1EFF) return true;
        
        // Latin Extended-A (0100-017F) - for Spanish/Italian
        if (ch >= 0x0100 && ch <= 0x017F) return true;
        
        return false;
    }

    bool is_space_char(char32_t ch) const {
        return ch == U' ' || ch == U'\t' || ch == U'\n' || ch == U'\r';
    }

    std::u32string normalize_text(const std::u32string& text) const {
        std::u32string result;
        result.reserve(text.size());
        
        for (char32_t ch : text) {
            // Convert to lowercase first
            if (ch >= U'A' && ch <= U'Z') {
                ch = ch - U'A' + U'a';
            }
            
            // Skip combining diacritical marks (0300-036F)
            if (ch >= 0x0300 && ch <= 0x036F) {
                continue;
            }
            
            if (!is_space_char(ch) && is_word_char(ch)) {
                result.push_back(ch);
            }
        }
        
        return result;
    }

public:
    explicit Forword(const std::string& forbidden_words_file) {
        std::locale::global(std::locale("")); // Use system locale for correct UTF-8 conversion
        forbidden_words = load_forbidden_words(forbidden_words_file);
        build_trie();
        build_failure_links();
    }

    bool search(const std::string& text) const {
        if (text.empty()) return false;

        // Use normalize_utf8_with_mapping to obtain normalized text and mapping vector.
        auto [normalized_input, mapping] = normalize_utf8_with_mapping(text);
        auto utf32_text = to_utf32(normalized_input);
        // Build filtered mapping for normalized_text (filtering out spaces/punctuation)
        std::u32string normalized_text;
        std::vector<size_t> norm_to_orig;
        for (size_t i = 0; i < utf32_text.size(); i++) {
            if (!is_space_char(utf32_text[i]) && is_word_char(utf32_text[i])) {
                normalized_text.push_back(utf32_text[i]);
                norm_to_orig.push_back(mapping[i]);
            }
        }
        const TrieNode* current = root.get();

        for (size_t i = 0; i < normalized_text.size(); i++) {
            char32_t ch = normalized_text[i];
            while (current != root.get() && !current->children.count(ch)) {
                current = current->fail;
            }
            if (current->children.count(ch)) {
                current = current->children.at(ch).get();
                for (const auto& word : current->output) {
                    std::u32string normalized_word = normalize_text(word);
                    size_t word_pos = i - normalized_word.length() + 1;
                    
                    if (word_pos <= i && 
                        normalized_text.substr(word_pos, normalized_word.length()) == normalized_word) {
                        if (word_pos < norm_to_orig.size() && i < norm_to_orig.size()) {
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

    std::string replace(const std::string& text, const std::string& replacement = "***") const {
        if (text.empty()) {
            return text;
        }

        // Use normalize_utf8_with_mapping to obtain normalized text and mapping vector.
        auto [normalized_input, mapping] = normalize_utf8_with_mapping(text);
        auto utf32_text = to_utf32(normalized_input);
        // Build filtered mapping for normalized_text (only for word chars)
        std::u32string normalized_text;
        std::vector<size_t> norm_to_orig;
        for (size_t i = 0; i < utf32_text.size(); i++) {
            if (!is_space_char(utf32_text[i]) && is_word_char(utf32_text[i])) {
                normalized_text.push_back(utf32_text[i]);
                norm_to_orig.push_back(mapping[i]);
            }
        }
        auto original_utf32 = to_utf32(text);  // Original text (unchanged)
        const TrieNode* current = root.get();
        std::set<std::pair<size_t, size_t>> matches;
        size_t pos = 0;

        // Find all matches
        while (pos < normalized_text.length()) {
            char32_t ch = normalized_text[pos];
            
            while (current != root.get() && !current->children.count(ch)) {
                current = current->fail;
            }
            
            if (current->children.count(ch)) {
                current = current->children.at(ch).get();
                
                for (const auto& word : current->output) {
                    std::u32string normalized_word = normalize_text(word);
                    size_t word_pos = pos - normalized_word.length() + 1;
                    
                    if (word_pos <= pos &&
                        normalized_text.substr(word_pos, normalized_word.length()) == normalized_word) {
                        if (word_pos < norm_to_orig.size() && pos < norm_to_orig.size()) {
                            size_t orig_start = norm_to_orig[word_pos];
                            size_t orig_end = norm_to_orig[pos];

                            // Extend boundaries to include adjacent spaces
                            while (orig_start > 0 && is_space_char(original_utf32[orig_start - 1])) {
                                orig_start--;
                            }
                            while (orig_end < original_utf32.length() - 1 && is_space_char(original_utf32[orig_end + 1])) {
                                orig_end++;
                            }

                            matches.insert({orig_start, orig_end + 1});
                        }
                    }
                }
            }
            pos++;
        }

        // Filter overlapping matches: remove any match that is completely contained
        // in another match with greater length.
        std::vector<std::pair<size_t, size_t>> filtered;
        for (const auto& m : matches) {
            bool contained = false;
            for (const auto& n : matches) {
                if (m != n && n.first <= m.first && n.second >= m.second &&
                    ((n.second - n.first) > (m.second - m.first))) {
                    contained = true;
                    break;
                }
            }
            if (!contained)
                filtered.push_back(m);
        }

        // Sort filtered intervals in descending order of start index to safely replace from end to start.
        std::sort(filtered.begin(), filtered.end(), [](auto a, auto b) { return a.first > b.first; });
        
        // Replace filtered matches from end to start.
        for (const auto& m : filtered) {
            size_t start = m.first;
            size_t end = m.second;
            
            std::u32string prefix = original_utf32.substr(0, start);
            std::u32string suffix = end < original_utf32.length() ? original_utf32.substr(end) : U"";
            
            // Ensure single space before and after replacement
            if (!prefix.empty() && !is_space_char(prefix.back())) {
                prefix += U" ";
            }
            if (!suffix.empty() && !is_space_char(suffix.front())) {
                suffix = U" " + suffix;
            }
            
            original_utf32 = prefix + to_utf32(replacement) + suffix;
        }

        return to_utf8(original_utf32);
    }

    // Wrapper for normalize_utf8_with_mapping that returns only the normalized UTF-8 string.
    static std::string normalize_utf8(const std::string & input) {
        return std::get<0>(normalize_utf8_with_mapping(input));
    }
};

#endif // FORWORD_H 