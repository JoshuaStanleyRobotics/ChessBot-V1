/*
   Description: This program is used to control a chess robot that reads the board state using hall effect sensors and moves pieces with an electromagnet on a coreXY motion system.
        The player's moves are communicated over serial to a computer running Stockfish which takes in the user's moves and responds with the computer's moves.
*/

//Libraries
#include "AccelStepper.h"
const int step1 = 23, dir1 = 22, step2 = 13, dir2 = 12;           //Pinout for stepper drivers
const int spd = 1500;                                             //Max stepper motor speed
AccelStepper M1(AccelStepper::DRIVER, step1, dir1);
AccelStepper M2(AccelStepper::DRIVER, step2, dir2);

#include "MultiStepper.h"
MultiStepper motors;

//Pin Declarations
const int row[] = {19, 18, 5, 17, 16, 4, 2, 15};                  //Pinout for magnetic sensor supply by row
const int col[] = {26, 25, 33, 32, 35, 34, 39, 36};               //Pinout for magnetic sensor output by column
const int xLim = 0;                                               //Pinout for X-axis limit switch
const int yLim = 27;                                              //Pinout for Y-axis limit switch
const int mag[] = {14, 21};                                       //Pinouts driving electromagnet

//Constants
const int neutral[8][8] = {                                       //Array of magnetic sensor readings with no magnet present. Upadate with your own values determined experimentally
  {1535, 1535, 1557, 1520, 1555, 1526, 1569, 1507},
  {1490, 1586, 1600, 1535, 1535, 1531, 1535, 1520},
  {1581, 1560, 1557, 1535, 1533, 1511, 1553, 1535},
  {1535, 1504, 1494, 1502, 1535, 1441, 1489, 1498},
  {1525, 1507, 1509, 1503, 1531, 1531, 1535, 1523},
  {1560, 1557, 1573, 1557, 1531, 1575, 1519, 1520},
  {1568, 1584, 1513, 1527, 1511, 1522, 1575, 1513},
  {1579, 1507, 1535, 1535, 1524, 1535, 1535, 1527}
};

//Variables
long positions[2] = {0, 0};                                       //Array for motor positions
char board[8][9] = {"RP    pr", "NP    pn", "BP    pb", "QP    pq", "KP    pk", "BP    pb", "NP    pn", "RP    pr"};
bool currentBoard[8][8];
bool previousBoard[8][8];

char ply[6];
bool turn = 1;
bool invalid = 0;
bool debugging = 0;


void setup() {
  Serial.begin(115200);                                           //Initialize serial communication
  while (!Serial);

  for (int i = 0; i < 8; i++) {                                   //Initialize I/O pins
    pinMode(row[i], OUTPUT);
    pinMode(col[i], INPUT);
  }
  pinMode(step1, OUTPUT);
  pinMode(dir1, OUTPUT);
  pinMode(step2, OUTPUT);
  pinMode(dir2, OUTPUT);
  pinMode(xLim, INPUT_PULLUP);
  pinMode(yLim, INPUT_PULLUP);
  pinMode(mag[0], OUTPUT);
  pinMode(mag[1], OUTPUT);
  analogWrite(mag[0], LOW);
  analogWrite(mag[1], LOW);

  M1.setMaxSpeed(spd);                                            //Configure stepper motors
  M2.setMaxSpeed(spd);
  zero();                                                         //Function to home coreXY motion system
  motors.addStepper(M1);
  motors.addStepper(M2);

  scanBoard();
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      previousBoard[i][j] = currentBoard[i][j];
    }
  }
  if (debugging) {
    for (int j = 7; j > -1; j--) {
      Serial.println("+---+---+---+---+---+---+---+---+");
      Serial.print("| ");
      for (int i = 0; i < 8; i++) {
        Serial.print(currentBoard[i][j]);
        Serial.print(" | ");
      }
      Serial.println("");
    }
    Serial.println("+---+---+---+---+---+---+---+---+");
  }
  delay(250);
}


