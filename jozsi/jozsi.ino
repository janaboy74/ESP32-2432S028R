// Jozsi is just an open source interface
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Timezone.h>
//#include "Adafruit_GFX.h"
#include "time.h"
#include "TFT_Touch.h"
#include <sys/time.h>
#include <map>
#include <vector>
#include <chrono>
//#include <lvgl.h>
#include "Roboto_Regular.h"

#define mkcolor( red, green, blue ) (((red & 0xf8)<<8) + ((green & 0xfc)<<3) + (blue>>3))

using namespace std;

TFT_eSPI tft = TFT_eSPI();

#define RGB_RED_PIN 4
#define RGB_BLUE_PIN 17
#define RGB_GREEN_PIN 16

#define TOUCH_CS 33
#define TOUCH_CLK 25
#define TOUCH_DIN 32
#define TOUCH_DOUT 39

#define TFT_GREY 0x5AEB
#define TFT_BLACK 0x0000
#define TFT_NAVY 0x000F
#define TFT_DARKGREEN 0x03E0
#define TFT_DARKCYAN 0x03EF
#define TFT_MAROON 0x7800
#define TFT_PURPLE 0x780F
#define TFT_OLIVE 0x7BE0
#define TFT_LIGHTGREY 0xC618
#define TFT_DARKGREY 0x7BEF
#define TFT_BLUE 0x001F
#define TFT_GREEN 0x07E0
#define TFT_CYAN 0x07FF
#define TFT_RED 0xF800
#define TFT_MAGENTA 0xF81F
#define TFT_YELLOW 0xFFE0
#define TFT_WHITE 0xFFFF
#define TFT_ORANGE 0xFDA0
#define TFT_GREENYELLOW 0xB7E0
#define TFT_PINK 0xFC9F

#define AA_FONT_SMALL "NotoSansBold15"

#define BACKGROUND_COLOR 0x08C4  // http://www.barth-dev.de/online/rgb565-color-picker/
const char *ssid = "[PRIMARY-WIFI-SSID]";                // SSID of local network
const char *password = "[PRIMARY-WIFI-PASSWORD]";  // Password on network
const char *ssid2 = "[SECONDARY-WIFI-SSID]";                // SSID of local network
const char *password2 = "SECONDARY-WIFI-PASSWORD]";  // Password on network

String APIKEY = "[OpenWeatherMap API key]";

using namespace std;

WiFiClient client;
String result;

struct corestring : public std::string {
  corestring()
    : std::string() {
  }

  corestring(const std::string &src)
    : std::string(src) {
  }

  corestring(const char *src)
    : std::string(src) {
  }

  void formatva(const char *format, va_list &arg_list) {
    if (format) {
      va_list cova;
      va_copy(cova, arg_list);
      int size = vsnprintf(NULL, 0, format, cova);
      va_end(arg_list);
      resize(size);
      va_copy(cova, arg_list);
      vsnprintf(&at(0), size + 1, format, cova);
      va_end(arg_list);
    }
  }

  void format(const char *format, ...) {
    if (format) {
      va_list arg_list;
      va_start(arg_list, format);
      formatva(format, arg_list);
      va_end(arg_list);
    }
  }
};

struct corewstring : public std::wstring {
  corewstring()
    : std::wstring() {
  }

  corewstring(const std::wstring &src)
    : std::wstring(src) {
  }

  corewstring(const wchar_t *src)
    : std::wstring(src) {
  }

  void formatva(const wchar_t *format, va_list &arg_list) {
    if (format) {
      va_list cova;
      va_copy(cova, arg_list);
      int size = vswprintf(NULL, 0, format, cova);
      va_end(arg_list);
      resize(size);
      va_copy(cova, arg_list);
      vswprintf(&at(0), size + 1, format, cova);
      va_end(arg_list);
    }
  }

  void format(const wchar_t *format, ...) {
    if (format) {
      va_list arg_list;
      va_start(arg_list, format);
      formatva(format, arg_list);
      va_end(arg_list);
    }
  }
};

