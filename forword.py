from typing import List, Dict, Set, Optional
from dataclasses import dataclass, field
import re
import os
import sys


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
    
    @staticmethod
    def _normalize_word(word: str) -> str:
        """Normalize a word by removing spaces and converting to lowercase."""
        # First normalize accented characters
        normalized = ''
        for c in word:
            # Convert to lowercase
            if c.isupper():
                c = c.lower()
            
            # Map accented characters
            code = ord(c)
            if c in 'àáäâ': c = 'a'
            elif c in 'èéëê': c = 'e'
            elif c in 'ìíïî': c = 'i'
            elif c in 'òóöô': c = 'o'
            elif c in 'ùúüû': c = 'u'
            elif c == 'ñ': c = 'n'
            elif c == 'ß': 
                normalized += 'ss'
                continue
            # Skip combining diacritical marks
            elif 0x0300 <= code <= 0x036F:
                continue
            
            # Only keep alphanumeric characters
            if c.isalnum():
                normalized += c
        
        return normalized

    def __init__(self, forbidden_words_file: str):
        """
        Initialize Forword with a file containing forbidden words.
        
        Args:
            forbidden_words_file: Path to the file containing forbidden words (one per line)
        """
        if not os.path.exists(forbidden_words_file):
            raise FileNotFoundError(f"Forbidden words file not found: {forbidden_words_file}")

        self.root = TrieNode(is_root=True)
        self.forbidden_words = []
        normalized_to_original = {}  # Keep track of normalized forms

        with open(forbidden_words_file, 'r', encoding='utf-8') as f:
            for line in f:
                word = line.strip()
                if word:
                    normalized = self._normalize_text(word)
                    if normalized in normalized_to_original:
                        print(f"Warning: '{word}' is equivalent to existing word "
                              f"'{normalized_to_original[normalized]}' after normalization",
                              file=sys.stderr)
                        continue
                    normalized_to_original[normalized] = word
                    self.forbidden_words.append(word)

        self._build_trie()
        self._build_failure_links()

    def _build_trie(self) -> None:
        """Build trie structure from forbidden words."""
        for word in self.forbidden_words:
            normalized = self._normalize_text(word)
            node = self.root
            for char in normalized:
                node = node.children.setdefault(char, TrieNode())
            node.output.add(normalized)

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

    def _is_word_char(self, ch: str) -> bool:
        """Check if character is a word character in any supported language."""
        code = ord(ch)
        
        # Basic Latin letters and numbers
        if ch.isalnum():
            return True
            
        # Hangul ranges
        if 0xAC00 <= code <= 0xD7AF:  # Syllables
            return True
        if 0x1100 <= code <= 0x11FF:  # Jamo
            return True
        if 0x3130 <= code <= 0x318F:  # Compatibility Jamo
            return True
            
        # CJK Unified Ideographs
        if 0x4E00 <= code <= 0x9FFF:
            return True
            
        # Japanese
        if 0x3040 <= code <= 0x309F:  # Hiragana
            return True
        if 0x30A0 <= code <= 0x30FF:  # Katakana
            return True
            
        # Russian (Cyrillic)
        if 0x0400 <= code <= 0x04FF:
            return True
            
        # Spanish/Italian
        if 0x1E00 <= code <= 0x1EFF:  # Latin Extended Additional
            return True
        if 0x0100 <= code <= 0x017F:  # Latin Extended-A
            return True
            
        return False

    def _normalize_text(self, text: str) -> str:
        """Normalize text by converting to lowercase and removing accents."""
        normalized = ''
        for c in text:
            # Convert to lowercase
            if c.isupper():
                c = c.lower()
            
            # Map accented characters
            if c in 'àáäâ': c = 'a'
            elif c in 'èéëê': c = 'e'
            elif c in 'ìíïî': c = 'i'
            elif c in 'òóöô': c = 'o'
            elif c in 'ùúüû': c = 'u'
            elif c == 'ñ': c = 'n'
            elif c == 'ß': 
                normalized += 'ss'
                continue
            # Skip combining diacritical marks
            elif 0x0300 <= ord(c) <= 0x036F:
                continue
            
            # Only keep word characters
            if not c.isspace() and self._is_word_char(c):
                normalized += c
        
        return normalized

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
        matches = set()
        pos = 0

        # Build mapping between normalized and original text positions
        norm_to_orig = {}
        orig_to_norm = {}
        norm_pos = 0
        
        # First pass: find word boundaries and build position mappings
        words = []  # Store (start, end, is_alnum) tuples
        word_start = 0
        
        i = 0
        while i < len(text):
            # Skip leading spaces
            while i < len(text) and text[i].isspace():
                i += 1
            if i >= len(text):
                break
            
            # Mark word start
            word_start = i
            
            # Find word end
            while i < len(text) and not text[i].isspace():
                if text[i].isalnum():
                    norm_to_orig[norm_pos] = i
                    orig_to_norm[i] = norm_pos
                    norm_pos += 1
                i += 1
            
            # Store word boundary and type
            has_alnum = any(ch.isalnum() for ch in text[word_start:i])
            words.append((word_start, i, has_alnum))

        # Find all matches
        while pos < len(normalized_text):
            char = normalized_text[pos]
            
            while current != self.root and char not in current.children:
                current = current.fail
            
            if char in current.children:
                current = current.children[char]
                
                if current.output:
                    for word in current.output:
                        normalized_word = self._normalize_text(word)
                        word_pos = pos - len(normalized_word) + 1
                        
                        if word_pos >= 0 and normalized_text[word_pos:pos + 1] == normalized_word:
                            if word_pos in norm_to_orig and pos in norm_to_orig:
                                orig_start = norm_to_orig[word_pos]
                                orig_end = norm_to_orig[pos]
                                
                                # Extend boundaries to include adjacent spaces
                                while orig_start > 0 and text[orig_start - 1].isspace():
                                    orig_start -= 1
                                while orig_end + 1 < len(text) and text[orig_end + 1].isspace():
                                    orig_end += 1
                                    
                                matches.add((orig_start, orig_end + 1))
            
            pos += 1

        # Among overlapping matches, choose the shortest interval.
        filtered = []
        for m in sorted(matches, key=lambda x: (x[0], x[1]-x[0])):
            # 만약 이미 선택한 구간 중, m를 완전히 포함하면서 더 짧은(즉, 덜 확장된) 구간이 있다면 m은 선택하지 않음.
            if not any(sel != m and sel[0] <= m[0] and m[1] <= sel[1] and (sel[1]-sel[0] < m[1]-m[0]) for sel in filtered):
                filtered.append(m)

        # Replace matches from end to start in the filtered list.
        for start, end in sorted(filtered, reverse=True):
            prefix = result[:start]
            suffix = result[end:]
            
            # Ensure single space before and after replacement
            if prefix and not prefix[-1].isspace():
                prefix += " "
            if suffix and not suffix[0].isspace():
                suffix = " " + suffix
            
            result = prefix + replacement + suffix

        return result

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