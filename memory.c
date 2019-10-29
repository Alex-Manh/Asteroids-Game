/*
 * eeprom.c
 *
 * Created: 29/05/2019 7:13:33 PM
 *  Author: Minh Anh Bui
 */ 
#include <avr/eeprom.h>
#include <stdio.h>
#include <ctype.h>

#include "score.h"
#include "memory.h"
#include "terminalio.h"


// At most, 100 bytes in eeprom should be used
uint8_t signature=9;
uint32_t easyScores[5];
uint8_t scores[5][4];
uint8_t names[5][12];
uint8_t returnScore[4];
uint8_t returnName[12];
int startIndex = 8;

//As the program starts running,
//all old records are retrieved
void retrieve(void){
	uint8_t signatureCheck;
	uint8_t dataName[12];
	int recordIndex = startIndex;
	int dataIndex=0;
	
	for (uint8_t i=0; i<5;i++){
		uint32_t currentScore=0;
		signatureCheck = eeprom_read_byte((const uint8_t*)recordIndex);
		if (signatureCheck == signature)
		{
			int scoreIndex = recordIndex+2;
			int nameIndex = recordIndex+7;
			for (uint8_t z=0; z<4; z++)
			{
				scores[dataIndex][z] = eeprom_read_byte((uint8_t*)scoreIndex);
				currentScore+=scores[dataIndex][z];
				scoreIndex++;
			}
			
			for (uint8_t j=0; j<12; j++)
			{
				dataName[j] = eeprom_read_byte((uint8_t*)nameIndex);
				names[dataIndex][j] = dataName[j];
				nameIndex++;
			}
			easyScores[dataIndex] = currentScore;
			move_cursor(20, 20 + dataIndex);
			printf("Rank: %d , Score: %lu , Name: %s", dataIndex+1, currentScore, dataName);
			currentScore=0;
		} else {
			easyScores[dataIndex] = 0;
			for (uint8_t z=0; z<4; z++)
			{
				scores[dataIndex][z] = 0;
			}
			for (uint8_t j=0; j<12; j++)
			{
				names[dataIndex][j] = 0;
			}
		}
		dataIndex++;
		recordIndex+=20;		
	}
}

void update(){
	uint8_t dataScore[4];
	uint8_t dataName[12];
	int dataIndex = 0;
	int recordIndex = startIndex;
	for (uint8_t i=0; i<5;i++)
	{
		uint32_t tempScore = easyScores[dataIndex];
		if (tempScore!=0)
		{
			int scoreIndex = recordIndex+2;
			int nameIndex = recordIndex+7;
			
			eeprom_write_byte((uint8_t*)recordIndex, signature);
			for (uint8_t z=0; z<4; z++)
			{
				dataScore[z] = scores[dataIndex][z];
				eeprom_write_byte((uint8_t*)scoreIndex, dataScore[z]);
				scoreIndex++;
			}
			for (uint8_t j=0; j<12; j++)
			{
				dataName[j] = names[dataIndex][j];
				eeprom_write_byte((uint8_t*)nameIndex, dataName[j]);
				nameIndex++;
			}
		}
		dataIndex++;
		recordIndex+=20;
	}
}

//Ask the users for their names
void prompt_name(){
	returnName[12]=0;
	char nameInput;
	int index=0;
	
	move_cursor(50, 5);
	printf("New High Score");
	move_cursor(50, 7);
	printf("Your name is ");
	
	for (uint8_t i=0; i<12; i++)
	{
		nameInput = fgetc(stdin);
		if (isalpha(nameInput) || nameInput==' ')
		{
			returnName[index] = nameInput;
			move_cursor(50, 8);
			printf("%s", returnName);
			index++;
		} else if (nameInput == 127 || nameInput ==8)
		{
			//Move back 1 step to delete previous char
			returnName[index-1] = ' ';
			index = index-1;
			//i also counts the delete, so minus 2
			i = i-2;
			move_cursor(50, 8);
			printf("%s", returnName);
		}
		else if (nameInput == '\n')
		{
			break;
		}
	}
}

//This will be called at game over, 
//to check if the scores have beaten high scores
void check_record(){
	uint32_t current_score = get_score();
	returnScore[0] = (current_score & 0x000000ff);
	returnScore[1] = (current_score & 0x0000ff00) >> 8;
	returnScore[2] = (current_score & 0x00ff0000) >> 16;
	returnScore[3] = (current_score & 0xff000000) >> 24;
	
	for (uint8_t index=0; index<5; index++)
	{
		uint32_t dataScore = easyScores[index]; 
		//Check if the high score was beaten
		if (current_score > dataScore)
		{
			//if true, push the data down by 1
			for(uint8_t i=4; i> index; i--){
				easyScores[i] = easyScores[i-1];
				for (uint8_t j=0; j<12; j++)
				{
					names[i][j] = names[i-1][j];
				}
				for (uint8_t x=0; x<4; x++)
				{
					scores[i][x] = scores[i-1][x];
				}
			}
			
			//Push the new high score in
			easyScores[index] = current_score;
			for (uint8_t z=0; z<4; z++)
			{
				scores[index][z] = returnScore[z];
			}
			
			prompt_name();
			for (uint8_t j=0; j<12; j++)
			{
				names[index][j] = returnName[j];
			}
			update();
			break;
		}
	}
}
