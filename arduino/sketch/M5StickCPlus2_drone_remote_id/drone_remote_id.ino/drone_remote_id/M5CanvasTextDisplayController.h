#include <M5Unified.h>

/**
 * @class M5CanvasTextDisplayController
 * @brief M5GFXディスプレイ上でテキスト表示を効率的に行うためのコントローラクラス
 *
 * ダブルバッファリングを使用して、ちらつきのないテキスト更新を実現します
 * 文字単位のグリッドベースでのテキスト配置や、Arduino Printクラスライクな
 * インターフェースを提供します。画面の回転や文字サイズの変更にも対応しています
 */
class M5CanvasTextDisplayController {
public:
    /**
     * @brief M5CanvasTextDisplayController のコンストラクタ
     * @param display 操作対象の M5GFX (M5Display) オブジェクトへの参照
     * @note この時点ではM5Canvasオブジェクトのインスタンス化のみを行い、
     *       スプライトのメモリ確保 (createSprite) は begin() または setRotation() で行われます
     */
    M5CanvasTextDisplayController(M5GFX& display) : 
        _lcd(display), 
        _canvas1(nullptr), 
        _canvas2(nullptr),
        _activeCanvas(nullptr),
        _drawingCanvas(nullptr)
    {
        // M5Canvasオブジェクトを生成します。スプライトの作成はまだ行いません
        _canvas1 = new M5Canvas(&_lcd);
        _canvas2 = new M5Canvas(&_lcd);
    }

    /**
     * @brief M5CanvasTextDisplayController のデストラクタ
     *
     * 確保された2つのM5Canvasオブジェクトと、それらが持つスプライトメモリを解放します
     */
    ~M5CanvasTextDisplayController() {
        if (_canvas1) {
            _canvas1->deleteSprite(); // スプライトメモリを解放
        }
        if (_canvas2) {
            _canvas2->deleteSprite(); // スプライトメモリを解放
        }
        delete _canvas1; // M5Canvasオブジェクト自体を解放
        _canvas1 = nullptr;
        delete _canvas2; // M5Canvasオブジェクト自体を解放
        _canvas2 = nullptr;
    }

    /**
     * @brief ディスプレイコントローラの初期化を行います
     *
     * 画面の回転、テキストサイズ、文字色、背景色を設定し、
     * ダブルバッファ用のキャンバスを指定された回転に基づいて作成します
     *
     * @param initialTextSize 初期テキストサイズ (デフォルト: 1)
     * @param initialLineWrap 初期状態でテキストの行ラップを有効にするか (デフォルト: true)
     * @param textColor 初期文字色 (デフォルト: WHITE)
     * @param bgColor 初期背景色 (デフォルト: BLACK)
     * @param initialRotation 初期画面回転 (0-3、M5GFXの回転定数。デフォルト: 0)
     * @return 初期化に成功した場合は true、失敗した場合は false
     *         主にキャンバスの作成に失敗した場合に false を返します
     */
    bool begin(int initialTextSize = 1, bool initialLineWrap = true,
               uint16_t textColor = WHITE, uint16_t bgColor = BLACK,
               uint8_t initialRotation = 0) {
        _textColor = textColor;
        _bgColor = bgColor;
        _lineWrap = initialLineWrap;
        // 最初に回転を設定します。これによりキャンバスが適切なサイズで作成されます
        // begin() 内部では、まだ画面に表示する必要がないため、setRotationの第2引数 doShow は false にします
        if (!setRotation(initialRotation, false)) {
             Serial.println("Error: Failed to set initial rotation and create canvases in begin().");
             return false; // 回転設定失敗はキャンバス作成失敗を意味します
        }
        // setTextSizeはsetRotation内で一度呼ばれますが、
        // 引数で指定されたinitialTextSizeを確実に適用するために再度呼び出します
        // ここでも、まだ画面に表示する必要がないため、setTextSizeの第2引数 doShow は false にします
        setTextSize(initialTextSize, false);
        // 両方のキャンバスは setRotation -> _recreateCanvases -> _clearAndResetDrawingCanvas 及び
        // _activeCanvas->fillSprite(_bgColor) によって初期背景色でクリアされます
        // print用カーソルはsetTextSize -> _clearAndResetDrawingCanvas で初期化されます
        // 必要であれば、beginの最後に空の画面をshow()することもできますが、
        // 通常は最初の描画内容をセットした後にshow()を呼び出すため、ここでは行いません
        // show(); 
        return true;
    }
    
