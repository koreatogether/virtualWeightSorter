# Unity Test Framework with XML Output

## 소개

이 프로젝트는 Unity 테스트 프레임워크를 사용하여 DS18B20 센서 제어 시스템의 테스트를 구현하고, 테스트 결과를 텍스트 및 XML 형식으로 출력합니다.

## XML 출력 구현

테스트 결과를 JUnit 호환 XML 형식으로 출력하기 위해 다음과 같은 기능이 구현되었습니다:

1. `unity_xml_output.h`와 `unity_xml_output.cpp`: XML 출력을 위한 핵심 함수들을 제공합니다.
2. `unity_custom_output.h`와 `unity_custom_output.cpp`: Unity 테스트 출력을 가로채고 텍스트 및 XML 형식으로 출력합니다.
3. `unity_config.h`: Unity 프레임워크의 출력 핸들러를 사용자 정의 함수로 재정의합니다.

## 테스트 결과 파일

테스트 실행 시 다음과 같은 출력 파일이 생성됩니다:

- `test_result/unity_test_output.txt`: Unity 테스트 출력 텍스트 파일
- `test_result/unity_test_output.xml`: JUnit 호환 XML 형식의 테스트 결과 파일
- `test_result/direct_test_output.txt`: 직접 출력된 디버깅 메시지 파일
- `test_result/test_result.txt`: 표준 출력 리다이렉션 파일

## XML 형식

생성되는 XML 파일은 다음과 같은 형식을 따릅니다:

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<testsuites>
  <testsuite name="TestSuite">
    <testcase name="TestName" time="0.001">
      <!-- 성공한 테스트는 추가 태그가 없음 -->
    </testcase>
    <testcase name="FailingTest" time="0.002">
      <failure message="Test failed message">
        <![CDATA[
          File: filename.cpp
          Line: 123
        ]]>
      </failure>
    </testcase>
    <testcase name="IgnoredTest" time="0.000">
      <skipped message="Test ignored reason" />
    </testcase>
  </testsuite>
</testsuites>
```

## 사용 방법

테스트를 실행하려면 다음 명령을 사용하세요:

```
run_test.bat [all|app_menu|scenario1|utils]
```

- `all`: 모든 테스트 실행 (기본값)
- `app_menu`: App 메뉴 관련 테스트만 실행
- `scenario1`: 시나리오 1 관련 테스트만 실행
- `utils`: 유틸리티 함수 테스트만 실행

## XML 파일 활용

생성된 XML 파일은 다양한 CI/CD 시스템(Jenkins, GitLab CI, GitHub Actions 등)과 통합할 수 있으며, 테스트 결과를 시각화하는 도구에서도 활용할 수 있습니다.