class rng
{
    uint act;
    uint hash;
    void flip() {
        act = hash;
        act = 0xb9e92c37 + act * 0x372ce9b9;
        hash = act;
    }
public:
    rng() {
    }
    ~rng() {
    }
    void init( int initval = 0x372ce9b9 ) {
        act = initval;
        flip();
    }
    void initrnd() {
        init(clock());
    }
    uint getrnd() {
        flip();
        return hash >> 7;
    }
    uint getrnd( unsigned int min, unsigned int max ) {
      unsigned int range = max-min+1;
      return ( getrnd() % range ) + min;
    }
} rng;

int counter = 10;

// https://javl.github.io/image2cpp/
/*
const unsigned char jozsiLogo[] PROGMEM = {  // 45 x 13
  0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xf5, 0x7f, 0x78, 0xf7, 0x83, 0xdf, 0xba,
  0xfe, 0xa8, 0xef, 0xfb, 0xbf, 0x7d, 0xff, 0xe8, 0xdf, 0xfb, 0xff, 0xff, 0xff, 0xe8, 0xa0, 0x7b,
  0x0e, 0x30, 0x61, 0x68, 0xdf, 0xfa, 0xed, 0xde, 0xdf, 0x68, 0xef, 0xfa, 0xed, 0xdd, 0xe3, 0x68,
  0xf7, 0xba, 0xed, 0xdb, 0xfd, 0x68, 0xff, 0xc7, 0x16, 0x30, 0x43, 0x68, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xc8, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x18, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8
};
*/
const unsigned char temperatureIcon[] PROGMEM = {  // 21 x 22
  0x00, 0xf0, 0x00, 0x01, 0x98, 0x00, 0x01, 0x08, 0x00, 0x3d, 0x68, 0x80, 0x01, 0x69, 0xc0, 0x0d,
  0x6b, 0xe0, 0x01, 0x68, 0x00, 0x3d, 0x68, 0x00, 0x01, 0x68, 0x00, 0x0d, 0x6b, 0xe0, 0x01, 0x69,
  0xc0, 0x3d, 0x68, 0x80, 0x01, 0x68, 0x00, 0x03, 0x6c, 0x00, 0x06, 0x66, 0x00, 0x04, 0xf2, 0x00,
  0x05, 0xfa, 0x00, 0x05, 0xfa, 0x00, 0x04, 0xf2, 0x00, 0x06, 0x06, 0x00, 0x03, 0xfc, 0x00, 0x01,
  0xf8, 0x00
};

// 'WIFI', 20x17px
const unsigned char wifiIcon[] PROGMEM = {
  0x01, 0xf8, 0x00, 0x0e, 0x07, 0x00, 0x30, 0x00, 0xc0, 0x40, 0x00, 0x20, 0x81, 0xf8, 0x10, 0x06,
  0x06, 0x00, 0x18, 0x01, 0x80, 0x20, 0x00, 0x40, 0x00, 0x00, 0x00, 0x01, 0xf8, 0x00, 0x06, 0x06,
  0x00, 0x08, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x60, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00,
  0x00, 0x60, 0x00
};

struct Weather {
  // input
  String ipInfoServer = "ipapi.co";
  String ntpServer = "pool.ntp.org";
  String weatherServer = "api.openweathermap.org";
  String location = "Budapest";
  const long gmtOffsetSec = 1 * 60 * 60;
  const int daylightOffsetSec = 3600;
  // output
  String country;
  String weatherCondition;
  float temperature;
  float humidity;
  float pressure;
  int sunrise;
  int sunset;
  float speed;
  float tMin;
  float tMax;
  float visibility;
  float windAngle;
} jozsi;

long nextquery = 0;
long nexttouch = 0;
int cursorPosition = 0;
//vector<vector<uint32_t>> pixels;

