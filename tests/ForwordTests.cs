using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using System.Text;

namespace Forword.Tests
{
    [TestClass]
    public class ForwordTests
    {
        private string tempDir;
        private string forbiddenWordsFile;
        private Forword forword;

        [TestInitialize]
        public void Setup()
        {
            // Create temporary directory and file
            tempDir = Path.Combine(Path.GetTempPath(), "forword_test");
            Directory.CreateDirectory(tempDir);
            forbiddenWordsFile = Path.Combine(tempDir, "forbidden_words.txt");

            // Create forbidden words file
            File.WriteAllText(forbiddenWordsFile, "bad\nbadword\n나쁜말\n욕설", Encoding.UTF8);

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
            Assert.AreEqual("This is a *** word", forword.Replace("This is a bad word"));
            Assert.AreEqual("This is a ***", forword.Replace("This is a badword"));
        }

        [TestMethod]
        public void TestReplaceWithSpaces()
        {
            Assert.AreEqual("This is a *** word", forword.Replace("This is a b a d word"));
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
    }
} 