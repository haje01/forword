# forword
A simple library for quickly searching forbidden words in text messages

## Table of Contents
- [English](#english)
- [한국어](#한국어)

<a name="english"></a>
## English

### Introduction
forword is a simple library for quickly searching forbidden words in text messages. It reads forbidden words from a .csv file and quickly finds them in given text messages. The library is available for various programming languages.

### Key Features
- Loading text files containing forbidden words
- Fast search of forbidden words in given text
- Search capability even when forbidden words contain spaces or symbols
- Replacement function for detected forbidden words
- Multi-language support (UTF-8)

### Supported Languages
- C++ (11 or higher)
- C# (.NET Standard 2.0 or higher)
- Python (3.7 or higher)

### Installation
The library is provided as a single source code file (header-only library for C++), making it easy to add to your project.

Add the appropriate file for your programming language to your project from the following directory structure:

```
forword/
├── forword.h
├── forword.cs
└── forword.py
```

### Example Code

Assuming we have a forbidden words text file like this:

`forbidden_words.txt`
```
stupid
idiot
```

Here are example codes for each programming language:

#### C++
```cpp
#include "forword.h"

int main() {
    // Path to forbidden words file
    const char* forbidden_words_file = "forbidden_words.txt";

    // Create forbidden word search object
    Forword forword(forbidden_words_file);

    // Text to search
    const char* text = "This message contains forbidden words.";

    // Search for forbidden words
    bool is_forbidden = forword.search(text);

    // Print results
    printf("Forbidden word search result: %s\n", is_forbidden ? "Found" : "Not found");

    // Replace forbidden words
    const char* replaced_text = forword.replace(text, "***");
    printf("Replaced text: %s\n", replaced_text);

    return 0;
}
```

#### C#
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
        const string text = "This message contains forbidden words.";

        // Search for forbidden words
        bool is_forbidden = forword.Search(text);

        // Print results
        Console.WriteLine("Forbidden word search result: " + (is_forbidden ? "Found" : "Not found"));

        // Replace forbidden words
        string replaced_text = forword.Replace(text, "***");
        Console.WriteLine("Replaced text: " + replaced_text);
    }
}
```

#### Python
```python
from forword import Forword

forword = Forword("forbidden_words.txt")

text = "This message contains forbidden words."

is_forbidden = forword.search(text)
print(f"Forbidden word search result: {'Found' if is_forbidden else 'Not found'}")

replaced_text = forword.replace(text, "***")
print(f"Replaced text: {replaced_text}")
```

### Important Notes
- The forbidden words text file should contain one word per line.
- There's no need to register new forbidden words by adding spaces or symbols to existing ones.
- For frequent calls, it is recommended to initialize the Forword object once and reuse it.


## Testing (For Forword Developers)

### Prerequisites
Required tools for running tests in each language:

#### Python
- Python 3.7 or higher
- unittest (Python standard package)

#### C++
- Google Test
- C++17 compatible compiler
- CMake 3.10 or higher

#### C#
- .NET SDK
- MSTest

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
dotnet test
```

### Test Cases
The following test cases are implemented for all language versions:

1. **Basic Search Functionality**
   - Common forbidden word search
   - Handling cases with no forbidden words

2. **Spaces and Symbols Handling**
   - Words with spaces
   - Words with symbols
   - Irregular spacing

3. **Multi-language Support**
   - Korean text search
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
└── tests/
    ├── test_forword.cpp
    ├── ForwordTests.cs
    └── test_forword.py
```

<a name="한국어"></a>
## 한국어

## 소개 
forword는 텍스트 메시지에서 금칙어를 빠르게 검색하기 위한 간단한 라이브러리입니다.
금칙어 .csv 파일을 읽어 주어진 텍스트 메시지에서 빠르게 금칙어를 찾아줍니다. 다양한 프로그래밍 언어를 위한 라이브러리 형태로 제공됩니다.

### 주요 기능
- 금칙어가 등록된 텍스트 파일 로딩
- 주어진 텍스트 내 금칙어 고속 검색
- 텍스트 내 금칙어에 공백 또는 기호가 포함되어 있어도 검색 가능
- 검출된 금칙어 치환 기능
- 다국어 지원 (UTF-8)

### 지원 언어
- C++ (11 이상)
- C# (.NET Standard 2.0 이상)
- Python (3.7 이상)


### 설치 방법
단일 소스 코드 파일 (C++ 는 헤드 전용 라이브러리) 형태로 제공되기에 프로젝트에 추가하기 쉽습니다.

아래와 같은 디렉토리 구성으로, 원하시는 프로그래밍 언어에 맞는 파일을 프로젝트에 추가하시면 됩니다.

```
forword/
├── forword.h
├── forword.cs
└── forword.py
```

## 예제 코드

아래와 같은 금칙어 텍스트 파일이 있다고 가정할 때,

`forbidden_words.txt`

```
바보
멍청이
```

각 프로그래밍 언어에 맞는 예제 코드는 아래와 같습니다.

### C++
```cpp
#include "forword.h"

