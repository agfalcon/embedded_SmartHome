#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h> 
#include <softTone.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <wiringSerial.h>
#include <pthread.h>	
#include <time.h>
#include <string.h>
#include <wiringPiI2C.h>
#include <sys/ioctl.h>

#define SLAVE_ADDR_01 0x48
#define SWITCHGPIO 10 // SWITCH
#define LEDGPIO 14 //LED
#define BUZZERGPIO 16 //BUZZER
#define MOTORGPIO1 18 //MOTOR1
#define MOTORGPIO2 19 //MOTOR2
#define TOTAL_NOTES 32 //Buzzer Sound Size
#define BAUD_RATE 115200 //UART Baud Rate
#define BUFF_SIZE 100
#define SPEAKER_GPIO 12
#define SERVO_GPIO 13
#define SWITCH_GPIO 21

unsigned char input[BUFF_SIZE];
static const char* I2C_DEV = "/dev/i2c-1";
static const char* UART2_DEV = "/dev/ttyAMA1"; //UART2 연결을 위한 장치 파일

unsigned char serialRead(const int fd);
void serialWrite(const int fd, const unsigned char c);
void serialWriteBytes (const int fd, const char *s);

int UartControl();
void musicPlay();
void motorControl(int speed, int dir);
void ledControl(int Light);
void startGame();

time_t current_time;
struct tm tm;
struct tm alram_time;

pthread_mutex_t mutex;

int value;

int electricFee = 0;
int FeePlus = 1;

//Buzzer Notes
int notes[TOTAL_NOTES] = {
	330, 294, 262, 294, 330, 330, 330, 330,
	294, 294, 294, 294, 330, 330, 330, 330,
	330, 294, 262, 294, 330, 330, 330, 330,
	294, 294, 330, 294, 262, 262, 262, 262
};

//game notes
int game_notes[4] = {440 , 554 ,659 ,880};


void serialWriteBytes(const int fd, const char* s) {
	write(fd, s, strlen(s));
}


void serialReadGame(const int fd){

    read (fd, input, BUFF_SIZE);
}

unsigned char serialReadForStart(const int fd)
{
    unsigned char x;
    if(read (fd, &x, 1) != 1) //read 함수를 통해 1바이트 읽어옴
        return -1;
    return x; //읽어온 데이터 반환
}

void playCorrect(){
    softToneCreate (SPEAKER_GPIO);

    for(int i = 0 ; i < 4; i++){
        softToneWrite (SPEAKER_GPIO, game_notes[i]); //
        delay (250); //
    }
    softToneWrite (SPEAKER_GPIO, 0);
}

void playFalse(){
    softToneCreate (SPEAKER_GPIO);
    softToneWrite (SPEAKER_GPIO, 1100); 
    delay(300);
    softToneWrite(SPEAKER_GPIO,0);
}

void *updownGame()
{
	pthread_mutex_lock(&mutex);
	FeePlus = 3;
	pthread_mutex_unlock(&mutex);
	pinMode(SERVO_GPIO,PWM_OUTPUT); //
    int n=1;
    int fd_serial ; //UART2 파일 서술자
    unsigned char dat; //데이터 임시 저장 변수
    if ((fd_serial = serialOpen (UART2_DEV, BAUD_RATE)) < 0){ //UART2 포트 오픈
        printf ("Unable to open serial device.\n") ;
        return (void *)n;
    } 
    fflush(stdin);
    fflush(stdout);
    serialWriteBytes(fd_serial, "game start!\n\n1 ~ 50 중에 숫자를 하나 입력해주세요");

    //--------------------------기본 셋팅 시작 
    
    int led_gpio[3] = {23,24,25};
    //int rx,tx = 0,1;
    srand(time(NULL));
    int answer = random() % 50 +1 ;
    int divisor = 0;
    //디버그용 printf("answer : %d\n",answer);

    for(int i = 0;i<3;i++){
        pinMode(led_gpio[i], OUTPUT);
        digitalWrite(led_gpio[i], LOW);

    }
	
    pwmSetMode(PWM_MODE_MS); //모드 설정 
	pwmSetRange(3600); //범위 설정
	divisor = 19200000/ (50 * 3600); //나눗수 계산   
	pwmSetClock(divisor); //클락 설정

    pwmWrite(SERVO_GPIO,(423-63) * 90 / 180 + 63);
    //pwmWrite(servo_gpio,90);
    //-------------------------기본 셋팅 종료

    int index = 0;
    while(1){
        if(index == 3){
            serialWriteBytes(fd_serial, "you lose!\n");
            break;
        }   

        int int_input = 0;
        int flag = 1;
        while(flag){
            fflush(stdin); 
            if(serialDataAvail (fd_serial) ){ //읽을 데이터가 존재한다면,
                serialReadGame(fd_serial); //버퍼에서 1바이트 값을 읽음
                int_input = atoi(input);
                printf ("int_input : %d \n",int_input);
                //serialWrite(fd_serial, dat); //입력 받은 데이터를 다시 보냄 (Echo)
                flag = 0;
            }
            delay (10);
        }
        
       // printf("input : %c\n",input);
        printf("answer : %d\n",answer);
        
        if(answer == int_input){
            serialWriteBytes(fd_serial, "you win!\n");
            playCorrect();
            break;
        }
        else if(answer != int_input){
            digitalWrite(led_gpio[index],HIGH);

            index++;

            if(int_input>answer){
                pwmWrite(SERVO_GPIO,(423-63) * 0 / 180 + 63);
                delay(1000);
                pwmWrite(SERVO_GPIO,(423-63) * 90 / 180 + 63);
                serialWriteBytes(fd_serial, "down!\n");
            }
            else{
                //printf("up\n");
                pwmWrite(SERVO_GPIO,(423-63) * 180 / 180 + 63);
                delay(1000);
                pwmWrite(SERVO_GPIO,(423-63) * 90 / 180 + 63);
                serialWriteBytes(fd_serial, "up!\n");
            }
            playFalse();
        }
        
        
    }
    for(int i=0;i<3;i++) digitalWrite(led_gpio[i],HIGH);
    pthread_mutex_lock(&mutex);
	FeePlus = 1;
	pthread_mutex_unlock(&mutex);
    return (void*)n;
}

