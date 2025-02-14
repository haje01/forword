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

    std::u32string normalize_text(const std::u32string& text) const {
        std::u32string result;
        result.reserve(text.size());
        
        for (char32_t ch : text) {
            if (!std::isspace(ch) && std::isalnum(ch)) {
                result.push_back(ch);
            }
        }
        
        return result;
    }

public:
    explicit Forword(const std::string& forbidden_words_file) {
        forbidden_words = load_forbidden_words(forbidden_words_file);
        build_trie();
        build_failure_links();
    }

    bool search(const std::string& text) const {
        if (text.empty()) return false;

        auto utf32_text = to_utf32(text);
        auto normalized_text = normalize_text(utf32_text);
        
        auto current = root.get();
        
        for (char32_t ch : normalized_text) {
            while (current != root.get() && !current->children.count(ch)) {
                current = current->fail;
            }
            
            if (current->children.count(ch)) {
                current = current->children.at(ch).get();
                if (!current->output.empty()) {
                    return true;
                }
            }
        }
        
        return false;
    }

    std::string replace(const std::string& text, const std::string& replacement = "***") const {
        if (text.empty()) return text;

        auto utf32_text = to_utf32(text);
        auto normalized_text = normalize_text(utf32_text);
        
        std::vector<std::pair<size_t, size_t>> matches;
        auto current = root.get();
        
        for (size_t pos = 0; pos < normalized_text.size(); ++pos) {
            char32_t ch = normalized_text[pos];
            
            while (current != root.get() && !current->children.count(ch)) {
                current = current->fail;
            }
            
            if (current->children.count(ch)) {
                current = current->children.at(ch).get();
                
                for (const auto& word : current->output) {
                    size_t word_pos = pos - word.size() + 1;
                    if (normalized_text.substr(word_pos, word.size()) == word) {
                        matches.emplace_back(word_pos, pos + 1);
                    }
                }
            }
        }

        // Sort matches in reverse order to maintain correct positions
        std::sort(matches.begin(), matches.end(), 
            [](const auto& a, const auto& b) { return a.first > b.first; });

        auto result = utf32_text;
        for (const auto& match : matches) {
            size_t orig_start = match.first;
            size_t orig_end = match.second;
            
            // Extend match boundaries to include spaces
            while (orig_start > 0 && std::isspace(result[orig_start-1])) {
                --orig_start;
            }
            while (orig_end < result.size() && std::isspace(result[orig_end])) {
                ++orig_end;
            }
            
            result.replace(orig_start, orig_end - orig_start, to_utf32(replacement));
        }
        
        return to_utf8(result);
    }
};

#endif // FORWORD_H 