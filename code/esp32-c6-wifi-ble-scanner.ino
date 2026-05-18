#include <WiFi.h>
#include <Arduino_GFX_Library.h>
#include <NimBLEDevice.h>

#define LCD_DC   15
#define LCD_CS   14
#define LCD_SCK  7
#define LCD_MOSI 6
#define LCD_RST  21
#define LCD_BL   22

#define BTN_BOOT 9

Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);

Arduino_GFX *gfx = new Arduino_ST7789(
  bus,
  LCD_RST,
  3,
  true,
  172,
  320,
  34, 0, 34, 0
);

int menuIndex = 0;
int scanCount = 0;

bool inApp = false;
int activeApp = -1;

String menuItems[] = {
  "WiFi Scanner",
  "Systeminfo",
  "BLE Scanner",
  "RFID Scanner"
};

int menuCount = 4;

void drawTitle(String title) {
  gfx->fillScreen(RGB565_BLACK);

  gfx->setTextColor(RGB565_GREEN);
  gfx->setTextSize(2);
  gfx->setCursor(10, 10);
  gfx->println(title);

  gfx->drawLine(0, 35, 320, 35, RGB565_GREEN);
}

void drawMenu() {
  drawTitle("Cyberdeck");

  gfx->setTextSize(1);

  for (int i = 0; i < menuCount; i++) {
    int y = 55 + i * 30;

    if (i == menuIndex) {
      gfx->fillRect(5, y - 5, 300, 22, RGB565_GREEN);
      gfx->setTextColor(RGB565_BLACK);
    } else {
      gfx->setTextColor(RGB565_WHITE);
    }

    gfx->setCursor(15, y);
    gfx->print(i + 1);
    gfx->print(". ");
    gfx->println(menuItems[i]);
  }

  gfx->setTextColor(RGB565_CYAN);
  gfx->setCursor(10, 215);
  gfx->println("kurz=Start  lang=Weiter");
}

String signalText(int rssi) {
  if (rssi >= -50) return "sehr gut";
  if (rssi >= -65) return "gut";
  if (rssi >= -75) return "okay";
  return "schwach";
}

uint16_t signalColor(int rssi) {
  if (rssi >= -50) return RGB565_GREEN;
  if (rssi >= -65) return RGB565_YELLOW;
  if (rssi >= -75) return RGB565_CYAN;
  return RGB565_RED;
}

void wifiScanner() {
  scanCount++;

  drawTitle("WiFi Scanner");

  gfx->setTextColor(RGB565_YELLOW);
  gfx->setTextSize(1);
  gfx->setCursor(10, 50);
  gfx->println("Scanning...");

  int n = WiFi.scanNetworks();

  gfx->fillRect(0, 45, 320, 170, RGB565_BLACK);

  gfx->setTextColor(RGB565_WHITE);
  gfx->setCursor(10, 45);
  gfx->print("Netze: ");
  gfx->println(n);

  gfx->setCursor(215, 45);
  gfx->print("Scan ");
  gfx->println(scanCount);

  for (int i = 0; i < n && i < 8; i++) {
    int y = 65 + (i * 18);

    String ssid = WiFi.SSID(i);
    if (ssid.length() > 14) {
      ssid = ssid.substring(0, 14);
    }

    int rssi = WiFi.RSSI(i);

    gfx->setTextColor(RGB565_WHITE);
    gfx->setCursor(10, y);
    gfx->print(String(i + 1) + ". " + ssid);

    gfx->setTextColor(signalColor(rssi));
    gfx->setCursor(145, y);
    gfx->print(String(rssi) + "dBm");

    gfx->setCursor(220, y);
    gfx->print(signalText(rssi));
  }

  gfx->setTextColor(RGB565_CYAN);
  gfx->setCursor(10, 215);
  gfx->println("kurz=Refresh  lang=Menu");
}

