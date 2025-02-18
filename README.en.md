# forword

[한국어 문서](README.md)

A simple library for quickly searching forbidden words in text messages.

## Introduction
forword is a simple library for quickly searching forbidden words in text messages.
It reads forbidden words from a .txt file and quickly finds them in given text messages. It is provided as a library for various programming languages.

### Key Features
- Loading text files containing forbidden words
- High-speed search for forbidden words in given text
- Ability to detect forbidden words even when they contain spaces or symbols
- Function to replace detected forbidden words
- Multi-language support (UTF-8)

### Supported Languages

The following languages and characters are supported. For some supported languages, accented characters are normalized to basic characters, and the search is case-insensitive.

- **Latin-based:**
  - English
  - French: à → a, é → e, ç → c, ÿ → y, etc.
  - Portuguese: ã → a, õ → o, ç → c, etc.
  - Spanish: ñ → n
  - Italian: à, è, é, etc.
  - German: ä → a, ö → o, ü → u, ß → ss

- **Asian Languages:**
  - Korean: Hangul syllables and Jamo
  - Japanese: Hiragana, Katakana
  - Chinese: CJK Unified Ideographs
  - Thai: Basic consonants and vowels, with proper handling of tone marks

- **Others:**
  - Russian: Cyrillic characters

#### Why Normalize Accented Characters

There are several key reasons why we normalize accented characters to basic characters for some languages:

1. Prevent Filter Bypass
- Prevent users from bypassing word filters using accented characters
- Example: "bád", "bàd", "bâd" should all be recognized as "bad"

2. Registration Convenience
- Administrators don't need to register every accent variation of forbidden words
- Example: Registering "café" will automatically detect "cafe", "café", "cafè", etc.

3. Consistent Search
- Treat words as identical regardless of their various accent representations
- Example: French words "élève", "eleve", "ÉLÈVE" are all treated the same

4. Performance Optimization
- Normalizing to basic ASCII characters enables faster string comparison
- Can also reduce memory usage

For these reasons, Forword provides accent normalization as a core feature.

### Supported Programming Languages
- C++ (11 or higher)
- C# (.NET Standard 2.0 or higher)
- Python (3.7 or higher)


### Installation
Since it's provided as a single source code file (header-only library for C++), it's easy to add to your project.

With the following directory structure, simply add the file that matches your preferred programming language to your project:

```
forword/
├── forword.h
├── forword.cs
└── forword.py
```

## Code Examples

Assuming you have a forbidden words text file like this:

`forbidden_words.txt`
```
bad
badword
```

> Note: There's no need to register variations of the same forbidden word with added spaces or symbols.

Here are examples for each programming language:

### C++
```cpp
#include "forword.h"

int main() {
    // Path to forbidden words file
    const char* forbidden_words_file = "forbidden_words.txt";

    // Create forbidden word search object
    Forword forword(forbidden_words_file);

    // Text to search
    const char* text = "This message contains a forbidden word.";

    // Search for forbidden words
    bool is_forbidden = forword.search(text);

    // Print results
    printf("Search result: %s\n", is_forbidden ? "Found" : "Not found");

    // Replace forbidden words
    const char* replaced_text = forword.replace(text, "***");
    printf("Replaced text: %s\n", replaced_text);

    return 0;
}
```

### C#
```csharp
using Forword;

class Program
{
    static void Main(string[] args)
    {
        // Path to forbidden words file
        const string forbidden_words_file = "forbidden_words.txt";

        // Create forbidden word search object
        Forword forword = new Forword(forbidden_words_file);

        // Text to search
        const string text = "This message contains a forbidden word.";

        // Search for forbidden words
        bool is_forbidden = forword.Search(text);

        // Print results
        Console.WriteLine("Search result: " + (is_forbidden ? "Found" : "Not found"));

        // Replace forbidden words
        string replaced_text = forword.Replace(text, "***");
        Console.WriteLine("Replaced text: " + replaced_text);
    }
}
```

