#include <iostream>
#include <stdio.h> //sprintf 함수
#include <string.h>
#include <sys/types.h> //fork 함수
#include <sys/wait.h> // wait 함수
#include <unistd.h> //pipe 함수
#include <stdlib.h> // atoi 함수
#include <time.h> //time 함수
#include <fcntl.h> //open 함수

#define BUFF_SIZE 1024

using namespace std;

int add(int a, int b);
int sub(int a, int b);
int mul(int a, int b);
int divs(int a, int b);
void savetxt(char * allcal, int sum); //저장함수
int main() {
	int num_num;
	char *cal;
	char allcal[1000]; //전체수식
	char caltemp[100];
	int *num;
	int templen; //길이저장
	int status; // 상태저장 wait함수
	char tempnum[100]; // pipe에 저장용도
	char tempnum2[100];
	pid_t mainpid; // 메인 ps
	pid_t subpid;  // 서브 ps
	int result[2];
    int result2[2];	
	int lencount = 0;

	if (pipe(result) < 0) { 
		perror("pipe"); 
		exit(1); 
	}
	if (pipe(result2) <0) {
		perror("pipe");
		exit(1);
	}

	cout << "입력할 숫자를 입력해주세요." << endl;
	cin >> num_num;
			
	num = new int[num_num];
	cal = new char[num_num- 1];

	cout << "수식을 입력해주세요." << endl;
	for (int i = 0; i < num_num-1; i++) {		//수식 입력받는 부분
		cin >> num[i];
		sprintf(caltemp,"%d",num[i]);
		for(int len = strlen(caltemp), j = 0 ; j <len ; j++){
			allcal[lencount] = caltemp[j];
			lencount++;
		} 
		allcal[lencount] = ' ';
		lencount++;
		cin >> cal[i];
		allcal[lencount] = cal[i];
		lencount++;
		allcal[lencount] = ' ';
		lencount++;
	}
	cin >> num[num_num-1];
	sprintf(caltemp,"%d",num[num_num-1]);
	for(int len = strlen(caltemp), j = 0 ; j <len ; j++){
		allcal[lencount] = caltemp[j];
		lencount++;
	}
	allcal[lencount] = '\0';
	
	cout << allcal << endl;

	int count = 0;
	int temp = 0; // 임시 계산공간
	int tempfit = 0; // 이게 0이 아니면 곱셈 나눗셈을 한거임.
	int sum = num[count];

	while (1) {

	if (count == num_num-1)
	break;

	if (cal[count] == '*' || cal[count] == '/') {    //만약 * / 일 때
		if (cal[count] == '*') {
			cout << sum << "*" << num[count + 1] << endl;
			sum = mul(sum, num[count + 1]);
		}
		else if (cal[count] == '/') {
			cout << sum << "/" << num[count + 1] << endl;
			sum = divs(sum, num[count + 1]);
					}
			
		}
	else if (cal[count] == '+' || cal[count] == '-') { //만약 + -일 때
		if ((count == num_num - 2) || (cal[count + 1] == '+' || cal[count + 1] == '-')) {
		cout <<"\n----------------------중간계산--------------------------------"<< endl;
			if (cal[count] == '+') {
				cout << sum << "+" << num[count + 1] << endl;
				sum = add(sum, num[count + 1]);	
			}
			else if (cal[count] == '-') {
				cout << sum << "-" << num[count + 1] << endl;
				sum = sub(sum, num[count + 1]);
			}
		}
		else if (cal[count + 1] == '*' || cal[count + 1] == '/') {  //그 다음께 * / 이면
			mainpid = getpid(); //현재 ps받고
			subpid = fork(); //fork
			if (subpid == -1) {
				perror("Failed to fork");
				return 1;
			}	

			else if (subpid == 0) {
				cout << "--------서브 ps에서 곱셈부분 계산중----------" << endl;
				sleep(3);
				temp = num[count + 1];		
				for (int i = count + 1; i != num_num + 1; i++) {
					if (cal[i] == '*') {
						tempfit++;
						temp = mul(temp, num[i + 1]);
						}	
					else if (cal[i] == '/') {
						tempfit++;
				 		temp = divs(temp, num[i + 1]);
						}
					else
						break;
				}
				cout << "--------서브 ps에서 곱셈부분 계산 끝---------" << endl;
				
				sprintf(tempnum, "%d", temp);
				write(result[1], tempnum, strlen(tempnum)); //pipe에 저장
				cout << "pipe에 temp 저장성공!" << endl;

				sprintf(tempnum2, "%d", tempfit);
				write(result2[1], tempnum2, strlen(tempnum2)); //pipe에 저장
				cout << "pipe에 tempfit 저장성공!" << endl;
				
				cout <<"-------------서브 ps종료합니다---------------" <<endl;
				exit(0); //서브 ps 종료
			}
			else { // 메인 ps
				wait(&status);
				cout <<endl;
				cout << "------------메인 ps작동 시작-----------------" << endl;
				templen = read(result[0], tempnum , 100);
				tempnum[templen] = '\0'; //문자열로만들고
				temp = atoi(tempnum); //int로 변환
				cout << " pipe에서 temp로 int형 변환 완료!" << endl;

				templen = read(result2[0], tempnum, 100);
				tempnum[templen] = '\0'; //문자열로만들고
				tempfit = atoi(tempnum); //int로 변환
				cout << " pipe에서 tempfit으로 int형 변환 완료!" << endl;

			}

		if (tempfit != 0) {//뒤에 곱셈이어지는 부분까지 다 더해줌
		cout <<"\n----------------------중간계산--------------------------------"<< endl;
			if (cal[count] == '+') {
				cout << sum << " + " << temp << endl;
				sum += temp;
			}
			else if (cal[count] == '-') {
				cout << sum << " - " << temp << endl;
				sum -= temp;
			}
			count += tempfit;
			}
		}
		tempfit = 0;
	}
	count++;
	}
cout << "\n--------------------------결과---------------------------"<< endl;
cout << "sum = " << sum << endl;

cout << "\n-------------결과를 result.txt에 저장합니다---------------"<< endl;
savetxt(allcal,sum);
cout << "\n---------------------result.txt출력----------------------"<< endl;
int fd;
char buff[BUFF_SIZE];
fd = open("./result.txt", O_RDONLY, 0644);
read(fd,buff,BUFF_SIZE);
puts(buff);
close(fd);
}


int add(int a, int b)
{
		return a + b;
}
int sub(int a, int b)
{
		return a - b;
}
int mul(int a, int b)
{
		return a * b;
}
int divs(int a, int b)
{
		return a / b;
}
void savetxt(char * allcal, int sum) {
	int fd;
	char pill[20] = "\n입력수식 : ";
	char pill2[20] = "\t결과 : "; 
	char sumpill[100];
	sprintf(sumpill,"%d",sum);

	fd = open("./result.txt", O_CREAT|O_RDWR|O_APPEND, 0644);
	write(fd,pill,strlen(pill));
	if(write(fd,allcal,strlen(allcal)))
		perror("writeerror");
	write(fd,pill2,strlen(pill2));
	write(fd,sumpill,strlen(sumpill));
	close(fd);
}
