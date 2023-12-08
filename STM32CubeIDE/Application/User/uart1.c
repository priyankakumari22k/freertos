/*
 * uart.c
 *
 *  Created on: Oct 31, 2023
 *      Author: 158180
 */
#include "stm32l5xx_hal.h"
#include "stm32l5xx_hal_uart.h"
#include "stm32l5xx_hal_uart_ex.h"
#include "uart1.h"
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "queue.h"

//#define ERASE_FLASH_MANUALLY
#define FLASH_DOUBLEWORD_TYPEPROGRAM
#define FLASH_USER_START_ADDR   ADDR_FLASH_PAGE_125   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     (ADDR_FLASH_PAGE_125 + FLASH_PAGE_SIZE - 1)

static uint8_t input_data[20];
static uint64_t no_node=0;
static uint32_t FirstPage = 0, NbOfPages = 0, BankNumber = 0;
uint32_t Address = ADDR_FLASH_PAGE_125;
uint32_t PageError = 0;
uint8_t buffer[2000];

FLASH_EraseInitTypeDef EraseInitStruct;
static uint32_t GetPage(uint32_t );
static uint32_t GetBank(uint32_t );

uint8_t name[]="\nEnter name >>>  ";
uint8_t roll[]="\nEnter roll number >>>  ";
QueueHandle_t xPointerQueue;


typedef enum UART_INPUT_RET{
		UART_RET_CMD_SUCCESS,
		UART_RET_CMD_ERROR
}E_UART_INPUT_RET;

typedef enum RETURN_STATUS{
		RET_SUCCESS=0,
		RET_INVALID_ERR
}E_RETURN_STATUS;

/**
* @brief data is transmitted through this Function to terminal
* @param data:information which need to be transmitted
*		 length: length of data
* @retval None
*/
void uart_transmit(uint8_t *data,int length)
{
	int i=0;
	while (i<length)
	{
	while (!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = data[i++];
	}
}

/**
* @brief data stored in queue are transfered in required API
* @param None
* @retval received character
*/
uint8_t uart_receive()
{   static uint8_t received_data = 0;
	while(1)
	{
	if(xQueueReceive(xPointerQueue, &received_data, 0) == pdTRUE)
	return received_data;
	}
}

/**
* @brief Checking weather the entered input is number or not
* @param input: user entered input
* @retval return message for error or success
*/
E_UART_INPUT_RET check_input_number(int input)
{
	if(input>0 && input<125)
		return UART_RET_CMD_SUCCESS;
	else{
		memset(buffer,0,strlen(buffer));
		sprintf(buffer,"*****ERR: Input data is not a digit and error code is: %d*****",UART_RET_CMD_ERROR);
		uart_transmit(buffer,strlen(buffer));
		return UART_RET_CMD_ERROR;
	}
}

/**
* @brief Checking weather the entered input are all alphabet or not
* @param input: user entered input
* @retval return message for error or success
*/
E_UART_INPUT_RET check_input_alphabet(uint8_t *input)
{
	for(int i=0;i<(strlen(input)-2);i++)
	{
		if(!((input[i]>'a' && input[i]<'z') || (input[i]>'A' && input[i]<'Z'))){
			memset(buffer,0,strlen(buffer));
			sprintf(buffer,"*****ERR: Input data is not alphabetic and error code is: %d*****",UART_RET_CMD_ERROR);
			uart_transmit(buffer,strlen(buffer));
			return UART_RET_CMD_ERROR;
		}
	}
	return UART_RET_CMD_SUCCESS;
}

/**
* @brief Storing the received data from the terminal in a buffer
* @param None
* @retval The address of the buffer
*/
uint8_t * input()
{
	int i=0;
	memset(input_data,'\0',sizeof(input_data));
	do
	{
		input_data[i++]=uart_receive();
	}while (!(strstr((char *)input_data, UART_CLI_STR_TERM_CR_LF)));
	return input_data;
}

/**
* @brief Creating new node and adding it to link list
* @param head: It is the starting address of the link list or address of first node
* @retval None
*/
void create_node(struct node **head)
{
	uint8_t *data=NULL;
	E_RETURN_STATUS retValue;
	int i=0;
	struct node *ptr= *head;
	struct node *new=calloc(sizeof(struct node),1);
	if(*head==NULL)
	{
		//Student Roll number
		uart_transmit(roll, sizeof(roll));
		data=(uint8_t*)input();
		if((retValue=check_input_number(atoi(data)))!=RET_SUCCESS)
			return;
		new->roll_no=atoi(data);

		//Student name
		uart_transmit(name, sizeof(name));
		data=(uint8_t*)input();
		if((retValue=check_input_alphabet(data))!=RET_SUCCESS)
		  return;
		memcpy(new->name,data,strlen(data));
		if(strlen(new->name)>NAME_SIZE)
		{
		uart_transmit("*****ERR: Name length size exceeded*****\n", 42);
		memset(new->name,0,strlen(new->name));
		return;
		}
		else{
		no_node++;
		*head=new;
		new->next=NULL;
		}
	}
	else
	{
		//Student Roll number
		uart_transmit(roll, sizeof(roll));
		data=(uint8_t*)input();
		if((retValue=check_input_number(atoi(data)))!=RET_SUCCESS)
		  return;
		int num=atoi(data);
		while(ptr!= NULL){
			if(ptr->roll_no==num)
			{
			uart_transmit("*****ERR: Roll no. already exists*****\n", 40);
			return;
			}
			ptr=ptr->next;
		}
		new->roll_no=num;

		//STUDENT NAME
		uart_transmit(name, strlen(name));
		data=(uint8_t*)input();
		if((retValue=check_input_alphabet(data))!=RET_SUCCESS)
		  return;
		memcpy(new->name,data,strlen(data));
		if(strlen(new->name)>NAME_SIZE)
		{
		uart_transmit("*****ERR: Name length size exceeded*****\n", 42);
		memset(new->name,0,strlen(new->name));
		return;
		}
		else{
		no_node++;
		ptr= *head;
		new->next=NULL;
		while(ptr->next != NULL)
			ptr=ptr->next;
		ptr->next=new;
		}
	}

}

/**
* @brief swapping two nodes if current node value is greater than then the next one
* @param current_node: Current node address
* 		 next_node: Next node Address
* @retval Address of next code after swapping
*/
struct node* swap(struct node* current_node, struct node* next_node)
{
	struct node* temp = next_node->next;
	next_node->next = current_node;
	current_node->next = temp;
	return next_node;
}

/**
* @brief Comparing the two node roll number for making it in increasing order
* @param head: Starting address of linklist
*		 count: Number of nodes in linklist
* @retval None
*/
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

/**
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
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

/**
  * @brief  Gets the bank of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The bank of a given address
  */
static uint32_t GetBank(uint32_t Addr)
{
	return FLASH_BANK_1;
}

/**
* @brief Flash Initialization Function
* @param None
* @retval None
*/
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

#ifdef ERASE_FLASH_MANUALLY
	if(HAL_FLASHEx_Erase(&EraseInitStruct, &PageError)!=HAL_OK)
		 {
			 uart_transmit("Error\n", 6);
		 }
	HAL_FLASH_Lock();
#endif
}