struct unicode {
    uint32_t *text;
    unicode() : text( NULL ) {};
    ~unicode() {
        free( text );
        text = NULL;
    };
    size_t utf8_strlen(const char* text) {
        size_t i = 0;
        size_t num_chars = 0;

        while (text[i] != 0) {
            num_chars++;

            if ((text[i] & 0b10000000) == 0) {
                i += 1;
            } else if ((text[i] & 0b11100000) == 0b11000000) {
                i += 2;
            } else if ((text[i] & 0b11110000) == 0b11100000) {
                i += 3;
            } else {
                i += 4;
            }
        }

        return num_chars;
    }

    uint32_t* utf8_to_utf32(const char* input) {
        size_t num_chars = utf8_strlen(input);
        text = (uint32_t*) realloc( text, sizeof(uint32_t) * ( num_chars + 1 ));
        size_t i = 0;

        for( size_t n = 0; n < num_chars; n++) {
            if(( input[ i ] & 0b10000000) == 0) {
                text[n] = input[i] & 0b01111111;
                i += 1;
            } else if ((input[i] & 0b11100000) == 0b11000000) {
                text[n] = (input[i] & 0b00011111) << 6 | (input[i + 1] & 0b00111111);
                i += 2;
            } else if ((input[i] & 0b11110000) == 0b11100000) {
                text[n] = (input[i] & 0b00001111) << 12 | (input[i + 1] & 0b00111111) << 6 | (input[i + 2] & 0b00111111);
                i += 3;
            } else {
                text[n] = (input[i] & 0b00000111) << 18 | (input[i + 1] & 0b00111111) << 12 | (input[i + 2] & 0b00111111) << 6 | (input[i + 3] & 0b00111111);
                i += 4;
            }
        }

        text[ num_chars ] = 0;
        return text;
    }
    operator uint32_t *() {
        return text;
    }
    uint32_t * c_str() {
        return text;
    }
};

// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       // Central European Standard Time
Timezone CE(CEST, CET);
TimeChangeRule *tcr;

