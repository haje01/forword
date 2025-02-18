using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using System.Text;
using ForwordLib;

[TestClass]
public class TestForword
{
    private string tempDir = "";
    private string forbiddenWordsFile = "";
    private Forword forword = null!;

    [TestInitialize]
    public void Setup()
    {
        tempDir = Path.GetTempPath() + "forword_test";
        forbiddenWordsFile = Path.Combine(tempDir, "forbidden_words.txt");
        Directory.CreateDirectory(tempDir);

        File.WriteAllText(forbiddenWordsFile, "bad\nbadword\n나쁜말\n욕설");
        
        forword = new Forword(forbiddenWordsFile);
    }

    [TestCleanup]
    public void Cleanup()
    {
        // Clean up temporary files
        Directory.Delete(tempDir, true);
    }

    [TestMethod]
    public void TestBasicSearch()
    {
        Assert.IsTrue(forword.Search("This is a bad word"));
        Assert.IsTrue(forword.Search("This is a badword"));
        Assert.IsFalse(forword.Search("This is good"));
    }

    [TestMethod]
    public void TestSearchWithSpaces()
    {
        Assert.IsTrue(forword.Search("This is a b a d word"));
        Assert.IsTrue(forword.Search("b-a-d"));
        Assert.IsTrue(forword.Search("b.a.d"));
    }

    [TestMethod]
    public void TestSearchKorean()
    {
        Assert.IsTrue(forword.Search("이것은 나쁜말 입니다"));
        Assert.IsTrue(forword.Search("이것은 욕설 입니다"));
        Assert.IsFalse(forword.Search("이것은 좋은말 입니다"));
    }

    [TestMethod]
    public void TestBasicReplace()
    {
        Assert.AreEqual("This is a ***", forword.Replace("This is a bad word"));
        Assert.AreEqual("This is a ***", forword.Replace("This is a badword"));
    }

    [TestMethod]
    public void TestReplaceWithSpaces()
    {
        Assert.AreEqual("This is a ***", forword.Replace("This is a b a d word"));
        Assert.AreEqual("***", forword.Replace("b-a-d"));
    }

    [TestMethod]
    public void TestReplaceKorean()
    {
        Assert.AreEqual("이것은 *** 입니다", forword.Replace("이것은 나쁜말 입니다"));
        Assert.AreEqual("이것은 *** 입니다", forword.Replace("이것은 욕설 입니다"));
    }

    [TestMethod]
    public void TestEmptyInput()
    {
        Assert.IsFalse(forword.Search(""));
        Assert.AreEqual("", forword.Replace(""));
    }

    [TestMethod]
    public void TestWordsWithWhitespace()
    {
        // Create forbidden words file with whitespace
        File.WriteAllText(forbiddenWordsFile, "bad \n  badword\n\t나쁜말\n  욕설  \n", Encoding.UTF8);

        var forword = new ForwordLib.Forword(forbiddenWordsFile);

        // Test search
        Assert.IsTrue(forword.Search("This is a bad word"));
        Assert.IsTrue(forword.Search("This is a badword"));
        Assert.IsTrue(forword.Search("이것은 나쁜말 입니다"));
        Assert.IsTrue(forword.Search("이것은 욕설 입니다"));

        // Test replace
        Assert.AreEqual("This is a ***", forword.Replace("This is a bad word"));
        Assert.AreEqual("이것은 *** 입니다", forword.Replace("이것은 욕설 입니다"));
    }

    [TestMethod]
    public void TestMultilingualSupport()
    {
        // Create forbidden words file with multilingual words
        File.WriteAllText(forbiddenWordsFile,
            // French
            "français\n" +   // French
            "garçon\n" +     // boy
            "café\n" +       // coffee
            // Portuguese
            "coração\n" +    // heart
            "não\n" +        // no
            "ação\n" +       // action
            // Thai
            "สวัสดี\n" +      // hello
            "ขอบคุณ\n" +      // thank you
            // Existing test words
            "坏话\n" +      // Chinese
            "ばか\n" +      // Japanese
            "плохой\n" +    // Russian
            "málaga\n" +    // Spanish
            "cattività\n",  // Italian
            Encoding.UTF8);

        var forword = new ForwordLib.Forword(forbiddenWordsFile);

        // Test French
        Assert.IsTrue(forword.Search("Je parle français"));
        Assert.IsTrue(forword.Search("Le garcon est la"));  // without ç
        Assert.IsTrue(forword.Search("Un café noir"));
        Assert.AreEqual(
            "Je parle *** bien",
            forword.Replace("Je parle français bien")
        );
        
        // Test Portuguese
        Assert.IsTrue(forword.Search("Meu coração"));
        Assert.IsTrue(forword.Search("Eu não sei"));
        Assert.IsTrue(forword.Search("Uma ação importante"));
        Assert.AreEqual(
            "Meu *** bate",
            forword.Replace("Meu coração bate")
        );
        
        // Test Thai
        Assert.IsTrue(forword.Search("พูดว่า สวัสดี ครับ"));
        Assert.IsTrue(forword.Search("พูด ขอบคุณ ครับ"));
        Assert.AreEqual(
            "พูดว่า *** ครับ",
            forword.Replace("พูดว่า สวัสดี ครับ")
        );

        // Test Chinese
        Assert.IsTrue(forword.Search("这是一个坏话的例子"));
    }

