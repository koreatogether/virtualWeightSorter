#ifndef MOCK_COMMUNICATOR_H
#define MOCK_COMMUNICATOR_H

#include "../Communicator.h"
#include <vector>
#include <string>
#include <queue>

class MockCommunicator : public Communicator
{
public:
    MockCommunicator()
    {
        reset();
    }

    void reset()
    {
        _beginCalled = false;
        _printedMessages.clear();
        while (!_inputBuffer.empty())
        {
            _inputBuffer.pop();
        }
        _readStringUntilCalled = false;
        _readStringUntilResult = "";
        _readNonBlockingLineCalled = false;
        _readNonBlockingLineResult = "";
        _clearInputBufferCalled = false;
        _isSerialAvailableResult = false;
    }

    void begin() override
    {
        _beginCalled = true;
    }

    void print(const char *message) override
    {
        _printedMessages.push_back(message);
    }

    void println(const char *message) override
    {
        _printedMessages.push_back(std::string(message) + "\n");
    }

    void print(String message) override
    {
        _printedMessages.push_back(message.getString());
    }

    void println(String message) override
    {
        _printedMessages.push_back(message.getString() + "\n");
    }

    void println() override
    {
        _printedMessages.push_back("\n");
    }

    void print(int value) override
    {
        _printedMessages.push_back(std::to_string(value));
    }

    void println(int value) override
    {
        _printedMessages.push_back(std::to_string(value) + "\n");
    }

    void print(float value) override
    {
        _printedMessages.push_back(std::to_string(value));
    }

    void println(float value) override
    {
        _printedMessages.push_back(std::to_string(value) + "\n");
    }

    int readUserInput(unsigned long timeout) override
    {
        // Mocking for readUserInput if needed
        return -1;
    }

    String readStringInput(unsigned long timeout) override
    {
        // Mocking for readStringInput if needed
        return "";
    }

    bool isSerialAvailable() override
    {
        return _isSerialAvailableResult;
    }

    String readStringUntil(char terminator) override
    {
        _readStringUntilCalled = true;
        if (!_inputBuffer.empty())
        {
            String result = _inputBuffer.front();
            _inputBuffer.pop();
            // 버퍼가 비면 더 이상 입력이 없다고 설정
            if (_inputBuffer.empty())
            {
                _isSerialAvailableResult = false;
            }
            return result;
        }
        return _readStringUntilResult;
    }

    String readNonBlockingLine() override
    {
        _readNonBlockingLineCalled = true;
        if (!_inputBuffer.empty())
        {
            String result = _inputBuffer.front();
            _inputBuffer.pop();
            return result;
        }
        return "";
    }

    void clearInputBuffer() override
    {
        _clearInputBufferCalled = true;
        while (!_inputBuffer.empty())
        {
            _inputBuffer.pop();
        }
    }

    // Helper for tests
    const std::vector<std::string> &getPrintedMessages() const
    {
        return _printedMessages;
    }

    void addInput(const std::string &input)
    {
        _inputBuffer.push(input);
    }

    void setReadStringUntilResult(const String &result)
    {
        _readStringUntilResult = result;
    }

    void setReadNonBlockingLineResult(const String &result)
    {
        _readNonBlockingLineResult = result;
    }

    void setIsSerialAvailableResult(bool result)
    {
        _isSerialAvailableResult = result;
    }

    // 특정 문자열이 출력되었는지 확인하는 헬퍼 함수
    bool wasStringPrinted(const std::string &target) const
    {
        for (const auto &msg : _printedMessages)
        {
            if (msg.find(target) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    // 출력 기록을 지우는 헬퍼 함수
    void clearHistory()
    {
        _printedMessages.clear();
    }

    // 테스트용 추가 메소드들
    void setNextInput(const String &input)
    {
        _inputBuffer.push(input.c_str());
        _isSerialAvailableResult = true; // 입력이 있으면 Serial이 available하다고 설정
    }

    String getLastOutput() const
    {
        if (_printedMessages.empty())
        {
            return "";
        }
        return String(_printedMessages.back().c_str());
    }

    // Flags to check if methods were called
    bool _beginCalled;
    bool _readStringUntilCalled;
    bool _readNonBlockingLineCalled;
    bool _clearInputBufferCalled;

private:
    std::vector<std::string> _printedMessages;
    std::queue<std::string> _inputBuffer;
    String _readStringUntilResult;
    String _readNonBlockingLineResult;
    bool _isSerialAvailableResult;
};

#endif // MOCK_COMMUNICATOR_H