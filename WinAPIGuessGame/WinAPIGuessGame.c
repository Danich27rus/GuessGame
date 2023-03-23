// WinAPIGuessGame.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "WinAPIGuessGame.h"

#define WIN_COEFF 1.25

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

// Отправить объявления функций, включенных в этот модуль кода:
// Прототипы функций
ATOM                WindowGuessClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

//Все доп. переменные
typedef struct GameStatus {
    bool GameStarted;
    wint_t CashValue;
    wint_t GamesPlayed;
    wint_t AttemptCounter;
    errno_t numberToGuess;
} GameStatus;

static wchar_t TextBoxBuff[1000];

static GameStatus* game = NULL;

HWND hwndGame, hwndAttempt, hwndCashValue, hwndHintCheckBox, hwndCombo, 
hwndOutput, hwndInput, 
hwndBet, hwndBetInput, 
hwndLeftBorder, hwndRightBorder;
//HBRUSH hBrushLabel = NULL;
//COLORREF clrLabelBkGnd;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINAPIGUESSGAME, szWindowClass, MAX_LOADSTRING);
    WindowGuessClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINAPIGUESSGAME));

    MSG msg;

    srand(time(NULL));

    // Цикл основного сообщения:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: WindowGuessClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM WindowGuessClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINAPIGUESSGAME));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINAPIGUESSGAME);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    game = (struct GameStatus*)malloc(sizeof(struct GameStatus));
    game->AttemptCounter = 0;
    game->GameStarted = 0;
    game->GamesPlayed = 0;

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   //Если прописать CW_USEDEFAULT в ширину/высоту - высота и ширина выбирается системой
   HWND hWnd = CreateWindowEx(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, //WS_OVERLAPPEDWINDOW //WS_EX_TRANSPARENT
      CW_USEDEFAULT, 0, 640, 480, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // = L"\n\
    // ";
    wchar_t buf[10], 
        buff_num[10], 
        attempts_value[_MAX_ITOSTR_BASE10_COUNT],
        games_value[_MAX_ITOSTR_BASE10_COUNT],
        correct_number[_MAX_ITOSTR_BASE10_COUNT];
    errno_t buff_num_int, bet_buff_int = 0, leftBorder = -100, rightBorder = 100;
    bool hint_flag;
    const wchar_t* AttemptItems[] = { L"4", L"5", L"6", L"7", L"8" };
    wchar_t* StringGamesCounter = L"Количество игр:\r\n";
    wchar_t* StringAttemptCounter = L"Количество попыток:\r\n";
    wchar_t* StringCashValue = L"Количество монет на счету:\r\n";
    wchar_t* StringBetEmpty = L"Поле 'Ставка' не может быть пустым\r\n";
    wchar_t* StringCashBet = L"Ставка:\r\n";
    wchar_t* StringRange = L"Диапазон значений:\r\n\r\nОт:\r\n\r\nДо:";

    wchar_t* GameStart = L"Игра началась\r\n";
    wchar_t* IncorrectAnswer = L"Неправильный ответ\r\n";
    wchar_t* CorrectAnswer = L"Вы угадали число!\r\nХотите попробовать снова?\r\n";
    wchar_t* INCAnswerLow = L"Ответ меньше введённого числа\r\n";
    wchar_t* INCAnswerHigh = L"Ответ больше введённого числа\r\n";
    wchar_t* ZeroAttempts = L"У вас закончились попытки\r\nХотите попробовать снова?\r\n";
    wchar_t* CNumber = L" - правильный ответ\r\n";

    switch (message)
    {
    case WM_CREATE:
    {
        // для TextBoxа онли CreateWindowEx
        hwndInput = CreateWindowEx(WS_EX_WINDOWEDGE,
            TEXT("Edit"), NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            20, 310, 150, 20, hWnd, (HMENU)IDC_INPUTTEXTBOX, NULL, NULL);

        hwndOutput = CreateWindowEx(WS_EX_WINDOWEDGE,
            TEXT("Edit"), NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_LEFT | WS_VSCROLL | ES_LEFT | ES_MULTILINE |
            ES_AUTOHSCROLL | ES_AUTOVSCROLL,
            20, 80, 350, 220, hWnd, (HMENU)IDC_OUTPUTTEXTBOX, NULL, NULL);

        hwndBetInput = CreateWindowEx(WS_EX_WINDOWEDGE,
            TEXT("Edit"), NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            470, 130, 115, 20, hWnd, (HMENU)IDC_INPUTBOXBET, NULL, NULL);

        //GroupBox окна игры
        CreateWindowW(L"Button", L"Окно игры",
            WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            10, 20, 380, 360, hWnd, (HMENU)IDC_GROUPBOX, NULL, NULL);

        //GroupBox настройки игры
        CreateWindowW(L"Button", L"Окно настроек",
            WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            400, 20, 200, 360, hWnd, (HMENU)IDC_GROUPBOX, NULL, NULL);

        //ComboBox с кол-вом попыток
        /*hwndCombo = CreateWindowW(L"Combobox", NULL,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWN,
            450, 95, 70, 125, hWnd, (HMENU)IDC_COMBOBOX, NULL, NULL);*/

        CreateWindowW(L"Static", StringGamesCounter,
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            20, 50, 150, 25, hWnd, (HMENU)IDC_STATIC, NULL, NULL);

        CreateWindowW(L"Static", StringAttemptCounter,
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            415, 70, 150, 25, hWnd, (HMENU)IDC_STATIC, NULL, NULL);

        CreateWindowW(L"Static", StringCashValue,
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            415, 90, 150, 35, hWnd, (HMENU)IDC_STATIC, NULL, NULL);

        CreateWindowW(L"Static", StringCashBet,
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            415, 130, 50, 35, hWnd, (HMENU)IDC_STATIC, NULL, NULL);

        CreateWindowW(L"Static", StringRange,
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            415, 180, 150, 85, hWnd, (HMENU)IDC_STATIC, NULL, NULL);

        hwndGame = CreateWindowW(L"Static", L"0",
            WS_VISIBLE | WS_CHILD,
            180, 50, 20, 25, hWnd, (HMENU)IDC_STATIC, NULL, NULL);   //Количество сыграных игр

        hwndAttempt = CreateWindowW(L"Static", L"",
            WS_VISIBLE | WS_CHILD,
            565, 70, 20, 25, hWnd, (HMENU)IDC_STATIC, NULL, NULL);  //Количество попыток

        hwndCashValue = CreateWindowW(L"Static", L"100",
            WS_VISIBLE | WS_CHILD,
            470, 107, 30, 20, hWnd, (HMENU)IDC_STATIC, NULL, NULL);  //Количество денег

        hwndHintCheckBox = CreateWindowW(L"Button", L"Подсказки выключены",
            WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
            20, 380, 185, 25, hWnd, (HMENU)IDC_HINT, NULL, NULL);    //Чекбокс подсказки

        CreateWindowW(L"Button", L"Сгенерировать число",
            WS_VISIBLE | WS_CHILD,
            20, 340, 150, 25, hWnd, (HMENU)IDC_ROLL, NULL, NULL);

        CreateWindowW(L"Button", L"Проверить число",
            WS_VISIBLE | WS_CHILD,
            220, 340, 150, 25, hWnd, (HMENU)IDC_NUMBERCHECK, NULL, NULL);

        CreateWindowW(L"Button", L"Повторить игру",
            WS_VISIBLE | WS_CHILD,
            430, 340, 150, 25, hWnd, (HMENU)IDC_RESTART, NULL, NULL);

        CreateWindowW(L"Button", L"Сделать ставку",
            WS_VISIBLE | WS_CHILD,
            470, 150, 115, 25, hWnd, (HMENU)IDC_INPUTBET, NULL, NULL);

        //Поля ввода, убраны вниз чтобы белый прямоугольник не наплывал на них
        hwndLeftBorder = CreateWindowEx(WS_EX_WINDOWEDGE,
            TEXT("Edit"), L"-100",
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            470, 210, 115, 20, hWnd, (HMENU)IDC_INPUTBOXFROM, NULL, NULL);

        hwndRightBorder = CreateWindowEx(WS_EX_WINDOWEDGE,
            TEXT("Edit"), L"100",
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            470, 240, 115, 20, hWnd, (HMENU)IDC_INPUTBOXTO, NULL, NULL);

        CheckDlgButton(hWnd, IDC_HINT, BST_UNCHECKED);
        EnableWindow(GetDlgItem(hWnd, IDC_RESTART), false);
        //EnableWindow(GetDlgItem(hWnd, IDC_COMBOBOX), false);

        for (int i = 0; i < 5; i++) {
            SendMessageW(hwndCombo, CB_ADDSTRING, 0, (LPARAM)AttemptItems[i]);
        }
    }
    break;
    case WM_COMMAND:
    {
        hint_flag = IsDlgButtonChecked(hWnd, IDC_HINT);

        if (LOWORD(wParam) == IDC_NUMBERCHECK)
        {
            if (!game->GameStarted) {
                SetWindowText(hwndOutput, L"Игра ещё не начата!");
                break;
            }
            --game->AttemptCounter;
            /*CreateWindowW(L"Static", L"0",
                WS_VISIBLE | WS_CHILD | SS_WHITEFRAME,
                180, 50, 20, 25, hWnd, (HMENU)IDC_STATIC, NULL, NULL);*/ //Костыль на перерисовке
            StringCbPrintf(buf, 10, L"%ld", game->AttemptCounter);
            SetWindowText(hwndAttempt, buf);
            GetDlgItemText(hWnd, IDC_INPUTTEXTBOX, buff_num, 10);
            buff_num_int = _wtoi(buff_num);
            if (!game->AttemptCounter) {
                wcsncat_s(TextBoxBuff, 1000, ZeroAttempts, wcslen(ZeroAttempts));
                EnableWindow(GetDlgItem(hWnd, IDC_NUMBERCHECK), false);
                EnableWindow(GetDlgItem(hWnd, IDC_RESTART), true);
                SetDlgItemText(hWnd, IDC_OUTPUTTEXTBOX, TextBoxBuff);
                _itow_s(game->numberToGuess, correct_number, _MAX_ITOSTR_BASE10_COUNT, 10);
                wcsncat_s(TextBoxBuff, 1000, correct_number, wcslen(correct_number));
                wcsncat_s(TextBoxBuff, 1000, CNumber, wcslen(CNumber));
                ++game->GamesPlayed;
                _itow_s(game->GamesPlayed, games_value, _MAX_ITOSTR_BASE10_COUNT, 10);
                SetWindowText(hwndGame, games_value);
                SetDlgItemText(hWnd, IDC_OUTPUTTEXTBOX, TextBoxBuff);
                break;
            }
            if (buff_num_int == game->numberToGuess) {
                wcsncat_s(TextBoxBuff, 1000, CorrectAnswer, wcslen(CorrectAnswer));
                SetDlgItemText(hWnd, IDC_OUTPUTTEXTBOX, TextBoxBuff);
                //GameStarted = 0;
                EnableWindow(GetDlgItem(hWnd, IDC_NUMBERCHECK), false);
                EnableWindow(GetDlgItem(hWnd, IDC_RESTART), true);
                ++game->GamesPlayed;
                _itow_s(game->GamesPlayed, games_value, _MAX_ITOSTR_BASE10_COUNT, 10);
                SetWindowText(hwndGame, games_value);
                GetWindowText(hwndCashValue, buff_num, 10);
                bet_buff_int = _wtoi(buff_num);
                bet_buff_int += (game->CashValue * WIN_COEFF);
                StringCbPrintf(buff_num, 10, L"%ld", bet_buff_int);
                SetWindowText(hwndCashValue, buff_num);

            }
            if (buff_num_int < game->numberToGuess) {
                if (!hint_flag) {
                    wcsncat_s(TextBoxBuff, 1000, IncorrectAnswer, wcslen(IncorrectAnswer));
                }
                else {
                    wcsncat_s(TextBoxBuff, 1000, INCAnswerHigh, wcslen(INCAnswerHigh));
                }
                SetDlgItemText(hWnd, IDC_OUTPUTTEXTBOX, TextBoxBuff);
            }
            if (buff_num_int > game->numberToGuess) {
                if (!hint_flag) {
                    wcsncat_s(TextBoxBuff, 1000, IncorrectAnswer, wcslen(IncorrectAnswer));
                }
                else {
                    wcsncat_s(TextBoxBuff, 1000, INCAnswerLow, wcslen(INCAnswerLow));
                }
                SetDlgItemText(hWnd, IDC_OUTPUTTEXTBOX, TextBoxBuff);
            }
        }
        if (LOWORD(wParam) == IDC_RESTART) {
            game->GameStarted = 0;
            EnableWindow(GetDlgItem(hWnd, IDC_RESTART), false);
            EnableWindow(GetDlgItem(hWnd, IDC_NUMBERCHECK), true);
            EnableWindow(GetDlgItem(hWnd, IDC_ROLL), true);
            EnableWindow(GetDlgItem(hWnd, IDC_HINT), true);
            EnableWindow(GetDlgItem(hWnd, IDC_COMBOBOX), false);
            EnableWindow(GetDlgItem(hWnd, IDC_INPUTBET), true);
            SetWindowTextW(hwndAttempt, L"");
            SetWindowTextW(hwndBetInput, L"");
            TextBoxBuff[0] = '\0';
            game->AttemptCounter = 0;
            game->CashValue = 0;
        }
        if (LOWORD(wParam) == IDC_HINT) {
            if (!hint_flag) {
                CheckDlgButton(hWnd, IDC_HINT, BST_CHECKED);
                SetWindowTextW(hwndHintCheckBox, L"Подсказки вклчюены");
            }
            else {
                CheckDlgButton(hWnd, IDC_HINT, BST_UNCHECKED);
                SetWindowTextW(hwndHintCheckBox, L"Подсказки выклчюены");
            }
        }
        if (LOWORD(wParam) == IDC_ROLL) {
            GetWindowText(hwndLeftBorder, buff_num, 10);
            leftBorder = _wtoi(buff_num);
            GetWindowText(hwndRightBorder, buff_num, 10);
            rightBorder = _wtoi(buff_num);
            if (game->CashValue == NULL) {
                SetWindowText(hwndOutput, StringBetEmpty);
                break;
            }
            game->GameStarted = 1;
            game->numberToGuess = rand() % (rightBorder - leftBorder + 1) + leftBorder;
            for (int i = 0; i < 10; ++i) {
                if (uint_pow(2, i) > abs(game->numberToGuess)) {
                    _itow_s(i, attempts_value, _MAX_ITOSTR_BASE10_COUNT, 10);
                    SetWindowText(hwndAttempt, attempts_value);
                    game->AttemptCounter = i;
                    break;
                }
            }
            //SetWindowTextW(hwndAttempt, AttemptItems[sel]);
            //game->AttemptCounter = _wtoi(AttemptItems[sel]);
            EnableWindow(GetDlgItem(hWnd, IDC_ROLL), false);
            EnableWindow(GetDlgItem(hWnd, IDC_HINT), false);
            EnableWindow(GetDlgItem(hWnd, IDC_COMBOBOX), false);
            wcsncat_s(TextBoxBuff, 1000, GameStart, wcslen(GameStart));
            SetWindowText(hwndOutput, TextBoxBuff);
        }
        if (LOWORD(wParam) == IDC_INPUTBET) {
            GetDlgItemText(hWnd, IDC_INPUTBOXBET, buff_num, 10);
            if (buff_num[0] == L'\0')               //Проверка на то, что поле со ставкой пустое
            {
                SetWindowText(hwndOutput, StringBetEmpty);
                break;
            }
            game->CashValue = _wtoi(buff_num);
            GetWindowText(hwndCashValue, buff_num, 10);
            bet_buff_int = _wtoi(buff_num);
            bet_buff_int -= game->CashValue;
            StringCbPrintf(buff_num, 10, L"%ld", bet_buff_int);
            SetWindowText(hwndCashValue, buff_num);
            //GetDlgItemText(hWnd, IDC_INPUTBOXBET, buff_num, 10);
            EnableWindow(GetDlgItem(hWnd, IDC_INPUTBET), false);
            buff_num[0] = '\0';
        }
        int wmId = LOWORD(wParam);
        // Разобрать выбор в меню:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_CTLCOLORSTATIC:                                         // Do not allow DefWindowProc() to process this message!
    {                                                               // When a WM_CTLCOLORSTATIC message comes through, return
        SetBkMode((HDC)wParam, TRANSPARENT);                        // from the Window Procedure call without a DefWindowProc()
        //return GetWindowLongPtr(hWnd, 0 * sizeof(void*));         // call. Instead return the HBRUSH stored as an instance
        return (LRESULT)GetStockObject(WHITE_BRUSH); //NULL_BRUSH   // variable as part of the WNDCLASSEX object.
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

unsigned uint_pow(unsigned base, unsigned exp)
{
    unsigned result = 1;
    while (exp)
    {
        if (exp % 2)
            result *= base;
        exp /= 2;
        base *= base;
    }
    return result;
}