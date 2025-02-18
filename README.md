# forword

[English Documentation](README.en.md)

주어진 텍스트 메시지에서 금칙어를 빠르게 찾아주는 간단한 라이브러리.

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

다음과 같은 언어와 문자를 지원합니다. 모든 지원 언어에 대해 악센트 문자는 기본 문자로 정규화되며, 대소문자를 구분하지 않습니다.

- **라틴 계열:**
  - 영어 (English)
  - 프랑스어 (French): à → a, é → e, ç → c, ÿ → y 등
  - 포르투갈어 (Portuguese): ã → a, õ → o, ç → c 등
  - 스페인어 (Spanish): ñ → n
  - 이탈리아어 (Italian): à, è, é 등
  - 독일어 (German): ä → a, ö → o, ü → u, ß → ss

- **아시아 언어:**
  - 한국어: 한글 음절 및 자모
  - 일본어: 히라가나, 가타카나
  - 중국어: CJK 통합 한자
  - 타이어: 기본 자음과 모음, 성조 기호 처리

- **기타:**
  - 러시아어: 키릴 문자

#### 악센트 문자를 정규화하는 이유

일부 언어에 대해 악센트 문자를 기본문자로 정규화하는 주요 이유들은 다음과 같습니다:

1. 금칙어 우회 방지
- 사용자들이 악센트를 사용하여 금칙어 필터를 우회하는 것을 방지
- 예: "bád", "bàd", "bâd" 모두 "bad"로 인식되어야 함

2. 등록 편의성
- 관리자가 금칙어를 등록할 때 모든 악센트 변형을 일일이 등록할 필요가 없음
- 예: "café"를 금칙어로 등록하면 "cafe", "café", "cafè" 등 모든 변형을 자동으로 감지
3. 일관성 있는 검색
- 언어별로 다양한 악센트 표현이 있더라도 동일한 단어로 처리
- 예: 프랑스어의 "élève", "eleve", "ÉLÈVE" 모두 동일하게 처리
4. 성능 최적화
- 문자열 비교 시 기본 ASCII 문자로 정규화하면 더 빠른 비교 가능
- 메모리 사용량도 줄일 수 있음

이러한 이유로 Forword 에서는 악센트 정규화를 기본 기능으로 제공합니다.

### 지원 프로그래밍 언어
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

> 주의: 동일 금칙어에 공백이나 기호를 포함하여 새로운 금칙어로 등록하실 필요는 없습니다.

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

## 커스텀 무시 기호 적용 예제
이 섹션에서는 사용자 정의 무시 기호를 사용하여 금칙어 검색 동작을 커스터마이징 하는 방법을 보여줍니다.

### C#
```csharp
// 예제: 하이픈('-')과 공백(' ') 만 무시하는 경우
var customSymbols = new[] { '-', ' ' };
Forword forwordCustom = new Forword("forbidden_words.txt", customSymbols);

if (forwordCustom.Search("b-a-d-w-o-r-d"))
{
    Console.WriteLine("금칙어가 검출되었습니다.");
}
else
{
    Console.WriteLine("금칙어가 검출되지 않았습니다.");
}
```

### Python
```python
# 예제: 하이픈('-')과 공백만 무시하도록 설정한 경우
custom_symbols = {'-', ' '}
forword = Forword("forbidden_words.txt", ignored_symbols=custom_symbols)

if forword.search("b-a-d-w-o-r-d"):
    print("금칙어가 검출되었습니다.")
else:
    print("금칙어가 검출되지 않았습니다.")
```

### C++
```cpp
#include "forword.h"
#include <unordered_set>
#include <iostream>

int main(){
    // 예제: 하이픈('-')과 공백만 무시하는 경우
    std::unordered_set<char> customSymbols{'-', ' '};
    Forword forword("forbidden_words.txt", customSymbols);
    
    if(forword.search("b-a-d-w-o-r-d"))
        std::cout << "금칙어가 검출되었습니다." << std::endl;
    else
        std::cout << "금칙어가 검출되지 않았습니다." << std::endl;
        
    return 0;
}
```

## 주의할 점 
- 금칙어 텍스트 파일은 한 줄에 한 단어씩 있는 텍스트 파일이어야 합니다.
- 같은 금칙어에 공백 또는 기호를 추가하여 새로운 금칙어로 등록하실 필요는 
없습니다.
- 빈번하게 호출하는 경우 Forword 객체를 처음 한 번만 초기화하고, 재사용하는 
것을 권장합니다.

## Forword Developers

### Prerequisites
각 언어별 테스트를 실행하기 위해 필요한 도구:

#### Python
- Python 3.7 이상
- unittest (Python 기본 패키지)

#### C++
- Google Test
- A C++17 compatible compiler
- CMake 3.10 이상

#### C#
- .NET SDK
- MSTest

### 벤치마킹
각 언어별 구현의 성능을 측정하기 위한 벤치마크 도구가 제공됩니다.

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

벤치마크는 다음과 같은 조건에서 실행됩니다:
- 동일한 금칙어 파일 (bad, badword, 나쁜말, 욕설)
- 동일한 입력 텍스트 ("이것은 나쁜말 입니다. This is a bad word. 여기에 욕설이 있습니다.")
- 10,000회 반복 실행

벤치마크 결과는 다음 정보를 포함합니다:
- 총 실행 횟수
- 총 소요 시간 (초)
- 초당 처리량 (operations/sec)

### 테스트 실행

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
모든 언어 버전에 대해 동일한 테스트 케이스가 구현되어 있습니다:

1. **기본 검색 기능**
   - 일반적인 금칙어 검색
   - 금칙어가 없는 경우 처리

2. **공백과 기호 처리**
   - 금칙어 내 공백이 있는 경우
   - 기호가 포함된 경우
   - 불규칙한 공백 처리

3. **다국어 지원**
   - 다국어 금칙어 검색
   - UTF-8 인코딩 처리

4. **치환 기능**
   - 기본 치환 동작
   - 공백과 기호가 포함된 경우 치환
   - 다국어 텍스트 치환

5. **예외 처리**
   - 빈 입력 처리
   - 파일 로딩 실패 처리

### 프로젝트 구조
```
forword/
├── forword.h
├── forword.cs
├── forword.py
├── sample_data/
├── bench/
│   ├── bench_forword.cpp
│   ├── bench_forword.cs
│   └── bench_forword.py
└── tests/
    ├── test_forword.cpp
    ├── test_forword.cs
    └── test_forword.py
```
