# Embedded_SmartHome
## 스마트 홈을 위한 라즈베리 파이 기반 프로그램
###  영상 
<img src = "https://user-images.githubusercontent.com/46450349/207894326-71848e94-fd92-4746-a4d3-b574310dad0a.mp4" width= "40%" height="40%">

----

## 1. 개발 목적
임베디드 시스템(라즈베리 파이)를 활용하여 스마트 홈을 제작하고 집의 기기들을 제어하는 것을 목표로 한다.   
추가로, 단순 기기들의 제어 기능뿐만이 아니라 여가시간 중 즐길 수 있는 게임도 탑재한다.

----


## 2. 전체 시스템 구조
<img src = "https://user-images.githubusercontent.com/46674066/207770377-415019a8-d35f-4e05-a736-fc1f1df2b1cd.png" width= "40%" height="40%">

+ 전체 시스템은 스마트폰-라즈베리파이-알람 시스템-게임 시스템-가전 시스템으로 구성되어있다.   

+ **스마트폰이 라즈베리 파이와의 UART(Bluetooth)통신을 통해 기기들을 제어하는 과정은 다음과 같다.**      
1. 라즈베리파이와 스마트폰을 UART 통신으로 연결한다.
2. 스마트폰에서 라즈베리파이에게 1을 입력하여 보낸다.
3. 라즈베리파이가 데이터를 받게되면 시스템이 가동되고, 스마트폰으로 사용 UI를 출력한다.
4. 스마트폰은 UI를 통해 사용할 기능의 번호를 보낸다. 
5. 스마트폰은 전달받은 번호를 통해 해당 기능을 실행한다.   



<img src = "https://user-images.githubusercontent.com/46674066/207801598-d7d11bbf-a575-4cd1-a72b-5f1b20f083df.png" width= "30%" height="30%">


----
## 3. 개별 시스템 구조 및 기능

### 3.1 알람
<img src = "https://user-images.githubusercontent.com/46674066/207796227-d4d261d3-f57a-419c-b4d0-62bfc2509693.png" width= "40%" height="40%">

+ 스마트폰과의 UART 통신을 통해 값을 입력받아 알람을 설정하는 기능이다.   
+ 알람이 설정된 시간에 모터를 이용하여 커튼을 걷게 하는 설정을 적용할 수 있다.   


+ **스마트폰으로 알람 설정을 하는 방법은 다음과 같다.**
1. 알람을 설정할 달(month)을 입력, F키를 입력하여 입력을 완료했다는 메세지를 라즈베리파이에게 전달한다.
2. 알람을 설정할 일(day)을 입력, F키를 입력하여 입력을 완료했다는 메세지를 라즈베리파이에게 전달한다.
3. 알람을 설정할 시(hour)를 입력, F키를 입력하여 입력을 완료했다는 메세지를 라즈베리파이에게 전달한다.
4. 알람을 설정할 분(minute)을 입력, F키를 입력하여 입력을 완료했다는 메세지를 라즈베리파이에게 전달한다.

+ 알림이 설정된 시각이 되면 Passive Buzzer를 이용해 소리로 해당 시간임을 알리며, 동시에 LED도 켜진다.    

+ 커튼걷기 설정을 했다면 모터가 동작하여 커튼을 걷는다. 
<img src = "https://user-images.githubusercontent.com/46674066/207802235-796ea552-3b00-4bfa-998d-77ce7aa6a287.png" width= "30%" height="30%">

---

### 3.2 UP & DOWN GAME
<img src = "https://user-images.githubusercontent.com/46674066/207797552-5c75bf33-55e4-4500-9848-bdf1ef7980d9.png" width= "40%" height="40%">

+ 스마트폰과의 UART 통신을 통해 값을 입력받아 UP & DOWN 게임을 실행하는 기능이다.<br>

+ **UP & DOWN 게임이란?**
> 1. 생각한 숫자를 하나 말한다.
> 2. 정답이 말한 숫자보다 높다면 UP / 낮으면 DOWN으로 알려준다.
> 3. 정해진 기회 안에 UP / DOWN을 이용하여 정답을 유추해야 한다.

