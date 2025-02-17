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
            public TrieNode Fail { get; set; }
            public HashSet<string> Output { get; } = new HashSet<string>();
            public bool IsRoot { get; set; }
        }

        private readonly TrieNode root;
        private readonly List<string> forbiddenWords;

        public Forword(string forbiddenWordsFile)
        {
            try
            {
                forbiddenWords = LoadForbiddenWords(forbiddenWordsFile);
                root = new TrieNode { IsRoot = true };
                BuildTrie();
                BuildFailureLinks();
            }
            catch (Exception ex)
            {
                throw new Exception($"Failed to initialize Forword: {ex.Message}", ex);
            }
        }

        private List<string> LoadForbiddenWords(string filePath)
        {
            try
            {
                return File.ReadAllLines(filePath, Encoding.UTF8)
                    .Where(line => !string.IsNullOrWhiteSpace(line))
                    .Select(line => line.Trim())
                    .ToList();
            }
            catch (Exception ex)
            {
                throw new Exception($"Failed to load forbidden words file: {ex.Message}", ex);
            }
        }

        private void BuildTrie()
        {
            foreach (var word in forbiddenWords)
            {
                var node = root;
                foreach (var ch in word)
                {
                    if (!node.Children.ContainsKey(ch))
                    {
                        node.Children[ch] = new TrieNode();
                    }
                    node = node.Children[ch];
                }
                node.Output.Add(word);
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

                    var failure = current.Fail;
                    while (failure != null && !failure.Children.ContainsKey(pair.Key))
                    {
                        failure = failure.Fail;
                    }

                    child.Fail = failure?.Children.GetValueOrDefault(pair.Key) ?? root;

                    // Add outputs from failure node
                    foreach (var output in child.Fail.Output)
                    {
                        child.Output.Add(output);
                    }
                }
            }
        }

        private string NormalizeText(string text)
        {
            return new string(text.Where(ch => !char.IsWhiteSpace(ch) && char.IsLetterOrDigit(ch)).ToArray());
        }

        public bool Search(string text)
        {
            if (string.IsNullOrEmpty(text))
                return false;

            var normalizedText = NormalizeText(text);
            var current = root;

            foreach (var ch in normalizedText)
            {
                while (current != root && !current.Children.ContainsKey(ch))
                {
                    current = current.Fail;
                }

                if (current.Children.TryGetValue(ch, out var next))
                {
                    current = next;
                    if (current.Output.Count > 0)
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        public string Replace(string text, string replacement = "***")
        {
            if (string.IsNullOrEmpty(text))
                return text;

            var result = text;
            var normalizedText = NormalizeText(text);
            var current = root;
            var matches = new SortedSet<(int start, int end)>();
            var pos = 0;

            // Build mapping between normalized and original text positions
            var normToOrig = new Dictionary<int, int>();
            var normPos = 0;

            // First pass: find word boundaries and build position mappings
            for (int i = 0; i < text.Length; i++)
            {
                if (char.IsLetterOrDigit(text[i]))
                {
                    normToOrig[normPos] = i;
                    normPos++;
                }
            }

            // Find all matches
            while (pos < normalizedText.Length)
            {
                var ch = normalizedText[pos];
                
                while (current != root && !current.Children.ContainsKey(ch))
                {
                    current = current.Fail;
                }
                
                if (current.Children.ContainsKey(ch))
                {
                    current = current.Children[ch];
                    
                    foreach (var word in current.Output)
                    {
                        var normalizedWord = NormalizeText(word);
                        var wordPos = pos - normalizedWord.Length + 1;
                        
                        if (wordPos <= pos && 
                            normalizedText.Substring(wordPos, normalizedWord.Length) == normalizedWord)
                        {
                            if (normToOrig.ContainsKey(wordPos) && normToOrig.ContainsKey(pos))
                            {
                                var origStart = normToOrig[wordPos];
                                var origEnd = normToOrig[pos];

                                // Extend boundaries to include adjacent spaces
                                while (origStart > 0 && char.IsWhiteSpace(text[origStart - 1]))
                                {
                                    origStart--;
                                }
                                while (origEnd < text.Length - 1 && char.IsWhiteSpace(text[origEnd + 1]))
                                {
                                    origEnd++;
                                }

                                matches.Add((origStart, origEnd + 1));
                            }
                        }
                    }
                }
                pos++;
            }

            // Replace matches from end to start
            foreach (var (start, end) in matches.Reverse())
            {
                var prefix = result.Substring(0, start);
                var suffix = end < result.Length ? result.Substring(end) : "";
                
                // Ensure single space before and after replacement
                if (prefix.Length > 0 && !char.IsWhiteSpace(prefix[prefix.Length - 1]))
                {
                    prefix += " ";
                }
                if (suffix.Length > 0 && !char.IsWhiteSpace(suffix[0]))
                {
                    suffix = " " + suffix;
                }
                
                result = prefix + replacement + suffix;
            }

            return result;
        }
    }
} 