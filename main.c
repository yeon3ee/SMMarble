//
//  main.c
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05. Updated by Cheyeon Park on 2023/12
//
//

//Include necessary header files
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "smm_object.h"
#include "smm_database.h"
#include "smm_common.h"

//Define file path for board, food, and festival configurations
#define BOARDFILEPATH "marbleBoardConfig.txt"
#define FOODFILEPATH "marbleFoodConfig.txt"
#define FESTFILEPATH "marbleFestivalConfig.txt"

//Define constants for the program
#define MAX_NAME_LENGTH 				50
#define NUM_GRADES                      9
#define MAX_DICE                        6
#define MAX_NODE        				100
#define GRADUATE_CREDIT					30
#define MAX_PLAYER 						10

// Initialize variables for the board, food, and festival nodes
static int board_nr;
static int food_nr;
static int festival_nr;
static int player_nr; 

 
// Define a structure for a player
typedef struct player{
        int energy;
        int position; //현재위치 
        char name[MAX_CHARNAME];
        int accumCredit; //누적학점 
        int flag_graduate; //졸업여부  (flag로 0 or 1)
} player_t;

// Declare an array of player structures
static player_t *cur_player;


// Arrays to store player information
static int player_energy[MAX_PLAYER];
static int player_position[MAX_PLAYER];
static char player_name[MAX_PLAYER][MAX_CHARNAME];
// Array to store passed nodes
int passedNodesCount = 0;
 
//function prototypes
int isGraduated(void); //check if any player is graduated
void generatePlayers(int n, int initEnergy); //generate a new player
void printGrades(int player); //print grade history of the player
//Print the courses the player has taken and their respective credits 
void goForward(int player, int step); //make player go "step" steps on the board (check if player is graduated)
void printPlayerStatus(void); //print all player status at the beginning of each turn
//smmObjGrade_e joinLecture(int player, char *lectureName, int credit); 
void actionNode(int player);
float calcAverageGrade(int player); //calculate average grade of the player
//int smmObjGrade_e takeLecture(int player, char *lectureName, int credit); //take the lecture (insert a grade of the player)
void* findGrade(int player, char *lectureName); //find the grade from the player's grade history
int FoodChargeConfig(void);
    
    
int isGraduated(void) {
    int i;

    for (i = 0; i < player_nr; i++) {
        if (cur_player[i].flag_graduate) {
            return 1;  //  If there is a graduated player
        }
    }

    return 0;  
    // No graduated players
}

// Print status of all players
void printGrades(int player)
{
     int i;
     void *gradePtr;
     for (i=0; i<smmdb_len(LISTNO_OFFSET_GRADE + player); i++)
     {
         gradePtr = smmdb_getData(LISTNO_OFFSET_GRADE + player, i);
         printf("%s : %i\n", smmObj_getNodeName((int)gradePtr), smmObj_getNodeGrade(gradePtr));
     }
}

//모든 플레이어의 상태 출력 함 수 
void printPlayerStatus(void)
{
     int i;
     
     for (i=0; i<player_nr; i++)
     {
         printf("\n*----------------PLAYER STATUS----------------*\n");
         printf("%s :  credit : %i, energy : %i, position : %i\n",
                     cur_player[i].name,
                     cur_player[i].accumCredit,
                     cur_player[i].energy,
                     cur_player[i].position);
         printf("*-----------------------------------------------*\n");
     }
}

// Generate players with given number and initial energy
void generatePlayers(int n, int initEnergy)
{
     int i;
     //n time loop
     for (i=0; i<n; i++)
     {
         //input name
         printf("Input player %i's name: ", i);
         scanf("%s", cur_player[i].name);
         fflush(stdin);
         
         //set position
         cur_player[i].position = 0;
         
         //set energy
         cur_player[i].energy = initEnergy;
         cur_player[i].accumCredit = 0;
         cur_player[i].flag_graduate = 0;
     } 
}