class fontrender {
    float minx, maxx, miny, maxy;
    float defscale;
    std::map< long, std::vector<std::vector<std::pair<float,float>>>> font;
    std::map< long, float > lettersizes;
    std::map< long, float > letterminxs;
    vector<unsigned short> image;
    vector<char> imgmap;
public:
    fontrender() {}
    void setFont( std::map< long, std::vector<std::vector<std::pair<float,float>>>> _font ) {
        font = _font;
    }
    void init() {
        bool firstletter = true;
        for( auto &letter : font ) {
            float lminx, lmaxx;
            float lminy, lmaxy;
            bool first = true;
            for( auto &poly : letter.second ) {
                for( auto &pnt : poly ) {
                    if( lminx > pnt.first || first )
                        lminx = pnt.first;
                    if( lmaxx < pnt.first || first )
                        lmaxx = pnt.first;
                    if( lminy > pnt.second || first )
                        lminy = pnt.second;
                    if( lmaxy < pnt.second || first )
                        lmaxy = pnt.second;
                    first = false;
                }
            }
            lettersizes.insert(std::pair<long, float>( letter.first, lmaxx - lminx ));
            letterminxs.insert(std::pair<long, float>( letter.first, lminx ));
            if( minx > lminx || firstletter )
                minx = lminx;
            if( maxx < lmaxx || firstletter )
                maxx = lmaxx;
            if( miny > lminy || firstletter )
                miny = lminy;
            if( maxy < lmaxy || firstletter )
                maxy = lmaxy;
            firstletter = false;
        }
        float sclx = maxx - minx;
        float scly = maxy - miny;
        defscale = 1 / ( scly > sclx ? scly : sclx );
    }
    void render( TFT_eSPI &tft, int x, int y, int minw, int minh, const char *_text, float fontsize, unsigned short color = 0xffff, unsigned short bgcolor = 0x0000, int halign = 0, int valign = 0 ) {
        float scale = fontsize * defscale;
        float delta = 2 + fontsize / 20;
        float spacesize = 1 + fontsize / 4;
        int textsize = 0;
        unicode text;
        text.utf8_to_utf32( _text );

        for( auto chr = text.c_str(); *chr; ++chr ) {
            if( *chr == ' ' || *chr == '\t' ) {
                textsize += spacesize;
                continue;
            }
            if( textsize )
                textsize += delta;
            textsize += lettersizes[( long ) *chr ] * scale;
        }

        int mapw = textsize + 1, maph = ( maxy - miny ) * defscale * fontsize + 1;
        if( minw < mapw )
            minw = mapw;
        if( minh < maph )
            minh = maph;

        int extx = minw - mapw;
        int exty = minh - maph;
        int shiftx = ( halign + 1 ) * mapw * 0.5;
        int shifty = ( valign + 1 ) * maph * 0.5;

        //-1 0
        //0 -> ( minw - mapw ) * 0.5
        //1 -> minw - mapw
        int left = 0.5 * ( minw - mapw ) * ( 1 + halign );
        int top = 0.5 * ( minh - maph ) * ( 1 + valign );
        int right = left + mapw;
        int bottom = top + maph;

        if( !strlen( _text )) {
            tft.fillRect( x , y, minw, minh, bgcolor );
            return;
        }

        tft.fillRect( x, y, left, minh, bgcolor );
        tft.fillRect( x + right, y, minw - right, minh, bgcolor );
        tft.fillRect( x, y, minw, top, bgcolor );
        tft.fillRect( x, y + bottom, minw, minh - bottom, bgcolor );
        /*
        tft.fillRect( x + right, y + top, 0.5 * ( halign + 1 ) * ( minw - mapw ), minh, bgcolor );
        tft.fillRect( x, y + top, minw, shifty - top, bgcolor );
        tft.fillRect( x, y + bottom, minw, 0.5 * ( valign + 1 ) * ( minh - maph ), bgcolor );
        */
        textsize = 0;
        for( auto chr = text.c_str(); *chr; ++chr ) {
            auto letter = font[( long ) *chr ];
            int xsize = lettersizes[( long ) *chr ] * scale + 1;
            if( *chr == ' ' || *chr == '\t' ) {
                tft.fillRect( x + left + textsize, y, spacesize + delta, maph, bgcolor );
                textsize += spacesize + delta;
                continue;
            }
            if( textsize ) {
                tft.fillRect( x + left + textsize + 1, y, delta -1, maph, bgcolor );
                textsize += delta;
            }
            imgmap.resize( xsize * maph );
            fill(imgmap.begin(), imgmap.end(), 0);
            for( auto &poly : letter ) {
                for( auto pnt = poly.begin(); pnt != poly.end(); ++pnt ) {
                    auto next = pnt;
                    if( ++next == poly.end() )
                        next = poly.begin();
                    int p1x = ( pnt->first - letterminxs[( long ) *chr ]) * scale;
                    int p1y = ( maxy - pnt->second ) * scale + 1;
                    int p2x = ( next->first - letterminxs[( long ) *chr ]) * scale;
                    int p2y = ( maxy - next->second ) * scale + 1;
                    float dy = p2y - p1y;
                    float dx = p2x - p1x;
                    if( int( dy ) == 0 )
                        continue;
                    float chminx = p1x;
                    float chminy = p1y, chmaxy = p2y;
                    float slope = dx / dy;
                    if( dy < 0 ) {
                        chminx = p2x;
                        swap( chminy, chmaxy );
                    }
                    float x = chminx;
                    for( float y = chminy; y < chmaxy; ++y ) {
                        imgmap[ x + xsize * y ] = !imgmap[ x + xsize * y ];
                        x += slope;
                    }
                }
            }
#if 1 // FILL
            for( int y = 0; y < maph; ++y ) {
                bool draw = false;
                for( int x = 0; x < xsize; ++x ) {
                    if( imgmap[ x + xsize * y ] ) {
                        draw = !draw;
                    }
                    if( draw ) {
                        imgmap[ x + xsize * y ] = 1;
                    }
                }
            }
#endif
            image.resize( xsize * maph );
            for( int y = 0; y < maph; ++y ) {
                for( int x = 0; x < xsize; ++x ) {
                    if( imgmap[ x + xsize * y ] ) {
                      image[ x + xsize * y ] = color;
                    } else {
                      image[ x + xsize * y ] = bgcolor;
                    }
                }
            }
            tft.pushImage( x + textsize + left, y + top, xsize, maph, &*image.begin() );
            textsize += lettersizes[( long ) *chr ] * scale;
        }
    }
} fontrender;

