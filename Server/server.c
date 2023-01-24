#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<unistd.h>
#include<pthread.h>

//gcc server.c -lpthread -Wall -o server

char client_message[128];
char buffer[128];


struct player{
  int socket;
  int turn;
  int mark;
};
struct board{
  int board[9][9];
};
struct wBoard{
  int wb[9];
};
struct room{
  struct player players[2];
  struct board board;
  struct wBoard wb;
  int lastMovei;
};


typedef struct node {
  struct room room;
  struct node * next;
} listNode;

listNode * head = NULL;

struct room createEmptyRoom();
struct player createEmptyPlayer(int socket);

listNode * addRoom(){
  struct room newRoom = createEmptyRoom();
  if(head == NULL){
    head = (listNode *) malloc(sizeof(listNode));
    head->room = newRoom;
    head->next = NULL;
    return head;
  }
  listNode * curr = head;
  while (curr->next != NULL) {
    curr = curr->next;
  }

  curr->next = (listNode *) malloc(sizeof(listNode));  
  curr->next->room = newRoom;
  curr->next->next = NULL; 
  return curr->next;
}

void printRooms2(){
  listNode * curr = head;
  while(curr != NULL){
    printf("{%d, %d} ", curr->room.players[0].socket, curr->room.players[1].socket);
    curr = curr->next;
  }
  printf("\n\n");
}

listNode * joinPlayerToRoom(struct player p){
  listNode * curr = head;

  //if there is 0 rooms case
  if(curr == NULL){
    curr = addRoom();
    curr->room.players[0] = p;
    return curr;
  }
  //check if space in rooms
  while(curr != NULL){
    if(curr->room.players[0].socket == 0) {
      curr->room.players[0] = p;
      return curr;
    } else if(curr->room.players[1].socket == 0) {
      curr->room.players[1] = p;
      return curr;
    }
    curr = curr->next;
  }
  //there is no free space in rooms create new one
  curr = addRoom();
  curr->room.players[0] = p;
  return curr;
}

void removePlayerFromRoom(int playerSocket){
  listNode * curr = head;
  struct player p = createEmptyPlayer(0);
  while(curr != NULL){
    if(curr->room.players[0].socket == playerSocket){
      curr->room.players[0] = p;
      return;
    } else if(curr->room.players[1].socket == playerSocket){
      curr->room.players[1] = p;
      return;
    }
    curr = curr->next;
  }
}

int getPlayerIndexInRoom(listNode * node, int playerSocket){
  if(node->room.players[0].socket == playerSocket) return 0;
  return 1;
}

void sendMsgToSocket(int socket, char * message){
  int n = send(socket, message, strlen(message), 0);
  if (n < 0){printf("ERROR writing message to client socket");}
  
}

void sendMsgToRoom(listNode * room, char * message){
  sendMsgToSocket(room->room.players[0].socket, message);
  sendMsgToSocket(room->room.players[1].socket, message); 
}

struct player createEmptyPlayer(int playerSocket){
  struct player p;

  p.socket = playerSocket;
  p.turn = 0;
  p.mark = 0;
  return p;
}

int canInsertToBoard(int i, int j, listNode * room){
  if(room->room.board.board[i][j] == 0) return 1; 
  return 0;
  
}

int validMovei(int i, int lastMovei){
  if(lastMovei == 10 || i == lastMovei) return 1;
  return 0;
}

int checkIfBothPlayersInRoom(listNode * room){
  if( ( room->room.players[0].socket != 0 ) && ( room->room.players[1].socket != 0 )) return 1;
  return 0;
}

void printPlayerInfo(struct player player){
  printf("socket:%d, turn:%d, mark:%d}\n", player.socket, player.turn, player.mark);
}

void printBoard(int b[9][9], int wb[9]){
  printf("Board:\n");
  for(int i=0; i<9; i++){
    printf("{");
    for(int j=0; j<9; j++){
      if (j==8){
        printf("%d",b[i][j]);
        continue;
      } else {
        printf("%d,",b[i][j]);
      }
    }
    printf("}\n");
  }

  printf("Win board:\n{");
  for(int i=0; i<9; i++){printf("%d ", wb[i]);}
  printf("}\n\n");
}

