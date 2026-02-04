아두이노 나노 + nextion display

나노보드 2개로 

한개는 bypass 모드로 usb to ttl 용도로 쓴다.

E:\project\20_Systems_Monitoring\boiler_4x_temp_monitoring\docs\nextionHMI\byPassConnect\bypassMode.md

파일에 설명되어 있다.


나머지 한개는 

컴퓨터 -> 아두이노 코드 업로드 용인데

하드웨어 시리얼로 nextion display랑 통신해야 하니 코드 업로드 할땐 하드웨어시리얼 0 , 1핀을 잠시 빼놔야 한다. 

중요 사항 : 동시에 bypass 모드 용 보드 의 hardware serial 0 , 1핀 과 코드 업로드용 보드의 hardware serial 0 , 1핀이 연결되어 있으면 tft 파일 업로드가 안되므로 
할때 분리시켜야 한다.