void setup() {
  Serial.begin(115200);
  Roboto_Regular::init();
  fontrender.setFont( Roboto_Regular::font );
  fontrender.init();
  tft.begin();
  tft.init();
  tft.setRotation(3);  //2
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);

  Serial.println("Connecting");
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  digitalWrite(RGB_RED_PIN, 1);
  digitalWrite(RGB_BLUE_PIN, 1);
  digitalWrite(RGB_GREEN_PIN, 0);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    if( cursorPosition == 10 )
      WiFi.begin(ssid2, password2);
    else if( cursorPosition > 20 )
      break;
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.setCursor(180, 10);
    tft.drawString("Connecting " + String(cursorPosition) + "...", 10, 10, 4);
    cursorPosition++;
  }

  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  digitalWrite(RGB_RED_PIN, 1);
  digitalWrite(RGB_BLUE_PIN, 0);
  digitalWrite(RGB_GREEN_PIN, 1);

  tft.setCursor(10, 10);

  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  tft.fillRect(0, 0, 320, 240, TFT_YELLOW);
  tft.drawRect(2, 2, 318, 238, TFT_DARKGREY);
  tft.fillRect(3, 3, 313, 233, BACKGROUND_COLOR);  // http://www.barth-dev.de/online/rgb565-color-picker/
#if 0
  tft.drawBitmap(4, 4, jozsiLogo, 45, 13, TFT_GREENYELLOW, BACKGROUND_COLOR);
  tft.drawRect(3, 3, 47, 15, TFT_DARKGREY);
#endif
  tft.setTextColor(TFT_YELLOW, BACKGROUND_COLOR);
  nextquery = millis() / 1000L;
  nexttouch = millis() / 1000L;
  struct tm tm;
  tm.tm_year = 2023 - 1900;
  tm.tm_mon = 1;
  tm.tm_mday = 28;
  tm.tm_hour = 21;
  tm.tm_min = 51;
  tm.tm_sec = 00;
  time_t t = mktime(&tm);
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);
#if 0
  for(int v = -20; v < 20; ++v) {
    vector<uint32_t> line;
    for(int h = -20; h < 20; ++h) {
      float dist = sqrt( v*v + h*h );
        uint32_t color = 0x08C4;
      if( dist < 21 ) {
        short r = 1;//0x08C4 // 00001 000110 00100 1 6 4
        short g = 6;
        short b = 4;
        float colInt = ( 19 - dist ) / 4.f;
        if( colInt < 0 )
          colInt = 0;
        else if( colInt > 1.f ) {
          r = 31;
          g = 63;
          b = 0;
        } else {
          r = 31 * colInt + ( 1 - colInt ) * r;//0x08C4 // 00001 000110 00100 1 6 4
          g = 63 * colInt + ( 1 - colInt ) * g;
          b = 0 * colInt + ( 1 - colInt ) * b;
        }
        int v2 = v + 0 * 8;
        int h2 = h + 1 * 8;
        float dist2 = sqrt( v2*v2 + h2*h2 );
        if( dist2 < 11 ) {
          colInt = ( 9 - dist2 ) / 3.f;
          if( colInt < 0 )
            colInt = 0;
          else if( colInt > 1.f ) {
            r = 0;
            g = 0;
            b = 31;
          } else {
            r = 0 * colInt + ( 1 - colInt ) * r;//0x08C4 // 00001 000110 00100 1 6 4
            g = 0 * colInt + ( 1 - colInt ) * g;
            b = 31 * colInt + ( 1 - colInt ) * b;
          }
        }
        color = ( r << 11 ) + ( g << 5 ) + b;
      }
      line.push_back( color );
    }
    pixels.push_back( line );
  }