    /**
     * @brief LCD表示とキャンバスの回転を設定します
     *
     * このメソッドを呼び出すと、内部のキャンバスは現在のLCDの幅と高さに合わせて再作成されます
     * 再作成後、現在のテキストサイズ設定に基づいて文字の行数・列数が再計算され、
     * 描画キャンバスはクリアされます
     *
     * @param rotation 新しい画面の回転方向 (0, 1, 2, 3)。M5GFXの回転定数に対応します
     * @param doShow trueの場合、回転設定後に現在の描画キャンバスの内容をLCDに表示します (デフォルト: true)
     *               falseの場合、表示は行いません
     * @return キャンバスの再作成と設定に成功した場合は true、失敗した場合は false
     */
    bool setRotation(uint8_t rotation, bool doShow = true) {
        _lcd.setRotation(rotation); // LCD自体の回転を設定
        // 新しい画面寸法に合わせてキャンバスを再作成します
        if (!_recreateCanvases()) {
            Serial.println("Error: Failed to recreate canvases for new rotation.");
            return false;
        }
        // 文字サイズ関連の情報を再計算します
        // setTextSizeを呼び出すことで、新しい画面寸法に基づいた _rows, _cols が計算され、
        // 描画キャンバスがクリアされ、フォント設定も適用されます
        // ここでは doShow = false で呼び出し、最後の show() でまとめて表示します
        setTextSize(_textSize, false);
        if (doShow) {
            show(); // 新しい回転とサイズで画面を更新します
        }
        return true;
    }

    /**
     * @brief テキストのサイズを設定します
     *
     * 文字サイズを変更すると、画面に表示可能な行数 (_rows) と列数 (_cols) が再計算されます
     * また、描画キャンバスはクリアされ、カーソル位置もリセットされます
     *
     * @param size 新しいテキストサイズ (1以上の整数)。M5GFXのsetTextSizeに渡されます
     * @param doShow trueの場合、文字サイズ変更後に現在の描画キャンバスの内容をLCDに表示します (デフォルト: true)
     *               falseの場合、表示は行いません
     * @note 内部でフォントサイズを取得し、それに基づいて行数・列数を計算します
     *       デフォルトフォント (font 0) が使用されます
     */
    void setTextSize(int size, bool doShow = true) {
        if (size < 1) size = 1; // サイズは1以上を保証
        _textSize = size;
        // 両方のキャンバスに新しい文字サイズとデフォルトフォント(font 0)を設定します
        if (_drawingCanvas) {
            _drawingCanvas->setTextSize(_textSize);
            _drawingCanvas->setTextFont(0); // 標準フォントを使用
        }
        if (_activeCanvas) {
            _activeCanvas->setTextSize(_textSize);
            _activeCanvas->setTextFont(0);  // 標準フォントを使用
        }
        // フォントの高と幅を取得します。_drawingCanvasが存在することを期待します
        if(_drawingCanvas) {
            _fontHeight = _drawingCanvas->fontHeight();
            _fontWidth = _drawingCanvas->fontWidth();
        } else {
            // フォールバック: _drawingCanvasが未作成の場合 (通常は発生しないはず)
             _fontHeight = 8 * _textSize; // 標準フォントの高さの目安
             _fontWidth = 6 * _textSize;  // 標準フォントの幅の目安 (font 0は可変幅だが、createSprite用には固定値が必要な場合がある)
        }
        // fontHeight/Widthが0になるケース(フォント未設定時など)へのフォールバック
        if (_fontHeight == 0) _fontHeight = 8 * _textSize; // 8は標準フォントの基本高さ
        if (_fontWidth == 0) _fontWidth = 6 * _textSize;   // 6は標準フォントの多くの文字の基本幅
        // 現在のLCDの幅と高さから、表示可能な文字の行数と列数を計算します
        _rows = _lcd.height() / _fontHeight;
        _cols = _lcd.width() / _fontWidth;
        // 描画キャンバスをクリアし、カーソル位置をリセットします
        _clearAndResetDrawingCanvas();
        if (doShow) {
            show(); // 新しい文字サイズで画面を更新します
        }
    }

