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
#include <iostream>
#include <unordered_set>

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

    static const std::unordered_set<char> DEFAULT_IGNORED_SYMBOLS;
    std::unordered_set<char> ignored_symbols_;
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
            
            // Map accented characters
            switch(ch) {
                // Unified accent mapping for all languages
                // a with accents (French, Portuguese, German)
                case U'à': case U'á': case U'â': case U'ã': case U'ä': ch = U'a'; break;
                // e with accents (French, Portuguese)
                case U'è': case U'é': case U'ê': case U'ë': ch = U'e'; break;
                // i with accents (French, Portuguese)
                case U'ì': case U'í': case U'î': case U'ï': ch = U'i'; break;
                // o with accents (French, Portuguese)
                case U'ò': case U'ó': case U'ô': case U'õ': case U'ö': ch = U'o'; break;
                // u with accents (French, Portuguese, German)
                case U'ù': case U'ú': case U'û': case U'ü': ch = U'u'; break;
                // Other special characters
                case U'ÿ': ch = U'y'; break;  // French
                case U'ç': ch = U'c'; break;  // French, Portuguese
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
        std::unordered_map<std::string, std::string> normalized_to_original;
        // Open file in binary mode
        std::ifstream file(file_path, std::ios::binary);
        
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open forbidden words file: " + file_path);
        }

        // Read first few bytes to check BOM
        char bom[4] = {0};
        file.read(bom, 4);
        
        // Determine encoding from BOM
        enum class Encoding {
            Unknown,
            UTF8,
            UTF16LE,
            UTF16BE,
            UTF32LE,
            UTF32BE
        } encoding = Encoding::Unknown;
        
        size_t bom_size = 0;
        
        if (bom[0] == (char)0xEF && bom[1] == (char)0xBB && bom[2] == (char)0xBF) {
            encoding = Encoding::UTF8;
            bom_size = 3;
        }
        else if (bom[0] == (char)0xFF && bom[1] == (char)0xFE) {
            if (bom[2] == 0 && bom[3] == 0) {
                encoding = Encoding::UTF32LE;
                bom_size = 4;
            } else {
                encoding = Encoding::UTF16LE;
                bom_size = 2;
            }
        }
        else if (bom[0] == (char)0xFE && bom[1] == (char)0xFF) {
            encoding = Encoding::UTF16BE;
            bom_size = 2;
        }
        else if (bom[0] == 0 && bom[1] == 0 && bom[2] == (char)0xFE && bom[3] == (char)0xFF) {
            encoding = Encoding::UTF32BE;
            bom_size = 4;
        }
        else {
            // No BOM found, assume UTF-8
            encoding = Encoding::UTF8;
            bom_size = 0;
        }

        // Reset file position after BOM
        file.clear();
        file.seekg(bom_size);
        
        // Read file content based on encoding
        std::string line;
        std::vector<char> buffer;
        buffer.resize(1024);  // Initial buffer size
        
        while (true) {
            if (encoding == Encoding::UTF8) {
                if (!std::getline(file, line)) break;
            }
            else {
                // Read appropriate number of bytes based on encoding
                size_t char_size = (encoding == Encoding::UTF16LE || encoding == Encoding::UTF16BE) ? 2 : 4;
                file.read(buffer.data(), char_size);
                if (file.gcount() < (long)char_size) break;
                
                // Convert to UTF-8
                if (encoding == Encoding::UTF16LE) {
                    uint16_t ch = *reinterpret_cast<uint16_t*>(buffer.data());
                    line.push_back(ch & 0xFF);
                    line.push_back((ch >> 8) & 0xFF);
                }
                else if (encoding == Encoding::UTF16BE) {
                    uint16_t ch = *reinterpret_cast<uint16_t*>(buffer.data());
                    ch = ((ch & 0xFF) << 8) | ((ch >> 8) & 0xFF);
                    line.push_back(ch & 0xFF);
                    line.push_back((ch >> 8) & 0xFF);
                }
                // Similar handling for UTF32...
            }
            
            // Trim whitespace from both ends
            line.erase(0, line.find_first_not_of(" \t\n\r"));
            line.erase(line.find_last_not_of(" \t\n\r") + 1);
            
            if (!line.empty()) {  // Skip empty lines after trimming
                // First normalize UTF-8 string to handle accents
                auto normalized_utf8 = Forword::normalize_utf8(line);
                // Then convert to UTF-32 and normalize spaces/symbols
                auto utf32_word = to_utf32(normalized_utf8);
                // 금칙어는 항상 기본 정규화 규칙을 사용
                std::u32string normalized_word;
                for (char32_t ch : utf32_word) {
                    if (ch >= U'A' && ch <= U'Z') {
                        ch = ch - U'A' + U'a';
                    }
                    if (ch >= 0x0300 && ch <= 0x036F) {
                        continue;
                    }
                    if (DEFAULT_IGNORED_SYMBOLS.find(to_utf8(std::u32string(1, ch))[0]) == 
                        DEFAULT_IGNORED_SYMBOLS.end() && 
                        is_word_char(ch)) {
                        normalized_word.push_back(ch);
                    }
                }
                
                // Convert normalized word back to string for comparison
                std::string normalized_str = to_utf8(normalized_word);
                
                // Check if this normalized form already exists
                auto it = normalized_to_original.find(normalized_str);
                if (it != normalized_to_original.end()) {
                    std::cerr << "Warning: '" << line << "' is equivalent to existing word '" 
                             << it->second << "' after normalization\n";
                    continue;  // Skip this word
                }
                
                // Store the mapping and add the word
                normalized_to_original[normalized_str] = line;
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
        
        // Thai characters (0E00-0E7F)
        if (ch >= 0x0E00 && ch <= 0x0E7F) return true;
        
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
            
            // Convert to UTF-8 to check against ignored_symbols
            std::string utf8_ch = to_utf8(std::u32string(1, ch));
            if (!utf8_ch.empty() && 
                ignored_symbols_.find(utf8_ch[0]) == ignored_symbols_.end()) {
                result.push_back(ch);
            }
        }
        
        return result;
    }

