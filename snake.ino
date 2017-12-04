#include "LedControl.h"
#include "LiquidCrystal.h"

LedControl lc=LedControl(12,11,10,1);
LiquidCrystal lcd(13, 8, 7, 5, 4, 3);

const int okPin = A1, leftPin = A2, rightPin = A0, joyX = A3, joyY = A4, ledPin = A5;
bool okState, leftState, rightState, wallsState, ledState[8][8], foodState;
int gameMoment, menu, position, selectedSpeed=1, score, direction=1, previousDirection=1, foodLine, foodColumn, xValue, yValue, speed, line, column, firstAcces;
unsigned long previousTimeSnake, previousTimeFood,previousTime;
char menuOption[3][17] = {"<-    Speed   ->", "<-    Play    ->", "<-    Mode    ->"}, modeOption[2][17]={"<-  No walls  ->", "<-    Walls   ->"};

struct dot {
  int line, column;
  dot *address;
};
dot *front, *back, *newDot;

void showMenu(int &menu){
  lcd.setCursor(0, 0);
  lcd.print("      MENU ");
  lcd.setCursor(0,1);
  lcd.print(menuOption[menu]); //Afiseaza stringul corespunzator fiecarui meniu identificat printr-un numar
}

void startGame(int &gameMoment) { 
  unsigned long currentTime = millis();
  okState = digitalRead(okPin);
  if(currentTime-previousTime >= 300) {            //Nu se apasa butonul "OK" ,deci se afiseaza un meniu de intro
    lcd.setCursor(5, 0);
    lcd.print("SNAKE"); 
    position = position % 16 + 1;
    if(position <= 16){
      lcd.setCursor(0,1);
      for(int i=0; i<=position; i++) 
        lcd.print(" ");
      lcd.print("-----");
      position++;
    }
  previousTime = currentTime;
  } else  if(okState == HIGH) {
             menu = 1;           //Jocul trece in faza a doua, deschizandu-se meniul pe optiunea identificata prin 1, optiunea "Play"
             showMenu(menu);
             gameMoment++;
             delay(300);        //1)
          }
}

void mainMenu(int &gameMoment){ 
  okState = digitalRead(okPin);
  leftState = digitalRead(leftPin);
  rightState = digitalRead(rightPin);
  if(leftState == HIGH) { // Se realizeaza deplasarea in meniu catre optiunea din stanga
    menu = (menu+2)%3;
    showMenu(menu);
    delay(300);                                                                                     //1)
  } else if(rightState == HIGH){ // Se realizeaza deplasarea in meniu catre optiunea din drepata
           menu = ( menu + 1 ) % 3;
           showMenu(menu);
           delay(300);
         } else if(okState == HIGH) { //Se intra in meniul specific optiunii
                  gameMoment = menu + 2;
                  delay(300);
                  firstAcces = 1;
                }
}

void speedMenu(int &gameMoment) {
  if(firstAcces == 1) {   //Afiseaza meniul la viteza selectata initial sau anterior
    lcd.setCursor(5, 0);
    lcd.print("Speed");
    lcd.setCursor(3,1);
    for(int i=4; i<=3+selectedSpeed; i++) 
      lcd.print("-");
    for(int i=3+selectedSpeed; i<=10; i++) 
      lcd.print(" ");
    firstAcces = 0;
  }
  okState = digitalRead(okPin);
  leftState = digitalRead(leftPin);
  rightState = digitalRead(rightPin);
  lcd.setCursor(3,1);
  if(leftState == HIGH){      //Viteza scade
     if(selectedSpeed != 1){ 
       selectedSpeed--;
       lcd.setCursor(3+selectedSpeed,1);
       lcd.print(" ");
      }
      delay(300);                                                                                  //1)
  } else if(rightState == HIGH){   //Viteza creste
           if(selectedSpeed != 10){
             selectedSpeed++;
             lcd.setCursor(2+selectedSpeed,1);
             lcd.print("-");
           } 
           delay(300);
         } else if(okState == HIGH){ //Este aleasa o viteza si se revine in meniul cu optiuni
                 showMenu(menu);
                 gameMoment = 1;
                 delay(300);
                }
}

