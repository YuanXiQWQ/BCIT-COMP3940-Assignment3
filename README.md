# Assignment 3

---

## How to compile & run

Server (thread-pool, factory, AOP, custom exceptions)

1) `cd UploadServer`
2) `javac *.java`
3) `java UploadServer`
   -> Listening on 8082...

Client (console multipart/form-data POST)

1) `cd ConsoleApp`
2) `javac UploadClient.java`
3) `java UploadClient "<absolute-or-relative-path-to-file>" "<caption>" "<date>"`

## Examples

### Browser GET:

Open http://localhost:8082/

### Client POST:

```bash
java UploadClient ".\test.txt" "from-console" "2025-10-06"
```

### curl POST:

```bash
curl -v -F "caption=abc" -F "date=2025-10-06" -F "fileName=@C:\path\to\file.png" http://localhost:8082/
```