void loop() {
  if (turn) {                                                     //Player's turn
    scanBoard();
    if (compareBoard()) {                                         //Compare to the previous board state to see if a move was made
      if (board[ply[0] - 97][ply[1] - 49] == 'P' && ply[3] == '8') { //if pawn makes it to opposite side, promote to queen
        if (debugging) Serial.println("Promoting to Queen");
        board[ply[0] - 97][ply[1] - 49] = 'Q';
        ply[4] = 'q';
      }
      if ((board[ply[0] - 97][ply[1] - 49] == 'K') && (ply[0] == 'e' && ply[1] == '1')) { //Look for castling move
        if (ply[2] == 'c') {      //Queen side castling
          board[0][0] = ' ';
          board[3][0] = 'R';
          delay(2000);
        }
        else if (ply[2] == 'g') { //King side castling
          board[7][0] = ' ';
          board[5][0] = 'R';
          delay(2000);
        }
      }
      updateBoard(ply);                                           //Update the board array with the player ply
      Serial.println(ply);                                        //Send the player's move to the computer
      for (int i = 0; i < 5; i++) ply[i] = ' ';
      turn = 0;                                                   //Switch to the computer player's turn
    }
  }

  else {                                                          //Computer's turn
    while (Serial.available() == 0) delay(5);                     //Wait for a serial input
    String response = Serial.readStringUntil('\n');
    if (debugging) Serial.println(response);
    response.toCharArray(ply, 6);

    if (board[ply[0] - 97][ply[1] - 49] == ' ') {                 //if the departing tile is empty, an invalid move was sent back
      if (debugging) Serial.print("Invalid move sent: ");
      invalid = 1;
      char reverse[5];
      for (int i = 0; i < 5; i++) reverse[i] = ply[i];
      if (debugging) Serial.println(reverse);

      ply[0] = reverse[2];
      ply[1] = reverse[3];
      ply[2] = reverse[0];
      ply[3] = reverse[1];
    }
    else if (board[ply[2] - 97][ply[3] - 49] != ' ') {            //if the destination tile is occupied, topple the captured piece and remove it from the board state
      if (debugging) Serial.println("Toppling Piece");
      goTo(ply[2] - 97, ply[3] - 49);
      analogWrite(mag[1], 255);
      delay(5);
      analogWrite(mag[1], 0);
      currentBoard[ply[2] - 97][ply[3] - 49] = 0;
    }

    if (board[ply[0] - 97][ply[1] - 49] != 'n' && board[ply[0] - 97][ply[1] - 49] != 'N') {                   //typical move
      goTo(ply[0] - 97, ply[1] - 49);
      analogWrite(mag[!invalid], 255);
      goTo(ply[2] - 97, ply[3] - 49);
      pulse(!invalid);
      updateBoard(ply);
    }
    else {                                                        //knight move special case
      if (debugging) Serial.println("Knight Move!");
      goTo(ply[0] - 97, ply[1] - 49);
      analogWrite(mag[!invalid], 255);
      goTo(ply[0] - 97 + 0.5 * ((ply[2] - ply[0]) / (abs(ply[2] - ply[0]))), ply[1] - 49 + 0.5 * ((ply[3] - ply[1]) / (abs(ply[3] - ply[1]))));
      analogWrite(mag[!invalid], 155); //60% power 155
      goTo(ply[2] - 97 - 0.5 * ((ply[2] - ply[0]) / (abs(ply[2] - ply[0]))), ply[3] - 49 - 0.5 * ((ply[3] - ply[1]) / (abs(ply[3] - ply[1]))));
      analogWrite(mag[!invalid], 255);
      delay(5);
      goTo(ply[2] - 97, ply[3] - 49);
      pulse(!invalid);
      updateBoard(ply);
    }

    if ((board[ply[2] - 97][ply[3] - 49] == 'k') && (ply[0] == 'e' && ply[2] == 'g')) { //king side castling special case
      if (debugging) Serial.println("King Side Castling");
      goTo('h' - 97, '8' - 49);
      analogWrite(mag[!invalid], 255);
      goTo('h' - 97.5, '8' - 49.5);
      analogWrite(mag[!invalid], 170);
      goTo('g' - 97.5, '8' - 49.5);
      analogWrite(mag[!invalid], 255);
      delay(5);
      goTo('f' - 97, '8' - 49);
      pulse(!invalid);
      updateBoard("h8f8");
    }
    else if ((board[ply[2] - 97][ply[3] - 49] == 'k') && (ply[0] == 'e' && ply[2] == 'c')) { //queen side castling special case
      if (debugging)Serial.println("Queen Side Castling");
      goTo('a' - 97, '8' - 49);
      analogWrite(mag[!invalid], 255);
      goTo('b' - 97.5, '8' - 49.5);
      analogWrite(mag[!invalid], 170);
      goTo('d' - 97.5, '8' - 49.5);
      analogWrite(mag[!invalid], 255);
      delay(5);
      goTo('d' - 97, '8' - 49);
      pulse(!invalid);
      updateBoard("a8d8");
    }

    if (ply[4] == '+') {                                          //Check: bump king
      for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
          if (board[i][j] == 'K') {
            goTo(i, j);
            analogWrite(mag[1], 255);
            delay(5);
            analogWrite(mag[1], 0);
          }
        }
      }
    }
    else if (ply[4] == '#') {                                     //Checkmate: topple king
      for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
          if (board[i][j] == 'K') {
            goTo(i, j);
            analogWrite(mag[1], 255);
            delay(20);
            analogWrite(mag[1], 0);
          }
        }
      }
    }

    invalid = 0;
    turn = 1;                                                     //Switch to the player's turn
    for (int i = 0; i < 5; i++) ply[i] = ' ';
  }
  delay(100);
}


