/*
 * uart.c
 *
 *  Created on: Oct 31, 2023
 *      Author: 158180
 */
//#include "main.h"
#include "stm32l5xx_hal.h"
#include "stm32l5xx_hal_uart.h"
#include "stm32l5xx_hal_uart_ex.h"
#include "uart1.h"
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "queue.h"


#define FLASH_DOUBLEWORD_TYPEPROGRAM
#define FLASH_USER_START_ADDR   ADDR_FLASH_PAGE_125   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     (ADDR_FLASH_PAGE_125 + FLASH_PAGE_SIZE - 1)

//uint8_t i;
uint64_t rdata;
static uint64_t no_node=0;
static uint32_t FirstPage = 0, NbOfPages = 0, BankNumber = 0;
uint32_t Address = ADDR_FLASH_PAGE_125;
uint32_t PageError = 0;

__IO uint32_t MemoryProgramStatus = 0;
uint64_t datafromflash[200];
uint8_t buffer[100];

FLASH_EraseInitTypeDef EraseInitStruct;
static uint32_t GetPage(uint32_t );
static uint32_t GetBank(uint32_t );

uint8_t name[]="\nEnter name >>>  ";
uint8_t roll[]="\nEnter roll number >>>  ";
QueueHandle_t xPointerQueue;
char Rollnodata[3];
static uint8_t data = 0;


void uart_transmit(uint8_t *data,int length)
{
	int i=0;
	while (i<length)
	{
	while (!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = data[i++];
	}
}

uint8_t uart_receive()
{
	while(1)
	{
	if(xQueueReceive(xPointerQueue, &data, 0) == pdTRUE)
	return data;
	}
}

void create_node(struct node **head)
{
	uint8_t data[20];
	int i=0;
	no_node++;
	struct node *ptr= *head;
	struct node *new=calloc(sizeof(struct node),1);
	if(*head==NULL)
	{
		uart_transmit(roll, sizeof(roll));
		memset(data,'0',sizeof(data));
		do
		{
		data[i++]=uart_receive();
		}while (!(strstr((char *)data, UART_CLI_STR_TERM_CR_LF)));
		new->roll_no=atoi(data);

		uart_transmit(name, sizeof(name));
		memset(data,'0',sizeof(data));
		i=0;
		do{
		data[i]=uart_receive();
		new->name[i]=data[i];
		i++;
		}while (!(strstr((char *)data, UART_CLI_STR_TERM_CR_LF)));

		if(strlen(new->name)>14)
		{
		uart_transmit("ERR: Name length size exceeded\n", 32);
		no_node--;
		memset(new->name,0,strlen(new->name));
		return;
		}
		else{
		*head=new;
		new->next=NULL;
		}
	}
	else
	{
		uart_transmit(roll, sizeof(roll));
		i=0;
		memset(data,'0',sizeof(data));
		do{
		data[i++]=uart_receive();
		}while (!(strstr((char *)data, UART_CLI_STR_TERM_CR_LF)));
		int num=atoi(data);

		while(ptr!= NULL){
			if(ptr->roll_no==num)
			{
			uart_transmit("ERR: Roll no. already exists\n", 30);
			no_node--;
			return;
			}
			ptr=ptr->next;
		}
		new->roll_no=atoi(data);
		uart_transmit(name, sizeof(name));
		i=0;
		memset(data,'0',sizeof(data));
		do{
		data[i]=uart_receive();
		new->name[i]=data[i];
		i++;
		}while (!(strstr((char *)data, UART_CLI_STR_TERM_CR_LF)));

		if(strlen(new->name)>14)
		{
		uart_transmit("ERR: Name length size exceeded\n", 32);
		no_node--;
		memset(new->name,0,strlen(new->name));
		return;
		}
		else{
		ptr= *head;
		new->next=NULL;
		while(ptr->next != NULL)
			ptr=ptr->next;
		ptr->next=new;
		}
	}

}

struct node* swap(struct node* current_node, struct node* next_node)
{
	struct node* temp = next_node->next;
	next_node->next = current_node;
	current_node->next = temp;
	return next_node;
}

void inorder(struct node** head, uint8_t count)
{
	struct node** head_copy;
	uint8_t i, j;
	for (i = 0; i <= count ; i++) {
		head_copy = head;
		for (j = 0; j <  count- i - 1; j++) {
			struct node* current_node = *head_copy;
			struct node* next_node = current_node->next;
			if (current_node->roll_no > next_node->roll_no) {
				*head_copy = swap(current_node, next_node);
			}
			head_copy = &(*head_copy)->next;
		}
	}
}


static uint32_t GetPage(uint32_t Addr)
{
	uint32_t page = 0;
	if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
	{
	/* Bank 1 */
	page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
	}
	else
	{
	/* Bank 2 */
	page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
	}
	return page;
}


static uint32_t GetBank(uint32_t Addr)
{
	return FLASH_BANK_1;
}

void copy_data(struct node **head)
{
	struct node *ptr= *head;
	HAL_FLASH_Unlock();
	if(HAL_FLASHEx_Erase(&EraseInitStruct, &PageError)!=HAL_OK)
	{
		uart_transmit("Erasing Error\n", 15);
	}

	if(ptr==NULL)
	{
		uart_transmit("**Empty**\n", 11);
		return;
	}
	Address=FLASH_USER_START_ADDR;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD ,Address,no_node);     //FLASH_TYPEPROGRAM_DOUBLEWORD
	Address=Address+8;
	int i=0;
	while(ptr!=NULL)
	 {
		uint8_t store_data[16]="";
		memset(store_data,0,sizeof(store_data));
		uint8_t refference[8]="";
		memcpy(&store_data[0],&(ptr->roll_no),2);
		memcpy(&store_data[2],ptr->name,14);
		memcpy(refference,&store_data[8],8);
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD ,Address, *(uint64_t * )store_data);
		Address=Address+8;
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD ,Address, *(uint64_t * )refference);
		Address=Address+8;
		ptr=ptr->next;
	 }
}