int Compare_time(){
	int act =0;

	if (tm.tm_mon == alram_time.tm_mon && tm.tm_mday == alram_time.tm_mday && tm.tm_hour == alram_time.tm_hour
		&& tm.tm_min == alram_time.tm_min) {
		act = 1;
	}
	
	return act;
}

void *Uart(void *arg) { //1번 스레드가 실행할 함수
	//printf("dd");
	UartControl();
	pthread_exit(NULL); //스레드 함수 종료
	
	// 전기세 
}

void *Time(void *arg){
	pinMode(SWITCHGPIO, INPUT);
	while (1){
	   current_time = time(NULL);
	   tm = *localtime(&current_time);
	   /*printf("now: %d-%d-%d %d:%d:%d\n",
       tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
       tm.tm_hour, tm.tm_min, tm.tm_sec);*/
       
       if(Compare_time()){
			if(digitalRead(SWITCHGPIO) == HIGH){
				alram_time.tm_hour = 0;
				alram_time.tm_min = 0;
			}
		   ledControl(1);
		   //motorControl(100, 1); //커튼을 연다.
		   musicPlay();
		   delay(1000);
		   electricFee += FeePlus;
		}	
		pthread_mutex_lock(&mutex);
       
       electricFee += FeePlus;
       pthread_mutex_unlock(&mutex);
       delay(1000);
      printf("%d\n", electricFee); 
	}
	
	pthread_exit(NULL); //스레드 함수 종료
}

int main()
{	
	//pthread_mutex_init(&mutex, NULL); 
	wiringPiSetupGpio();
	pinMode(MOTORGPIO1,PWM_OUTPUT);
	pinMode(MOTORGPIO2,PWM_OUTPUT);
	
	pthread_mutex_init(&mutex, NULL);
	pwmSetMode(PWM_MODE_MS);
	pwmSetRange(100);
	pwmSetClock(96);
	ledControl(0);
	motorControl(0,0);
	pthread_t tid1, tid2; //스레드 변수 선언
	
	value = 0; //공유 변수 초기화
	//1번 스레드 생성, Time
	if(pthread_create(&tid1, NULL, Time, NULL) != 0) { 
		fprintf(stderr, "pthread create error\n");
		exit(1);
	}
	
	//2번 스레드 생성, Uart
	if(pthread_create(&tid2, NULL, Uart, NULL) != 0) { 
		fprintf(stderr, "pthread create error\n");
		exit(1);
	}
	
	
	//1번 스레드 종료 후 리소스 회수
	if(pthread_join(tid1, NULL) != 0) { 
		fprintf(stderr, "pthread join error\n");
		exit(1);
	}
	
	//2번 스레드 종료 후 리소스 회수
	if(pthread_join(tid2, NULL) != 0) { 
		fprintf(stderr, "pthread join error\n");
		exit(1);
	}
	pthread_mutex_destroy(&mutex); //mutex 변수 해제
	
	exit(0);

	return 0;
}