void modeMenu(int &gameMoment){
  lcd.setCursor(5, 0);
  lcd.print(" Mode");
  lcd.setCursor(0,1);
  lcd.print(modeOption[wallsState]); //Afiseaza meniul la modul selectat initial sau anterior (cu pereti sau fara)
  okState = digitalRead(okPin);
  leftState = digitalRead(leftPin);
  rightState = digitalRead(rightPin);
  if(leftState == HIGH || rightState == HIGH){ //Se afiseaza celalat mod
      wallsState = (wallsState+1)%2;
      lcd.setCursor(0,1);
      lcd.print(modeOption[wallsState]);
      delay(300);                                                                           //1)
  } else if(okState == HIGH){       //Este ales modul si se revine in meniul cu optiuni
                  showMenu(menu);
                  gameMoment = 1;
                  delay(300);
                }
}

void playGame(int &gameMoment){
  lcd.setCursor(0,0);             //Se stinge lcd-ul
  lcd.print("                 ");
  lcd.setCursor(0,1);
  lcd.print("                 ");
  back = new dot;               
  front = new dot;
  front->line = 4;
  front->column = 2;
  front->address = NULL;
  back->line = 4;
  back->column = 1;
  back->address = front;
  lc.setLed(0,back->line,back->column,true);    //Sarpele apare ca fiind format din 2 puncte (4,1), (4,2)
  lc.setLed(0,front->line,front->column,true);
  ledState[back->line][back->column] = ledState[front->line][front->column] = 1; //In matrice se marcheaza care leduri sunt aprinse
  if(wallsState == 1)                         //Daca este ales modul cu pereti, se va borda matricea
    for(int border=0; border<=7; border++) {
       lc.setLed(0,0,border,true);
       lc.setLed(0,7,border,true);
       lc.setLed(0,border,0,true);
       lc.setLed(0,border,7,true);
     }
  for(int border=0; border<=7; border++)
     ledState[0][border] = ledState[7][border] = ledState[border][0] = ledState[border][7] = wallsState;      
  foodLine = random(8);                       //Punctul ce reprezinta mancarea va lua o pozitie aleator sub forma unui led care nu este deja aprins
  foodColumn = random(8);
  while(ledState[foodLine][foodColumn] == 1)
    foodLine = random(8),foodColumn = random(8);
  lc.setLed(0,foodLine,foodColumn,true);
  foodState = 1;          //Punctul de mancare initial este aprins
  gameMoment = 5;
  speed = selectedSpeed;  //Viteaza initiala reprezinta viteza selectata de jucator in meniul Speed
  line = 4;
  column = 2;
}

void gameEnd(int &gameMoment){
  while(back != NULL){          //Este stearsa lista alocata dinamic ce memora sarpele
    newDot = back;
    back = back->address;
    delete(newDot);
  }
  lc.clearDisplay(0);        //Pe LCD apare scorul
  lcd.setCursor(0,0);
  lcd.print("Score:");
  lcd.setCursor(8,0);
  lcd.print(score);
  lcd.setCursor(0, 1);
  lcd.print("OK for MENU      ");
  okState = digitalRead(okPin);
  if(okState == HIGH){    //Cand se apasa OK, se revine in meniul cu optiuni
    gameMoment = 1;
    menu = 1;
    showMenu(menu);
    delay(300);                                                                                                   //1)
   }
  if(gameMoment == 1) {               //Se reatribuie variabilelor valorile necesare inceperii unui eventual joc
   direction = previousDirection = 1;
   score = 0;
   for(int i=1; i<=6; i++) 
     for(int j=1; j<=6; j++) 
        ledState[i][j] = 0;
   for(int i=0; i<=7; i++)
     ledState[0][i] = ledState[7][i] = ledState[i][0] = ledState[i][7] = wallsState;    
  }
}