void flash_intialization()
{
	HAL_FLASH_Unlock();
	FirstPage = GetPage(FLASH_USER_START_ADDR);
	NbOfPages = GetPage(FLASH_USER_END_ADDR) - FirstPage + 1;
	BankNumber = GetBank(FLASH_USER_START_ADDR);

	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks       = BankNumber;
	EraseInitStruct.Page        = FirstPage;
	EraseInitStruct.NbPages     = NbOfPages;
//	      if(HAL_FLASHEx_Erase(&EraseInitStruct, &PageError)!=HAL_OK)
//	      	     {
//	      	    	 uart_transmit("Error\n", 6);
//	      	     }
//	      HAL_FLASH_Lock();
}

void add_data(struct node **head)
{
	create_node(head);
	inorder(head,(uint8_t)no_node);
}


void delete_data(struct node **head)
{
	struct node *prev;
	struct node *ptr=*head;

	uart_transmit(roll, sizeof(roll));
	int i=0;
	uint8_t data[20];
	memset(data,0,sizeof(data));
	do
	{
		data[i++]=uart_receive();
	}while (!(strstr((char *)data, UART_CLI_STR_TERM_CR_LF)));
	int ROLL_NO=atoi(data);
	if( ptr==NULL)
	  {
		uart_transmit("**Empty**\n", 11);
	  }
	else
	  {
		//single element in link list
		if(ptr!=NULL && ptr->roll_no==ROLL_NO)
		 {
		  *head=ptr->next;
		  free(ptr);
		  no_node--;
		 }
		else
		 {
			while(ptr !=NULL && ptr->roll_no!=ROLL_NO)
			{
			prev=ptr;
			ptr=ptr->next;
			}
			if(ptr==NULL && prev->roll_no!=ROLL_NO){
				uart_transmit("ERR: Invalid roll number\n", 26);
				return;
			}
			else{
			prev->next=ptr->next;
			free(ptr);
			no_node--;
			}
		 }
	  }
}


void search_data(struct node **head)
{
	struct node *ptr=*head;
	uart_transmit(roll, sizeof(roll));
	int i=0;
	uint8_t data[20];
	memset(data,0,sizeof(data));
	do
	{
	data[i++]=uart_receive();
	}while (!(strstr((char *)data, UART_CLI_STR_TERM_CR_LF)));
	int ROLLno=atoi(data);

	if(ptr==NULL){
	uart_transmit("**Empty**\n",11);
	return;
	}

	while(ptr->next!=NULL && ptr->roll_no!= ROLLno)
	{
	ptr=ptr->next;
	}

	if(ptr->next==NULL && ptr->roll_no!=ROLLno)
	{
	uart_transmit("ERR: Invalid roll number\n", 26);
	return;
	}
	memset(buffer,0,strlen(buffer));
	sprintf(buffer,"  %d  %s",ptr->roll_no,ptr->name);
	uart_transmit(buffer, strlen(buffer));
}


void print_data(struct node *head)
{
	struct node *ptr=head;
	if(ptr==NULL){
		uart_transmit("**Empty**\n",11);
		return;
	}
	uart_transmit("\n",2);
	while(ptr!= NULL)
	{
		memset(buffer,0,sizeof(buffer));
		sprintf(buffer,"  %d    %s",ptr->roll_no,ptr->name);
		uart_transmit(buffer, strlen(buffer));
		ptr=ptr->next;
	}
}


void store_data(struct node **head)
{
	copy_data(head);
}

void backup(struct node **head)
{
	int rollno=0;
	uint8_t name[14]="";
	uint8_t debug_buffer[100]="";
	uint64_t nodes=0;
	HAL_FLASH_Unlock();
	Address=FLASH_USER_START_ADDR;
	nodes=*( uint64_t *)Address;
	Address=Address+8;

	if(nodes==-1)
	return;

	memset(buffer,0,sizeof(buffer));
	memset(datafromflash,0,sizeof(datafromflash));
	for(int i=0;i<nodes*2;i++)
	{
	  rdata=*( uint64_t *)Address;
	  datafromflash[i]=rdata;
	  Address=Address+8;
	}
	HAL_FLASH_Lock();
	memcpy(buffer,datafromflash,nodes*16);
	int i=0;
	while(i<nodes*16)
	{   int k=0;
		memcpy(&rollno,&buffer[i],2);
		k=0;
		i+=2;
		memset(name,0,sizeof(name));
		int n=0;
		while(n<14)	{
		  name[k++]=buffer[i++];
		n++;
		}
		sprintf(debug_buffer,"Roll_no %d and name %s",rollno,name);
		uart_transmit(debug_buffer, strlen(debug_buffer));
		add_to_linklist(head,rollno,name);
		inorder(head,(uint8_t)no_node);
	}
}


void add_to_linklist(struct node **head,int rollno ,uint8_t *name_t)
{
	struct node *ptr=*head;
	struct node *new=calloc(sizeof(struct node),1);
	no_node++;
	if(*head==NULL)
	{
	 new->roll_no=rollno;//rlno;
	 int i=0;

	 while(name_t[i]!='\0')
	 {
		new->name[i]=name_t[i];
		i++;
	 }
		*head=new;
	 new->next=NULL;
	}
	else
	{
	new->roll_no=rollno;//rlno;
	int i=0;
	while(name_t[i]!='\0')
	{
	new->name[i]=name_t[i];
	i++;
	}
	new->next=NULL;
	while(ptr->next != NULL)
	  ptr=ptr->next;
	ptr->next=new;
	}
}