#endif
}

TFT_Touch touch( TOUCH_CS, TOUCH_CLK, TOUCH_DIN, TOUCH_DOUT );
float angle;

const char *monthNames[] = { "Január", "Február", "Március", "Április", "Május", "Június", "Július", "Augusztus", "Szeptember", "Október", "November", "December" };
const char *weekdayNames[] = { "Hétfő", "Kedd", "Szerda", "Csütörtök", "Péntek", "Szombat", "Vasárnap" };
static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
static int mdays[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

int dayNumber( int year, int month, int day ) {
    if( month > 12 || month < 1 )
        return 0;
    year -= month < 3;
    return ( year + year / 4 - year / 100 + year / 400 + t[ month - 1 ] + day + 6 ) % 7;
}

int monthDays( int year, int month ) {
    if( month > 12 || month < 1 )
        return 0;
    if( 2 == month )
        return  (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0)) ? 29 : 28;
    return mdays[ month - 1 ];
}


const char * daynames[]={"Vasárnap","Hétfő","Kedd","Szerda","Csütörtök","Péntek","Szombat"};

void drawRect( int x, int y, int w, int h, short bgcolor, bool flip = false ) {
  short ct = TFT_WHITE;
  short cb = TFT_GREY;
  short cc = bgcolor;
  tft.drawLine( x        , y + h - 1 , x        , y         , ct);
  tft.drawLine( x        , y         , x + w - 1, y         , ct);
  tft.drawLine( x + w - 1, y         , x + w - 1, y + h - 1 , cb);
  tft.drawLine( x + w - 1, y + h - 1 , x        , y + h - 1 , cb);
  //tft.fillRect( x + 1, y + 1, w - 3, h - 3, bgcolor );
}