void game(int &gameMoment){
  xValue = analogRead(joyX);          //In functie de pozitia joystickului, este stabilita directia de mers a sarpelui
  yValue = analogRead(joyY);
  if(400 <= xValue && xValue <= 600) {
    if(yValue > 600) direction = 3;
    if(yValue < 400) direction = 1;
  }
  if(400 <= yValue && yValue <= 600){
    if(xValue > 600) direction = 4;
    if(xValue < 400) direction = 2;
  }
  if(previousDirection%2 == direction%2) direction = previousDirection; /*Daca directia este aceeasi cu cea anterioara, directia ramane aceeasi intrucat sarpele nu poate m
                                                                         merge inapoi*/
  unsigned long currentTime = millis();
  if(currentTime-previousTimeFood >= 400){  //La 0,4 secunde mancarea licareste
    if(foodState == 0){
      lc.setLed(0,foodLine,foodColumn,true);
      foodState = 1;
    } else {
            lc.setLed(0,foodLine,foodColumn,false);
            foodState = 0;
           }
    previousTimeFood = currentTime;
  }
  if(previousDirection != direction || currentTime-previousTimeSnake >= (11-speed)*100){  /*Dupa o perioada invers proportionala cu viteza sarpelui la momentul respectiv 
                                                                                          sau daca juctorul schimba directia coordonatele capului se vor schimba*/
    previousDirection = direction;
    switch(direction){
      case 1: { 
        column = (front->column+1)%8;
        break;
      }
      case 2: { 
        line = (front->line+7)%8;
        break;
      }
      case 3: { 
        column = (front->column+7)%8;
        break;
      }
      case 4: { 
        line = (front->line+1)%8;
        break;
      }
    }
    if(line != foodLine || column != foodColumn)
      if(ledState[line][column] == 1){                  //In cazul in care capul sarpelui isi atinge corpul,jocul se termina, fapt semnalat prin aprinderea ledului
       gameMoment = 6;
       digitalWrite(ledPin,HIGH);
       delay(1000);                                     //Delay pentru aprinderea ledului si stingerea lui la o secunda
       digitalWrite(ledPin,LOW);    
      } else {                                         //Daca nu ajunge la mancare, sarpele avanseaza conform coordonatelor deja stabilite
               ledState[line][column] = 1;
               lc.setLed(0,line,column,true);
               lc.setLed(0,back->line,back->column,false);
               ledState[back->line][back->column] = 0;
               front->address = back;
               front = back;
               back = back->address;
               front->line = line;
               front->column = column;
               front->address = NULL;
              }
    else {                                          //Daca ajunge la punctul ce  mancare, acesta devine capul sapelui si se genereaza o alta mancare
           score++;
           if(foodState == 0) 
              lc.setLed(0,foodLine,foodColumn,true);
           newDot = new dot;
           newDot->line = foodLine;
           newDot->column = foodColumn;
           newDot->address = NULL;
           front->address = newDot;
           front = newDot;
           foodLine = random(8), foodColumn = random(8);
           while(ledState[foodLine][foodColumn] == 1) 
              foodLine = random(8), foodColumn = random(8);
           lc.setLed(0,foodLine,foodColumn,true);
           foodState = 1;
           if(score%4 == 0 && speed!=10)                       //Viteza creste progresiv odata la 4 puncte acumulate
              speed++;
         }
    previousTimeSnake = currentTime;
    }
}


void setup() {
  pinMode(okPin,INPUT);
  pinMode(leftPin,INPUT);
  pinMode(rightPin,INPUT);
  pinMode(ledPin,OUTPUT);
  Serial.begin(9600);
  lc.shutdown(0,false);
  lc.setIntensity(0,8);
  lc.clearDisplay(0);
  lcd.begin(16, 2);
  analogWrite(6,60);
  randomSeed(analogRead(0));  
}

void loop() {
 switch(gameMoment) {
  case 0: {
    startGame(gameMoment);
    break;
  } 
  case 1: {
    mainMenu(gameMoment); 
    break;
  } 
  case 2: {
    speedMenu(gameMoment); 
    break;
  } 
  case 3: {
    playGame(gameMoment);
    break;
  } 
  case 4: {
    modeMenu(gameMoment);
    break;
  }
  case 5: {
    game(gameMoment); 
    break;
  }
  case 6: {       
    gameEnd(gameMoment);    
    break;
  }
 }
}
/*1)Delayurile sunt folosite pentru a nu fi citita imediat urmatoare valoare a butonului, astfel incat sa se ajunga exact in meniul dorit*/
