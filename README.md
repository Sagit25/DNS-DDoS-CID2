# DDoS-DNS-Puzzle-CID2
Project for Creative Integrated Design 2 (SNU CSE Course)

# Bot 구현 사항
1) NetworkHook 모듈
    * linux(arm64) 기준으로 빌드되어있음
    * sudo insmod NetworkHook.ko 로 설치 가능
    * sudo rmmod NetworkHook.ko 로 제거 가능
    * 네트워크 통신이 들어오고 나올 때 접근해서 로그를 띄움
    * TCP_Option을 Parsing을 구현해두었음
  
2) Socket_Test
    * linux(arm64)기준으로 빌드되어 있음
    * 간단한 TCP/IP 서버와 클라이언트가 구현되어 있음
    * 실행 파일은 NetworkHook 디렉터리에도 들어있음
  
3) Rawsocket
    * linux(x64) 기준으로 빌드되어 있음
    * Sock_Raw를 이용하여 TCP/IP Handshaking을 진행하는 클라이언트
    * https://github.com/MaxXor/raw-sockets-example/ 에서 허락 없이 무단으로 가져와 수정하였음
    * Puzzle과 관련된 TCP_option을 추가하여 보냄
