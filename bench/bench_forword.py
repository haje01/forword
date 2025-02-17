import time
import os
import sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from forword import Forword

def main():
    # Create forbidden words file
    forbidden_words_file = "forbidden_words.txt"
    with open(forbidden_words_file, "w", encoding="utf-8") as f:
        f.write("bad\nbadword\n나쁜말\n욕설")

    # Initialize Forword
    forword = Forword(forbidden_words_file)

    # Prepare test text
    text = "이것은 나쁜말 입니다. This is a bad word. 여기에 욕설이 있습니다."

    # Benchmark replace operation
    iterations = 10000
    start_time = time.time()
    
    for _ in range(iterations):
        forword.replace(text)
    
    elapsed = time.time() - start_time
    ops_per_sec = iterations / elapsed

    print(f"Python Forword Benchmark")
    print(f"------------------------")
    print(f"Operations: {iterations}")
    print(f"Total time: {elapsed:.2f} seconds")
    print(f"Ops/sec: {ops_per_sec:.2f}")

    # Cleanup
    os.remove(forbidden_words_file)

if __name__ == "__main__":
    main() 