+ **게임을 플레이하는 방법은 다음과 같다.**      
1. 플레이어는 1~50 사이 숫자를 입력한다.
2. 입력한 숫자와 비교하여 정답이 낮다면 DOWN, 높다면 UP을 출력한다.
3. 플레이어가 틀렸을 경우 LED 하나가 꺼지며 passive buzzer에서 오답알림음 재생 및 1~ 2가 반복된다. 
4. 플레이어가 3번안에 정답을 맞히지 못할 시, 게임이 종료된다.
5. 플레이어가 정답을 맞혔다면 passive buzzer에서 정답알림음 재생 및 게임이 종료된다.
<img src = "https://user-images.githubusercontent.com/46674066/207804464-864af04f-584a-4491-8913-0607161c8fb7.png" width= "30%" height="30%">    

---

### 3.3 현재 집 상태
<img src = "https://user-images.githubusercontent.com/46450349/207888669-b89dd2d5-95f0-430b-be82-f0f14195f2c4.PNG" width= "30%" height="30%">    

+ 현재 집 상태를 확인할 수 있는 기능 현재는 집안의 온도를 확인 할 수 있다.
+ 메인 메뉴에서 3을 입력하는 것으로 확인 할 수 있다.

---

### 3.4 선풍기 조절
+ UART통신으로 블루투스 선풍기를 조작한다.

<img src = "https://user-images.githubusercontent.com/46450349/207887990-ef1af77f-1a77-44be-85fb-89e2d90d18a1.PNG" width= "30%" height="30%">    
<img src = "https://user-images.githubusercontent.com/46450349/207888134-4d0d8202-f1fd-4c0e-bc61-68b9df8b588c.PNG" width= "30%" height="30%">   

+ 2장의 사진을 통해서 선풍기가 동작하는 것을 확인 할 수 있다.

1. 메인 메뉴에서 4를 입력하는 것으로 선풍기 조작 메뉴로 들어간다.
2. 1은 정지, 2는 미풍, 3은 중풍, 4는 강풍

---

### 3.5 전기요금 확인
<img src = "https://user-images.githubusercontent.com/46450349/207888669-b89dd2d5-95f0-430b-be82-f0f14195f2c4.PNG" width= "30%" height="30%">   

1. 메인 메뉴에서 5를 입력하는 것으로 전기 요금을 확인 할 수 있다.
2. 디폴트로 1초에 1씩 증가하게 설정되어 있다.
3. 게임(3.2)가 진행 중에는 1초에 3씩 증가하게 설정되어 있다.


----

## 4. 제한 조건 및 구현 내용

### 4.1 THREAD 사용

#### 시스템 기능 제어
+ 블루투스 연결을 통한 시스템 기능 제어
#### 알람
+ 실시간 시간 계산, 알람 설정 시간 비교, BUZZER / LED 동작
#### 게임
+ UP & DOWN 게임 실행, 스마트폰을 통한 정답 입력
      
### 4.2 MUTEX 사용

#### 전기요금 기능 사용
+ 기능별 사용시간에 비례하는 누적 전기요금기능 사용   
+ 기능별로 각각 다른 가중치를 적용해 전기요금 가산

----

## 5. 개발 시 문제점 및 해결방안


|문제점|문제점 설명|
|:---:|:---:|
|기능 추가|**일상생활**에 많이 사용할 수 있는 기능 추가 필요<br> 여러가지 기기들을 사용하므로 **전기요금**을 확인할 수 있는 기능을 추가해야함| 
|**해결법**|**해결법 설명**|
|전기요금 공유변수 사용|기능별 **가중치**를 적용하여 기능을 사용할 때 마다 요금이 가산되도록 함<br> 이때, 전기요금에 대한 공유변수를 사용하여 **쓰레드간에 값을 공유**할 수 있도록 함|