// Function to roll a die gpt.
int rolldie(int player) {
    char a;
    printf("\nThis is %s's turn ::::  Press any key to roll a die (press g to see grade): ", cur_player[player].name);
    a = getchar();

    if (a == 'g') {
        printGrades(player);
        return 0;
    } else {
        int dice_num = (rand() % MAX_DICE) + 1;
        printf("%i player's turn! press any button. \n", player + 1);
        a = getchar();

        if (a == 'g') {
            printGrades(player);
        } else {
            printf("dice %i.\n", dice_num);
        }

        return dice_num;
    }
}

// Function to take a turn for a player
void takeTurn(int player) {
    int die_result = rolldie(player);

    if (die_result > 0) {
        // 주사위 결과가 0보다 크면 이동 및 노드 액션 처리
        goForward(player, die_result);
        actionNode(player);
    }
}

//축제 관련 미션(파일에서 가져오기) 
void FestivalConfig(int player){
     
     
     int missionsIndex = rand() %5;
     char mission[MAX_NODE];
     
    
    //메모장 열기  
    FILE* festivalConfigFile = fopen("marbleFestivalConfig.txt", "r");
    if (festivalConfigFile == NULL) {
        perror("[Error] opening marbleFestivalConfig.txt");
        exit(EXIT_FAILURE);
    }
	
	//결과에 따른 결과물  
    int i;
    for (i = 0; i <= missionsIndex; i++) {
        if (fscanf(festivalConfigFile, "%[^\n]\n", mission) != 1) {
            perror("[Error] reading from marbleFestivalConfig.txt");
            exit(EXIT_FAILURE);
        }
    }
 	//파일 닫기  
    fclose(festivalConfigFile);

    //결과 프린트 
    printf("Your mission: %s\n", mission);

    
    int charge = FoodChargeConfig(); 
    cur_player[player].energy = cur_player[player].energy+charge;

    
    printf("%s completed the mission and gained %d energy. Current energy: %d\n", cur_player[player].name, charge, cur_player[player].energy);
}   
         

// Function to go forward a certain number of steps on the board
void goForward (int player, int step)
{
     void *boardPtr;
     cur_player[player].position += step;
     boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
     
     printf("%s go to node %i (name: %s)\n", 
                cur_player[player].name, cur_player[player].position,
                smmObj_getNodeName(boardPtr));
}



// Function to charge energy based on food configuration
int FoodChargeConfig(void) {
    
    FILE* foodConfigFile = fopen("marbleFoodConfig.txt", "r");
    if (foodConfigFile == NULL) {
        perror("[Error] opening marbleFoodConfig.txt");
        exit(EXIT_FAILURE);
    }

    int numChoices = 14; 
    int randomChoices = rand() % numChoices; 

    int charge;
    int i; 
    for (i = 0; i <= randomChoices; i++) {
        if (fscanf(foodConfigFile, "%*[^,], charge:%i\n", &charge) != 1) {
            perror("[Error] reading from marbleFoodConfig.txt");
            exit(EXIT_FAILURE);
        }
    }

    fclose(foodConfigFile);
    
    return charge;
}





