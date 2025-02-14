import unittest
import os
import tempfile
from forword import Forword

class TestForword(unittest.TestCase):
    def setUp(self):
        # Create temporary forbidden words file
        self.temp_dir = tempfile.mkdtemp()
        self.forbidden_words_file = os.path.join(self.temp_dir, "forbidden_words.txt")
        with open(self.forbidden_words_file, "w", encoding="utf-8") as f:
            f.write("bad\nbadword\n나쁜말\n욕설")
        
        self.forword = Forword(self.forbidden_words_file)

    def tearDown(self):
        # Clean up temporary files
        os.remove(self.forbidden_words_file)
        os.rmdir(self.temp_dir)

    def test_search_basic(self):
        """Test basic search functionality"""
        self.assertTrue(self.forword.search("This is a bad word"))
        self.assertTrue(self.forword.search("This is a badword"))
        self.assertFalse(self.forword.search("This is good"))

    def test_search_with_spaces(self):
        """Test search with spaces and symbols"""
        self.assertTrue(self.forword.search("This is a b a d word"))
        self.assertTrue(self.forword.search("b-a-d"))
        self.assertTrue(self.forword.search("b.a.d"))

    def test_search_korean(self):
        """Test search with Korean text"""
        self.assertTrue(self.forword.search("이것은 나쁜말 입니다"))
        self.assertTrue(self.forword.search("이것은 욕설 입니다"))
        self.assertFalse(self.forword.search("이것은 좋은말 입니다"))

    def test_replace_basic(self):
        """Test basic replacement functionality"""
        self.assertEqual(
            self.forword.replace("This is a bad word"),
            "This is a *** word"
        )
        self.assertEqual(
            self.forword.replace("This is a badword"),
            "This is a ***"
        )

    def test_replace_with_spaces(self):
        """Test replacement with spaces and symbols"""
        self.assertEqual(
            self.forword.replace("This is a b a d word"),
            "This is a *** word"
        )
        self.assertEqual(
            self.forword.replace("b-a-d"),
            "***"
        )

    def test_replace_korean(self):
        """Test replacement with Korean text"""
        self.assertEqual(
            self.forword.replace("이것은 나쁜말 입니다"),
            "이것은 *** 입니다"
        )
        self.assertEqual(
            self.forword.replace("이것은 욕설 입니다"),
            "이것은 *** 입니다"
        )

    def test_empty_input(self):
        """Test with empty input"""
        self.assertFalse(self.forword.search(""))
        self.assertEqual(self.forword.replace(""), "")

if __name__ == '__main__':
    unittest.main() 