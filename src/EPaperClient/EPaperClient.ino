/* Includes ------------------------------------------------------------------*/
#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include <stdlib.h>
#include <math.h>
#include "time.h"
#include <WiFi.h>

const char* ssid = "Your WiFi Is In Another Castle";
const char* password = "65138264692885557410";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

const int breite = EPD_7IN5_V2_WIDTH;
const int hoehe = EPD_7IN5_V2_HEIGHT;

const int DateStartX = 20;
const int DateStartY = 90;

const int TimeStartX = 20;
const int TimeStartY = 40;

struct tm timeinfo;

/* Server and IP address ------------------------------------------------------*/
IPAddress myIP;        // IP address in your local wifi net

/* Entry point ----------------------------------------------------------------*/
void setup()
{
    DEV_Module_Init();
    ConnectToWiFi();
    GetDateTime();

    Serial.println("e-Paper Init and Clear...\r\n");
    EPD_7IN5_V2_Init();
    EPD_7IN5_V2_Clear();
    DEV_Delay_ms(500);

    //Create a new image cache and select it for paint on
    UBYTE* FullImage = CreateFullImage();
    Paint_SelectImage(FullImage);
    Paint_Clear(WHITE);

    WriteLines(breite, hoehe);
    WriteUhrzeit(TimeStartX, TimeStartY);
    WritDate(DateStartX, DateStartY);

    WriteRoom("Wohnzimmer", 23.3, 0, (hoehe / 3));
    WriteRoom("Simon", 22.6, (breite / 3), (hoehe / 3));
    WriteRoom("Badezimmer", 18.0, 2 * (breite / 3), (hoehe / 3));
    WriteRoom("Schlafzimmer", 20.2, 0, 2 * (hoehe / 3));
    WriteRoom("Balkon", -5.7, (breite / 3), 2 * (hoehe / 3));

    //move image in display memory to show image 
    ClearScreen();
    EPD_7IN5_V2_Display(FullImage);
    free(FullImage);
    FullImage = NULL;
    DEV_Delay_ms(20000);
    Serial.println("Goto Sleep...\r\n");
    EPD_7IN5_V2_Sleep();
}


void InitDisplay()
{
    Serial.println("EPD_7IN5_V2_test Demo\r\n");
    DEV_Module_Init();

    Serial.println("e-Paper Init and Clear...\r\n");
    EPD_7IN5_V2_Init();
    EPD_7IN5_V2_Clear();
    DEV_Delay_ms(500);
}

void ConnectToWiFi()
{
    delay(10);

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    // Applying SSID and password
    WiFi.begin(ssid, password);

    // Waiting the connection to a router
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    // Connection is complete
    Serial.println("");
    Serial.println("WiFi connected");

    // Show obtained IP address in local Wifi net
    Serial.println(myIP = WiFi.localIP());
}


void GetDateTime()
{
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time -2 ");
        return;
    }
    else {
        Serial.printf("got it 2");
    }
}

void WriteTimeToSerial()
{
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();
}

void ClearScreen()
{
    EPD_7IN5_V2_Clear();
}

UBYTE* CreateFullImage()
{
    //Create a new image cache
    UBYTE* BlackImage;
    /* you have to edit the startup_stm32fxxx.s file and set a big enough heap size */
    UWORD Imagesize = ((EPD_7IN5_V2_WIDTH % 8 == 0) ? (EPD_7IN5_V2_WIDTH / 8) : (EPD_7IN5_V2_WIDTH / 8 + 1)) * EPD_7IN5_V2_HEIGHT;
    if ((BlackImage = (UBYTE*)malloc(Imagesize)) == NULL) {
        Serial.println("Failed to apply for black memory...\r\n");
        while (1);
    }

    //Serial.println("Paint_NewImage\r\n");
    Paint_NewImage(BlackImage, EPD_7IN5_V2_WIDTH, EPD_7IN5_V2_HEIGHT, 0, WHITE);

    return BlackImage;
}

