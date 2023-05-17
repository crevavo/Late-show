#include <M5EPD.h>
#include <SD.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#define TFCARD_CS_PIN 4

std::vector<String> fileList;
int imageCount = 0;
int currentImageIndex = 0;
M5EPD_Canvas canvas(&M5.EPD);

File root;
String filePath;

// Mode config
// 1: ALL FILE - List up files in SD, and display it by file name order
// 2: NUMBERED FILE - Display files in order of file name number e.g. "frame-00005.jpg"
int mode = 2;
int startIndex = 1;
int endIndex = 837;

int delayms = 60000;

void setup()
{
    // ----- Initialize -----
    M5.begin();
    M5.EPD.Clear(true);
    M5.EPD.SetRotation(0);
    SD.begin(TFCARD_CS_PIN, SPI, 40000000);

    // ----- Display Loading message -----
    canvas.createCanvas(960, 540);
    canvas.setTextSize(10);
    switch (mode)
    {
    case 1:
        canvas.drawString("LOADING...\nALL FILE MODE" , 20 , 20);
        break;
    case 2:
        canvas.drawString("Loading...\nNUMBERED FILE MODE" , 20 , 20);
        break;
    }    
    canvas.pushCanvas(0,0,UPDATE_MODE_GC16);

    // ----- Generate file list in SD /img folder -----
    File entry;
    switch (mode)
    {
    case 1:
        // ALL FILE MODE 
        root = SD.open("/");
        root.rewindDirectory();
        while ((entry = root.openNextFile()))
        {
            if (!entry.isDirectory())
            {
                String fileName = entry.name();
                if (fileName.endsWith(".jpg") || fileName.endsWith(".jpeg") || fileName.endsWith(".png") || fileName.endsWith(".bmp"))
                {
                    if (not fileName.startsWith("."))
                    {
                        fileList.push_back(fileName);
                    }                
                }
            }
            entry.close();
        }
        root.close();
        std::sort(fileList.begin(), fileList.end());
        imageCount = fileList.size();

        // ----- Debug -----
        for (int i=0; i < fileList.size(); i++){
            String p = fileList[i];
            Serial.println(p);
        }
        break;

    case 2:
        // NUMBERED FILE MODE
        imageCount = endIndex - startIndex;
        currentImageIndex = startIndex;
        break;

    }

}

void loop()
{
    // Pre process
    switch (mode)
    {
        case 1:
            // ALL FILE MODE
            if (currentImageIndex >= imageCount)
            {
                currentImageIndex = 0;
            }
            filePath = "/" + fileList[currentImageIndex];

            break;    

        case 2:
            // NUMBERED FILE MODE
            if (currentImageIndex > endIndex)
            {
                currentImageIndex = startIndex;
            }
            std::ostringstream sout;
            sout << "frame-" << std::setfill('0') << std::setw(5) << currentImageIndex << ".jpg";
            String fname(sout.str().c_str());
            filePath = "/" + fname;

            Serial.println("Path: " + String(filePath.c_str()));
            Serial.println("Count: " + String(currentImageIndex));
            break;
    }
    
    // -- Battery --
    int ivolt = M5.getBatteryVoltage();
    float volt = ivolt / 1000.0;
    float dmax = 4.35;
    float dmin = 3.2;
    float delt = volt - dmin;
	float percentage = (delt / (dmax - dmin)) * 100.0;
    char perStr[16];
    dtostrf(percentage, 2, 1 , perStr);

    Serial.println("Battery: " + String(volt) + " (V) / " + String(perStr) + " %");

    // ----- Display -----
    canvas.createCanvas(960, 540);
    canvas.setTextSize(2);
    canvas.setTextColor(0x100);
    canvas.drawJpgFile(SD, filePath.c_str());
    // canvas.drawString(String(perStr) + " % / " + String(volt) + " V" , 20 , 515);
    canvas.pushCanvas(0,0,UPDATE_MODE_GC16);

    // ----- Post process -----
    currentImageIndex++;
    delay(delayms);
}
