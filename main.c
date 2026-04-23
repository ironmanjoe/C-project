#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

//Tile drawings
#define MAP_T "  *  "
#define PLAYER_T "[o_o]"
#define ENEMY_T "[+v+]"
#define DOOR_T "[o  ]"

//globals
int width = 10;
int height = 10;

//game structures
enum Tiles{
    MAP,
    PLAYER,
    ENEMY,
    DOOR
};

//Creature - a moveable game object with a position and type
typedef struct{
    enum Tiles type;
    int x;
    int y;
} Creature;

//Map - a map with variable width and height
typedef struct{
    int width;
    int height;
    int *grid;
} Map;

//--- Terminal functions ---

void clsc() {
    printf("\033[2J\033[H");
}

void clearBuffer(){
	int c;
	while((c = getchar()) != '\n' && c != EOF);
}

//--- Map functions ---

//map initialization that creates a map of dynamic size
//I KNOW THI DOESN'T FREE THE MEMORY AFTERWARDS, THIS FUNCTION ONLY GETS CALLED ONCE
//AND WAS JUST TO TRY AND MAKE A MAP OF DYNAMIC SIZE. THE MEMORY IS FREED AFTER THE 
//PROGRAM FINISHES.
Map mapInit(int w, int h){
    Map map;
    map.width = w;
    map.height = h;
    map.grid = (int *)malloc(w*h*sizeof(int));
    return map;
}

//returns a tile from specified coordinates
int getTile(Map map, int x, int y){
    return map.grid[y*map.width + x];
}

//sets a tile at specified coordinates
void setTile(Map map, int x, int y, int val){
    map.grid[y*map.width + x] = val;
}

//sets a tile of a specified creature
void setCreature(Map map, Creature c){
    map.grid[c.y*map.width + c.x] = c.type;
}

//clears a creature tile
void clearCreature(Map map, Creature c){
    map.grid[c.y*map.width + c.x] = 0;
}

//prints the whole map to the console
void drawMap(Map map){
    
    int tile;
    
    for(int y = 0; y < map.height; y++){
        
        for(int x = 0; x < map.width; x++){
            
            tile = map.grid[y*map.width + x];
            
            switch(tile){
                case MAP:
                    printf("%s",MAP_T);
                    break;
                case PLAYER:
                    printf("%s",PLAYER_T);
                    break;
                case ENEMY:
                    printf("%s",ENEMY_T);
                    break;
                case DOOR:
                    printf("%s",DOOR_T);
                    break;
                default:
                    printf("?");
            }
        }
        printf("\n\n");
    }
}

//--- Creature functions ---

//get the absolute y distance of two creatures 
int yDist(Creature *a, Creature *b){
    int absolute = a->y - b->y;
    return (absolute < 0) ? absolute*-1 : absolute;
}

//get the absolute x distance of two creatures
int xDist(Creature *a, Creature *b){
    int absolute = a->x - b->x;
    return (absolute < 0) ? absolute*-1 : absolute;
}

//check if two creatures are within x tiles of each other
int tooClose(Creature *a, Creature *b, int x){
    return xDist(a,b) < x && yDist(a,b) < x;
}

//explicitly set creature position
void setPos(Creature *c, int x, int y){
    c->x = x;
    c->y = y;
}

//compare position of creatures
int comparePos(Creature *a, Creature *b){
    return(a->x == b->x && a->y == b->y);
}

//randomize creature position
void rPos(Creature *c){
    c->x = rand() % width;
    c->y = rand() % height;
}

void creatureInit(Creature *player, Creature *enemy, Creature *door){
    //The door is largely static except when first placing, so it will
    //be what the player and enemy positions are decided around
    rPos(door);
    //the player is positioned based on the door so they don't overlap
    do{
        rPos(player);
    } while(comparePos(door,player) != 0);
    
    //the enemy is positioned based on how close it is to the player
    do{
        rPos(enemy);
    } while(comparePos(enemy,player) != 0 || tooClose(enemy,player,3) != 0);
}