int checkWin(int b[9]){
  if(b[0] == b[1] && b[0] == b[2] && b[0] != 0) return 1;
  if(b[3] == b[4] && b[3] == b[5] && b[3] != 0) return 1;
  if(b[6] == b[7] && b[6] == b[8] && b[6] != 0) return 1;

  if(b[0] == b[3] && b[0] == b[6] && b[0] != 0) return 1;
  if(b[1] == b[4] && b[1] == b[7] && b[1] != 0) return 1;
  if(b[2] == b[5] && b[2] == b[8] && b[2] != 0) return 1;

  if(b[0] == b[4] && b[0] == b[8] && b[0] != 0) return 1;
  if(b[2] == b[4] && b[2] == b[6] && b[2] != 0) return 1;
  return 0;
}

int checkSdraw(int smallBoard[9]){
  if(checkWin(smallBoard)) return 0;

  for(int i=0; i<9; i++){
    if(smallBoard[i] == 0) return 0;
  }
  return 1;
}

int initServerSocket(){
  int serverSocket;
  struct sockaddr_in serverAddr;

  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if(serverSocket == -1) {
      printf("Create socket error\n");
      exit (EXIT_FAILURE);
  }
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(1106);
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  int r = bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
  if(r == -1) {
      printf("Bind error!\n");
      exit (EXIT_FAILURE);
  }

  if(listen(serverSocket, 6) != 0){
      printf("Error listen: Błąd podczas ustawiania gniazda jako giazdo pasywne\n");
      exit (EXIT_FAILURE);
  }

  printf("Server started.\nWaiting for connections.\n"); 
  return serverSocket;
}

void startGame(listNode * room){
  room->room.players[0].mark = 1;
  room->room.players[1].mark = 2;
  room->room.players[0].turn = 1;
  room->room.players[1].turn = 0;
  sendMsgToSocket(room->room.players[0].socket, "MARK 1");
  sendMsgToSocket(room->room.players[1].socket, "MARK 2");
  sendMsgToSocket(room->room.players[0].socket, "TURN 1");
  sendMsgToSocket(room->room.players[1].socket, "TURN 0");
}