void scanBoard() {                                                //Scans the sensor array for all the black pieces
  for (int j = 0; j < 8; j++) {
    digitalWrite(row[j], 255);
    for (int i = 0; i < 8; i++) {
      //int val = analogRead(col[i]) - neutral[j][i];
      //Serial.print(String(val) + '\t');
      if (neutral[j][i] - analogRead(col[i]) > 200) currentBoard[i][j] = 1;
      else currentBoard[i][j] = 0;
    }
    //Serial.println("");
    digitalWrite(row[j], LOW);
  }
  //Serial.println('\n');
}

bool compareBoard() {                                             //Compares the current board with the previous board and determines if a move was made
  bool departed = 0;
  bool arrived = 0;
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      if (currentBoard[i][j] != previousBoard[i][j]) {
        if (currentBoard[i][j]) {
          arrived = 1;
          ply[2] = i + 97;
          ply[3] = j + 49;
        }
        else {
          departed = 1;
          ply[0] = i + 97;
          ply[1] = j + 49;
        }
      }
    }
  }
  if (departed && arrived) return 1;                              //If a piece left one tile and arrived on another, then a full ply was detected
  else return 0;
}

void updateBoard(char p[]) {
  board[p[2] - 97][p[3] - 49] = board[p[0] - 97][p[1] - 49];      //update piece on destination tile
  board[p[0] - 97][p[1] - 49] = ' ';                              //remove piece from departed tile

  scanBoard();
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      previousBoard[i][j] = currentBoard[i][j];
    }
  }
  if (debugging) {
    for (int j = 7; j > -1; j--) {
      Serial.println("+---+---+---+---+---+---+---+---+");
      Serial.print("| ");
      for (int i = 0; i < 8; i++) {
        //Serial.print(currentBoard[i][j]);
        Serial.print(board[i][j]);
        Serial.print(" | ");
      }
      Serial.println("");
    }
    Serial.println("+---+---+---+---+---+---+---+---+");
  }
}

void goTo(float xPos, float yPos) {
  if (debugging) Serial.println("  Position: " + String(xPos) + ", " + String(yPos));
  yPos = yPos - 7;                                                //Translate the coordinate system
  positions[0] = 1200 * (xPos - yPos);                            //Rotate the coordinate system (for coreXY motion)
  positions[1] = 1200 * (xPos + yPos);

  //if (debugging) Serial.println("  Motor Position: " + String(positions[0]) + ", " + String(positions[1]));
  //if (debugging) Serial.println("");
  motors.moveTo(positions);
  motors.runSpeedToPosition();
}

void pulse(bool polarity) {                                       //Pulse the magnet to center a piece
  analogWrite(mag[polarity], 0);
  analogWrite(mag[!polarity], 255);
  delay(10);
  analogWrite(mag[!polarity], 0);
  analogWrite(mag[polarity], 255);
  delay(100);
  analogWrite(mag[polarity], 0);
}

void zero() {
  if (debugging) Serial.print("Homing X-Axis");
  M1.setSpeed(-1000);
  M2.setSpeed(-1000);
  while (digitalRead(xLim)) {
    M1.runSpeed();
    M2.runSpeed();
    if (debugging) Serial.print(".");
    delay(1);
  }
  M1.setSpeed(0);
  M2.setSpeed(0);
  M1.runSpeed();
  M2.runSpeed();
  if (debugging) Serial.println("");

  if (debugging) Serial.print("Homing Y-Axis");
  M1.setSpeed(-1000);
  M2.setSpeed(1000);
  while (digitalRead(yLim)) {
    M1.runSpeed();
    M2.runSpeed();
    if (debugging) Serial.print(".");
    delay(1);
  }
  M1.setSpeed(0);
  M2.setSpeed(0);
  M1.runSpeed();
  M2.runSpeed();
  if (debugging) Serial.println("");
  M1.setCurrentPosition(0);
  M2.setCurrentPosition(0);
}
