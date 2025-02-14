from typing import List, Dict, Set, Optional
from dataclasses import dataclass, field
import re


@dataclass
class TrieNode:
    children: Dict[str, 'TrieNode'] = field(default_factory=dict)
    fail: Optional['TrieNode'] = None
    output: Set[str] = field(default_factory=set)
    is_root: bool = False


class Forword:
    """
    A class for searching and replacing forbidden words in text.
    Uses the Aho-Corasick algorithm for efficient string matching.
    """
    
    def __init__(self, forbidden_words_file: str):
        """
        Initialize Forword with a file containing forbidden words.
        
        Args:
            forbidden_words_file: Path to the file containing forbidden words (one per line)
        """
        self.root = TrieNode(is_root=True)
        self.forbidden_words = self._load_forbidden_words(forbidden_words_file)
        self._build_trie()
        self._build_failure_links()

    def _load_forbidden_words(self, file_path: str) -> List[str]:
        """Load forbidden words from file."""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                return [line.strip() for line in f if line.strip()]
        except Exception as e:
            raise RuntimeError(f"Failed to load forbidden words file: {e}")

    def _build_trie(self) -> None:
        """Build trie structure from forbidden words."""
        for word in self.forbidden_words:
            node = self.root
            for char in word:
                node = node.children.setdefault(char, TrieNode())
            node.output.add(word)

    def _build_failure_links(self) -> None:
        """Build failure links for Aho-Corasick algorithm."""
        queue = []
        
        # Set failure links for depth 1 nodes
        for node in self.root.children.values():
            node.fail = self.root
            queue.append(node)
            
        # Build failure links for deeper nodes
        while queue:
            current = queue.pop(0)
            
            for char, child in current.children.items():
                queue.append(child)
                failure = current.fail
                
                while failure and char not in failure.children:
                    failure = failure.fail
                    
                child.fail = failure.children.get(char) if failure else self.root
                child.output.update(child.fail.output)

    def _normalize_text(self, text: str) -> str:
        """Remove spaces and symbols from text."""
        return re.sub(r'[\s\W_]', '', text)

    def search(self, text: str) -> bool:
        """
        Search for forbidden words in the given text.
        
        Args:
            text: Text to search for forbidden words
            
        Returns:
            bool: True if any forbidden word is found, False otherwise
        """
        if not text:
            return False

        normalized_text = self._normalize_text(text)
        current = self.root

        for char in normalized_text:
            while current is not None and char not in current.children:
                current = current.fail
            
            if current is None:
                current = self.root
                continue
                
            current = current.children[char]
            if current.output:
                return True

        return False

    def replace(self, text: str, replacement: str = "***") -> str:
        """
        Replace forbidden words in the text with the given replacement string.
        
        Args:
            text: Text to process
            replacement: String to replace forbidden words with
            
        Returns:
            str: Text with forbidden words replaced
        """
        if not text:
            return text

        result = text
        normalized_text = self._normalize_text(text)
        current = self.root
        matches = []
        pos = 0

        while pos < len(normalized_text):
            char = normalized_text[pos]
            
            while current is not None and char not in current.children:
                current = current.fail
            
            if current is None:
                current = self.root
                pos += 1
                continue
                
            current = current.children[char]
            
            if current.output:
                for word in current.output:
                    normalized_word = self._normalize_text(word)
                    word_pos = pos - len(normalized_word) + 1
                    if normalized_text[word_pos:pos + 1] == normalized_word:
                        matches.append((word_pos, pos + 1))
            
            pos += 1

        # Replace matches from end to start to maintain correct positions
        for start, end in sorted(matches, reverse=True):
            # Find the corresponding position in original text
            orig_start = start
            orig_end = end
            
            # Extend match boundaries to include spaces and symbols
            while orig_start > 0 and text[orig_start-1].isspace():
                orig_start -= 1
            while orig_end < len(text) and text[orig_end].isspace():
                orig_end += 1
                
            result = result[:orig_start] + replacement + result[orig_end:]

        return result 