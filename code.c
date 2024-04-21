// подключение необходимых библиотек
#include <AmperkaKB.h> // работа с клавиатурой
#include <SD.h> // карта памяти
#include <iarduino_IR_TX.h> // передатчи
#include <iarduino_IR_RX.h>  // приемник
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header

// инициализация необходимых параметров
hd44780_I2Cexp display;
const int LCD_COLS = 16;
const int LCD_ROWS = 4;

iarduino_IR_TX IC(A3); // задаем передачику пин
iarduino_IR_RX IR(22);  // задаем приемнику пин

AmperkaKB KB(46, 44, 42, 40, 38, 36, 34, 32); // для работы с клавиатурой

bool remote = false;
bool scan = false;
bool start_menu = true; // запуск главного меню

// методы
void menu(); // функция главного меню
int Scan(); // меню сканера
void ScanMode(); // сканирование и вывод
void SaveScan(); // загрузка в файл кодировок
void RemoteMode(); // меню пультика
void LoadFiles(File dir, int numTabs); // загрузка файлов из SD
void ShowFileName(); // показать имена файлов на SD

String fileNameSD = "TEST";
int idfileNameSD = 0;
String arrayFileName[100];
int IDarrayFileName = 0;
String code_array[10];

File myFile;

void setup()
{
  Serial.begin(9600);

  IR.begin();
  IC.begin();

  KB.begin(KB4x4);

  display.begin(LCD_COLS, LCD_ROWS);
  display.backlight();

}

void loop()
{
  KB.read();
  display.setCursor(0, 0);
  display.clear();

  if (scan)
  {
    int Mode;

    display.clear();
    Mode = Scan();

    if (Mode == 2) ScanMode();
    else if (Mode == 1) SaveScan();
    else start_menu = true;
    scan = false;
  }
  else
  {
    if (remote)
    {
      IDarrayFileName = 0;
      if (!SD.begin(53))
      {
        Serial.println("initialization failed!");
        display.clear();
        display.setCursor(4,2);
        display.print("ERROR OPEN SD");
        delay(3000);
        remote = false;
        start_menu = true;
      }
      else
      {
        Serial.println("initialization done.");
        myFile = SD.open("/");
        LoadFiles(myFile, 0);
        ShowFileName();
        myFile.close();
        remote = false;
      }
      SD.end();
    }

    else
    {
      if(start_menu)
      {
        display.clear();
        menu();
        start_menu = false;
      }
    }
  }
}


void menu()
{
  display.setCursor(6,0);
  display.print("MENU");
  display.setCursor(6,1);
  display.print("1 - REMOTE");
  display.setCursor(10, 2);
  display.print("2 - SCANNER");

  while (true)
  {
    KB.read();
    if (KB.justPressed())
    {
      Serial.print(KB.getNum);
      if(KB.getNum == 1)
      {
        remote = true;
        break;
      }
      else if (KB.getNum == 4)
      {
        scan = true;
        break;
      }
      else continue;
    }
    else continue;
  }
}

int Scan()
{
  display.clear();
  while (true)
  {
    display.setCursor(4, 0);
    display.print("CHOICE MOD");
    display.setCursor(4,1);
    display.print("1 - SAVE");
    display.setCursor(8,2);
    display.print("2 - SHOW");
    display.setCursor(8,3);
    display.print("B - EXIT");

    KB.read();

    if (KB.justPressed())
    {
      Serial.print(KB.getNum);
      if(KB.getNum == 1)
      {
        return 1;
      }
      else if (KB.getNum == 4)
      {
        return 2;
      }
      else if (KB.getNum == 0)
      {
        return 3;
      }
      else continue;

    }
    else continue;
  }
}

