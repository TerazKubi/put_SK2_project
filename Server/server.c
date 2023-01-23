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

struct indexes{
  int indexes[2];
};
struct player{
  int socket;
  int turn;
  int mark;
  struct indexes indexes;
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

struct room rooms[3];




struct indexes getIndexOfFirstFreeSpaceInArray(){
  static struct indexes idxs;
  idxs.indexes[0] = -1;
  idxs.indexes[1] = -1;

  for(int i=0; i<3; i++){
    for(int j=0; j<2; j++){
      if(rooms[i].players[j].socket == 0){
        idxs.indexes[0] = i;
        idxs.indexes[1] = j;
        return idxs;
      }
    }
  }
  return idxs;
}

void sendMsgToSocket(int socket, char * message){
  int n = send(socket, message, strlen(message), 0);
  if (n < 0){printf("ERROR writing message to client socket");}
  
}
void sendMsgToRoom(int roomId, char * message){
  sendMsgToSocket(rooms[roomId].players[0].socket, message);
  sendMsgToSocket(rooms[roomId].players[1].socket, message); 
}
struct player createEmptyPlayer(){
  struct player p;
  struct indexes idxs;
  for(int i=0; i<2; i++){idxs.indexes[i] = 0;}

  p.socket = 0;
  p.turn = 0;
  p.mark = 0;
  p.indexes = idxs;
  return p;
}
struct room createEmptyRoom(){
  struct room room;
  struct board b; 
  struct wBoard wb;
  struct player p = createEmptyPlayer();

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
int canInsertToBoard(int i, int j, int roomId){
  if(rooms[roomId].board.board[i][j] == 0) return 1; 
  return 0;
  
}

int validMovei(int i, int lastMovei){
  if(lastMovei == 10 || i == lastMovei) return 1;
  return 0;
}

int checkIfBothPlayerInRoom(int roomId){
  if( ( rooms[roomId].players[0].socket != 0 ) && ( rooms[roomId].players[1].socket != 0 )){
    return 1;
  } else {
    return 0;
  }
}

void printPlayerInfo(struct player player){
  printf("{room(i):%d, index(j):%d, socket:%d, turn:%d, mark:%d}\n", player.indexes.indexes[0], player.indexes.indexes[1], player.socket, player.turn, player.mark);
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

void printRooms(){
  printf("{");
  for(int i=0; i<3;i++){
    printf("{%d, %d}, ",rooms[i].players[0].socket, rooms[i].players[1].socket);
  }
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
  serverAddr.sin_port = htons(1107);
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

void startGame(int roomId){
  rooms[roomId].players[0].mark = 1;
  rooms[roomId].players[1].mark = 2;
  rooms[roomId].players[0].turn = 1;
  rooms[roomId].players[1].turn = 0;
  sendMsgToSocket(rooms[roomId].players[0].socket, "MARK 1");
  sendMsgToSocket(rooms[roomId].players[1].socket, "MARK 2");
  sendMsgToSocket(rooms[roomId].players[0].socket, "TURN 1");
  sendMsgToSocket(rooms[roomId].players[1].socket, "TURN 0");
}

void * socketThread(void *arg){
  int newSocket = *((int *)arg);
  int n;

  struct indexes idx_free_space = getIndexOfFirstFreeSpaceInArray();

  if(idx_free_space.indexes[0] == -1){
    printf("cant find free space. Exit thread\n");
    pthread_exit(NULL);
  }
  struct player player = {newSocket, 0, 0, idx_free_space};
  
  printf("Player connected. ");
  printPlayerInfo(player);
  
  int roomId = player.indexes.indexes[0];
  int playerIndexInRoom = player.indexes.indexes[1];

  int opponentIndexInRoom;
  if (playerIndexInRoom == 0){
    opponentIndexInRoom = 1;
  } else {
    opponentIndexInRoom = 0;
  }
  //join player to room
  rooms[roomId].players[playerIndexInRoom] = player;
  printf("Room nr:%d = {p1_soc:%d, p2_soc:%d}\n\n", roomId, rooms[roomId].players[playerIndexInRoom].socket, rooms[roomId].players[opponentIndexInRoom].socket);
  printRooms();
  
  sendMsgToSocket(newSocket, "CONNECTED");

  int opponentSocket = rooms[roomId].players[opponentIndexInRoom].socket;

  sleep(1);

  if(checkIfBothPlayerInRoom(roomId)) startGame(roomId);
    
  
  for(;;){
    n=recv(newSocket , client_message , 128 , 0);
    printf("(%d) n: %d", newSocket, n);
    printf("(%d) Message: %s\n", newSocket,client_message);
    if(n<1){
        break;
    }

    char *message = malloc(sizeof(client_message));   
    strcpy(message,client_message);
    
    opponentSocket = rooms[roomId].players[opponentIndexInRoom].socket;
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
    if(rooms[roomId].players[playerIndexInRoom].turn == 0){
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
    
    if(!canInsertToBoard(i, j, roomId)){
      printf("(%d) invalid move. Already placed mark.\n\n", newSocket);
      sendMsgToSocket(newSocket, "SERVERMSG invalid_move1");
      continue;
    }
    
    if(!validMovei(i, rooms[roomId].lastMovei)){
      printf("(%d) invalid move. Wrong square.\n\n", newSocket);
      sendMsgToSocket(newSocket, "SERVERMSG invalid_move2");
      continue;
    }
    
    //insert move to board
    rooms[roomId].board.board[i][j] = mark;

    char moveMsg[11] = "MOVE _ _ _";
    moveMsg[5] = ci;
    moveMsg[7] = cj;
    moveMsg[9] = cmark;    
    sendMsgToRoom(roomId, moveMsg);
    sleep(1);


    //check for sdraws
    if(checkSdraw(rooms[roomId].board.board[i])){
      rooms[roomId].wb.wb[i] = 3;
      char sdrawMsg[8] = "SDRAW _";
      sdrawMsg[6] = ci;
      sendMsgToRoom(roomId, sdrawMsg);
      sleep(1);
    }
      
    //check if small win
    if(checkWin(rooms[roomId].board.board[i])){
      rooms[roomId].wb.wb[i] = mark;
      char swinMsg[9] = "SWIN _ _";
      swinMsg[5] = ci;
      swinMsg[7] = cmark;
      
      sendMsgToRoom(roomId, swinMsg);
    }

    //check if big win
    if(checkWin(rooms[roomId].wb.wb)){
      sendMsgToSocket(newSocket, "BWIN WIN");
      sendMsgToSocket(opponentSocket, "BWIN LOST");
      memset(&client_message, 0, sizeof(client_message));
      printf("Game in room: %d ended. \n\n", roomId);
      continue;
    }

    //check if big draw
    if(checkSdraw(rooms[roomId].wb.wb)){
      sendMsgToSocket(newSocket, "BDRAW");
      sendMsgToSocket(opponentSocket, "BDRAW");
      memset(&client_message, 0, sizeof(client_message));
      continue;
    }

    //if next move is in won square or draw square let next user to input everywhere
    if(rooms[roomId].wb.wb[j] != 0){
      rooms[roomId].lastMovei = 10; //10 means next player can place move everywhere
    } else {
      rooms[roomId].lastMovei = j;
    }

    //swap turns
    rooms[roomId].players[playerIndexInRoom].turn = 0;
    rooms[roomId].players[opponentIndexInRoom].turn = 1;
    sendMsgToSocket(newSocket, "TURN 0");
    sendMsgToSocket(opponentSocket, "TURN 1");
    
    printBoard(rooms[roomId].board.board, rooms[roomId].wb.wb);
      
    memset(&client_message, 0, sizeof(client_message));
  }

  printf("Exit socketThread \n");
  struct player emptyPlayer = createEmptyPlayer();
  
  rooms[roomId].players[playerIndexInRoom] = emptyPlayer;
  struct board b; 
  struct wBoard wb;

  for(int i=0; i<9;i++){
    for(int j=0; j<9; j++){
      b.board[i][j] = 0;
    }
  }
  for(int i=0; i<9; i++){wb.wb[i] = 0;}
  rooms[roomId].board = b;
  rooms[roomId].wb = wb;
  rooms[roomId].lastMovei = 10;

  printRooms();
  
  //when u are leaving send info to your opponent
  opponentSocket = rooms[roomId].players[opponentIndexInRoom].socket;
  
  if(opponentSocket != 0){
    sendMsgToSocket(opponentSocket, "SERVERMSG opponent_left");
  }
  
  memset(&client_message, 0, sizeof(client_message));
  pthread_exit(NULL);
}



int main(){
  int serverSocket, newSocket;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;

  struct room emptyRoom = createEmptyRoom();
  
  for(int i=0; i<3; i++){rooms[i] = emptyRoom;}


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
