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

    std::vector<std::u32string> load_forbidden_words(const std::string& file_path) {
        std::vector<std::u32string> words;
        std::ifstream file(file_path);
        
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open forbidden words file: " + file_path);
        }

        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                words.push_back(to_utf32(line));
            }
        }

        return words;
    }

    void build_trie() {
        root = std::make_unique<TrieNode>();
        root->is_root = true;

        for (const auto& word : forbidden_words) {
            // Normalize the forbidden word for consistency with search/replace normalization.
            std::u32string norm_word = normalize_text(word);
            auto node = root.get();
            for (char32_t ch : norm_word) {
                if (!node->children[ch]) {
                    node->children[ch] = std::make_unique<TrieNode>();
                }
                node = node->children[ch].get();
            }
            node->output.push_back(norm_word);
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
        
        // Hangul Syllables (AC00-D7AF)
        if (ch >= 0xAC00 && ch <= 0xD7AF) return true;
        
        // Hangul Jamo (1100-11FF)
        if (ch >= 0x1100 && ch <= 0x11FF) return true;
        
        // Hangul Compatibility Jamo (3130-318F)
        if (ch >= 0x3130 && ch <= 0x318F) return true;
        
        return false;
    }

    bool is_space_char(char32_t ch) const {
        return ch == U' ' || ch == U'\t' || ch == U'\n' || ch == U'\r';
    }

    std::u32string normalize_text(const std::u32string& text) const {
        std::u32string result;
        result.reserve(text.size());
        
        for (char32_t ch : text) {
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

        auto utf32_text = to_utf32(text);
        auto normalized_text = normalize_text(utf32_text);
        const TrieNode* current = root.get();

        for (size_t i = 0; i < normalized_text.size(); i++) {
            char32_t ch = normalized_text[i];
            while (current != root.get() && !current->children.count(ch)) {
                current = current->fail;
            }
            if (current->children.count(ch)) {
                current = current->children.at(ch).get();
            }
            if (!current->output.empty()) {
                return true;
            }
        }

        return false;
    }

    std::string replace(const std::string& text, const std::string& replacement = "***") const {
        if (text.empty()) {
            return text;
        }

        auto utf32_text = to_utf32(text);
        auto normalized_text = normalize_text(utf32_text);
        const TrieNode* current = root.get();
        std::set<std::pair<size_t, size_t>> matches;
        size_t pos = 0;

        // Build mapping between normalized and original text positions
        std::map<size_t, size_t> norm_to_orig;
        size_t norm_pos = 0;

        // First pass: find word boundaries and build position mappings
        for (size_t i = 0; i < utf32_text.length(); ++i) {
            if (is_word_char(utf32_text[i])) {
                norm_to_orig[norm_pos] = i;
                norm_pos++;
            }
        }

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
                        if (norm_to_orig.count(word_pos) && norm_to_orig.count(pos)) {
                            size_t orig_start = norm_to_orig[word_pos];
                            size_t orig_end = norm_to_orig[pos];

                            // Extend boundaries to include adjacent spaces
                            while (orig_start > 0 && std::isspace(utf32_text[orig_start - 1])) {
                                orig_start--;
                            }
                            while (orig_end < utf32_text.length() - 1 && std::isspace(utf32_text[orig_end + 1])) {
                                orig_end++;
                            }

                            matches.insert({orig_start, orig_end + 1});
                        }
                    }
                }
            }
            pos++;
        }

        // Replace matches from end to start
        for (auto it = matches.rbegin(); it != matches.rend(); ++it) {
            size_t start = it->first;
            size_t end = it->second;
            
            std::u32string prefix = utf32_text.substr(0, start);
            std::u32string suffix = end < utf32_text.length() ? utf32_text.substr(end) : U"";
            
            // Ensure single space before and after replacement
            if (!prefix.empty() && !std::isspace(prefix.back())) {
                prefix += U" ";
            }
            if (!suffix.empty() && !std::isspace(suffix.front())) {
                suffix = U" " + suffix;
            }
            
            utf32_text = prefix + to_utf32(replacement) + suffix;
        }

        return to_utf8(utf32_text);
    }
};

#endif // FORWORD_H 