public:
    explicit Forword(
        const std::string& forbidden_words_file,
        const std::unordered_set<char>& ignored_symbols = DEFAULT_IGNORED_SYMBOLS
    ) : ignored_symbols_(ignored_symbols) {
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
        // Build filtered mapping for normalized_text (filtering out ignored symbols only)
        std::u32string normalized_text;
        std::vector<size_t> norm_to_orig;
        for (size_t i = 0; i < utf32_text.size(); i++) {
            std::string utf8_ch = to_utf8(std::u32string(1, utf32_text[i]));
            if (!utf8_ch.empty() && 
                ignored_symbols_.find(utf8_ch[0]) == ignored_symbols_.end()) {
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
        // Build filtered mapping for normalized_text (only filtering out ignored symbols)
        std::u32string normalized_text;
        std::vector<size_t> norm_to_orig;
        for (size_t i = 0; i < utf32_text.size(); i++) {
            std::string utf8_ch = to_utf8(std::u32string(1, utf32_text[i]));
            if (!utf8_ch.empty() && 
                ignored_symbols_.find(utf8_ch[0]) == ignored_symbols_.end()) {
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

    std::string normalize_word(const std::string& word) const {
        std::string normalized;
        for (char ch : word) {
            if (ignored_symbols_.find(ch) == ignored_symbols_.end() && is_word_char(ch)) {
                normalized += ch;
            }
        }
        return normalized;
    }

    static bool is_word_char(char ch) {
        // Implementation of is_word_char method
        // This should be implemented to match the original C++ version
        // For now, we'll keep the existing implementation
        return false; // Placeholder return, actual implementation needed
    }
};

// Define the static member variable
const std::unordered_set<char> Forword::DEFAULT_IGNORED_SYMBOLS = {
    ' ', '-', '.', '_', '\'', '"', '!', '?', '@', '#', '$', '%', '^', '&', '*',
    '(', ')', '+', '=', '[', ']', '{', '}', '|', '\\', '/', ':', ';', ',', '<', '>'
};

#endif // FORWORD_H 