    /**
     * @brief 指定した行・列に文字列を描画します
     *
     * 文字列が指定した位置から画面右端を超える場合、自動的に切り詰められます
     * 描画は内部の描画キャンバスに対して行われ、show()が呼び出されるまで画面には反映されません
     *
     * @param row 描画を開始する行 (0から始まるインデックス)
     * @param col 描画を開始する列 (0から始まるインデックス)
     * @param text 描画する文字列 (Stringオブジェクト)
     * @note このメソッドは文字の背景を指定された背景色で塗りつぶしてから文字を描画します
     */
    void setText(int row, int col, const String& text) {
        if (row < 0 || row >= _rows || col < 0 || col >= _cols || !_drawingCanvas) return;
        int x = col * _fontWidth; // ピクセル座標X
        int y = row * _fontHeight; // ピクセル座標Y
        _drawingCanvas->setTextDatum(TL_DATUM); // 左上基準で描画
        String sub = text;
        int maxLen = _cols - col; // 現在のカーソル位置から行末までの最大文字数
        if (text.length() > (unsigned int)maxLen) {
            sub = text.substring(0, maxLen); // 最大文字数に合わせて文字列を切り詰める
        }
        // 文字を描画する領域をまず背景色でクリアし、その上に文字を描画
        _drawingCanvas->setTextColor(_textColor, _bgColor); 
        _drawingCanvas->fillRect(x, y, sub.length() * _fontWidth, _fontHeight, _bgColor);
        _drawingCanvas->drawString(sub, x, y);
    }
    
    /**
     * @brief 指定した行・列に文字列を描画します (const char*版)
     * @param row 描画を開始する行 (0から始まるインデックス)
     * @param col 描画を開始する列 (0から始まるインデックス)
     * @param text 描画するCスタイル文字列
     */
    void setText(int row, int col, const char* text) {
        setText(row, col, String(text));
    }

    /**
     * @brief print/printlnメソッドで使用するカーソル位置を設定します
     * @param row 新しいカーソルの行 (0から始まるインデックス)
     * @param col 新しいカーソルの列 (0から始まるインデックス)
     * @note 指定された行・列が範囲外の場合は無視されます
     */
    void setCursor(int row, int col) {
        if (row >= 0 && row < _rows) _printCursorRow = row;
        if (col >= 0 && col < _cols) _printCursorCol = col;
    }

    /**
     * @brief 現在のカーソル位置から文字列を印字します (Arduino Printクラス互換)
     *
     * 改行コード('\\n')や行末での自動改行 (setLineWrapで設定可能) に対応します
     * 描画は内部の描画キャンバスに対して行われ、show()が呼び出されるまで画面には反映されません
     *
     * @param text 印字する文字列 (Stringオブジェクト)
     * @return 印字された文字数
     */
    size_t print(const String& text) {
        if (!_drawingCanvas) return 0; // 描画キャンバスがなければ何もしない
        _drawingCanvas->setTextColor(_textColor); // 文字色を設定 (背景色は指定しないモード)
        _drawingCanvas->setTextDatum(TL_DATUM);   // 左上基準で文字を描画
        size_t n = 0; // 印字した文字数をカウント
        for (unsigned int i = 0; i < text.length(); ++i) {
            char c = text.charAt(i);
            if (_printChar(c)) { // 内部メソッドで1文字ずつ処理
                n++;
            } else {
                break; // _printCharがfalseを返したら (画面外など)、ループを抜ける
            }
        }
        return n;
    }

    /** @brief 現在のカーソル位置から文字列を印字します (const char*版) @copydoc print(const String&) */
    size_t print(const char* text) { return print(String(text)); }
    /** @brief 現在のカーソル位置から1文字を印字します @copydoc print(const String&) */
    size_t print(char c) { if(!_drawingCanvas) return 0; return _printChar(c) ? 1 : 0; }
    /** @brief 現在のカーソル位置から整数を印字します @copydoc print(const String&) */
    size_t print(int val, int base = DEC) { return print(String(val, base)); }
    /** @brief 現在のカーソル位置から浮動小数点数を印字します @copydoc print(const String&) */
    size_t print(double val, int decimalPlaces = 2) { return print(String(val, decimalPlaces)); }

    /**
     * @brief 現在のカーソル位置から文字列を印字し、最後に改行します (Arduino Printクラス互換)
     * @param text 印字する文字列 (Stringオブジェクト)
     * @return 印字された文字数 (改行文字も含む)
     */
    size_t println(const String& text) {
        size_t n = print(text);
        if (_printChar('\n')) { // 改行文字を処理
             n++; // 改行も1文字としてカウント
        }
        return n;
    }