void systemInfo() {
  drawTitle("Systeminfo");

  gfx->setTextColor(RGB565_WHITE);
  gfx->setTextSize(1);

  gfx->setCursor(10, 55);
  gfx->print("Chip: ESP32-C6");

  gfx->setCursor(10, 80);
  gfx->print("Uptime: ");
  gfx->print(millis() / 1000);
  gfx->println(" sek");

  gfx->setCursor(10, 105);
  gfx->print("Free Heap: ");
  gfx->print(ESP.getFreeHeap());
  gfx->println(" bytes");

  gfx->setCursor(10, 130);
  gfx->print("CPU MHz: ");
  gfx->println(ESP.getCpuFreqMHz());

  gfx->setTextColor(RGB565_CYAN);
  gfx->setCursor(10, 215);
  gfx->println("kurz=Refresh  lang=Menu");
}

void bleScanner() {
  drawTitle("BLE Scanner");

  gfx->setTextColor(RGB565_YELLOW);
  gfx->setTextSize(1);
  gfx->setCursor(10, 50);
  gfx->println("Scanning 10 Sekunden...");

  NimBLEDevice::init("");
  NimBLEScan* pScan = NimBLEDevice::getScan();
  pScan->setActiveScan(true);
  pScan->setInterval(45);
  pScan->setWindow(15);

  NimBLEScanResults results = pScan->getResults(10, false);

  gfx->fillRect(0, 45, 320, 170, RGB565_BLACK);

  gfx->setTextColor(RGB565_WHITE);
  gfx->setCursor(10, 45);
  gfx->print("BLE Geraete: ");
  gfx->println(results.getCount());

  for (int i = 0; i < results.getCount() && i < 8; i++) {
const NimBLEAdvertisedDevice* device = results.getDevice(i);

    String name = device->getName().c_str();
String addr = device->getAddress().toString().c_str();
    if (name.length() == 0) {
  name = addr.substring(0, 17);
} else if (name.length() > 16) {
  name = name.substring(0, 16);
}

    int rssi = device->getRSSI();
    int y = 65 + i * 18;

    gfx->setTextColor(RGB565_WHITE);
    gfx->setCursor(10, y);
    gfx->print(String(i + 1) + ". " + name);

    gfx->setTextColor(signalColor(rssi));
    gfx->setCursor(170, y);
    gfx->print(String(rssi) + "dBm");
  }

  pScan->clearResults();

  gfx->setTextColor(RGB565_CYAN);
  gfx->setCursor(10, 215);
  gfx->println("kurz=Refresh  lang=Menu");
}

void placeholder(String name) {
  drawTitle(name);

  gfx->setTextColor(RGB565_YELLOW);
  gfx->setTextSize(1);
  gfx->setCursor(10, 70);
  gfx->println("Noch nicht eingebaut.");

  gfx->setTextColor(RGB565_WHITE);
  gfx->setCursor(10, 100);
  gfx->println("Kommt spaeter mit Modul.");

  gfx->setTextColor(RGB565_CYAN);
  gfx->setCursor(10, 215);
  gfx->println("kurz=Refresh  lang=Menu");
}

void runApp(int app) {
  if (app == 0) wifiScanner();
  if (app == 1) systemInfo();
  if (app == 2) bleScanner();
  if (app == 3) placeholder("RFID Scanner");
}

void openSelectedApp() {
  activeApp = menuIndex;
  inApp = true;
  runApp(activeApp);
}

void handleButton() {
  static bool lastState = HIGH;
  bool state = digitalRead(BTN_BOOT);

  if (lastState == HIGH && state == LOW) {
    unsigned long pressStart = millis();

    while (digitalRead(BTN_BOOT) == LOW) {
      delay(10);
    }

    unsigned long pressTime = millis() - pressStart;

    if (!inApp) {
      if (pressTime > 700) {
        menuIndex++;
        if (menuIndex >= menuCount) menuIndex = 0;
        drawMenu();
      } else {
        openSelectedApp();
      }
    } else {
      if (pressTime > 700) {
        inApp = false;
        activeApp = -1;
        drawMenu();
      } else {
        runApp(activeApp);
      }
    }
  }

  lastState = state;
}

void setup() {
  Serial.begin(115200);

  pinMode(BTN_BOOT, INPUT_PULLUP);

  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);

  gfx->begin();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(200);

  drawMenu();
}

void loop() {
  handleButton();
}
