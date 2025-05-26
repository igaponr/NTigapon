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
        // Canvasオブジェクトのインスタンス化は行うが、createSpriteはbeginまたはsetRotationで行う
        _canvas1 = new M5Canvas(&_lcd);
        _canvas2 = new M5Canvas(&_lcd);
    }

    ~M5CanvasTextDisplayController() {
        _deleteCanvases(); // 共通の解放処理を呼ぶ
    }

    // 初期化
    bool begin(int initialTextSize = 1, bool initialLineWrap = true, 
               uint16_t textColor = WHITE, uint16_t bgColor = BLACK, 
               uint8_t initialRotation = 0) { // 回転の初期値を引数に追加
        _textColor = textColor;
        _bgColor = bgColor;
        _lineWrap = initialLineWrap;

        // 最初に回転を設定（これによりキャンバスが適切なサイズで作成される）
        if (!setRotation(initialRotation, false)) { // false: begin中はまだshowしない
             return false; // 回転設定失敗＝キャンバス作成失敗
        }
        
        // setTextSizeはsetRotation内で呼ばれるため、ここでは不要な場合もあるが、
        // 引数のinitialTextSizeを確実に適用するために呼ぶ
        setTextSize(initialTextSize, false); // false: setTextSize中はまだshowしない
        
        // 両方のキャンバスを初期背景色でクリア
        // (setRotation -> _recreateCanvases -> _clearAndResetDrawingCanvas でクリアされる)
        // _drawingCanvas->fillSprite(_bgColor);
        // _activeCanvas->fillSprite(_bgColor); 

        // print用カーソル初期化 (setTextSizeでリセットされる)
        // _printCursorRow = 0;
        // _printCursorCol = 0;
        
        // 最初の表示内容を描画キャンバスにセットし、showで表示
        // 必要であれば、beginの最後に空の画面をshow()する
        // show(); 
        return true;
    }
    
    // 画面回転の設定
    // rotation: 0, 1, 2, 3 (M5GFXの回転定数)
    // doShow: 回転後に現在の描画キャンバスの内容を表示するか
    bool setRotation(uint8_t rotation, bool doShow = true) {
        _lcd.setRotation(rotation); // LCDの回転を実際に設定

        // キャンバスの再作成
        if (!_recreateCanvases()) {
            Serial.println("Failed to recreate canvases for new rotation.");
            return false;
        }

        // 文字サイズ関連の再計算 (内部でグリッド計算と描画キャンバスクリアが行われる)
        // setTextSizeを呼ぶことで、新しい画面寸法に合わせた_rows, _colsが計算される
        setTextSize(_textSize, false); // 現在の文字サイズで再計算、showはしない

        if (doShow) {
            show(); // 新しい回転とサイズで画面を更新
        }
        return true;
    }


    // 文字サイズ設定 (1, 2, 3...)
    // doShow: 文字サイズ変更後に現在の描画キャンバスの内容を表示するか
    void setTextSize(int size, bool doShow = true) {
        if (size < 1) size = 1;
        _textSize = size;

        // 両方のキャンバスに文字サイズとフォントを設定
        if (_drawingCanvas) {
            _drawingCanvas->setTextSize(_textSize);
            _drawingCanvas->setTextFont(0); 
        }
        if (_activeCanvas) {
            _activeCanvas->setTextSize(_textSize);
            _activeCanvas->setTextFont(0);
        }

        // _fontHeight, _fontWidth はどちらのキャンバスから取得しても同じはず
        if(_drawingCanvas) { // _drawingCanvasが存在することを期待
            _fontHeight = _drawingCanvas->fontHeight();
            _fontWidth = _drawingCanvas->fontWidth();
        } else { // フォールバック (通常はここに来ないはず)
             _fontHeight = 8 * _textSize; 
             _fontWidth = 8 * _textSize; 
        }

        if (_fontHeight == 0 || _fontWidth == 0) { // フォント情報取得失敗時のフォールバック
            _fontHeight = 8 * _textSize; 
            _fontWidth = 6 * _textSize;  // Font0の文字幅はだいたい6px程度
        }
        
        // _rows, _cols は現在のLCDの幅と高さから計算
        _rows = _lcd.height() / _fontHeight;
        _cols = _lcd.width() / _fontWidth;

        _clearAndResetDrawingCanvas(); // 描画キャンバスをクリアしカーソルリセット

        if (doShow) {
            show(); // 新しい文字サイズで画面を更新
        }
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
        _drawingCanvas->setTextColor(_textColor); 
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

        _drawingCanvas->pushSprite(0, 0);

        M5Canvas* temp = _activeCanvas;
        _activeCanvas = _drawingCanvas;
        _drawingCanvas = temp;

        _activeCanvas->pushSprite(_drawingCanvas, 0, 0); 
        
        if (_drawingCanvas) {
            _drawingCanvas->setCursor(0,0); 
        }
    }

    void setLineWrap(bool wrap) {
        _lineWrap = wrap;
    }

    void clearDrawingCanvas() {
        _clearAndResetDrawingCanvas();
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
    uint8_t getRotation() const { return _lcd.getRotation(); }


private:
    M5GFX& _lcd;
    M5Canvas* _canvas1;
    M5Canvas* _canvas2;
    M5Canvas* _activeCanvas; 
    M5Canvas* _drawingCanvas;

    int _rows = 0; // 初期値0が良いか、beginで設定されるまで不定でも良いか
    int _cols = 0;
    int _textSize = 1; // デフォルト文字サイズ
    int _fontHeight = 8;
    int _fontWidth = 6;

    bool _lineWrap = true; // デフォルトは行ラップ有効
    uint16_t _textColor = WHITE;
    uint16_t _bgColor = BLACK;

    int _printCursorRow = 0;
    int _printCursorCol = 0;

    // キャンバスのメモリ解放
    void _deleteCanvases() {
        if (_canvas1) {
            _canvas1->deleteSprite(); // スプライトメモリ解放
            // delete _canvas1; // コンストラクタでnewしたのでデストラクタでdelete
            // _canvas1 = nullptr; // ポインタを無効化
        }
        if (_canvas2) {
            _canvas2->deleteSprite();
            // delete _canvas2;
            // _canvas2 = nullptr;
        }
        // M5Canvasオブジェクト自体のdeleteはデストラクタで行う
    }
    
    // キャンバスの再作成 (現在のLCDの幅と高さに合わせて)
    bool _recreateCanvases() {
        _deleteCanvases(); // 既存のキャンバススプライトを解放

        // M5CanvasオブジェクトがnullでなければcreateSpriteを実行
        // (コンストラクタでnewされている前提)
        if (!_canvas1 || !_canvas1->createSprite(_lcd.width(), _lcd.height())) {
            Serial.printf("Failed to create canvas1 (%d x %d)\n", _lcd.width(), _lcd.height());
            return false;
        }
        if (!_canvas2 || !_canvas2->createSprite(_lcd.width(), _lcd.height())) {
            Serial.printf("Failed to create canvas2 (%d x %d)\n", _lcd.width(), _lcd.height());
            if (_canvas1) _canvas1->deleteSprite(); // canvas1も解放
            return false;
        }

        // スプライト作成後、キャンバスポインタを再割り当て
        // (通常は同じオブジェクトを使い続けるが、エラー処理などで作り直す場合も考慮)
        _drawingCanvas = _canvas1; 
        _activeCanvas = _canvas2;  

        _clearAndResetDrawingCanvas(); // 新しい描画キャンバスをクリア
        if (_activeCanvas) _activeCanvas->fillSprite(_bgColor); // アクティブキャンバスもクリア

        return true;
    }

    // 描画中キャンバスをクリアし、カーソル類をリセットする
    void _clearAndResetDrawingCanvas() {
        if (!_drawingCanvas) return;
        _drawingCanvas->fillSprite(_bgColor);
        _drawingCanvas->setCursor(0,0); // M5Canvasのピクセルカーソル
        _printCursorRow = 0;           // 文字グリッドカーソル
        _printCursorCol = 0;
    }


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
    // 初期回転を1 (横向き) に設定
    if (!displayController_ptr->begin(1, true, WHITE, BLACK, 1)) { 
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

    // --- 1回目の表示 (回転1: 横向き) ---
    dc.clearDrawingCanvas(); 
    dc.setTextSize(1); // これにより描画キャンバスはクリアされ、showも呼ばれる
    dc.setCursor(0,0);
    dc.setTextColor(GREEN);
    dc.println("Frame 1: Rotated 1");
    dc.print("Hello Landscape!");
    String dim = "Dim:" + String(dc.getCols()) + "x" + String(dc.getRows());
    dc.setText(dc.getRows() -1, 0, dim);
    dc.show(); 

    delay(3000);

    // --- 2回目の表示 (回転0: 縦向き) ---
    dc.setRotation(0); // 回転を0に戻す (内部でshowも呼ばれる)
    dc.setTextColor(YELLOW);
    dc.setCursor(3,0); // 新しいグリッドに合わせてカーソル設定
    dc.println("Frame 2: Rotated 0");
    dc.print("Hello Portrait!");
    dim = "Dim:" + String(dc.getCols()) + "x" + String(dc.getRows());
    dc.setText(dc.getRows() -1, 0, dim);
    dc.show(); 

    delay(3000);
    
    // --- 3回目の表示 (回転3: 横向き逆) ---
    dc.setRotation(3, false); // 回転だけしてshowはしない
    dc.clearDrawingCanvas();  // 明示的にクリア
    dc.setTextColor(CYAN);
    dc.setText(0, 0, "Frame 3: Rotated 3"); 
    dc.setCursor(1,0); 
    dc.println("Hello Landscape Inv!");
    dim = "Dim:" + String(dc.getCols()) + "x" + String(dc.getRows());
    dc.setText(dc.getRows() -1, 0, dim);
    dc.show(); // ここで表示

    delay(3000);

    // --- 4回目の表示 (文字サイズ変更、回転は3のまま) ---
    dc.setTextSize(2); // これにより描画キャンバスはクリアされ、showも呼ばれる
    // dc.clearDrawingCanvas(); // setTextSize内でクリアされる
    dc.setTextColor(MAGENTA);
    dc.setCursor(0, 0);
    dc.print("Size 2 Rot 3");
    // setTextSizeがshowを呼ぶので、このshowは不要な場合もある
    // dc.show(); 
}