int main() {
    // 금칙어 파일 경로
    const char* forbidden_words_file = "forbidden_words.txt";

    // 금칙어 검색 객체 생성
    Forword forword(forbidden_words_file);

    // 검색할 텍스트
    const char* text = "이 메시지에는 금칙어가 포함되어 있습니다.";

    // 금칙어 검색
    bool is_forbidden = forword.search(text);

    // 결과 출력
    printf("금칙어 검색 결과: %s\n", is_forbidden ? "찾음" : "못찾음");

    // 금칙어 치환
    const char* replaced_text = forword.replace(text, "***");
    printf("치환된 텍스트: %s\n", replaced_text);

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
        // 금칙어 파일 경로
        const string forbidden_words_file = "forbidden_words.txt";

        // 금칙어 검색 객체 생성
        Forword forword = new Forword(forbidden_words_file);

        // 검색할 텍스트
        const string text = "이 메시지에는 금칙어가 포함되어 있습니다.";

        // 금칙어 검색
        bool is_forbidden = forword.Search(text);

        // 결과 출력
        Console.WriteLine("금칙어 검색 결과: " + (is_forbidden ? "찾음" : "못찾음"));

        // 금칙어 치환
        string replaced_text = forword.Replace(text, "***");
        Console.WriteLine("치환된 텍스트: " + replaced_text);
    }
}
```

### Python
```python
from forword import Forword

forword = Forword("forbidden_words.txt")

text = "이 메시지에는 금칙어가 포함되어 있습니다."

is_forbidden = forword.search(text)
print(f"금칙어 검색 결과: {'찾음' if is_forbidden else '못찾음'}")

replaced_text = forword.replace(text, "***")
print(f"치환된 텍스트: {replaced_text}")
```


## 주의할 점 
- 금칙어 텍스트 파일은 한 줄에 한 단어씩 있는 텍스트 파일이어야 합니다.
- 같은 금칙어에 공백 또는 기호를 추가하여 새로운 금칙어로 등록하실 필요는 없습니다.
- 빈번하게 호출하실 경우 Forword 객체를 처음 한 번만 초기화하고, 재사용하시는 것을 권장합니다.


## 테스트하기 (Forword 개발자용)

### Prerequisites
각 언어별 테스트를 실행하기 위해 필요한 도구:

#### Python
- Python 3.7 이상
- unittest (Python 기본 패키지)

#### C++
- Google Test
- C++17 이상 지원 컴파일러
- CMake 3.10 이상

#### C#
- .NET SDK
- MSTest

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
dotnet test
```

### Test Cases
모든 언어 버전에 대해 동일한 테스트 케이스가 구현되어 있습니다:

1. **기본 검색 기능**
   - 일반적인 금칙어 검색
   - 금칙어가 없는 경우 처리

2. **공백과 기호 처리**
   - 금칙어 내 공백이 있는 경우
   - 기호가 포함된 경우
   - 불규칙한 공백 처리

3. **다국어 지원**
   - 한글 금칙어 검색
   - UTF-8 인코딩 처리

4. **치환 기능**
   - 기본 치환 동작
   - 공백과 기호가 포함된 경우 치환
   - 다국어 텍스트 치환

5. **예외 처리**
   - 빈 입력 처리
   - 파일 로딩 실패 처리

### Project Structure
```
forword/
├── forword.h
├── forword.cs
├── forword.py
└── tests/
    ├── test_forword.cpp
    ├── ForwordTests.cs
    └── test_forword.py
```