void loop() {

  long cursec = millis() / 1000L;

  if ( cursec >= nextquery && cursorPosition <= 20 ) {
    nextquery = millis() / 1000L + 30 * 60;
    getWeatherData();
    configTime(jozsi.gmtOffsetSec, jozsi.daylightOffsetSec, jozsi.ntpServer.c_str());
    rng.initrnd();
  }

  delay(10);

  if( 0 ) {
//  if( touch.Pressed() ) {
    int xmin = 260;
    int xmax = -20;
    int xrange = xmax - xmin;
    int ymin = -16;
    int ymax = 360;
    int yrange = ymax - ymin;
    tft.drawCircle( xmin + ( touch.ReadRawX() * xrange / 4096 ), ymin + ( touch.ReadRawY() * yrange / 4096 ), 10, TFT_WHITE);
  }
#if 0 // rotated logo
  int x = 120, y = 250;
  float dx = sin(angle);
  float dy = cos(angle);
  for(int v = -20; v < 20; ++v) {
    for(int h = -20; h < 20; ++h) {
      int px = h * dx - v * dy;
      int py = v * dx + h * dy;
      if(( -20 < px && px < 20 ) &&
         ( -20 < py && py < 20 )) {
          tft.drawPixel(x+h, y+v, pixels[px+20][ py +20 ]);
      }
    }
  }
#endif
//  tft.fillCircle( x + sin(angle) * 10, y + cos(angle) * 10, 10, TFT_YELLOW );
  angle += 30e-2f;
//  tft.fillCircle( x + sin(angle) * 10, y + cos(angle) * 10, 10, TFT_BLUE );

  tft.setTextColor(TFT_GREENYELLOW, BACKGROUND_COLOR);
  tft.drawString(jozsi.location, 7, 8, 4);
  tft.setCursor(8, 27);
  tft.setTextColor(TFT_GREEN, BACKGROUND_COLOR);
  tft.drawString( String(jozsi.weatherCondition), 8, 32, 1);

  long rssi = WiFi.RSSI();

  tft.setTextColor(TFT_BLUE, BACKGROUND_COLOR);
  tft.setCursor(258, 17);
  tft.print(String(rssi) + "dbm");
  std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  long cint = millis & 0x1ff;
  if( cint > 0xff )
    cint = 0x1ff - cint;
  tft.drawBitmap(293, 4, wifiIcon, 20, 17, mkcolor(0,0,cint));

  struct tm timeinfo;

  time_t utc = time( NULL );
  time_t local = CE.toLocal( utc, &tcr );
  
  gmtime_r(&local,&timeinfo);

  tft.setTextColor(0xF780, BACKGROUND_COLOR);
  corestring clock;
  clock.format("%02i:%02i:%02i", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  tft.drawString(clock.c_str(), 8, 46, 7);

  tft.setTextColor(TFT_YELLOW, BACKGROUND_COLOR);
  tft.drawString(String(jozsi.temperature), 135, 11, 2);
  tft.drawBitmap(175, 9, temperatureIcon, 21, 22, TFT_YELLOW);

  tft.setTextColor(TFT_RED, BACKGROUND_COLOR);
  tft.setCursor(199, 8);
  tft.print(jozsi.tMax);

  tft.setTextColor(TFT_BLUE, BACKGROUND_COLOR);
  tft.setCursor(199, 22);
  tft.print(jozsi.tMin);

  tft.setTextColor(0xF780, BACKGROUND_COLOR);
  corestring wind;
  wind.format("Wind:", String(jozsi.speed).c_str());
  tft.drawString(wind.c_str(), 250, 40, 2);
  wind.format("%s m/s", String(jozsi.speed).c_str());
  tft.drawString(wind.c_str(), 250, 56, 2);

  clock.format("%04i.%02i.%02i", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);

  fontrender.render( tft, 240, 78, 72, 17, clock.c_str(), 16.3, TFT_YELLOW, BACKGROUND_COLOR, 1, 0 );
  fontrender.render( tft, 240, 96, 72, 18, daynames[timeinfo.tm_wday], 17, TFT_YELLOW, BACKGROUND_COLOR, 1, 0 );

  time_t rawtime = CE.toLocal( utc, &tcr );
  static const long daysecs = 24 * 60 * 60;
  long selective = 1683064800;

  long delta = ( rawtime - selective ) / daysecs;
  delta = 27 - ( delta % 28 );
  rawtime += delta * daysecs;

  gmtime_r(&rawtime,&timeinfo);
  clock.format("Szelektív");
  fontrender.render( tft, 250, 120, 62, 18, clock.c_str(), 17, TFT_GREEN, BACKGROUND_COLOR, 1, 0 );
  clock.format("%04i.%02i.%02i", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday + 0);
  fontrender.render( tft, 250, 140, 62, 14, clock.c_str(), 14, TFT_GREEN, BACKGROUND_COLOR, 1, 0 );

  rawtime = CE.toLocal( utc, &tcr );
  gmtime_r( &rawtime, &timeinfo );

  drawRect( 4, 100, 105, 20, BACKGROUND_COLOR );
  fontrender.render( tft, 5, 101, 103, 18, monthNames[timeinfo.tm_mon], 17, mkcolor( 130, 60, 200 ), mkcolor( 3, 5, 50 ), 0, 0 );

  int lastDay = monthDays( timeinfo.tm_year + 1900, timeinfo.tm_mon + 1 );
  int firstDayOfWeek = dayNumber( timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, 1 );
  //drawRect( 2, 190, 236, 120, BACKGROUND_COLOR );
  int rwidth = 236 / 7;
  int rheight = 130 / 7;
  int xpos = 4;
  int ypos = 121;
  for( int day = 0; day < firstDayOfWeek; ++day ) {
    drawRect( xpos, ypos, rwidth, rheight, BACKGROUND_COLOR );
    fontrender.render( tft, xpos + 1, ypos + 1, rwidth - 2, rheight - 2, "", 14, mkcolor( 10, 10, 10 ), mkcolor( 10, 10, 10 ), 0, 0 );
    xpos += rwidth;
  }
  int lines = 1;
  for( int day = 1; day <= lastDay; ++day ) {
    short color;
    color = mkcolor( 80, 80, 80 );
    if( firstDayOfWeek > 4 )
      color = mkcolor( 120, 120, 40 );
    if( day == timeinfo.tm_mday )
      color = mkcolor( 0, 140, 0 );
    tft.setTextColor( color, BACKGROUND_COLOR );
    drawRect( xpos, ypos, rwidth, rheight, BACKGROUND_COLOR );
    clock.format( "%d", day );
    fontrender.render( tft, xpos + 1, ypos + 1, rwidth - 2, rheight - 2, clock.c_str(), 16.3, mkcolor( 20, 10, 40 ), color, 0, 0 );
    //tft.drawString(clock.c_str(), xpos + 7, ypos + 1, 2);
    xpos += rwidth;
    if( ++firstDayOfWeek > 6 ) {
        firstDayOfWeek = 0;
        ypos += rheight;
        xpos = 4;
        ++lines;
    }
  }
  for( ; firstDayOfWeek <=6; ++firstDayOfWeek ) {
    drawRect( xpos, ypos, rwidth, rheight, BACKGROUND_COLOR );
    fontrender.render( tft, xpos + 1, ypos + 1, rwidth - 2, rheight - 2, "", 14, mkcolor( 10, 10, 10 ), mkcolor( 10, 10, 10 ), 0, 0 );
    xpos += rwidth;
  }
  if( lines < 6 ) {
    ypos += rheight;
    tft.fillRect( 4, ypos, rwidth * 7, rheight, BACKGROUND_COLOR );
  }
}

void getWeatherData()  //client function to send/receive GET request data.
{
  // Connect to weather server
  if (client.connect(jozsi.weatherServer.c_str(), 80)) {
    client.println("GET /data/2.5/weather?q=Budapest&appid=[OpenWeatherMap API key]&units=metric");
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("connection failed");  // error message if no client connect
    Serial.println();
  }

  while (client.connected() && !client.available())
    delay(1);

  while (client.connected() || client.available()) {
    char c = client.read();
    result = result + c;
  }

  client.stop();  //stop client
  result.replace('[', ' ');
  result.replace(']', ' ');
  Serial.println(result);
  char jsonArray[result.length() + 1];
  result.toCharArray(jsonArray, sizeof(jsonArray));
  jsonArray[result.length() + 1] = '\0';
  DynamicJsonDocument json_obj(1024);
  auto error = deserializeJson(json_obj, jsonArray);

  if (error) {
    Serial.println("parseObject() failed");
  }

  jozsi.weatherCondition = (const char *)json_obj["weather"]["description"];
  // jozsi.location = (const char*)json_obj["name"]; // it's an input now
  jozsi.country = (const char *)json_obj["sys"]["country"];
  jozsi.temperature = json_obj["main"]["temp"];
  jozsi.humidity = json_obj["main"]["humidity"];
  jozsi.pressure = json_obj["main"]["pressure"];
  jozsi.sunrise = json_obj["sys"]["sunrise"];
  jozsi.sunset = json_obj["sys"]["sunset"];
  jozsi.speed = json_obj["wind"]["speed"];
  jozsi.tMin = json_obj["main"]["temp_min"];
  jozsi.tMax = json_obj["main"]["temp_max"];
  jozsi.visibility = json_obj["visibility"];
  jozsi.windAngle = json_obj["wind"]["deg"];
}
