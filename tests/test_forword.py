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
            "This is a ***"
        )
        self.assertEqual(
            self.forword.replace("This is a badword"),
            "This is a ***"
        )

    def test_replace_with_spaces(self):
        """Test replacement with spaces and symbols"""
        self.assertEqual(
            self.forword.replace("This is a b a d word"),
            "This is a ***"
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

    def test_words_with_whitespace(self):
        """Test words with leading/trailing whitespace in forbidden words file"""
        # Create temporary file with words containing whitespace
        with open(self.forbidden_words_file, "w", encoding="utf-8") as f:
            f.write("bad \n  badword\n\t나쁜말\n  욕설  \n")
        
        forword = Forword(self.forbidden_words_file)
        
        # Test search
        self.assertTrue(forword.search("This is a bad word"))
        self.assertTrue(forword.search("This is a badword"))
        self.assertTrue(forword.search("이것은 나쁜말 입니다"))
        self.assertTrue(forword.search("이것은 욕설 입니다"))

        # Test replace
        self.assertEqual(
            forword.replace("This is a bad word"),
            "This is a ***"
        )
        self.assertEqual(
            forword.replace("이것은 욕설 입니다"),
            "이것은 *** 입니다"
        )

    def test_multilingual_support(self):
        """Test support for various languages"""
        # Create temporary file with multilingual forbidden words
        with open(self.forbidden_words_file, "w", encoding="utf-8") as f:
            f.write("坏话\n")      # Chinese
            f.write("ばか\n")      # Japanese
            f.write("плохой\n")    # Russian
            f.write("málaga\n")    # Spanish
            f.write("cattività\n")  # Italian
        
        forword = Forword(self.forbidden_words_file)
        
        # Test Chinese
        self.assertTrue(forword.search("这是一个坏话的例子"))
        self.assertEqual(
            forword.replace("这是一个坏话的例子"),
            "这是一个 *** 的例子"
        )
        
        # Test Japanese
        self.assertTrue(forword.search("これはばかな例です"))
        self.assertEqual(
            forword.replace("これはばかな例です"),
            "これは *** な例です"
        )
        
        # Test Russian
        self.assertTrue(forword.search("это плохой пример"))
        self.assertEqual(
            forword.replace("это плохой пример"),
            "это *** пример"
        )
        
        # Test Spanish
        self.assertTrue(forword.search("es un ejemplo de málaga"))
        self.assertEqual(
            forword.replace("es un ejemplo de málaga"),
            "es un ejemplo de ***"
        )
        
        # Test Italian
        self.assertTrue(forword.search("un esempio di cattività"))
        self.assertEqual(
            forword.replace("un esempio di cattività"),
            "un esempio di ***"
        )

if __name__ == '__main__':
    unittest.main() 