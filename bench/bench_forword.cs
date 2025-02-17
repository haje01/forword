using System;
using System.IO;
using System.Text;
using System.Diagnostics;
using ForwordLib;

class BenchForword
{
    static void Main()
    {
        // Create forbidden words file
        string forbidden_words_file = "forbidden_words.txt";
        File.WriteAllText(forbidden_words_file, "bad\nbadword\n나쁜말\n욕설", Encoding.UTF8);

        // Initialize Forword
        var forword = new Forword(forbidden_words_file);

        // Prepare test text
        string text = "이것은 나쁜말 입니다. This is a bad word. 여기에 욕설이 있습니다.";

        // Benchmark replace operation
        int iterations = 10000;
        var stopwatch = Stopwatch.StartNew();
        
        for (int i = 0; i < iterations; i++)
        {
            forword.Replace(text);
        }
        
        stopwatch.Stop();
        double elapsed_sec = stopwatch.ElapsedMilliseconds / 1000.0;
        double ops_per_sec = iterations / elapsed_sec;

        Console.WriteLine("C# Forword Benchmark");
        Console.WriteLine("--------------------");
        Console.WriteLine($"Operations: {iterations}");
        Console.WriteLine($"Total time: {elapsed_sec:F2} seconds");
        Console.WriteLine($"Ops/sec: {ops_per_sec:F2}");

        // Cleanup
        File.Delete(forbidden_words_file);
    }
} 