/**
* @brief Add new user data to linkinst
* @param head: Starting address of linklist
* @retval None
*/
void add_data(struct node **head)
{
	create_node(head);
	inorder(head,(uint8_t)no_node);
}

/**
* @brief Deletes the particular or required student data from the database
* @param head: Starting address of linklist
* @retval None
*/
void delete_data(struct node **head)
{
	struct node *prev;
	struct node *ptr=*head;
	E_RETURN_STATUS retValue;
	uart_transmit(roll, sizeof(roll));
	int i=0;
	uint8_t *data=NULL;
	data=(uint8_t*)input();
	if((retValue=check_input_number(atoi(data)))!=RET_SUCCESS)
	  return;
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
				uart_transmit("*****ERR: Invalid roll number*****\n", 36);
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

/**
* @brief Search the particular or required student data from the database
* @param head: Starting address of linklist
* @retval None
*/
void search_data(struct node **head)
{
	struct node *ptr=*head;
	E_RETURN_STATUS retValue;
	uart_transmit(roll, sizeof(roll));
	int i=0;
	uint8_t *data=NULL;
	data=(uint8_t*)input();
	if((retValue=check_input_number(atoi(data)))!=RET_SUCCESS)
	  return;
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
	uart_transmit("*****ERR: Invalid roll number*****\n", 36);
	return;
	}
	memset(buffer,0,strlen(buffer));
	sprintf(buffer,"  %d  %s",ptr->roll_no,ptr->name);
	uart_transmit(buffer, strlen(buffer));
}

/**
* @brief Prints the stored user data
* @param head: Starting address of linklist
* @retval None
*/
void print_data(struct node **head)
{
	struct node *ptr=*head;
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

/**
* @brief Stores the data of linkist in flash memory
* @param head: Starting address of linklist
* @retval None
*/
void store_data(struct node **head)
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
			uint8_t store_data[SIZE]="";
			memset(store_data,0,sizeof(store_data));
			uint8_t refference[8]="";
			memcpy(&store_data[0],&(ptr->roll_no),ROLL_NO_SIZE);
			memcpy(&store_data[2],ptr->name,NAME_SIZE);
			memcpy(refference,&store_data[8],SIZE/2);
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD ,Address, *(uint64_t * )store_data);
			Address=Address+8;
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD ,Address, *(uint64_t * )refference);
			Address=Address+8;
			ptr=ptr->next;
		 }
}

/**
* @brief Fetch the data from the flash memory on restart of the board
* @param head: Starting address of linklist
* @retval None
*/
void backup(struct node **head)
{
	int rollno=0;
	uint8_t name[NAME_SIZE]="";
	uint8_t debug_buffer[100]="";
	uint64_t datafromflash[125];
	uint64_t nodes=0;
	HAL_FLASH_Unlock();
	Address=FLASH_USER_START_ADDR;
	nodes=*( uint64_t *)Address;
	Address=Address+8;

	if(nodes==-1)
	return;

	memset(buffer,0,sizeof(buffer));
	memset(datafromflash,0,sizeof(datafromflash));
	memcpy(datafromflash,( uint64_t *)Address,nodes*SIZE);

	HAL_FLASH_Lock();
	memcpy(buffer,datafromflash,nodes*SIZE);
	int i=0;
	while(i<nodes*SIZE)
	{
		memcpy(&rollno,&buffer[i],ROLL_NO_SIZE);
		i+=2;
		memset(name,0,sizeof(name));
		memcpy(name,&buffer[i],NAME_SIZE);
		i+=NAME_SIZE;
		sprintf(debug_buffer,"Roll_no %d and name %s",rollno,name);
		uart_transmit(debug_buffer, strlen(debug_buffer));
		add_to_linklist(head,rollno,name);
	}
}

/**
* @brief Adding fetched data from flash memory to the linklist
* @param head: Starting address of linklist
* 		 rollno: Roll number of student
* 		 name_t: Name of the student
* @retval None
*/
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
	new->roll_no=rollno;
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