void WriteLines(UWORD width, UWORD height)
{
    Paint_DrawLine(2 * (width / 3), height, 2 * (width / 3), height / 3, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(width / 3, height, width / 3, height / 3, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(0, height / 3, width, height / 3, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(0, 2 * (height / 3), width, 2 * (height / 3), BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
}

void WriteUhrzeit(UWORD Xstart, UWORD Ystart)
{
    // Erstelle einen formatierten Text im gewünschten Format
    char formattedTime[50]; // Annahme: Der formatierte Text passt in 50 Zeichen

    // Verwende strftime, um den formatierten Text zu erstellen
    strftime(formattedTime, sizeof(formattedTime), "%02H", &timeinfo);
    Paint_DrawString_EN(Xstart, Ystart, formattedTime, &Font48, WHITE, BLACK);

    Paint_DrawString_EN(Xstart + 60, Ystart, ":", &Font48, WHITE, BLACK);

    strftime(formattedTime, sizeof(formattedTime), "%02M", &timeinfo);
    Paint_DrawString_EN(Xstart + 90, Ystart, formattedTime, &Font48, WHITE, BLACK);
}

void WritDate(UWORD Xstart, UWORD Ystart)
{
    // Erstelle einen formatierten Text im gewünschten Format
    char formattedTime[50]; // Annahme: Der formatierte Text passt in 50 Zeichen
    strftime(formattedTime, sizeof(formattedTime), "%a,", &timeinfo);
    Paint_DrawString_EN(Xstart, Ystart, formattedTime, &Font24, WHITE, BLACK);
    strftime(formattedTime, sizeof(formattedTime), "%d.%m.%Y", &timeinfo);
    Paint_DrawString_EN(Xstart + 70, Ystart, formattedTime, &Font24, WHITE, BLACK);
}

void WriteRoom(const char* name, float temperatur, UWORD Xstart, UWORD Ystart)
{
    int tempStartX = Xstart + 15;
    int tempStartY = Ystart + 70;
    WriteTemperature(temperatur, tempStartX, tempStartY);

    Paint_DrawString_EN(Xstart + 20, Ystart + 15, name, &Font24, WHITE, BLACK);
}

void WriteTemperature(float temperatur, UWORD Xstart, UWORD Ystart)
{
    const int DECIMAL_PLACES = 1;
    const int DISTANCE_GRAD_CELCIUS = 170;

    int length = 1 + DECIMAL_PLACES + (temperatur < 0 ? 1 : 0) + (std::abs(temperatur) >= 1 ? int(std::log10(std::abs(temperatur))) : 0);

    // Erstelle den Puffer mit der richtigen Größe
    char* buffer = new char[length + 1]; // +1 für den Nullterminator
    dtostrf(temperatur, 0, DECIMAL_PLACES, buffer);

    int widthCounter = 0;
    for (int i = 0; i <= length; ++i) {
        const char singleChar[] = { buffer[i], '\0' };
        Paint_DrawString_EN(Xstart + widthCounter, Ystart, singleChar, &FontPoiretOne, WHITE, BLACK);

        const char* singleCharPtr = singleChar;
        switch (*singleCharPtr)
        {
        case '0':
            widthCounter += 60;
            break;
        case '1':
            widthCounter += 25;
            break;
        case '.':
            widthCounter += 10;
            break;
        case '-':
            widthCounter += 30;
            break;
        default:
            widthCounter += 45;
            break;
        }
    }

    Paint_DrawString_EN(Xstart + DISTANCE_GRAD_CELCIUS, Ystart - 5, "o", &Font24, WHITE, BLACK);
    Paint_DrawString_EN(Xstart + DISTANCE_GRAD_CELCIUS + 13, Ystart, "C", &FontPoiretOne, WHITE, BLACK);
}

void printLocalTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }

    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    Serial.print("Day of week: ");
    Serial.println(&timeinfo, "%A");
    Serial.print("Month: ");
    Serial.println(&timeinfo, "%B");
    Serial.print("Day of Month: ");
    Serial.println(&timeinfo, "%d");
    Serial.print("Year: ");
    Serial.println(&timeinfo, "%Y");
    Serial.print("Hour: ");
    Serial.println(&timeinfo, "%H");
    Serial.print("Hour (12 hour format): ");
    Serial.println(&timeinfo, "%I");
    Serial.print("Minute: ");
    Serial.println(&timeinfo, "%M");
    Serial.print("Second: ");
    Serial.println(&timeinfo, "%S");

    Serial.println("Time variables");
    char timeHour[3];
    strftime(timeHour, 3, "%H", &timeinfo);
    Serial.println(timeHour);
    char timeWeekDay[10];
    strftime(timeWeekDay, 10, "%A", &timeinfo);
    Serial.println(timeWeekDay);
    Serial.println();
}


/* The main loop -------------------------------------------------------------*/
void loop()
{

}