// Function to handle actions at a node
void actionNode(int player)
{
    void* boardPtr = smmdb_getData(LISTNO_NODE, cur_player[player].position);
    int type = smmObj_getNodeType(boardPtr); 
    //int type = smmObj_getNodeType(cur_player[player].position);
    char* name = smmObj_getNodeName (boardPtr);
    void* gradePtr; 
    
    switch (type) 
    {
        //case lecture
        case SMMNODE_TYPE_LECTURE:
           takeLecture(player, name, smmObj_getNodeCredit(boardPtr));
           break;
           
        case SMMNODE_TYPE_RESTAURTANT:
            cur_player[player].energy += smmObj_getNodeEnergy(boardPtr);
            printf("-> Let's eat in %s and charge %d energies. (remained energy :%d)\n", cur_player[player].name, smmObj_getNodeName(boardPtr), smmObj_getNodeEnergy(boardPtr), cur_player[player].energy);
            break;

        case SMMNODE_TYPE_LABORATORY:
            
            if (cur_player[player].position) {
               
                int dice_result = rolldie(player);
                int experiment_threshold = 4; 

                 if (dice_result >= experiment_threshold) {
                   
                    cur_player[player].position = 0; 
                    printf("success! %d can exit this lab!", cur_player[player].name);
                } else {
                    
                    printf("Experiment result : %i, fail T_T. %d needs more experiment... ", cur_player[player].name);
                }
            } else {
                
                cur_player[player].position = 1;
                printf("Experiment time! Let's see if you can sarisfy professor, %d.  Energy: %d\n", cur_player[player].name, cur_player[player].energy);
            }
            break;

        case SMMNODE_TYPE_HOME:
            
            cur_player[player].energy += smmObj_getNodeEnergy(boardPtr);
            printf("%s returend to HOME! Energy charged by %d (total : %d)\n", cur_player[player].name, smmObj_getNodeEnergy(boardPtr), cur_player[player].energy);
            break;

        case SMMNODE_TYPE_GOTOLAB:
            
            cur_player[player].position = 1;
            printf("->This is not experiment time. You can go through this lab.");
            break;

        case SMMNODE_TYPE_FOODCHANCE: {
            int charge = FoodChargeConfig();
            
            cur_player[player].energy += charge;
        
            
            printf(" -> %s gets a food chance! Press any key to pick a food card: ", cur_player[player].name);
        
            
            getchar();
        
          
            printf(" -> %s picks %s and charges %d (remained energy: %d)\n",cur_player[player].name, smmObj_getNodeName(boardPtr), charge, cur_player[player].energy);
            break; 
        }

        case SMMNODE_TYPE_FESTIVAL:
            printf("%s participates to Snow Festival! Press any key to pick a festival card :", cur_player[player].name);
            
           //함수 실행  
            FestivalMission(player);
             
            break;
        default:
            break;
    }
}
    

void* findGrade(int player, char *lectureName) {
   
    int nodeIndex = LISTNO_OFFSET_GRADE + player;
    void* gradePtr = smmdb_getData(nodeIndex, 0);

    while (gradePtr != NULL) {
        char* gradeName = smmObj_getNodeName(gradePtr);

        if (strcmp(gradeName, lectureName) == 0) {
            
            return gradePtr;
        }

        
        int nextIndex = smmdb_addTail(nodeIndex, gradePtr);
        if (nextIndex >= 0) {
            gradePtr = smmdb_getData(nodeIndex, nextIndex);
        } 
		else {
            break;  
        }
    }
	
    return NULL; //없으면 NULL 
}

