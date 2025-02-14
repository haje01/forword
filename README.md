# forword
A simple library for quickly searching forbidden words in text messages

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