void HomeConditionShow(int fd_serial) {
	int i2c_fd; //i2c
	int cnt = 0; //
	int preVal = 0; //
	int curVal = 0; //
	int threshold = 150; //
	int adcChannel1 = 0; //
	int adcChannel2 = 1;
	
	if ((i2c_fd = wiringPiI2CSetupInterface (I2C_DEV, SLAVE_ADDR_01)) < 0 ){
		printf("wiringPi2CSetup Failed: \n");
		return;
	}
	

	char buf[100];
	serialWriteBytes(fd_serial, "현재 집 상태\n");
	wiringPiI2CWrite(i2c_fd, 0x40 | adcChannel1);
	preVal= wiringPiI2CRead(i2c_fd); // (0~255)
	curVal = wiringPiI2CRead(i2c_fd); // ADC
	if(curVal < 140){
		serialWriteBytes(fd_serial, "현재 밝음\n");
	}
	else{
		serialWriteBytes(fd_serial, "현재 어두움\n");
	}

	wiringPiI2CWrite(i2c_fd, 0x40 | adcChannel2);
	preVal= wiringPiI2CRead(i2c_fd); // (0~255)
	curVal = wiringPiI2CRead(i2c_fd); // ADC
		
	sprintf(buf, "현재 온도 : %d도\n", preVal+12);
	serialWriteBytes(fd_serial, buf);
	

	delay(500);
	fflush(stdout);

}


//Uart Read
unsigned char serialRead(const int fd)
{
	unsigned char x;
	if(read (fd, &x, 1) != 1) //read 함수를 통해 1바이트 읽어옴
		return -1;
	return x; //읽어온 데이터 반환
}

//Uart Write
void serialWrite(const int fd, const unsigned char c)
{
	write (fd, &c, 1); //write 함수를 통해 1바이트 씀
}

void DateRead(int fd_serial){
	unsigned char dat ;
	serialWriteBytes(fd_serial, "!! 알람이 실행됩니다 !! \n");
	serialWriteBytes(fd_serial, "값을 입력한 후 F키 입력해주세요 \n");
	int count = 0;
	int date[4] = { 0, 0, 0, 0 };
	serialWriteBytes(fd_serial, "달을 입력해주세요 \n");
	fflush(stdout);
	dat = serialRead(fd_serial);
	while (1) {
		if (serialDataAvail(fd_serial)) { //버퍼에 읽을 데이터가 있을 때까지 반복
			dat = serialRead(fd_serial); //버퍼에서 1바이트 값을 읽음
			fflush(stdout);
		}
		else{
			continue;
		}
		if(dat<0)
			continue;
		if (dat == 'F') {
			break;
		}
		else {
			if (date[0] == 0) {
				if(dat == '1')
					date[0] = 1;
				if(dat == '2')
					date[0] = 2;
				if(dat == '3')
					date[0] = 3;
				if(dat == '4')
					date[0] = 4;
				if(dat == '5')
					date[0] = 5;
				if(dat == '6')
					date[0] = 6;
				if(dat == '7')
					date[0] = 7;
				if(dat == '8')
					date[0] = 8;
				if(dat == '9')
					date[0] = 9;

			}
			else {
				date[0] = date[0] * 10 + dat - 48;
				//printf("%d",date[0]);
				break;
			}
		}
		delay(2000);
	}
	printf("%d", date[0]);
	serialWriteBytes(fd_serial, "일을 입력해주세요 \n");
	fflush(stdout);
	dat = serialRead(fd_serial);
	while (1) {
		if (serialDataAvail(fd_serial)) { //버퍼에 읽을 데이터가 있을 때까지 반복
			dat = serialRead(fd_serial); //버퍼에서 1바이트 값을 읽음
			fflush(stdout);
		}
		if (dat == 'F') {
			break;
		}
		else {
			if (date[1] == 0) {
				if(dat == '1')
					date[1] = 1;
				if(dat == '2')
					date[1] = 2;
				if(dat == '3')
					date[1] = 3;
				if(dat == '4')
					date[1] = 4;
				if(dat == '5')
					date[1] = 5;
				if(dat == '6')
					date[1] = 6;
				if(dat == '7')
					date[1] = 7;
				if(dat == '8')
					date[1] = 8;
				if(dat == '9')
					date[1] = 9;
			}
			else {
				date[1] = date[1] * 10 + dat - 48;
				//printf("%d",date[0]);
				break;
			}
		}
		delay(2000);
	}
	printf("%d", date[1]);
	serialWriteBytes(fd_serial, "시을 입력해주세요 \n");
	fflush(stdout);
	dat = serialRead(fd_serial);
	while (1) {
		if (serialDataAvail(fd_serial)) { //버퍼에 읽을 데이터가 있을 때까지 반복
			dat = serialRead(fd_serial); //버퍼에서 1바이트 값을 읽음
			fflush(stdout);
		}
		if (dat == 'F') {
			break;
		}
		else {
			if (date[2] == 0) {
				if(dat == '1')
					date[2] = 1;
				if(dat == '2')
					date[2] = 2;
				if(dat == '3')
					date[2] = 3;
				if(dat == '4')
					date[2] = 4;
				if(dat == '5')
					date[2] = 5;
				if(dat == '6')
					date[2] = 6;
				if(dat == '7')
					date[2] = 7;
				if(dat == '8')
					date[2] = 8;
				if(dat == '9')
					date[2] = 9;
				
			}
			else {
				date[2] = date[2] * 10 + dat - 48;
				//printf("%d",date[0]);
				break;
			}
		}
		delay(2000);
	}
	printf("%d", date[2]);
	serialWriteBytes(fd_serial, "분을 입력해주세요 \n");
	fflush(stdout);
	dat = serialRead(fd_serial);
	while (1) {
		if (serialDataAvail(fd_serial)) { //버퍼에 읽을 데이터가 있을 때까지 반복
			dat = serialRead(fd_serial); //버퍼에서 1바이트 값을 읽음
			fflush(stdout);
		}
		if (dat == 'F') {
			break;
		}
		else {
			if (date[3] == 0) {
				if(dat == '1')
					date[3] = 1;
				if(dat =='2')
					date[3] = 2;
				if(dat == '3')
					date[3] = 3;
				if(dat == '4')
					date[3] = 4;
				if(dat == '5')
					date[3] = 5;
				if(dat == '6')
					date[3] = 6;
				if(dat == '7')
					date[3] = 7;
				if(dat == '8')
					date[3] = 8;
				if(dat == '9')
					date[3] = 9;
				
			}
			else {
				date[3] = date[3] * 10 + dat - 48;
				//printf("%d",date[0]);
				break;
			}
		}
		delay(2000);
	}
	printf("%d", date[3]);
	int i =0;
	for(i=0;i<4;i++){
		printf("ff %d\n" ,date[i]);
	}
	alram_time.tm_mon = date[0] - 1;
	alram_time.tm_mday = date[1];
	alram_time.tm_hour = date[2];
	alram_time.tm_min = date[3];
	serialWriteBytes(fd_serial, "알람설정 완료!! \n");
}

