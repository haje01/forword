using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace Forword
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

            var normalizedText = NormalizeText(text);
            var matches = new List<(int Start, int End)>();
            var current = root;

            for (int pos = 0; pos < normalizedText.Length; pos++)
            {
                var ch = normalizedText[pos];

                while (current != root && !current.Children.ContainsKey(ch))
                {
                    current = current.Fail;
                }

                if (current.Children.TryGetValue(ch, out var next))
                {
                    current = next;

                    foreach (var word in current.Output)
                    {
                        var wordPos = pos - word.Length + 1;
                        if (normalizedText.Substring(wordPos, word.Length) == NormalizeText(word))
                        {
                            matches.Add((wordPos, pos + 1));
                        }
                    }
                }
            }

            // Sort matches in reverse order to maintain correct positions
            matches.Sort((a, b) => b.Start.CompareTo(a.Start));

            var result = new StringBuilder(text);
            foreach (var (start, end) in matches)
            {
                var origStart = start;
                var origEnd = end;

                // Extend match boundaries to include spaces
                while (origStart > 0 && char.IsWhiteSpace(text[origStart - 1]))
                {
                    origStart--;
                }
                while (origEnd < text.Length && char.IsWhiteSpace(text[origEnd]))
                {
                    origEnd++;
                }

                result.Remove(origStart, origEnd - origStart);
                result.Insert(origStart, replacement);
            }

            return result.ToString();
        }
    }
} 