### Python
```python
from forword import Forword

forword = Forword("forbidden_words.txt")

text = "This message contains a forbidden word."

is_forbidden = forword.search(text)
print(f"Search result: {'Found' if is_forbidden else 'Not found'}")

replaced_text = forword.replace(text, "***")
print(f"Replaced text: {replaced_text}")
```

## Custom Ignore Symbols Examples
This section demonstrates how to customize the forbidden word search behavior using user-defined ignore symbols.

### C#
```csharp
// Example: Only ignore hyphens('-') and spaces(' ')
var customSymbols = new[] { '-', ' ' };
Forword forwordCustom = new Forword("forbidden_words.txt", customSymbols);

if (forwordCustom.Search("b-a-d-w-o-r-d"))
{
    Console.WriteLine("Forbidden word detected.");
}
else
{
    Console.WriteLine("No forbidden word detected.");
}
```

### Python
```python
# Example: Only ignore hyphens('-') and spaces
custom_symbols = {'-', ' '}
forword = Forword("forbidden_words.txt", ignored_symbols=custom_symbols)

if forword.search("b-a-d-w-o-r-d"):
    print("Forbidden word detected.")
else:
    print("No forbidden word detected.")
```

### C++
```cpp
#include "forword.h"
#include <unordered_set>
#include <iostream>

int main(){
    // Example: Only ignore hyphens('-') and spaces
    std::unordered_set<char> customSymbols{'-', ' '};
    Forword forword("forbidden_words.txt", customSymbols);
    
    if(forword.search("b-a-d-w-o-r-d"))
        std::cout << "Forbidden word detected." << std::endl;
    else
        std::cout << "No forbidden word detected." << std::endl;
        
    return 0;
}
```

## Important Notes
- The forbidden words text file should contain one word per line.
- There's no need to register variations of the same forbidden word with added spaces or symbols.
- For frequent calls, it's recommended to initialize the Forword object once and reuse it.

## Forword Developers

### Prerequisites
Tools needed to run tests for each language:

#### Python
- Python 3.7 or higher
- unittest (Python standard package)

#### C++
- Google Test
- A C++17 compatible compiler
- CMake 3.10 or higher

#### C#
- .NET SDK
- MSTest

### Benchmarking
Benchmark tools are provided to measure the performance of each language implementation.

#### Python
```bash
python bench/bench_forword.py
```

#### C++
```bash
mkdir build && cd build
cmake ..
make
./forword_bench
```

#### C#
```bash
cd bench
dotnet run
```

Benchmarks are run under the following conditions:
- Same forbidden words file (bad, badword, badlanguage, profanity)
- Same input text ("This is a bad word. This contains profanity.")
- 10,000 iterations

Benchmark results include:
- Total number of executions
- Total time taken (seconds)
- Throughput (operations/sec)

### Running Tests

#### Python
```bash
python -m unittest tests/test_forword.py
```

#### C++
```bash
# Build and run tests
mkdir build && cd build
cmake ..
make
./forword_test
```

#### C#
```bash
dotnet test tests/test_forword.csproj
```

### Test Cases
The same test cases are implemented for all language versions:

1. **Basic Search Functionality**
   - Regular forbidden word search
   - Handling cases with no forbidden words

2. **Space and Symbol Handling**
   - Words with spaces
   - Words with symbols
   - Irregular spacing

3. **Multi-language Support**
   - Multi-language forbidden word search
   - UTF-8 encoding handling

4. **Replacement Function**
   - Basic replacement behavior
   - Replacement with spaces and symbols
   - Multi-language text replacement

5. **Exception Handling**
   - Empty input handling
   - File loading failure handling

### Project Structure
```
forword/
├── forword.h
├── forword.cs
├── forword.py
├── bench/
│   ├── bench_forword.cpp
│   ├── bench_forword.cs
│   └── bench_forword.py
└── tests/
    ├── test_forword.cpp
    ├── test_forword.cs
    └── test_forword.py
```