int main(int argc, const char * argv[]) {
    
    FILE* fp;
    char name[MAX_CHARNAME];
    int type;
    int credit;
    int energy;
    int i; 
    int initEnergy;
    int turn = 0;
    
    board_nr = 0;
    food_nr = 0;
    festival_nr = 0;
    
    srand(time(NULL));
    
    
    //1. import parameters ---------------------------------------------------------------------------------
    //1-1. boardConfig 
    //error handling code
    if ((fp = fopen(BOARDFILEPATH, "r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", BOARDFILEPATH);
        getchar();
        return -1;
    }
    
    printf("Reading board...\n");
    do {
    // Store the parameter set
    void *boardObj = smmObj_genObject(name, type, type, credit, energy, 0); 
    smmdb_addTail(LISTNO_NODE, boardObj);

    if (type == SMMNODE_TYPE_HOME)
        initEnergy = energy;
    board_nr++;

    // Continue reading while the condition is true
	} while (fscanf(fp, "%*d. %[^,], type:%d, credit:%d, energy:%d", name, &type, &credit, &energy) == 4);

	fclose(fp);
    for (i = 0; i < board_nr; i++)
    {
        void *boardObj = smmdb_getData(LISTNO_NODE, i);
    
        printf("=> %i: %s, type: %i, credit %i, energy %i\n",
               i, smmObj_getNodeName(boardObj),
               type, smmObj_getNodeCredit(boardObj), smmObj_getNodeEnergy(boardObj));
    }
    
    printf("Total number of board nodes : %i\n", board_nr);
    
    
    for (i=0; i<board_nr; i++)
    {
        void *boardObj = smmdb_getData(LISTNO_NODE, i); 
        
        printf("node %i : %s, %i(%s), credit %i, energy %i\n", 
            i, smmObj_getNodeName(boardObj),  
            smmObj_getNodeType(boardObj), smmObj_getNodeType(smmObj_getNodeName(boardObj)),
            smmObj_getNodeCredit(boardObj), smmObj_getNodeEnergy(boardObj));
        
    }
    
    
    //2. food card config 
    //error handling code
    if ((fp = fopen(FOODFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
        return -1; 
    }
    
    printf("\n\n Reading food card component......\n");
   
   
    while (fscanf(fp, "%[^,], charge:%d", name, &credit) == 2) //read a food parameter set
    {
       //store the parameter set
       void *foodObj = smmObj_genObject(name, LISTNO_FOODCARD, type, credit, energy, 0);
       smmdb_addTail(LISTNO_NODE, foodObj);

       printf("%s, charge: %d", name, credit); // print food card information
       food_nr++; //while문 돌때마다 변수 하나씩 증가  
    }
    fclose(fp);
    printf("\nTotal number of food cards : %i\n", food_nr);
    
    
    
    //3. festival card config 
    if ((fp = fopen(FESTFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
        return -1;
    }
    
    printf("\n\nReading festival card component......\n");
    

while (fscanf(fp, "%" STRINGIFY(MAX_NAME_LENGTH) "[^\n]", name) == 1) {
    // store the parameter set
    void *festivalObj = smmObj_genObject(name, LISTNO_FESTCARD, 0, 0, 0, 0);
    smmdb_addTail(LISTNO_NODE, festivalObj);

    printf("%s\n", name); 
    festival_nr++;

    int c;
    while ((c = fgetc(fp)) != '\n' && c != EOF) {
        // read and discard characters until a newline or end-of-file
    }
}

fclose(fp);
printf("Total number of festival cards: %i\n", festival_nr);


    
    //2. Player configuration ---------------------------------------------------------------------------------
    
    // Input player number to player_nr
printf("input the player number : ");
scanf("%d", &player_nr);
fflush(stdin);

// Check if the entered player number is valid
if (player_nr < 0 || player_nr > MAX_PLAYER) {
    // Handle the case where the entered player number is invalid
    printf("Invalid player number. Please enter a number between 0 and %d.\n", MAX_PLAYER);
    // You might want to loop back and ask for input again, or exit the program, depending on your requirements
}

    
    cur_player=(player_t*)malloc(player_nr*sizeof(player_t));
    
    generatePlayers(player_nr,initEnergy);
    
    
    //3. SM Marble game starts ---------------------------------------------------------------------------------
    while (1) //is anybody graduated?
    {
        int die_result;
        int noLab;
        
        
        //4-1. initial printing
        printPlayerStatus();
        
        //4-2. die rolling (if not in experiment)   
        die_result = rolldie(turn);
        
        //4-3. go forward
        goForward(turn, die_result);
        if (isGraduated())
            break;
		//4-4. take action at the destination node of the board
        actionNode(turn);

        //End condition
        // check if any player has graduated
        
        
        //4-5. next turn
        turn = (turn + 1)%player_nr;
        
        
    }
    printf("Cog!! \n ----------Game End-----------\n");
    for(i=0;i<player_nr;i++)
    {
    	printf("player %i chart\n",i+1);
    	printGrades(i);
    }
    
    free(cur_player);
    
    
    return 0;
} 
//main.c