void SaveScan()
{
  String bufNameFileSD = fileNameSD + String(idfileNameSD) + ".txt";
  Serial.println(bufNameFileSD);
  File dataFile = SD.open(bufNameFileSD, FILE_WRITE);

  if (dataFile)
  {
    String bufCode = "NONE";
    idfileNameSD ++;
    display.clear();
    while (true)
    {
      display.setCursor(5,0);
      display.print("PRESSED B TO EXIT");
      display.setCursor(5,1);
      display.print("SCANNER");
      display.setCursor(8,2);
      display.print(bufCode);

      KB.read();

      if (IR.check())
      {
        Serial.println(IR.data); // если сигнал пришел, выводим в порт
        bufCode = IR.data;
        dataFile.print(bufCode);
        dataFile.print("\n");
      }

      if (KB.justPressed())
      {
        if(KB.getNum == 0)
        {
          scan = false;
          start_menu = true;
          dataFile.close();
          break;
        }
        else continue;
      }
    }
  }
  else
  {
    dataFile.close();
    display.clear();
    display.setCursor(0,0);
    display.print("ERROR CREATE FILE");
    delay(3000);
    start_menu = true;
  }
}

void ScanMode()
{
  String bufCode = "NONE";
  display.clear();
  while (true)
  {
    display.setCursor(0,0);
    display.print("PRESSED B TO EXIT");
    display.setCursor(5,1);
    display.print("SCANNER");
    display.setCursor(9,2);
    display.print(bufCode);

    KB.read();

    if (IR.check())
    {
      Serial.println(IR.data); // если сигнал пришел, выводим в порт

      bufCode = IR.data;
    }

    if (KB.justPressed())
    {
      if(KB.getNum == 0)
      {
        scan = false;
        start_menu = true;
        break;
      }
    }
  }
}

void RemoteMode(String filename_with_code)
{
  int buf_id_code_array = 0;

  File file_with_code = SD.open(filename_with_code);

  for (int i = 0; i < 10; i++) {code_array[i] = "";}

  while (file_with_code.available()) {code_array[buf_id_code_array] = file_with_code.readStringUntil('\n');
  Serial.print(code_array[buf_id_code_array]);
  Serial.print("\n");
  buf_id_code_array++;}

  file_with_code.close();
  display.clear();
  while (true)
  {
    KB.read();

    display.setCursor(0,0);
    display.print("PRESSED B TO EXIT");
    display.setCursor(8,1);
    display.print(filename_with_code);

    if (KB.justPressed())
    {

      if(KB.getNum == 1)
      {
        long int code = (uint64_t)code_array[0].toInt();
        Serial.print(code_array[0].toInt());
        Serial.print("\n");
        IC.send(code_array[0].toInt());
      }

      else if(KB.getNum == 4)
      {
        long int code = (uint64_t)code_array[1].toInt();
        Serial.print(code_array[1].toInt());
        Serial.print("\n");
        IC.send(code_array[1].toInt());
      }

      else if(KB.getNum == 7)
      {
        long int code = (uint64_t)code_array[2].toInt();
        Serial.print(code_array[2].toInt());
        Serial.print("\n");
        IC.send(code_array[2].toInt());
      }

      else if(KB.getNum == 2)
      {
        long int code = (uint64_t)code_array[3].toInt();
        Serial.print(code);
        IC.send(code_array[3].toInt());
      }

      else if(KB.getNum == 5)
      {
        long int code = (uint64_t)code_array[4].toInt();
        Serial.print(code);
        IC.send(code_array[4].toInt());
      }

      else if(KB.getNum == 8)
      {
        long int code = (uint64_t)code_array[5].toInt();
        Serial.print(code);
        IC.send(code_array[5].toInt());
      }

      else if(KB.getNum == 3)
      {
        long int code = (uint64_t)code_array[6].toInt();
        Serial.print(code);
        IC.send(code_array[6].toInt());
      }

      else if(KB.getNum == 6)
      {
        long int code = (uint64_t)code_array[7].toInt();
        Serial.print(code);
        IC.send(code_array[7].toInt());
      }

      else if(KB.getNum == 9)
      {
        long int code = (uint64_t)code_array[8].toInt();
        Serial.print(code);
        IC.send(code_array[8].toInt());
      }

      else if(KB.getNum == 11)
      {
        long int code = (uint64_t)code_array[9].toInt();
        Serial.print(code_array[9].toInt());
        IC.send(code_array[9].toInt());
      }

      else if(KB.getNum == 0)
      {
        break;
      }
    }
  }
}