void * socketThread(void *arg){
  int newSocket = *((int *)arg);
  int n;

  struct player player = createEmptyPlayer(newSocket);
  listNode * room = joinPlayerToRoom(player);
  
  printf("Player connected. ");
  printPlayerInfo(player);
  
  
  int playerIndexInRoom = getPlayerIndexInRoom(room, newSocket);
  int opponentIndexInRoom = (playerIndexInRoom == 0)? 1 : 0;
  
  printRooms2();
  
  sendMsgToSocket(newSocket, "CONNECTED");

  int opponentSocket = room->room.players[opponentIndexInRoom].socket;

  sleep(1);

  if(checkIfBothPlayersInRoom(room)) startGame(room);
    
  
  for(;;){
    n=recv(newSocket , client_message , 128 , 0);
    printf("(%d) n: %d", newSocket, n);
    printf("(%d) Message: %s\n", newSocket,client_message);
    if(n<1){
        break;
    }

    char *message = malloc(sizeof(client_message));   
    strcpy(message,client_message);
    
    opponentSocket = room->room.players[opponentIndexInRoom].socket;
    printf("(%d) Opponent socket: %d\n", newSocket, opponentSocket);


    //handle client messages===============================================================================   
    if (message[0] != 'M'){
      memset(&client_message, 0, sizeof(client_message));
      printf("Unhandled client message");
      continue;
    }
    //if there is no opponent continue
    if(opponentSocket == 0){
      sendMsgToSocket(newSocket, "SERVERMSG no_opponent");
      memset(&client_message, 0, sizeof(client_message));
      printf("(%d) no opponent.\n\n", newSocket);
      continue;
    }

    // if is not player turn
    if(room->room.players[playerIndexInRoom].turn == 0){
      sendMsgToSocket(newSocket, "SERVERMSG not_turn");
      memset(&client_message, 0, sizeof(client_message));
      printf("(%d) not your turn.\n\n", newSocket);
      continue;
    }

    char ci = message[1];
    char cj = message[2];
    char cmark = message[3];
    int i = ci - 48;
    int j = cj - 48;
    int mark = cmark - 48;
    
    if(!canInsertToBoard(i, j, room)){
      printf("(%d) invalid move. Already placed mark.\n\n", newSocket);
      sendMsgToSocket(newSocket, "SERVERMSG invalid_move1");
      continue;
    }
    
    if(!validMovei(i, room->room.lastMovei)){
      printf("(%d) invalid move. Wrong square.\n\n", newSocket);
      sendMsgToSocket(newSocket, "SERVERMSG invalid_move2");
      continue;
    }
    
    //insert move to board
    room->room.board.board[i][j] = mark;

    char moveMsg[11] = "MOVE _ _ _";
    moveMsg[5] = ci;
    moveMsg[7] = cj;
    moveMsg[9] = cmark;    
    sendMsgToRoom(room, moveMsg);
    sleep(1);


    //check for sdraws
    if(checkSdraw(room->room.board.board[i])){
      room->room.wb.wb[i] = 3;
      char sdrawMsg[8] = "SDRAW _";
      sdrawMsg[6] = ci;
      sendMsgToRoom(room, sdrawMsg);
      sleep(1);
    }
      
    //check if small win
    if(checkWin(room->room.board.board[i])){
      room->room.wb.wb[i] = mark;
      char swinMsg[9] = "SWIN _ _";
      swinMsg[5] = ci;
      swinMsg[7] = cmark;   
      sendMsgToRoom(room, swinMsg);
      sleep(1);
    }

    //check if big win
    if(checkWin(room->room.wb.wb)){
      sendMsgToSocket(newSocket, "BWIN WIN");
      sendMsgToSocket(opponentSocket, "BWIN LOST");
      memset(&client_message, 0, sizeof(client_message));
      printf("Game ended. (%d) won. \n\n", newSocket);
      continue;
    }

    //check if big draw
    if(checkSdraw(room->room.wb.wb)){
      sendMsgToRoom(room, "BDRAW");
      memset(&client_message, 0, sizeof(client_message));
      continue;
    }

    //if next move is in won square or draw square let next user to input everywhere
    if(room->room.wb.wb[j] != 0){
      room->room.lastMovei = 10; //10 means next player can place move everywhere
    } else {
      room->room.lastMovei = j;
    }

    //swap turns
    room->room.players[playerIndexInRoom].turn = 0;
    room->room.players[opponentIndexInRoom].turn = 1;
    sendMsgToSocket(newSocket, "TURN 0");
    sendMsgToSocket(opponentSocket, "TURN 1");
    
    printBoard(room->room.board.board, room->room.wb.wb);

    printf("Room:\n");
    printf("p1 turn: %d, p2 turn %d\n", room->room.players[0].turn, room->room.players[0].turn);
      
    memset(&client_message, 0, sizeof(client_message));
  }

  printf("Exit socketThread \n");
    
  removePlayerFromRoom(newSocket);

  struct board b; 
  struct wBoard wb;
  for(int i=0; i<9;i++){
    for(int j=0; j<9; j++){
      b.board[i][j] = 0;
    }
  }
  for(int i=0; i<9; i++){wb.wb[i] = 0;}
  room->room.board = b;
  room->room.wb = wb;
  room->room.lastMovei = 10;
  
  //when u are leaving send info to your opponent
  opponentSocket = room->room.players[opponentIndexInRoom].socket;
  
  if(opponentSocket != 0){
    sendMsgToSocket(opponentSocket, "SERVERMSG opponent_left");
  }
  
  printRooms2();
  memset(&client_message, 0, sizeof(client_message));
  pthread_exit(NULL);
}



int main(){
  int serverSocket, newSocket;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;
  

  serverSocket = initServerSocket();

  pthread_t thread_id;

  while(1){
    addr_size = sizeof serverStorage;
    newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);

    if( pthread_create(&thread_id, NULL, socketThread, &newSocket) != 0 ) printf("Failed to create thread\n");

    pthread_detach(thread_id);    
  }
  return 0;
}

struct room createEmptyRoom(){
  struct room room;
  struct board b; 
  struct wBoard wb;
  struct player p = createEmptyPlayer(0);

  for(int i=0; i<9;i++){
    for(int j=0; j<9; j++){b.board[i][j] = 0;}
  }
  for(int i=0; i<9; i++){wb.wb[i] = 0;}

  for(int i=0; i<2; i++){room.players[i] = p;}
  
  room.board = b;
  room.wb = wb;
  room.lastMovei = 10;

  return room;
}