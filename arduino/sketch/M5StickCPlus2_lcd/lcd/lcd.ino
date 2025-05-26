#include <M5StickCPlus2.h>

class M5CanvasTextDisplayController {
public:
    M5CanvasTextDisplayController(M5GFX& display) : 
        _lcd(display), 
        _canvas1(nullptr), 
        _canvas2(nullptr),
        _activeCanvas(nullptr),
        _drawingCanvas(nullptr)
    {
        _canvas1 = new M5Canvas(&_lcd);
        _canvas2 = new M5Canvas(&_lcd);
    }

    ~M5CanvasTextDisplayController() {
        if (_canvas1) {
            _canvas1->deleteSprite();
            delete _canvas1;
            _canvas1 = nullptr;
        }
        if (_canvas2) {
            _canvas2->deleteSprite();
            delete _canvas2;
            _canvas2 = nullptr;
        }
    }

    bool begin(int initialTextSize = 1, bool initialLineWrap = true, 
               uint16_t textColor = WHITE, uint16_t bgColor = BLACK) {
        _textColor = textColor;
        _bgColor = bgColor;
        _lineWrap = initialLineWrap;

        if (!_canvas1 || !_canvas1->createSprite(_lcd.width(), _lcd.height())) {
            Serial.println("Failed to create canvas1");
            return false;
        }
        if (!_canvas2 || !_canvas2->createSprite(_lcd.width(), _lcd.height())) {
            Serial.println("Failed to create canvas2");
            if (_canvas1) _canvas1->deleteSprite();
            return false;
        }
        
        _drawingCanvas = _canvas1; 
        _activeCanvas = _canvas2;

        setTextSize(initialTextSize);
        
        _drawingCanvas->fillSprite(_bgColor);
        _activeCanvas->fillSprite(_bgColor); 

        _printCursorRow = 0;
        _printCursorCol = 0;
        
        return true;
    }

    void setTextSize(int size) {
        if (size < 1) size = 1;
        _textSize = size;

        if (_drawingCanvas) {
            _drawingCanvas->setTextSize(_textSize);
            _drawingCanvas->setTextFont(0); 
        }
        if (_activeCanvas) {
            _activeCanvas->setTextSize(_textSize);
            _activeCanvas->setTextFont(0);
        }

        if(_drawingCanvas) {
            _fontHeight = _drawingCanvas->fontHeight();
            _fontWidth = _drawingCanvas->fontWidth();
        } else if (_activeCanvas) { 
            _fontHeight = _activeCanvas->fontHeight();
            _fontWidth = _activeCanvas->fontWidth();
        } else { 
             _fontHeight = 8 * _textSize; 
             _fontWidth = 8 * _textSize;
        }

        if (_fontHeight == 0 || _fontWidth == 0) {
            _fontHeight = 8 * _textSize;
            _fontWidth = 6 * _textSize; 
        }
        
        _rows = _lcd.height() / _fontHeight;
        _cols = _lcd.width() / _fontWidth;

        if (_drawingCanvas) {
            _drawingCanvas->fillSprite(_bgColor);
            _drawingCanvas->setCursor(0,0); 
        }
        _printCursorRow = 0; 
        _printCursorCol = 0;
    }

    void setText(int row, int col, const String& text) {
        if (row < 0 || row >= _rows || col < 0 || col >= _cols || !_drawingCanvas) return;

        int x = col * _fontWidth;
        int y = row * _fontHeight;

        _drawingCanvas->setTextDatum(TL_DATUM); 
        
        String sub = text;
        int maxLen = _cols - col;
        if (text.length() > (unsigned int)maxLen) {
            sub = text.substring(0, maxLen);
        }
        // setTextでは指定領域を背景色でクリアしてから文字を描画
        _drawingCanvas->setTextColor(_textColor, _bgColor); 
        _drawingCanvas->fillRect(x, y, sub.length() * _fontWidth, _fontHeight, _bgColor);
        _drawingCanvas->drawString(sub, x, y);
    }
    
    void setText(int row, int col, const char* text) {
        setText(row, col, String(text));
    }

    void setCursor(int row, int col) {
        if (row >= 0 && row < _rows) _printCursorRow = row;
        if (col >= 0 && col < _cols) _printCursorCol = col;
    }

    size_t print(const String& text) {
        if (!_drawingCanvas) return 0;
        _drawingCanvas->setTextColor(_textColor); // 背景は透過 (既存の描画に重ねる)
        _drawingCanvas->setTextDatum(TL_DATUM);

        size_t n = 0;
        for (unsigned int i = 0; i < text.length(); ++i) {
            char c = text.charAt(i);
            if (_printChar(c)) {
                n++;
            } else {
                break; 
            }
        }
        return n;
    }
    size_t print(const char* text) { return print(String(text)); }
    size_t print(char c) { if(!_drawingCanvas) return 0; return _printChar(c) ? 1 : 0; }
    size_t print(int val, int base = DEC) { return print(String(val, base)); }
    size_t print(double val, int decimalPlaces = 2) { return print(String(val, decimalPlaces)); }

    size_t println(const String& text) {
        size_t n = print(text);
        if (_printChar('\n')) {
             n++;
        }
        return n;
    }
    size_t println(const char* text) { return println(String(text)); }
    size_t println(char c) { return println(String(c));}
    size_t println(int val, int base = DEC) { return println(String(val, base)); }
    size_t println(double val, int decimalPlaces = 2) { return println(String(val, decimalPlaces)); }
    size_t println() { return _printChar('\n') ? 1 : 0; }


