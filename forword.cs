using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace ForwordLib
{
    public class Forword
    {
        private class TrieNode
        {
            public Dictionary<char, TrieNode> Children { get; } = new Dictionary<char, TrieNode>();
            public TrieNode? Fail { get; set; }
            public HashSet<string> Output { get; } = new HashSet<string>();
            public bool IsRoot { get; set; }
        }

        private static bool IsThaiDiacritic(char ch)
        {
            int code = (int)ch;
            return (0x0E30 <= code && code <= 0x0E3A) ||  // Thai vowel marks
                   (0x0E47 <= code && code <= 0x0E4E);     // Thai tone marks
        }

        // 기본적으로 무시할 기호들
        private static readonly HashSet<char> DefaultIgnoredSymbols = new HashSet<char>
        {
            ' ', '-', '.', '_', '\'', '"', '!', '?', '@', '#', '$', '%', '^', '&', '*',
            '(', ')', '+', '=', '[', ']', '{', '}', '|', '\\', '/', ':', ';', ',', '<', '>'
        };

        private readonly HashSet<char> ignoredSymbols;
        private readonly TrieNode root;
        private readonly List<string> forbiddenWords;

        private string NormalizeWord(string word)
        {
            var normalized = new StringBuilder();
            foreach (var c in word)
            {
                char ch = c;
                // Convert to lowercase
                if (char.IsUpper(ch))
                    ch = char.ToLower(ch);
                
                // Map accented characters
                switch (ch)
                {
                    // Unified accent mapping for all languages
                    // a with accents (French, Portuguese, German)
                    case 'à': case 'á': case 'â': case 'ã': case 'ä': ch = 'a'; break;
                    // e with accents (French, Portuguese)
                    case 'è': case 'é': case 'ê': case 'ë': ch = 'e'; break;
                    // i with accents (French, Portuguese)
                    case 'ì': case 'í': case 'î': case 'ï': ch = 'i'; break;
                    // o with accents (French, Portuguese)
                    case 'ò': case 'ó': case 'ô': case 'õ': case 'ö': ch = 'o'; break;
                    // u with accents (French, Portuguese, German)
                    case 'ù': case 'ú': case 'û': case 'ü': ch = 'u'; break;
                    // Other special characters
                    case 'ÿ': ch = 'y'; break;  // French
                    case 'ç': ch = 'c'; break;  // French, Portuguese
                    case 'ñ': ch = 'n'; break;
                    case 'ß':
                        normalized.Append("ss");
                        continue;
                }
                
                // Skip combining diacritical marks
                if (ch >= '\u0300' && ch <= '\u036F')
                    continue;
                
                if (!DefaultIgnoredSymbols.Contains(ch) && IsWordChar(ch))
                    normalized.Append(ch);
            }
            return normalized.ToString();
        }

        private string NormalizeForSearch(string text)
        {
            var normalized = new StringBuilder();
            foreach (var c in text)
            {
                char ch = c;
                
                // Convert to lowercase
                if (char.IsUpper(ch))
                    ch = char.ToLower(ch);
                
                // Map accented characters
                switch (ch)
                {
                    // Unified accent mapping for all languages
                    // a with accents (French, Portuguese, German)
                    case 'à': case 'á': case 'â': case 'ã': case 'ä': ch = 'a'; break;
                    // e with accents (French, Portuguese)
                    case 'è': case 'é': case 'ê': case 'ë': ch = 'e'; break;
                    // i with accents (French, Portuguese)
                    case 'ì': case 'í': case 'î': case 'ï': ch = 'i'; break;
                    // o with accents (French, Portuguese)
                    case 'ò': case 'ó': case 'ô': case 'õ': case 'ö': ch = 'o'; break;
                    // u with accents (French, Portuguese, German)
                    case 'ù': case 'ú': case 'û': case 'ü': ch = 'u'; break;
                    // Other special characters
                    case 'ÿ': ch = 'y'; break;  // French
                    case 'ç': ch = 'c'; break;  // French, Portuguese
                    case 'ñ': ch = 'n'; break;
                    case 'ß':
                        normalized.Append("ss");
                        continue;
                }
                
                // Skip combining diacritical marks
                if (ch >= '\u0300' && ch <= '\u036F')
                    continue;
                
                // 만약 사용자 정의 무시 기호(ignoredSymbols)가 기본값과 동일하다면,
                // 기존처럼 단어 문자(IsWordChar)만 남김.
                // 그렇지 않으면, 전달된 ignoredSymbols에 있는 문자만 제거하고 그대로 유지.
                if (ignoredSymbols.SetEquals(DefaultIgnoredSymbols))
                {
                    if (!ignoredSymbols.Contains(ch) && IsWordChar(ch))
                        normalized.Append(ch);
                }
                else
                {
                    if (!ignoredSymbols.Contains(ch))
                        normalized.Append(ch);
                }
            }
            
            return normalized.ToString();
        }

        public Forword(string forbiddenWordsFile, IEnumerable<char>? ignoredSymbols = null)
        {
            if (!File.Exists(forbiddenWordsFile))
                throw new FileNotFoundException($"Forbidden words file not found: {forbiddenWordsFile}");

            this.ignoredSymbols = ignoredSymbols != null ? 
                new HashSet<char>(ignoredSymbols) : 
                new HashSet<char>(DefaultIgnoredSymbols);

            forbiddenWords = new List<string>();
            var normalizedToOriginal = new Dictionary<string, string>();

            foreach (string? line in File.ReadLines(forbiddenWordsFile))
            {
                string word = line?.Trim() ?? "";
                if (!string.IsNullOrEmpty(word))
                {
                    string normalized = NormalizeWord(word);
                    if (normalizedToOriginal.TryGetValue(normalized, out string? existingWord))
                    {
                        Console.Error.WriteLine($"Warning: '{word}' is equivalent to existing word " +
                                              $"'{existingWord}' after normalization");
                        continue;
                    }
                    normalizedToOriginal[normalized] = word;
                    forbiddenWords.Add(word);
                }
            }

            try
            {
                root = new TrieNode { IsRoot = true };
                BuildTrie();
                BuildFailureLinks();
            }
            catch (Exception ex)
            {
                throw new Exception($"Failed to initialize Forword: {ex.Message}", ex);
            }
        }

        private void BuildTrie()
        {
            foreach (var word in forbiddenWords)
            {
                var normalized = NormalizeWord(word);
                var node = root;
                foreach (var ch in normalized)
                {
                    if (!node.Children.ContainsKey(ch))
                    {
                        node.Children[ch] = new TrieNode();
                    }
                    node = node.Children[ch];
                }
                node.Output.Add(normalized);
            }
        }

        private void BuildFailureLinks()
        {
            var queue = new Queue<TrieNode>();

            // Set failure links for depth 1 nodes
            foreach (var child in root.Children.Values)
            {
                child.Fail = root;
                queue.Enqueue(child);
            }

            // Build failure links for deeper nodes
            while (queue.Count > 0)
            {
                var current = queue.Dequeue();

                foreach (var pair in current.Children)
                {
                    var child = pair.Value;
                    queue.Enqueue(child);

                    var failure = current.Fail!;
                    while (failure != null && !failure.Children.ContainsKey(pair.Key))
                    {
                        failure = failure.Fail;
                    }

                    child.Fail = failure?.Children.GetValueOrDefault(pair.Key) ?? root;

                    // Add outputs from failure node
                    if (child.Fail != null)
                    {
                        foreach (var output in child.Fail.Output)
                        {
                            child.Output.Add(output);
                        }
                    }
                }
            }
        }

        public bool Search(string text)
        {
            if (string.IsNullOrEmpty(text))
                return false;

            var normalizedText = NormalizeForSearch(text);
            TrieNode? current = root;

            foreach (var ch in normalizedText)
            {
                while (current != null && !current.Children.ContainsKey(ch))
                {
                    current = current.Fail;
                }

                if (current == null)
                {
                    current = root;
                    continue;
                }

                current = current.Children.GetValueOrDefault(ch);
                if (current == null)
                {
                    current = root;
                    continue;
                }

                if (current.Output.Any())
                    return true;
            }

            return false;
        }

        public string Replace(string text, string replacement = "***")
        {
            if (string.IsNullOrEmpty(text))
                return text;

            var normalizedText = NormalizeForSearch(text);
            var result = text;
            TrieNode? current = root;
            var matches = new List<(int start, int end)>();
            var pos = 0;

            // Build mapping between normalized and original text positions
            var normToOrig = new Dictionary<int, int>();
            var origToNorm = new Dictionary<int, int>();
            var normPos = 0;

            // First pass: find word boundaries and build position mappings
            for (int i = 0; i < text.Length; i++)
            {
                if (IsWordChar(text[i]))
                {
                    normToOrig[normPos] = i;
                    origToNorm[i] = normPos;
                    normPos++;
                }
            }

            // Find all matches
            while (pos < normalizedText.Length)
            {
                var ch = normalizedText[pos];
                
                while (current != null && !current.Children.ContainsKey(ch))
                {
                    current = current.Fail;
                }
                
                if (current == null)
                {
                    current = root;
                    pos++;
                    continue;
                }

                current = current.Children.GetValueOrDefault(ch);
                if (current == null)
                {
                    current = root;
                    pos++;
                    continue;
                }

                foreach (var word in current.Output)
                {
                    var normalizedWord = NormalizeWord(word);
                    var wordPos = pos - normalizedWord.Length + 1;
                    
                    if (wordPos <= pos && 
                        normalizedText.Substring(wordPos, normalizedWord.Length) == normalizedWord)
                    {
                        if (normToOrig.ContainsKey(wordPos) && normToOrig.ContainsKey(pos))
                        {
                            var origStart = normToOrig[wordPos];
                            // Find the last word character after the match
                            var origEnd = origStart;
                            for (int i = origStart; i < text.Length; i++)
                            {
                                if (IsWordChar(text[i]) && origToNorm.ContainsKey(i))
                                {
                                    var normIndex = origToNorm[i];
                                    if (normIndex <= pos)
                                    {
                                        origEnd = i;
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                            }
                            // If there is a whitespace immediately after the match, include it.
                            if (origEnd < text.Length - 1 && char.IsWhiteSpace(text[origEnd + 1]))
                            {
                                origEnd++;
                            }

                            matches.Add((origStart, origEnd + 1));
                        }
                    }
                }
                pos++;
            }

            // For overlapping matches starting at the same index, keep only the longest match
            var filteredMatches = matches
                .GroupBy(m => m.start)
                .Select(g => g.OrderByDescending(m => m.end).First())
                .ToList();

            // Replace matches from end to start in the filtered list.
            foreach (var match in filteredMatches.OrderByDescending(m => m.start))
            {
                var (start, end) = match;
                // Extend boundary to include adjacent Thai diacritic marks
                while (end < text.Length && IsThaiDiacritic(text[end]))
                {
                    end++;
                }

                var prefix = result.Substring(0, start);
                var suffix = end < result.Length ? result.Substring(end).TrimStart() : "";
                
                // 매칭된 단어 앞의 공백을 하나만 유지
                if (prefix.Length > 0)
                {
                    var lastChar = prefix[prefix.Length - 1];
                    prefix = prefix.TrimEnd();
                    
                    // 접두사가 비어있지 않으면 공백 추가
                    if (!string.IsNullOrEmpty(prefix))
                    {
                        prefix = prefix + " ";
                    }
                }
                
                // 접미사가 비어있지 않으면 공백 추가
                if (!string.IsNullOrEmpty(suffix))
                {
                    suffix = " " + suffix;
                }
                
                result = prefix + replacement + suffix;
            }

            return result;
        }

        private bool IsWordChar(char ch)
        {
            // Basic Latin letters and numbers
            if (char.IsLetterOrDigit(ch)) return true;
            
            int code = (int)ch;
            
            // Hangul ranges
            if (code >= 0xAC00 && code <= 0xD7AF) return true;  // Syllables
            if (code >= 0x1100 && code <= 0x11FF) return true;  // Jamo
            if (code >= 0x3130 && code <= 0x318F) return true;  // Compatibility Jamo
            
            // CJK Unified Ideographs
            if (code >= 0x4E00 && code <= 0x9FFF) return true;
            
            // Japanese
            if (code >= 0x3040 && code <= 0x309F) return true;  // Hiragana
            if (code >= 0x30A0 && code <= 0x30FF) return true;  // Katakana
            
            // Russian (Cyrillic)
            if (code >= 0x0400 && code <= 0x04FF) return true;
            
            // Spanish/Italian
            if (code >= 0x1E00 && code <= 0x1EFF) return true;  // Latin Extended Additional
            if (code >= 0x0100 && code <= 0x017F) return true;  // Latin Extended-A
            
            return false;
        }

        private bool IsCJK(char ch)
        {
            int code = (int)ch;
            
            // CJK Unified Ideographs
            if (code >= 0x4E00 && code <= 0x9FFF) return true;
            
            // Japanese
            if (code >= 0x3040 && code <= 0x309F) return true;  // Hiragana
            if (code >= 0x30A0 && code <= 0x30FF) return true;  // Katakana
            
            return false;
        }
    }
} 