    /** @brief 現在のカーソル位置から文字列を印字し、最後に改行します (const char*版) @copydoc println(const String&) */
    size_t println(const char* text) { return println(String(text)); }
    /** @brief 現在のカーソル位置から1文字を印字し、最後に改行します @copydoc println(const String&) */
    size_t println(char c) { return println(String(c));}
    /** @brief 現在のカーソル位置から整数を印字し、最後に改行します @copydoc println(const String&) */
    size_t println(int val, int base = DEC) { return println(String(val, base)); }
    /** @brief 現在のカーソル位置から浮動小数点数を印字し、最後に改行します @copydoc println(const String&) */
    size_t println(double val, int decimalPlaces = 2) { return println(String(val, decimalPlaces)); }
    /** @brief 現在のカーソル位置から改行のみ行います @copydoc println(const String&) */
    size_t println() { return _printChar('\n') ? 1 : 0; }

    /**
     * @brief 描画キャンバスの内容をLCDに表示し、バッファを切り替えます
     *
     * 具体的には、現在の描画キャンバス (_drawingCanvas) の内容を物理LCDに転送し、
     * その後、アクティブキャンバス (_activeCanvas) と描画キャンバスの役割を入れ替えます
     * 新しい描画キャンバス (元のアクティブキャンバス) には、古い描画キャンバスの内容がコピーされます
     * これにより、次のフレームの描画をスムーズに開始できます
     *
     * @note M5GFXのpushSpriteは、コピー元スプライトの内容をコピー先スプライトに上書きコピーする
     *       1. _drawingCanvas を LCD にプッシュ (表示)
     *       2. _activeCanvas と _drawingCanvas のポインタをスワップ
     *       3. 新しい _drawingCanvas (元 _activeCanvas) に、新しい _activeCanvas (元 _drawingCanvas) の内容をコピー
     *          これにより、次のフレームで差分描画や継続描画が容易になる
     */
    void show() {
        if (!_drawingCanvas || !_activeCanvas) return; // キャンバスがなければ何もしない
        // 1. 現在の描画キャンバスの内容をLCDに表示
        _drawingCanvas->pushSprite(0, 0);
        // 2. アクティブキャンバスと描画キャンバスの役割をスワップ
        M5Canvas* temp = _activeCanvas;
        _activeCanvas = _drawingCanvas;
        _drawingCanvas = temp;
        // 3. 新しい描画キャンバス (元々のアクティブキャンバスだったもの) に、
        //    新しいアクティブキャンバス (元々の描画キャンバスだったもの、つまり表示された内容) の内容をコピーします
        //    これにより、次の描画サイクルで、表示されている内容から継続して変更を加えることができます
        if (_drawingCanvas && _activeCanvas) { // 両方が有効な場合のみコピー
            _activeCanvas->pushSprite(_drawingCanvas, 0, 0);
        }
        // 新しい描画キャンバスのM5Canvas内部カーソルをリセット (ピクセル単位のカーソル)
        if (_drawingCanvas) {
            _drawingCanvas->setCursor(0,0); 
        }
    }

    /**
     * @brief print/printlnメソッド使用時の行末での自動改行を有効/無効にします
     * @param wrap trueで有効、falseで無効
     */
    void setLineWrap(bool wrap) {
        _lineWrap = wrap;
    }

    /**
     * @brief 描画キャンバスの内容を現在の背景色でクリアし、カーソルをリセットします
     *
     * この変更は show() が呼び出されるまで画面には反映されません
     */
    void clearDrawingCanvas() {
        _clearAndResetDrawingCanvas();
    }

    /**
     * @brief 画面全体を指定された色で塗りつぶします
     *
     * このメソッドは物理LCDだけでなく、内部の描画キャンバスとアクティブキャンバスも
     * 指定色で塗りつぶし、背景色設定(_bgColor)も更新します
     * print/printlnカーソルもリセットされます
     * この変更は即座にLCDに反映されます
     *
     * @param color 塗りつぶす色
     */
    void fillScreen(uint16_t color) {
        _bgColor = color; // 新しい背景色を保存
        _lcd.fillScreen(_bgColor); // 物理LCDを塗りつぶし
        // 内部キャンバスも新しい背景色で塗りつぶし
        if (_drawingCanvas) _drawingCanvas->fillSprite(_bgColor);
        if (_activeCanvas) _activeCanvas->fillSprite(_bgColor);
        // print/printlnカーソルとM5Canvasのピクセルカーソルをリセット
        _printCursorRow = 0;
        _printCursorCol = 0;
        if (_drawingCanvas) _drawingCanvas->setCursor(0,0);
    }

