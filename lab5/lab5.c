#include <stdio.h>
#include <stdlib.h>

#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"

#include <pthread.h>


#define Queue_length 4
#define DTAT_FILE "data.txt"

struct node
{
	int priority;
	int info;
	struct node *link;
}*HeadNode=NULL;


void insert(int item, int item_priority);
void print();
int ChechEmpty();
void search(int);
int del();
int Thread1CurrentProcessNumber;
int Thread1CurrentProcessIndex;
int Thread2HelpingFlag = 0;
void LED_Blink(int);
//void SevenSeg();
//void KeyboardTouch();
//void LCD_Display(int);

void *CheckForMatch(void* rank);


int thread_count;
int dequeue[10000];
pthread_mutex_t access_dequeue;
pthread_mutex_t access_currentnumber;
int rulenumber[2];
int rulepriority[2];
int elementNumber = 0;


int main(int argc, char* const argv[])
{
	int i;
	int n = 0;
	int element = 0;
	int search_element = 0;
	int priority = 0;
	FILE *fp1;
	FILE *fp2;
	FILE * rule;
	FILE *countfp;
	int num = 0;
	struct node *ptr;
	int cnt;
	char string[3];
	int indexx = 0;

	long thread;
	pthread_t* thread_handles;
	pthread_mutex_init(&access_dequeue, NULL);
	pthread_mutex_init(&access_currentnumber, NULL);

	printf("please enter the key\n");
	//KeyboardTouch();
	printf("============== good Keyboard touch!! =====================\n");

	rule = fopen("rule.txt","r");
	while(fscanf(rule,"%s %d %s %s %s %d",&string[0],&rulepriority[indexx],&string[1],&string[2],&string[3],&rulenumber[indexx]) != EOF)
	{
		printf("number %d is priority %d\n",rulenumber[indexx], rulepriority[indexx]);
		indexx++;
	}


	thread_count = indexx;
	thread_handles = (pthread_t*) malloc (thread_count*sizeof(pthread_t)); 


	countfp = fopen(DTAT_FILE,"r");
	while(fscanf(countfp,"%d",&cnt) != EOF)
	{
		//printf("%d ", cnt);
		elementNumber++;
	}

	printf("number of data = %d\n",elementNumber );

	//printf("\n");


	fp1 = fopen(DTAT_FILE,"r");	
	for(i = 0; i < Queue_length; i++)
	{
		fscanf(fp1,"%d",&num);
		element = num;
		//printf("%d " ,element);
		if((element % rulenumber[0]) == 0)
        {
        	priority = rulepriority[0];
        }
        else if( (element % rulenumber[1]) == 0 )
			priority = rulepriority[1];
        else
        	if(i < 3)
				priority = 3;
			else
				priority = i;
        insert(element,priority);
	}


	ptr = HeadNode;
	i = 0;
	int priority_shift = elementNumber;
	int flag = 0;

	fp2 = fopen(DTAT_FILE,"r");
	int start = 0;

	while( i < elementNumber )
	{
		flag = fscanf(fp2,"%d",&num);		
		if(start >= Queue_length)
		{
			dequeue[i] = del();//ptr -> info;
			printf("dequeue[%d] = %d  ",i,dequeue[i] );
			
			/*LCD_Display(dequeue[i]);
			sleep(1);

			if(dequeue[i]%3 == 0)
			{
				SevenSeg();
				LED_Blink(2);
				sleep(1);
			}*/

			i++;
			priority_shift++;
			if(flag != -1)
			{
				element = num;
				printf("read : %d ",element );
				if((element % rulenumber[0] ) == 0)
				{
					priority = rulepriority[0];
				}
				else if((element % rulenumber[1]) == 0)
					priority = rulepriority[1];
				else 
					priority = priority_shift+1;
				insert(element,priority);
			}
			print();
		}
		else
		{
			start++;
		}
	}

	/*int j;
	printf("dequeue :  ");
	for(j = 0; j < elementNumber; j++)
	{
		printf("%d ",dequeue[j]);
		if( (dequeue[j]%rulenumber[0]) == 0)
		{
			LED_Blink(2);
			SevenSeg();
			LCD_Display(dequeue[j]);
			sleep(5);
		}
	}*/
	
	for (thread = 0; thread < thread_count; thread++) 
	{ 
        pthread_create(&thread_handles[thread], NULL,CheckForMatch, (void*) thread);  
    }
    
    for (thread = 0; thread < thread_count; thread++) 
	{
		pthread_join(thread_handles[thread], NULL); 
	}

	LED_Blink(2);
	free(thread_handles);

	return 0;
}


