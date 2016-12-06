#include <EEPROM.h>
#include <SoftwareSerial.h>

SoftwareSerial gsm(10, 11);

uint8_t ch = 0;
String val = "";

bool oneStart = true;                                             //  статус первого запуска устройства
String phone = "";                                                //  тут будет номер мастер тел в виде строки
int phonePref, phoneCode, phoneOne, phoneTwo;
String ch1, ch2, ch3, ch4;

int EEPROM_int_read(int addr) 
{    
    byte raw[2];
    for(byte i = 0; i < 2; i++) raw[i] = EEPROM.read(addr+i);
    int &num = (int&)raw;
    return num;
}

void EEPROM_int_write(int addr, int num)
{
    byte raw[2];
    (int&)raw = num;
    for(byte i = 0; i < 2; i++) EEPROM.write(addr+i, raw[i]);
}

void sms(String text, String phone)
{
    gsm.println("AT+CMGS=\"+" + phone + "\""); delay(500);
    gsm.print(text); delay(500);
    gsm.print((char)26); delay(2000);
}

void clearsms()
{
    gsm.println("AT+CMGD=1,4");
}

void setup() 
{   
    // Инициализация UART
    Serial.begin(9600);
 
    // Инициализация GSM
    gsm.begin(9600); delay(2000);

    //  Отправляем команды АТ для инициализации режимов
    gsm.println("AT+CLIP=1"); delay(100);
    gsm.println("AT+CMGF=1"); delay(100);
    gsm.println("AT+CSCS=\"GSM\""); delay(100);
    gsm.println("AT+CMGD=1,4"); delay(100);
    gsm.println("AT+CNMI=2,2"); delay(100);
    gsm.println("ATE0"); delay(100);
  
    //  Если не первый запуск
    if(EEPROM.read(0) == 55)
    {
        //  Вытаскиваем номер из епром и кидаем в переменную номер тел...  и преобразовываем телефон в строку
        phonePref = EEPROM_int_read(1);
        phoneCode = EEPROM_int_read(3);
        phoneOne = EEPROM_int_read(5);
        phoneTwo = EEPROM_int_read(7);

        if(String(phonePref).length() == 1) ch1 = String(phonePref); 
        
        if(String(phoneCode).length() == 1) { ch2 = "00" + String(phoneCode); } else if(String(phoneCode).length() == 2) { ch2 = "0" + String(phoneCode); } else { ch2 = String(phoneCode); } 
        
        if(String(phoneOne).length() == 1) { ch3 = "00" + String(phoneOne); } else if(String(phoneOne).length() == 2) { ch3 = "0" + String(phoneOne); } else { ch3 = String(phoneOne); } 
        
        if(String(phoneTwo).length() == 1) { ch4 = "000" + String(phoneTwo); } else if(String(phoneTwo).length() == 2) { ch4 = "00" + String(phoneTwo); } else if(String(phoneTwo).length() == 3) { ch4 = "0" + String(phoneTwo); } else { ch4 = String(phoneTwo); } 
        
        phone = ch1 + ch2 + ch3 + ch4;

        //  Отправляем СМС что всё в норме на номер мастер тел
        sms("OK", phone);  // Отправляем СМС на номер +79517956505
        
        //  Меняем статус программы на стандартный режим работы
        oneStart = false;
    }    
}

void loop() 
{   
    // Цицл будет работать только пока первый запуск и отсутствует номер мастер телефона
    while(oneStart)
    {
        // Если GSM модуль что-то послал
        if (gsm.available()) 
        {  
            delay(200);  // Ожидаем заполнения буфера
            
            // Cохраняем входную строку в переменную val
            while (gsm.available())
            {  
                ch = gsm.read();
                val += char(ch);
                delay(10);
            }
            
            // Eсли звонок обнаружен, то проверяем номер
            if (val.indexOf("RING") > -1)
            {                
                gsm.println("ATH0"); // Разрываем связь
                
                // Serial.println(val);

                //  Переводим в число номер телефона в виде 4-х частей
                phonePref = (val.substring(19, 20)).toInt();
                phoneCode = (val.substring(20, 23)).toInt();
                phoneOne =  (val.substring(23, 26)).toInt();
                phoneTwo =  (val.substring(26, 30)).toInt();
                                
                //  Записываем в ЕЕПРОМ
                EEPROM.write(0, 55);
       
                EEPROM_int_write(1, phonePref);
                EEPROM_int_write(3, phoneCode);
                EEPROM_int_write(5, phoneOne);
                EEPROM_int_write(7, phoneTwo);

                if(String(phonePref).length() == 1) ch1 = String(phonePref); 
        
                if(String(phoneCode).length() == 1) { ch2 = "00" + String(phoneCode); } else if(String(phoneCode).length() == 2) { ch2 = "0" + String(phoneCode); } else { ch2 = String(phoneCode); } 
                
                if(String(phoneOne).length() == 1) { ch3 = "00" + String(phoneOne); } else if(String(phoneOne).length() == 2) { ch3 = "0" + String(phoneOne); } else { ch3 = String(phoneOne); } 
                
                if(String(phoneTwo).length() == 1) { ch4 = "000" + String(phoneTwo); } else if(String(phoneTwo).length() == 2) { ch4 = "00" + String(phoneTwo); } else if(String(phoneTwo).length() == 3) { ch4 = "0" + String(phoneTwo); } else { ch4 = String(phoneTwo); } 
                
                phone = ch1 + ch2 + ch3 + ch4;

                // Serial.print("SAVE MASTER +");
                // Serial.println(phone);
                               
                //  Отправляем информацию о том что номер сохранен
                //  sms("Number " + phone + " save", phone);  // Отправляем СМС на номер

                //  номер записан - можно сменить статус программы на стандартный режим работы
                oneStart = false;
                
                break;
            } 
    
            val = "";  // Очищаем переменную команды
            gsm.flush();
        }    
    }

    // Цицл стандартный режим работы
    while(!oneStart)
    {
        //  здесь код программы - обработка датчиков и прочее
        
        if (gsm.available())
        {
            delay(200);  // Ожидаем заполнения буфера

            while (gsm.available()) 
            {
                ch = gsm.read();
                val += char(ch);
                delay(10);
            }
            
            //----------------------- ЕСЛИ ВЫЗОВ -----------------------// 
            if (val.indexOf("RING") > -1)
            {      
                //  ЕСЛИ ВЫЗОВ ОТ ХОЗЯИНА      
                if (val.indexOf(phone) > -1)
                {                 
                    gsm.println("ATH0"); // Разрываем связь
                    Serial.println("CALL");
                }
            }

            //----------------------- ЕСЛИ СМС -----------------------//
            if (val.indexOf("+CMT") > -1) 
            {
                //  ЕСЛИ SMS ОТ ХОЗЯИНА
                if (val.indexOf(phone) > -1) 
                {                    
                    if (val.indexOf("SMS") > -1) 
                    {
                        Serial.println("SMS");
                    }
                }
            }

            val = "";  // Очищаем переменную команды
            gsm.flush();
        }
    }
}