void OnOffPan(int fd_serial){

	int status = 0;

	serialWriteBytes(fd_serial,"1. 정지	2. 미풍    3. 약풍   4. 강풍\n");
	
	fflush(stdout);

	unsigned char dat;
	dat = serialRead(fd_serial);
	while (1) {
		if (serialDataAvail(fd_serial)) {
			dat = serialRead (fd_serial); 
			fflush(stdout);
			printf("%c", dat);
		}
		if(dat=='1')
		{
			motorControl(0,0);
			break;
		}
		else if(dat=='2')
		{
			motorControl(30,0);
			break;
		}
		else if(dat=='3')
		{
			motorControl(60,0);
			break;
		}
		else if(dat=='4')
		{
			motorControl(100,0);
			break;
		}
	}
	
	
}

void startGame(){
	pthread_t p_thread; //thread ID 저장 변수
    int r_val; //pthread 관련 함수 반환 값 저장
    int status; //thread 종료시 반환하는 값 저장 변수
    int value = 229; //쓰레드 호출 함수에 전달할 인자
    int fd_serial;
    int int_input = 0;
    int flag = 1;
    int switch_input = 0;
    unsigned char dat;
    
    if ((fd_serial = serialOpen (UART2_DEV, BAUD_RATE)) < 0){ //UART2 포트 오픈
        printf ("Unable to open serial device.\n") ;
        return ;
    } 
    
    pinMode(SWITCH_GPIO,INPUT); // switch pinmode 설정 
    
    
    while(flag){
        if(serialDataAvail(fd_serial) ){ //읽을 데이터가 존재한다면,
            serialReadGame(fd_serial);
        }
        switch_input = digitalRead(SWITCH_GPIO); 
        if(atoi(input) == 1 || switch_input == 1){ //game start Trigger
            r_val = pthread_create(&p_thread, NULL, updownGame, (void *)&value);
            if(r_val < 0){ 
                perror("pthread_create() error\n");
                exit(0);
            }
            
            r_val = pthread_join(p_thread, (void **)&status);
            if(r_val < 0){ //반환 값이 0보다 작으면 pthread_join 오류
                perror("pthread_join() error\n");
                exit(0);
            }
        
            flag = 0;
        }    
    }
}