void *CheckForMatch(void *rank)
{
	int my_rank = (int)rank + 1;
	int my_priority;
	int My_MatchNumber;
	int i;
	FILE *fp_thread;
	char buffer[100];
	int OutputFileNumber[elementNumber];
	int outputindex = 0;
	int StartNumber = 0;
	int StartIndex = 0;

	sprintf(buffer,"thread%d.txt",my_rank);
	fp_thread = fopen(buffer,"w");

	//printf("thread %d is worrking ... \n",my_rank );
	fprintf(fp_thread, "threadID : %d PID : %d \n",my_rank-1,getpid());


	for (i = 0; i < thread_count; i++)		// get their own number for finding in dequeue[]
	{
		if(my_rank == rulepriority[i])
		{
			My_MatchNumber = rulenumber[i];
		}
	}

	if(my_rank == 2)
	{
		sleep(1);
	}

	printf("thread %d is worrking ... ",my_rank );


	/********************* working section **********************/
	for (i = 0; i < elementNumber; i++)
	{
		if ( (dequeue[i] % My_MatchNumber) == 0)
		{
			OutputFileNumber[outputindex] = dequeue[i];
			outputindex++;
			if (my_rank == 1 && outputindex > 5)
			{
				printf("thread1 : %d \n",dequeue[i] );
				pthread_mutex_lock(&access_currentnumber);
				Thread1CurrentProcessNumber = dequeue[i];
				Thread1CurrentProcessIndex = i;
				pthread_mutex_unlock(&access_currentnumber);
				dequeue[i] = My_MatchNumber + 1;
				usleep(500000);
			}
			else if(my_rank == 2)
			{
				printf("thread2 : %d \n",dequeue[i] );
				usleep(10000);
			}
			else
				printf("%d ",dequeue[i] );
		}
	}	
	/************************************************************/


	for (i = 0; i < outputindex; i++)
	{
		fprintf(fp_thread, "%d ",OutputFileNumber[i] );

	}
	printf("\n");


	/*********************** for thread2 only helping mode *********************************/
	/**************************************************************************/


	if( my_rank == 2)
	{
		StartNumber = Thread1CurrentProcessNumber;
		StartIndex = Thread1CurrentProcessIndex + 1;
		outputindex = 0;
		My_MatchNumber = rulenumber[0];

		printf("================ thread1 is working untill %d, index = %d ===================\n",StartNumber, StartIndex-1 );
		printf("helping thread 1 : \n");

		for (i = StartIndex; i < elementNumber; i++)
		{
			if ( (dequeue[i] % My_MatchNumber) == 0)
			{
				OutputFileNumber[outputindex] = dequeue[i];
				outputindex++;
				printf("thread 2 : %d \n",dequeue[i] );
				//fprintf(fp_thread, "%d ",dequeue[i] );
				dequeue[i] = My_MatchNumber+1;
					
				pthread_mutex_lock(&access_currentnumber);
				Thread1CurrentProcessNumber = dequeue[i];
				Thread1CurrentProcessIndex = i;
				pthread_mutex_unlock(&access_currentnumber);
				usleep(300000);
			}
		}

		fprintf(fp_thread, " the following belongs to thread1 : \n");	

		for (i = 0; i < outputindex; i++)
		{
			fprintf(fp_thread, "%d ",OutputFileNumber[i] );

		}
	}

	/*********************** for thread2 only *********************************/
	/**************************************************************************/

	return NULL;
}