void LoadFiles(File dir, int numTabs)
{
  while (true) {

      File entry =  dir.openNextFile();
      if (!entry) break;

      else
      {
        if (!entry.isDirectory())
        {
          String nameFile = entry.name();
          arrayFileName[IDarrayFileName++] = nameFile;
          entry.close();
        }
        else
        {
          entry.close();
        }
      }

    }
}

void pop(String* buf_array, int index, int &size)
{
  for (int i = index; i < size - 1; i++)
  {
    buf_array[i] = buf_array[i + 1];
  }
  size--;
}

void ShowFileName()
{
  int bufId = 0;
  int index = 0;
  String buf_array[100];

  for (int i = 0; i < IDarrayFileName; i++)
  {
    buf_array[index++] = arrayFileName[i];
  }
  display.clear();
  while (true)
  {
    String bufFileName = buf_array[bufId];

    display.setCursor(0,0);
    display.print("PRESSED B TO EXIT");
    display.setCursor(5,1);
    display.print("File Name");
    display.setCursor(8,2);
    display.print(bufFileName);

    KB.read();

    if (KB.justPressed())
    {
      if(KB.getNum == 10)
      {
        if (bufId == 0) bufId = 0;
        else bufId--;
      }
      else if(KB.getNum == 12)
      {
        if (bufId == index - 1) bufId = index - 1;
        else if (bufId == 0 && index == 0) bufId = 0;
        else bufId++;
      }
      else if(KB.getNum == 14) // выбрать файл
      {
        RemoteMode(buf_array[bufId]);
      }
      else if(KB.getNum == 0) // назад в меню
      {
        start_menu = true;
        break;
      }
      else if(KB.getNum == 1) // A-10, B-11, C-12, D-13, #-15, *-14
      {
        // А Б В Г
        bufId = 0;
        for (int i = 0; i < index;)
        {
          Serial.print(buf_array[i]);
          if (buf_array[i].indexOf("A") == -1 && buf_array[i].indexOf("B") == -1 && buf_array[i].indexOf("C") == -1)
          {
            Serial.print(buf_array[i]);
            Serial.print("\n");
            pop(buf_array, i, index);
          }
          else i++;
        }

        if (index == 0) buf_array[0] = buf_array[IDarrayFileName];
        //Serial.print(buf_array[0]);
        Serial.print('\n');
        Serial.print(index);
        Serial.print('\n');
      }
      else if(KB.getNum == 4) // A-10, B-11, C-12, D-13, #-15, *-14
      {
        // Д Е Е Ж З
        bufId = 0;
        for (int i = 0; i < index;)
        {
          Serial.print(buf_array[i]);
          if (buf_array[i].indexOf("D") == -1 && buf_array[i].indexOf("E") == -1 && buf_array[i].indexOf("F") == -1)
          {
            Serial.print(buf_array[i]);
            Serial.print("\n");
            pop(buf_array, i, index);
          }
          else i++;
        }

        if (index == 0) buf_array[0] = buf_array[IDarrayFileName];
        //Serial.print(buf_array[0]);
        Serial.print('\n');
        Serial.print(index);
        Serial.print('\n');
        //break;

      }
      else if(KB.getNum == 7) // A-10, B-11, C-12, D-13, #-15, *-14
      {
        // И Й К Л
        bufId = 0;
        for (int i = 0; i < index;)
        {
          Serial.print(buf_array[i]);
          if (buf_array[i].indexOf("G") == -1 && buf_array[i].indexOf("H") == -1 && buf_array[i].indexOf("I") == -1)
          {
            Serial.print(buf_array[i]);
            Serial.print("\n");
            pop(buf_array, i, index);
          }
          else i++;
        }

        if (index == 0) buf_array[0] = buf_array[IDarrayFileName];
        Serial.print('\n');
        Serial.print(index);
        Serial.print('\n');
      }
      else if(KB.getNum == 2) // A-10, B-11, C-12, D-13, #-15, *-14
      {
        // М Н О П
        bufId = 0;
        for (int i = 0; i < index;)
        {
          Serial.print(buf_array[i]);
          if (buf_array[i].indexOf("J") == -1 && buf_array[i].indexOf("K") == -1 && buf_array[i].indexOf("L") == -1)
          {
            Serial.print(buf_array[i]);
            Serial.print("\n");
            pop(buf_array, i, index);
          }
          else i++;
        }

        if (index == 0) buf_array[0] = buf_array[IDarrayFileName];
        Serial.print('\n');
        Serial.print(index);
        Serial.print('\n');
      }
      else if(KB.getNum == 5) // A-10, B-11, C-12, D-13, #-15, *-14
      {
        // Р С Т У
        bufId = 0;
        for (int i = 0; i < index;)
        {
          Serial.print(buf_array[i]);
          if (buf_array[i].indexOf("M") == -1 && buf_array[i].indexOf("N") == -1 && buf_array[i].indexOf("O") == -1)
          {
            Serial.print(buf_array[i]);
            Serial.print("\n");
            pop(buf_array, i, index);
          }
          else i++;
        }

        if (index == 0) buf_array[0] = buf_array[IDarrayFileName];
        Serial.print('\n');
        Serial.print(index);
        Serial.print('\n');
      }
      else if(KB.getNum == 8) // A-10, B-11, C-12, D-13, #-15, *-14
      {
        // Ф Х Ц Ч
        bufId = 0;
        for (int i = 0; i < index;)
        {
          Serial.print(buf_array[i]);
          if (buf_array[i].indexOf("P") == -1 && buf_array[i].indexOf("Q") == -1 && buf_array[i].indexOf("R") == -1 && buf_array[i].indexOf("S") == -1)
          {
            Serial.print(buf_array[i]);
            Serial.print("\n");
            pop(buf_array, i, index);
          }
          else i++;
        }

        if (index == 0) buf_array[0] = buf_array[IDarrayFileName];
        Serial.print('\n');
        Serial.print(index);
        Serial.print('\n');
      }
      else if(KB.getNum == 3) // A-10, B-11, C-12, D-13, #-15, *-14
      {
        // Ш Щ Ъ Ы
        bufId = 0;
        for (int i = 0; i < index;)
        {
          Serial.print(buf_array[i]);
          if (buf_array[i].indexOf("T") == -1 && buf_array[i].indexOf("U") == -1 && buf_array[i].indexOf("V") == -1)
          {
            Serial.print(buf_array[i]);
            Serial.print("\n");
            pop(buf_array, i, index);
          }
          else i++;
        }

        if (index == 0) buf_array[0] = buf_array[IDarrayFileName];
        Serial.print('\n');
        Serial.print(index);
        Serial.print('\n');
      }
      else if(KB.getNum == 6) // A-10, B-11, C-12, D-13, #-15, *-14
      {
        // Ь Э Ю Я
        bufId = 0;
        for (int i = 0; i < index;)
        {
          Serial.print(buf_array[i]);
          if (buf_array[i].indexOf("W") == -1 && buf_array[i].indexOf("X") == -1 && buf_array[i].indexOf("Y") == -1 && buf_array[i].indexOf("Z") == -1)
          {
            Serial.print(buf_array[i]);
            Serial.print("\n");
            pop(buf_array, i, index);
          }
          else i++;
        }

        if (index == 0) buf_array[0] = buf_array[IDarrayFileName];
        Serial.print('\n');
        Serial.print(index);
        Serial.print('\n');
      }
      else if(KB.getNum == 9) // A-10, B-11, C-12, D-13, #-15, *-14
      {
        bufId = 0;
        for (int i = 0; i < index; i++)
        {
          if (buf_array[i].indexOf('Y') == -1 && buf_array[i].indexOf('Z') == -1)
          {
            pop(buf_array, i, index);
            index--;
          }
        }
      }
      else if(KB.getNum == 15) // A-10, B-11, C-12, D-13, #-15, *-14
      {
        bufId = 0;
        index = 0;
        for (int i = 0; i < IDarrayFileName; i++)
        {
          buf_array[index++] = arrayFileName[i];
        }
      }
      else continue;
    }
    else continue;
  }

}