    /**
     * @brief 画面全体を現在の背景色 (_bgColor) で塗りつぶします
     *
     * この変更は即座にLCDに反映されます
     * 内部キャンバスもクリアされ、カーソルもリセットされます
     */
    void fillScreen() {
        fillScreen(_bgColor);
    }

    /**
     * @brief 文字色を設定します
     * @param color 新しい文字色
     */
    void setTextColor(uint16_t color) {
        _textColor = color;
    }

    /**
     * @brief 背景色を設定します
     * @note このメソッドは背景色設定(_bgColor)を更新するのみで、画面のクリアは行いません
     *       画面を新しい背景色でクリアしたい場合は、fillScreen(color) や clearDrawingCanvas() の後に show() を使用してください
     * @param color 新しい背景色
     */
    void setBgColor(uint16_t color) {
        _bgColor = color;
    }
    
    /** @brief 現在の画面設定での最大行数を取得します @return 行数 */
    int getRows() const { return _rows; }
    /** @brief 現在の画面設定での最大列数を取得します @return 列数 */
    int getCols() const { return _cols; }
    /** @brief print/printlnメソッドの現在のカーソル行を取得します @return カーソル行 */
    int getPrintCursorRow() const { return _printCursorRow; }
    /** @brief print/printlnメソッドの現在のカーソル列を取得します @return カーソル列 */
    int getPrintCursorCol() const { return _printCursorCol; }
    /** @brief 現在のテキストサイズを取得します @return テキストサイズ */
    int getTextSize() const { return _textSize; }
    /** @brief 現在の画面回転設定を取得します @return 回転設定値 (0-3) */
    uint8_t getRotation() const { return _lcd.getRotation(); }

private:
    M5GFX& _lcd;                  ///< M5GFX (M5Display) オブジェクトへの参照
    M5Canvas* _canvas1;           ///< ダブルバッファリング用キャンバス1
    M5Canvas* _canvas2;           ///< ダブルバッファリング用キャンバス2
    M5Canvas* _activeCanvas;      ///< 現在LCDに表示されている内容を保持するキャンバス (表示バッファ)
    M5Canvas* _drawingCanvas;     ///< 次に表示する内容を描画するためのキャンバス (描画バッファ)

    int _rows = 0;                ///< 現在のフォントサイズと画面寸法での最大表示行数
    int _cols = 0;                ///< 現在のフォントサイズと画面寸法での最大表示列数
    int _textSize = 1;            ///< 現在のテキストサイズ
    int _fontHeight = 8;          ///< 現在のフォントの高さ (ピクセル単位)
    int _fontWidth = 6;           ///< 現在のフォントの幅 (ピクセル単位、主に基準として使用)
    bool _lineWrap = true;        ///< print/printlnで自動行ラップを行うか
    uint16_t _textColor = WHITE;  ///< 現在の文字色
    uint16_t _bgColor = BLACK;    ///< 現在の背景色
    int _printCursorRow = 0;      ///< print/println用カーソルの現在行
    int _printCursorCol = 0;      ///< print/println用カーソルの現在列

    /**
     * @brief 既存のキャンバスのスプライトメモリを解放します
     * @note M5Canvasオブジェクト自体はdeleteしません。スプライトのメモリのみを解放します
     *       オブジェクトのdeleteはデストラクタで行います
     */
    void _deleteCanvases() {
        if (_canvas1) {
            _canvas1->deleteSprite(); // スプライトメモリを解放
        }
        if (_canvas2) {
            _canvas2->deleteSprite(); // スプライトメモリを解放
        }
        // ポインタをnullptrにはしません。オブジェクトは再利用されるためです
    }
    