void insert(int item,int item_priority)
{
	struct node *tmp;
	struct node *p;

	tmp = (struct node*)malloc(sizeof(struct node));

	if(tmp == NULL)
	{
		printf("Memory not available\n");
		return;
	}
	
	tmp->info = item;
	tmp->priority = item_priority;

	if( ChechEmpty() || item_priority < HeadNode->priority )
	{
		tmp->link = HeadNode;
		HeadNode = tmp;
	}
	else
	{
		p = HeadNode;
		while( p->link != NULL && p->link->priority <= item_priority )
		p = p->link;
		tmp->link = p->link;
		p->link = tmp;
	}
}


int ChechEmpty()
{
	if( HeadNode == NULL )
		return 1;
	else
		return 0;
}

void print()
{
	struct node *ptr;
	ptr = HeadNode;
	if( ChechEmpty() )
		printf("Queue is empty");
	else
	{    
		printf("queue :  ");
		//printf("Priority     element\n");
	
		while(ptr != NULL)
		{
			printf("%d ",ptr->info);
			//printf("%5d        %5d\n",ptr->priority, ptr->info);
			ptr = ptr->link;
		}
	}
	printf("\n");
}

void search(int element)
{
	struct node *ptr;
	ptr = HeadNode;
	int flag = 0;

	while(flag == 0 && ptr != NULL)
	{
		if(element == ptr->info)
		{
			flag = 1;
		}
		else
		{
			ptr = ptr->link;
		}
	}

	if(flag == 0)
		printf("not found\n");
	else
		printf("found %d and priority is %d\n",ptr->info,ptr->priority );
}
int del()
{
	struct node *tmp;
	int item;
	if( ChechEmpty() )
	{
		printf("Queue Underflow\n");
		exit(1);
	}
	else
	{
		tmp = HeadNode;
		item = tmp -> info;
		HeadNode = HeadNode -> link;
		free(tmp);
	}
	return item;
}


void LED_Blink(int times)
{
	int fd;
	int retval;
	int i;
 	unsigned short data;

	if( (fd = open("/dev/lcd", O_RDWR)) < 0 )
	{
		printf("Open_/dev/lcd_faild.LED \n");
		exit(-1);
	}

	for (i = 0; i < times; i++)
	{
		data = LED_ALL_ON;
		ioctl(fd, LED_IOCTL_SET	, &data);
		//printf("turn on all led lamps\n");
		sleep(1);

		data = LED_ALL_OFF;
		ioctl(fd,LED_IOCTL_SET, &data);
		//printf("turn off all led\n");
		sleep(1);
	}

	//close(fd);
}
/*
void SevenSeg()
{
	_7seg_info_t data;
	int fd, ret, i;

	if( (fd = open("/dev/lcd", O_RDWR)) < 0 )
	{
		printf("Open_/dev/lcd_faild. 7SEG\n");
		exit(-1);
	}

	ioctl(fd, _7SEG_IOCTL_ON,NULL);
	data.Mode = _7SEG_MODE_PATTERN;
	data.Which = _7SEG_ALL;
	data.Value = 0x08770806;
	ioctl(fd,_7SEG_IOCTL_SET,&data);
	//ioctl(fd,_7SEG_IOCTL_OFF);
	//close(fd);
}

void LCD_Display(int number)
{
	int fd;
	lcd_write_info_t display;

	if( (fd = open("/dev/lcd", O_RDWR)) < 0 )
	{
		printf("Open_/dev/lcd_faild. LCD \n");
		exit(-1);
	}

	ioctl(fd,LCD_IOCTL_CLEAR,NULL);
	display.Count = sprintf((char*)display.Msg,"%d",number);
	ioctl(fd,LCD_IOCTL_WRITE, &display);

	ioctl(fd,LCD_IOCTL_CUR_GET,&display);
	//printf("the posistion of cursor is at (%d,%d)\n",display.CursorX, display.CursorY );

	//close(fd);

}

void KeyboardTouch()
{
	unsigned short key;
	int fd,ret;

	if( (fd = open("/dev/lcd", O_RDWR)) < 0 )
	{
		printf("Open_/dev/lcd_faild. KeyboardTouch\n");
		exit(-1);
	}

	ioctl(fd,KEY_IOCTL_CLEAR, key);
	ioctl(fd,KEY_IOCTL_WAIT_CHAR,key);

	//close(fd);
}

*/