//Uart Start
int UartControl()
{
	int i2c_fd;
	char buf[100];
	int select = 0;
	int fd_serial ; //UART2 파일 서술자
	if ((fd_serial = serialOpen (UART2_DEV, BAUD_RATE)) < 0){ //UART2 포트 오픈
		printf ("Unable to open serial device.\n") ;
		return 1 ;
	}

	//printf("dd");
	unsigned char dat;
	while (1) {
		if (serialDataAvail(fd_serial)) {
			dat = serialRead (fd_serial); 
			fflush(stdout);
			break;
		}
	}

	serialWriteBytes(fd_serial,"사용할 기능을 입력하세요 \n");
	serialWriteBytes(fd_serial,"1. 알람 2. 게임 3. 현재 집의상태 4. 선풍기 조절 5. 전기 요금 확인 6. 종료\n");
	
	fflush(stdout);
	while (1) {
		if (serialDataAvail(fd_serial)) { //읽을 데이터가 존재한다면,
			dat = serialRead(fd_serial); //버퍼에서 1바이트 값을 읽음
			fflush(stdout);
		}
		else{
			continue;
		}
		switch (dat)
			{
			case '1':
				DateRead(fd_serial);
				serialWriteBytes(fd_serial,"사용할 기능을 입력하세요 \n");
				serialWriteBytes(fd_serial,"1. 알람 2. 게임 3. 현재 집의상태 4. 선풍기 조절 5. 전기 요금 확인 6. 종료\n");
				break;
			case '2':
				startGame();
				serialWriteBytes(fd_serial,"사용할 기능을 입력하세요 \n");
				serialWriteBytes(fd_serial,"1. 알람 2. 게임 3. 현재 집의상태 4. 선풍기 조절 5. 전기 요금 확인 6. 종료\n");
				break;

			case '3':
				HomeConditionShow(fd_serial);
				serialWriteBytes(fd_serial,"사용할 기능을 입력하세요 \n");
				serialWriteBytes(fd_serial,"1. 알람 2. 게임 3. 현재 집의상태 4. 선풍기 조절 5. 전기 요금 확인 6. 종료\n");
				break;
			case '4':
				OnOffPan(fd_serial);
				serialWriteBytes(fd_serial,"사용할 기능을 입력하세요 \n");
				serialWriteBytes(fd_serial,"1. 알람 2. 게임 3. 현재 집의상태 4. 선풍기 조절 5. 전기 요금 확인 6. 종료\n");
				break;
			case '5':
				sprintf(buf, "전기 요금 : %d\n", electricFee);
				serialWriteBytes(fd_serial,buf);
				break;
			case '6':
				break;
			
			
			}
			

			if (dat == '6') {
				break;
			}
		delay(10);
	}
	fflush(stdout);
	dat = serialRead(fd_serial);
	serialWriteBytes(fd_serial,"프로그램 종료\n");
}


//Buzzer Play
void musicPlay()
{
	int i;
	softToneCreate (BUZZERGPIO);
 
	for(i = 0 ; i < TOTAL_NOTES; i++)
	{
		if(digitalRead(SWITCHGPIO) == HIGH){
			alram_time.tm_hour = 0;
			alram_time.tm_min = 0;
			break;
		}
		softToneWrite (BUZZERGPIO, notes[i]); 
		delay (250); 
	}
	softToneWrite (BUZZERGPIO, 0); // Buzzer Down
	motorControl(0, 1);
}

//DCMOTOR
//speed : 100, dir : 0(CW), 1(CCW)
void motorControl(int speed, int dir){
	pinMode(MOTORGPIO1,PWM_OUTPUT);
	pinMode(MOTORGPIO2,PWM_OUTPUT);
	if(dir == 0){ 	// CW
		pwmWrite(MOTORGPIO1, speed);
		pwmWrite(MOTORGPIO2, 0);
	}else if(dir ==1){ // CCW
		pwmWrite(MOTORGPIO1, 0);
		pwmWrite(MOTORGPIO2, speed);
	}
}

//LED
//Light : 1(On), 0(OFF)
void ledControl(int Light)
{
	pinMode(LEDGPIO, OUTPUT);
	//pwmSetMode(PWM_MODE_MS);
	//pwmSetRange(200);
	
	if(Light == 1)
	{
		digitalWrite(LEDGPIO, HIGH);
	}
	else if (Light == 0)
	{
		digitalWrite(LEDGPIO, LOW);
	}
}	