    /**
     * @brief LCDの現在の幅と高さに合わせて、2つのキャンバスのスプライトを再作成します
     *
     * 既存のスプライトは解放され、新しいサイズのものが確保されます
     * 作成後、_drawingCanvas と _activeCanvas に適切に割り当てられ、
     * 描画キャンバスはクリアされ、アクティブキャンバスも背景色で塗りつぶされます
     *
     * @return キャンバスの再作成に成功した場合は true、失敗した場合は false
     */
    bool _recreateCanvases() {
        _deleteCanvases(); // 既存のキャンバススプライトを解放 (メモリリーク防止)
        // M5Canvasオブジェクト (_canvas1, _canvas2) はコンストラクタでnewされている前提です
        // ここでは、それらのオブジェクトに対してスプライトを作成します
        int w = _lcd.width();
        int h = _lcd.height();
        if (!_canvas1 || !_canvas1->createSprite(w, h)) {
            Serial.printf("Error: Failed to create canvas1 (%d x %d)\n", w, h);
            return false;
        }
        if (!_canvas2 || !_canvas2->createSprite(w, h)) {
            Serial.printf("Error: Failed to create canvas2 (%d x %d)\n", w, h);
            if (_canvas1) _canvas1->deleteSprite(); // canvas1が成功していた場合、それも解放
            return false;
        }
        // スプライト作成後、描画用と表示用にポインタを割り当てます
        // 初期状態では_canvas1を描画用、_canvas2を表示用(アクティブ)とすることが多いですが、
        // show()でスワップされるので、どちらでも機能します
        // begin()やsetRotation()の直後では、どちらも同じ状態(クリアされている)なので、
        // どちらを_drawingCanvasにしても問題ありません
        _drawingCanvas = _canvas1;
        _activeCanvas = _canvas2;
        _clearAndResetDrawingCanvas();          // 新しい描画キャンバスをクリア＆カーソルリセット
        if (_activeCanvas) _activeCanvas->fillSprite(_bgColor); // アクティブキャンバスも背景色でクリア
        return true;
    }

    /**
     * @brief 描画キャンバス (_drawingCanvas) を現在の背景色でクリアし、
     *        print/println用の文字カーソルとM5Canvasのピクセルカーソルをリセットします
     */
    void _clearAndResetDrawingCanvas() {
        if (!_drawingCanvas) return;
        _drawingCanvas->fillSprite(_bgColor);
        _drawingCanvas->setCursor(0,0); // M5Canvasのピクセル単位のカーソルを左上に
        _printCursorRow = 0;            // 文字グリッドベースのカーソル行を0に
        _printCursorCol = 0;            // 文字グリッドベースのカーソル列を0に
    }

    /**
     * @brief print/printlnメソッドの内部処理として、1文字を描画キャンバスに印字します
     *
     * 改行コード('\\n')の処理、行末での自動改行 (設定されている場合)、画面範囲外のチェックを行います
     *
     * @param c 印字する文字
     * @return 文字の印字に成功した場合は true、画面外などで印字できなかった場合は false
     */
    bool _printChar(char c) {
        if (!_drawingCanvas) return false; // 描画キャンバスがない
        // 現在のカーソル行が画面の最大行数を超えているか、またはちょうど最大行数だが改行しようとしている場合は失敗
        if (_printCursorRow >= _rows) {
            return false;
        }
        _drawingCanvas->setTextColor(_textColor); // 文字色を設定 (print系では背景色は透過)
        _drawingCanvas->setTextDatum(TL_DATUM);   // 左上基準で文字を描画 (M5GFXのdrawString/drawCharのデフォルト)
        if (c == '\n') { // 改行文字の場合
            _printCursorRow++;    // 次の行へ
            _printCursorCol = 0;  // 行頭へ
            if (_printCursorRow >= _rows) { // 改行した結果、画面外になった場合
                // _printCursorRow = _rows; // カーソル位置を最終行に留めるか、超えたままにするか。ここでは超えたまま
                return false; // これ以上印字できない
            }
            return true; // 改行成功
        }
        // 通常文字の場合、まず現在のカーソル列が画面の最大列数を超えているかチェック
        if (_printCursorCol >= _cols) {
            if (_lineWrap) { // 行ラップが有効な場合
                _printCursorRow++;    // 次の行へ
                _printCursorCol = 0;  // 行頭へ
                if (_printCursorRow >= _rows) { // 改行した結果、画面外になった場合
                    // _printCursorRow = _rows;
                    return false; // これ以上印字できない
                }
                // 改行して新しい行に移動したので、再度この文字を処理するために再帰呼び出し...はせず、
                // この後の処理で現在の_printCursorRow, _printCursorColを使って描画する
            } else { // 行ラップが無効な場合
                return false; // これ以上印字できない
            }
        }
        // 描画位置を計算 (文字グリッド座標からピクセル座標へ)
        int x = _printCursorCol * _fontWidth;
        int y = _printCursorRow * _fontHeight;
        // M5CanvasのdrawCharを使って1文字描画
        // drawCharは背景を透過して文字のみを描画する
        _drawingCanvas->drawChar(c, x, y, _drawingCanvas->getTextFont()); // フォント指定は現在の設定を使用
        _printCursorCol++; // カーソルを1文字分進める
        return true; // 1文字印字成功
    }
};