    [TestMethod]
    public void TestDuplicateWordWarning()
    {
        // Create temporary file with duplicate words
        string tempFile = Path.GetTempFileName();
        File.WriteAllText(tempFile,
            "badword\n" +
            "b a d w o r d\n" +    // Same as "badword" after normalization
            "BAD-WORD\n" +         // Same as "badword" after normalization
            "málaga\n" +
            "malaga\n" +           // Same as "málaga" after normalization
            "scheiße\n" +
            "scheisse\n");         // Same as "scheiße" after normalization

        // Redirect Console.Error
        using var sw = new StringWriter();
        Console.SetError(sw);

        // Create forword instance (this should trigger warnings)
        var forword = new Forword(tempFile);

        // Get warning messages
        string warnings = sw.ToString();

        // Check for expected warnings
        Assert.IsTrue(warnings.Contains("'b a d w o r d' is equivalent to existing word 'badword'"));
        Assert.IsTrue(warnings.Contains("'BAD-WORD' is equivalent to existing word 'badword'"));
        Assert.IsTrue(warnings.Contains("'malaga' is equivalent to existing word 'málaga'"));
        Assert.IsTrue(warnings.Contains("'scheisse' is equivalent to existing word 'scheiße'"));

        // Test that words are properly detected
        Assert.IsTrue(forword.Search("This is a badword"));
        Assert.IsTrue(forword.Search("This is a b a d w o r d"));
        Assert.IsTrue(forword.Search("This is málaga"));
        Assert.IsTrue(forword.Search("This is malaga"));
        Assert.IsTrue(forword.Search("This is scheiße"));
        Assert.IsTrue(forword.Search("This is scheisse"));

        // Clean up
        File.Delete(tempFile);
    }

    [TestMethod]
    public void TestCustomIgnoredSymbols()
    {
        // Create temporary file with test words
        string tempFile = Path.GetTempFileName();
        File.WriteAllText(tempFile, "badword\ntest");
        
        try
        {
            // Test with default ignored symbols
            var defaultForword = new Forword(tempFile);
            Assert.IsTrue(defaultForword.Search("b-a-d-w-o-r-d"));
            Assert.IsTrue(defaultForword.Search("t.e.s.t"));
            Assert.IsTrue(defaultForword.Search("b a d w o r d"));
            
            // Test with custom ignored symbols (only hyphen and space)
            var customForword = new Forword(tempFile, new[] { '-', ' ' });
            Assert.IsTrue(customForword.Search("b-a-d-w-o-r-d"));
            Assert.IsTrue(customForword.Search("b a d w o r d"));
            Assert.IsFalse(customForword.Search("b.a.d.w.o.r.d"));  // period is not ignored
            Assert.IsFalse(customForword.Search("t.e.s.t"));        // period is not ignored
            
            // Test with empty ignored symbols
            var strictForword = new Forword(tempFile, Array.Empty<char>());
            Assert.IsFalse(strictForword.Search("b-a-d-w-o-r-d"));
            Assert.IsFalse(strictForword.Search("b a d w o r d"));
            Assert.IsFalse(strictForword.Search("b.a.d.w.o.r.d"));
            Assert.IsTrue(strictForword.Search("badword"));
            
            // Test replacement with custom ignored symbols
            Assert.AreEqual("This is ***", 
                customForword.Replace("This is b-a-d-w-o-r-d"));
            Assert.AreEqual("This is ***", 
                customForword.Replace("This is b a d w o r d"));
            Assert.AreEqual("This is b.a.d.w.o.r.d",  // not replaced
                customForword.Replace("This is b.a.d.w.o.r.d"));
        }
        finally
        {
            // Clean up
            File.Delete(tempFile);
        }
    }
} 