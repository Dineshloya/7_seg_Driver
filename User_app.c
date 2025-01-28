#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#define PINS 12

int fd,i;
char *seg_lut1[10]={"11111100","01100000","11011010","11110010","01100110","10110110","10111110","11100000","11111110","11110110"};
char *seg_lut2[4]={"0111","1011","1101","1110"};
char clear[12]="111111111111";
char write_buf[13];

void four_digit_display(int num)
{
	int dig[4];
	dig[0]=num/1000;
	dig[1]=(num/100)%10;
	dig[2]=(num/10)%10;
	dig[3]=num%10;
	for(i=0;i<50;i++)
	{
	for(int j=0;j<4;j++)
	{
		write(fd,clear,strlen(clear));
		snprintf(write_buf,13,"%s%s",seg_lut1[dig[j]],seg_lut2[j]);
		write(fd,write_buf,strlen(write_buf));
	//	printf("%s\n",write_buf);
		usleep(5*1000);
	}
	}
	write(fd,clear,strlen(clear));
	usleep(1*1000);
}

int main()
{
	int  op,n=0;
	if((fd=open("/dev/multi_gpio",O_WRONLY))<0)
	{
		printf("Cannot open device file...\n");
		return 0;
	}
	while(1)
	{
	label:  system("clear");
	        printf("Please Enter the number you want to print(0-9999) or -1 to print sequence\n");
		scanf(" %d",&op);
		if(op==-1)
		{
			while((n++)<10000)
			{
				four_digit_display(n);;
			}
		}
		else if((op<0)||(op>9999))
		{
			printf("Invalid Number entered\n");
			goto label;
		}
		else
		{
			four_digit_display(op);
		}
	}
	
	close(fd);
}
