import unittest
import os
import tempfile
from forword import Forword
import sys
from io import StringIO

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
            # French
            f.write("français\n")   # French
            f.write("garçon\n")     # boy
            f.write("café\n")       # coffee
            # Portuguese
            f.write("coração\n")    # heart
            f.write("não\n")        # no
            f.write("ação\n")       # action
            # Thai
            f.write("สวัสดี\n")      # hello
            f.write("ขอบคุณ\n")      # thank you
            # Existing test words
            f.write("坏话\n")      # Chinese
            f.write("ばか\n")      # Japanese
            f.write("плохой\n")    # Russian
            f.write("málaga\n")    # Spanish
            f.write("cattività\n")  # Italian
        
        forword = Forword(self.forbidden_words_file)
        
        # Test French
        self.assertTrue(forword.search("Je parle français"))
        self.assertTrue(forword.search("Le garcon est la"))  # without ç
        self.assertTrue(forword.search("Un café noir"))
        self.assertEqual(
            forword.replace("Je parle français bien"),
            "Je parle *** bien"
        )
        
        # Test Portuguese
        self.assertTrue(forword.search("Meu coração"))
        self.assertTrue(forword.search("Eu não sei"))
        self.assertTrue(forword.search("Uma ação importante"))
        self.assertEqual(
            forword.replace("Meu coração bate"),
            "Meu *** bate"
        )
        
        # Test Thai
        self.assertTrue(forword.search("พูดว่า สวัสดี ครับ"))
        self.assertTrue(forword.search("พูด ขอบคุณ ครับ"))
        self.assertEqual(
            forword.replace("พูดว่า สวัสดี ครับ"),
            "พูดว่า *** ครับ"
        )
        
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

    def test_duplicate_word_warning(self):
        # Create temporary file with duplicate words
        with tempfile.NamedTemporaryFile(mode='w', encoding='utf-8', delete=False) as f:
            f.write("badword\n"
                   "b a d w o r d\n"    # Same as "badword" after normalization
                   "BAD-WORD\n"         # Same as "badword" after normalization
                   "málaga\n"
                   "malaga\n"           # Same as "málaga" after normalization
                   "scheiße\n"
                   "scheisse\n")        # Same as "scheiße" after normalization
            temp_file = f.name

        # Capture stderr
        stderr = StringIO()
        sys.stderr = stderr

        # Create forword instance (this should trigger warnings)
        forword = Forword(temp_file)

        # Restore stderr
        sys.stderr = sys.__stderr__

        # Get warning messages
        warnings = stderr.getvalue()

        # Check for expected warnings
        self.assertIn("'b a d w o r d' is equivalent to existing word 'badword'", warnings)
        self.assertIn("'BAD-WORD' is equivalent to existing word 'badword'", warnings)
        self.assertIn("'malaga' is equivalent to existing word 'málaga'", warnings)
        self.assertIn("'scheisse' is equivalent to existing word 'scheiße'", warnings)

        # Test that words are properly detected
        self.assertTrue(forword.search("This is a badword"))
        self.assertTrue(forword.search("This is a b a d w o r d"))
        self.assertTrue(forword.search("This is málaga"))
        self.assertTrue(forword.search("This is malaga"))
        self.assertTrue(forword.search("This is scheiße"))
        self.assertTrue(forword.search("This is scheisse"))

        # Clean up
        os.unlink(temp_file)

    def test_custom_ignored_symbols(self):
        # 임시 금칙어 파일 생성
        with tempfile.NamedTemporaryFile(mode='w', delete=False) as temp_file:
            temp_file.write('badword\ntest\n')
            temp_file_path = temp_file.name
        
        try:
            # 기본 무시 기호로 테스트
            default_forword = Forword(temp_file_path)
            self.assertTrue(default_forword.search('b-a-d-w-o-r-d'))
            self.assertTrue(default_forword.search('t.e.s.t'))
            self.assertTrue(default_forword.search('b a d w o r d'))
            
            # 사용자 정의 무시 기호로 테스트 (하이픈과 공백만)
            custom_forword = Forword(temp_file_path, ignored_symbols={'-', ' '})
            self.assertTrue(custom_forword.search('b-a-d-w-o-r-d'))
            self.assertTrue(custom_forword.search('b a d w o r d'))
            self.assertFalse(custom_forword.search('b.a.d.w.o.r.d'))
            self.assertFalse(custom_forword.search('t.e.s.t'))
            
            # 무시 기호 없이 테스트
            strict_forword = Forword(temp_file_path, ignored_symbols=set())
            self.assertFalse(strict_forword.search('b-a-d-w-o-r-d'))
            self.assertFalse(strict_forword.search('b a d w o r d'))
            self.assertFalse(strict_forword.search('b.a.d.w.o.r.d'))
            self.assertTrue(strict_forword.search('badword'))
            
            # 치환 기능 테스트
            self.assertEqual(
                'This is ***',
                custom_forword.replace('This is b-a-d-w-o-r-d')
            )
            self.assertEqual(
                'This is ***',
                custom_forword.replace('This is b a d w o r d')
            )
            self.assertEqual(
                'This is b.a.d.w.o.r.d',
                custom_forword.replace('This is b.a.d.w.o.r.d')
            )
        finally:
            os.unlink(temp_file_path)

if __name__ == '__main__':
    unittest.main() 