//ensures player stays within map boundaries by wrapping to other side of map
void playerBoundsHandler(Creature *player){
    if(player->x > width-1){
        player->x = 0;
    }
    else if(player->x < 0){
        player->x = width-1;
    }
    
    if(player->y > height-1){
        player->y = 0;
    }
    else if(player->y < 0){
        player->y = height-1;
    }
}

void clearCreatures(Map map, Creature player, Creature enemy, Creature door){
    clearCreature(map,player);
    clearCreature(map,enemy);
    clearCreature(map,door);
}

void updateCreatures(Map map, Creature player, Creature enemy, Creature door){
    setCreature(map,player);
    setCreature(map,enemy);
    setCreature(map,door);
}

int manageInput(Creature *player, char in){
    switch(in){
        case 'w':
            player->y--;
            break;
        case 'a':
            player->x--;
            break;
        case 's':
            player->y++;
            break;
        case 'd':
            player->x++;
            break;
        case 'q':
            player->y--;
            player->x--;
            break;
        case 'e':
            player->y--;
            player->x++;
            break;
        case 'z':
            player->y++;
            player->x--;
            break;
        case 'x':
            player->y++;
            player->x++;
            break;
        case 'r':
            printf("\nGood game.\n");
            return 0;
            break;
        default:
            printf("\nUnknown input.\n");
            return 1;
        }
        return 2;
}

void moveEnemy(Creature *enemy, Creature *player){
    int xDiff = player->x - enemy->x;
    int yDiff = player->y - enemy->y;
    
    if(xDiff >= 2){
        enemy->x += 2;
    }
    else if(xDiff == 1){
        enemy->x++;
    }
    
    if(xDiff <= -2){
        enemy->x -= 2;
    }
    else if(xDiff == -1){
        enemy->x--;
    }
    
    if(yDiff >= 2){
        enemy->y += 2;
    }
    else if(yDiff == 1){
        enemy->y++;
    }
    
    if(yDiff <= -2){
        enemy->y -= 2;
    }
    else if(yDiff == -1){
        enemy->y--;
    }
}

int main(){
    //seeding random values
    srand(time(NULL));
    
    //setting up map
    Map map = mapInit(width,height);
    
    //setting up movable map tiles
    Creature player = {PLAYER,0,0};
    Creature enemy = {ENEMY,1,0};
    Creature door = {DOOR,2,0};
    creatureInit(&player,&enemy,&door);
    updateCreatures(map,player,enemy,door);
    
    //draw map
    drawMap(map);

    //control flow
    int run = 1;
    
    //score
    int score = 0;
    
    //input management
    int inputFlags;
    char in;
    
    //main game loop
    while(run){
        //clear creatures off map
        clearCreatures(map,player,enemy,door);
        
        //displays score and prompts input
        printf("Score: %d\nYour move: ",score);
        
        //gets input
        scanf(" %c",&in);
        
        //updates player based on input
        inputFlags = manageInput(&player,in);
        
        //exits game
        if(inputFlags == 0){
            return 0;
        }
        
        //clears input buffer in case of typos or extra characters
        clearBuffer();
        
        //keeps player in map
        playerBoundsHandler(&player);
        
        //checks if player reached door, increments score and resets creatures
        if(comparePos(&player,&door)){ 
            score++;
            creatureInit(&player,&enemy,&door);
        }
        else if (inputFlags == 2){
            moveEnemy(&enemy,&player);
        }

        //only updates if a valid input was found, otherwise lets user try again
        if(inputFlags == 2){
            updateCreatures(map,player,enemy,door);
            clsc();
            drawMap(map);
            //if the enemy touches player, reset game
            if(comparePos(&player,&enemy)){
                printf("It got you.\n");
                printf("Continue? (y/n): ");
                scanf(" %c",&in);
                
                if(in == 'y'){
                    score = 0;
                    clearCreatures(map,player,enemy,door);
                    creatureInit(&player,&enemy,&door);
                    updateCreatures(map,player,enemy,door);
                    clsc();
                    drawMap(map);
                }
                else{
                    return 0;
                }
            }
        }
    }
    return 0;
}
