/*
SdFat library example sketch for SdFat library.
*/

#include <SPI.h>
#include <SdFat.h>

// CS 핀을 정의합니다. 사용하는 핀에 따라 변경해야 합니다.
const int chipSelect = 10;

// SdFat 객체를 생성합니다.
SdFat SD;

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        ; // 시리얼 모니터가 연결될 때까지 기다립니다.

    Serial.print("Initializing SD card...");

    // SD 카드 초기화를 시도합니다.
    if (!SD.begin(chipSelect))
    {
        Serial.println("initialization failed!");
        while (1)
            ; // 초기화 실패 시 무한 루프에 빠집니다.
    }
    Serial.println("initialization done.");

    // 파일 객체를 생성합니다.
    SdFile myFile;

    // 파일을 생성하고 엽니다. 파일이 이미 존재하면 덮어씁니다.
    if (!myFile.open("test.txt", O_RDWR | O_CREAT | O_TRUNC)) // 파일을 읽기/쓰기 모드로 열고, 파일이 없으면 생성하며, 기존 파일이 있으면 내용을 지움
    {
        Serial.println("opening test.txt failed");
        return;
    }

    // 파일에 데이터를 씁니다.
    myFile.println("Hello from SdFat!");
    myFile.print("This is a test file.");
    myFile.println(" And another line.");

    // 파일을 닫습니다.
    myFile.close();
    Serial.println("Wrote to file");

    // 파일을 다시 엽니다 (읽기 모드).
    if (!myFile.open("test.txt", O_READ))
    {
        Serial.println("reopening test.txt failed");
        return;
    }

    Serial.println("Reading from file:");

    // 파일의 내용을 읽어 시리얼 모니터에 출력합니다.
    while (myFile.available())
    {
        Serial.write(myFile.read());
    }

    // 파일을 닫습니다.
    myFile.close();
}

void loop()
{
    // 여기에는 추가적인 코드를 작성하지 않습니다.
}