    void show() {
        if (!_drawingCanvas || !_activeCanvas) return;

        // 1. _drawingCanvas (現在の描画内容) をLCDにプッシュ
        _drawingCanvas->pushSprite(0, 0);

        // 2. キャンバスをスワップ
        M5Canvas* temp = _activeCanvas;
        _activeCanvas = _drawingCanvas;  // _activeCanvas は今画面に表示された内容
        _drawingCanvas = temp;           // _drawingCanvas は以前画面に表示されていた内容 (次の描画ベース)

        // 3. _activeCanvas (今表示した内容) を _drawingCanvas (次の描画用) にコピーする
        //    M5GFX (LovyanGFX) の LGFX_Sprite::pushSprite(LGFX_Sprite* dstSprite, ...) を使用
        _activeCanvas->pushSprite(_drawingCanvas, 0, 0); 
        // 第4引数の透明色を指定しない場合、全ピクセルコピーとなる。
        
        // 4. 新しい _drawingCanvas のピクセルカーソルをリセット
        if (_drawingCanvas) {
            _drawingCanvas->setCursor(0,0); 
        }
        // print用文字カーソルは維持
    }

    void setLineWrap(bool wrap) {
        _lineWrap = wrap;
    }

    void clearDrawingCanvas() {
        if (!_drawingCanvas) return;
        _drawingCanvas->fillSprite(_bgColor);
        _printCursorRow = 0;
        _printCursorCol = 0;
        _drawingCanvas->setCursor(0,0);
    }

    void fillScreen(uint16_t color) {
        _bgColor = color; 
        _lcd.fillScreen(_bgColor); 

        if (_drawingCanvas) _drawingCanvas->fillSprite(_bgColor);
        if (_activeCanvas) _activeCanvas->fillSprite(_bgColor);  
        
        _printCursorRow = 0;
        _printCursorCol = 0;
        if (_drawingCanvas) _drawingCanvas->setCursor(0,0);
    }
    void fillScreen() {
        fillScreen(_bgColor);
    }
    
    void setTextColor(uint16_t color) {
        _textColor = color;
    }

    void setBgColor(uint16_t color) {
        _bgColor = color;
    }
    
    int getRows() const { return _rows; }
    int getCols() const { return _cols; }
    int getTextSize() const { return _textSize; }


private:
    M5GFX& _lcd;
    M5Canvas* _canvas1;
    M5Canvas* _canvas2;
    M5Canvas* _activeCanvas; 
    M5Canvas* _drawingCanvas;

    int _rows;
    int _cols;
    int _textSize;
    int _fontHeight;
    int _fontWidth;

    bool _lineWrap;
    uint16_t _textColor;
    uint16_t _bgColor;

    int _printCursorRow;
    int _printCursorCol;

    bool _printChar(char c) {
        if (!_drawingCanvas || _printCursorRow >= _rows) {
            return false; 
        }

        _drawingCanvas->setTextColor(_textColor); 
        _drawingCanvas->setTextDatum(TL_DATUM); 

        if (c == '\n') {
            _printCursorRow++;
            _printCursorCol = 0;
            if (_printCursorRow >= _rows) { 
                return false; 
            }
            return true;
        }

        if (_printCursorCol >= _cols) { 
            if (_lineWrap) {
                _printCursorRow++;
                _printCursorCol = 0;
                if (_printCursorRow >= _rows) { 
                    return false; 
                }
            } else {
                return false; 
            }
        }
        
        int x = _printCursorCol * _fontWidth;
        int y = _printCursorRow * _fontHeight;
        
        _drawingCanvas->drawChar(c, x, y);

        _printCursorCol++;
        return true;
    }
};

M5CanvasTextDisplayController* displayController_ptr = nullptr;


void setup() {
    M5.begin();
    Serial.begin(115200);

    displayController_ptr = new M5CanvasTextDisplayController(M5.Display);
    if (!displayController_ptr->begin(1, true, WHITE, BLACK)) { 
        Serial.println("Display Controller Begin FAILED!");
        while(1) { 
            M5.Speaker.tone(2000, 20); 
            delay(100 + 20);         
        }
    }
    M5CanvasTextDisplayController& dc = *displayController_ptr;
}

void loop() {
    M5.update();
    
    M5CanvasTextDisplayController& dc = *displayController_ptr;
    delay(2000);
    // --- 1回目の表示 ---
    dc.clearDrawingCanvas(); 
    dc.setTextSize(1); 
    dc.setCursor(0,0);
    dc.setTextColor(GREEN);
    dc.println("Frame 1: Initial");
    dc.print("Hello!");
    dc.show(); 

    delay(2000);

    // --- 2回目の表示 (Frame 1の内容に追記) ---
    dc.setTextColor(YELLOW);
    dc.println("Frame 2: Appended"); 
    dc.print("World!");
    dc.show(); 

    delay(2000);

    // --- 3回目の表示 (Frame 1+2 の内容に setText で上書き) ---
    dc.setTextColor(CYAN);
    dc.setText(0, 0, "Frame 3: Overwrite "); 
    dc.setCursor(1,0); 
    dc.println("With setText.");
    dc.show();

    delay(2000);

    // --- 4回目の表示 (clearしてから新しい内容) ---
    dc.clearDrawingCanvas(); 
    dc.setTextColor(MAGENTA);
    dc.setCursor(dc.getRows()/2, 1);
    dc.print("Frame 4: Cleared");
    dc.show();
    
    delay(2000);

    // --- 5回目の表示 (文字サイズ変更) ---
    dc.setTextSize(2); 
    dc.setTextColor(RED);
    dc.setCursor(0,0);
    dc.println("Size 2 Text!");
    dc.show();
}
