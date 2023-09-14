#define UNICODE
#include<windows.h>
#include<stdio.h>
#include<locale.h>

const int SLEEP_INTERVAL = 20; //50 FPS

typedef struct {
    DWORD button;
    DWORD axis[6];
    DWORD pov;
} JoystickState;

//函式傳回手把編號
//buttonNum傳回按鈕數，讀取手把輸入時會用到
static int findJoystick(int* buttonNum) {
    int joyId = -1;
    JOYCAPS joycaps;
    MMRESULT ret;
    for (int i = 0; i < 16; i++) {
        JOYINFO joyinfo;
        ret = joyGetPos(i, &joyinfo);
        if (ret != JOYERR_NOERROR) {
            continue;
        }
        ret = joyGetDevCaps(i, &joycaps, sizeof(JOYCAPS));
        if (ret != JOYERR_NOERROR) {
            continue;
        }

        joyId = i; //找到手把
        break;
    }
    if (joyId < 0) {
        return -1;
    }

    *buttonNum = joycaps.wNumButtons;
    //印出手把資訊
    printf("--find joystick %d\n", joyId);
    printf("name:%S\nbuttons:%d axes:%d\n", joycaps.szPname, joycaps.wNumButtons, joycaps.wNumAxes);
    printf("caps %x\n", joycaps.wCaps);
    printf("has POV %x\n", joycaps.wCaps & JOYCAPS_HASPOV);
    printf("POV is direction %x\n", joycaps.wCaps & JOYCAPS_POV4DIR);
    printf("POV is degree %x\n", joycaps.wCaps & JOYCAPS_POVCTS);
    return joyId;
}
int repov = 0;
POINT point;
bool msmode = 0;
int words[7] = { 0 };
int max = 0, min = 65536;
//手把拔出則傳回0，此時要重新尋找手把
static int readJoystickState(int joyId, int buttonNum, JoystickState* prevState) {
    MMRESULT ret;
    JOYINFOEX joyinfo;
    joyinfo.dwSize = sizeof(JOYINFOEX);
    joyinfo.dwFlags = JOY_RETURNALL;
    ret = joyGetPosEx(joyId, &joyinfo);
    if (ret != JOYERR_NOERROR) { //手把拔出，重新搜尋
        printf("--joystick is removed\n");
        return 0;
    }

    //跟前一個frame的狀態比對，找出有變化的輸入
    //按鈕
    
    DWORD bitFlag = 1;
    DWORD changedButtons = prevState->button ^ joyinfo.dwButtons;
    
    for (int i = 1; i <= buttonNum; i++, bitFlag = bitFlag << 1) {
        if (!(changedButtons & bitFlag)) { continue; }
        if (joyinfo.dwButtons & bitFlag) {
            printf("button %d: press\n", i);
            if(i == 1)
                keybd_event(VK_MENU, 0, 0, 0);// Alt键
            if(i == 2)
                keybd_event(VK_ESCAPE, 0, 0, 0);// Escape键
            if(i == 3)
                keybd_event(VK_TAB, 0, 0, 0);// Tab键
            if (i == 4)
                msmode = !msmode;
            if(i == 5)
                keybd_event(VK_SHIFT, 0, 0, 0);//shift鍵
            if(i == 6)
                keybd_event(VK_BACK, 0, 0, 0);//back鍵
            if(i == 7)
                keybd_event(VK_SPACE, 0, 0, 0);//space鍵
            if(i == 8)
                keybd_event(VK_RETURN, 0, 0, 0);//enter鍵    
            if (i == 11)
                ::mouse_event(MOUSEEVENTF_LEFTDOWN, point.x, point.y, 0, 0);
            if (i == 12)
                ::mouse_event(MOUSEEVENTF_RIGHTDOWN, point.x, point.y, 0, 0);
        }
        else {
            printf("button %d: release\n", i);
            if(i == 1)
                keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);// Alt键
            if(i == 2)
                keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);// Escape键
            if(i == 3)
                keybd_event(VK_TAB, 0, KEYEVENTF_KEYUP, 0);// Tab键
            if(i == 5)
                keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);//shift鍵
            if(i == 6)
                keybd_event(VK_BACK, 0, KEYEVENTF_KEYUP, 0);//back鍵
            if(i == 7)
                keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);//space鍵
            if(i == 8)
                keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);//enter鍵  
            if (i == 11)
                ::mouse_event(MOUSEEVENTF_LEFTUP, point.x, point.y, 0, 0);
            if (i == 12)
                ::mouse_event(MOUSEEVENTF_RIGHTUP, point.x, point.y, 0, 0);
        }
    }
    //軸
    DWORD* joyinfoAxis = &joyinfo.dwXpos;
    if (msmode) {
        int x, y;
        ::GetCursorPos(&point);
        if (joyinfoAxis[2] > 13683 && joyinfoAxis[2] < 49151 && joyinfoAxis[3]>13683 && joyinfoAxis[3] < 49151)
        {
            x = joyinfoAxis[2] / 1600 - 20;
            y = joyinfoAxis[3] / 1600 - 20;
        }
        else
        {
            x = joyinfoAxis[2] / 800 - 41;
            y = joyinfoAxis[3] / 800 - 41;
        }
        if (x == -1 && y == -1)
            x++, y++;
        point.x += x;
        point.y += y;
        //printf("mos: %d %d\n", x, y);
        ::SetCursorPos(point.x,point.y);
        int wheel = joyinfoAxis[1] / 400 - 82;
        if (wheel == -1)
            wheel = 0;
        ::mouse_event(MOUSEEVENTF_WHEEL, point.x, point.y, wheel, 0);
    }
    for (int i = 0; i < 6; i++) {
        if (prevState->axis[i] != joyinfoAxis[i]) {
            //printf("axis %d: %d\n", i, joyinfoAxis[i]);
            
            int axis = joyinfoAxis[i];
            if(!msmode)
            {
                bool change = 0;
                if (axis < 2494 && words[i] != 1)
                {
                    words[i] = 1;
                    change = 1;
                }
                if (axis > 2494 && axis < 20227 && words[i] != 2)
                {
                    words[i] = 2;
                    change = 1;
                }
                if (axis > 20227 && axis < 45036 && words[i] != 3)
                {
                    words[i] = 3;
                    change = 1;
                }
                if (axis > 45036 && axis < 63039 && words[i] != 4)
                {
                    words[i] = 4;
                    change = 1;
                }
                if (axis > 63039 && words[i] != 5)
                {
                    words[i] = 5;
                    change = 1;
                }


                if (change) {
                    if (words[0] == 3 && words[1] == 1 && words[2] == 3 && words[3] == 1) {
                        keybd_event('T', 0, 0, 0);
                        keybd_event('T', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 4 && words[1] == 2 && words[2] == 3 && words[3] == 1) {
                        keybd_event('B', 0, 0, 0);
                        keybd_event('B', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 5 && words[1] == 3 && words[2] == 3 && words[3] == 1) {
                        keybd_event('C', 0, 0, 0);
                        keybd_event('C', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 4 && words[1] == 4 && words[2] == 3 && words[3] == 1) {
                        keybd_event('D', 0, 0, 0);
                        keybd_event('D', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 3 && words[1] == 5 && words[2] == 3 && words[3] == 1) {
                        keybd_event('F', 0, 0, 0);
                        keybd_event('F', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 2 && words[1] == 4 && words[2] == 3 && words[3] == 1) {
                        keybd_event('G', 0, 0, 0);
                        keybd_event('G', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 1 && words[1] == 3 && words[2] == 3 && words[3] == 1) {
                        keybd_event('1', 0, 0, 0);
                        keybd_event('1', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 2 && words[1] == 2 && words[2] == 3 && words[3] == 1) {
                        keybd_event('8', 0, 0, 0);
                        keybd_event('8', 0, KEYEVENTF_KEYUP, 0);
                    }

                    if (words[0] == 3 && words[1] == 1 && words[2] == 2 && words[3] == 2) {
                        keybd_event('H', 0, 0, 0);
                        keybd_event('H', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 4 && words[1] == 2 && words[2] == 2 && words[3] == 2) {
                        keybd_event('J', 0, 0, 0);
                        keybd_event('J', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 5 && words[1] == 3 && words[2] == 2 && words[3] == 2) {
                        keybd_event('K', 0, 0, 0);
                        keybd_event('K', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 4 && words[1] == 4 && words[2] == 2 && words[3] == 2) {
                        keybd_event('L', 0, 0, 0);
                        keybd_event('L', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 3 && words[1] == 5 && words[2] == 2 && words[3] == 2) {
                        keybd_event('M', 0, 0, 0);
                        keybd_event('M', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 2 && words[1] == 4 && words[2] == 2 && words[3] == 2) {
                        keybd_event('N', 0, 0, 0);
                        keybd_event('N', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 1 && words[1] == 3 && words[2] == 2 && words[3] == 2) {
                        keybd_event('2', 0, 0, 0);
                        keybd_event('2', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 2 && words[1] == 2 && words[2] == 2 && words[3] == 2) {
                        keybd_event('7', 0, 0, 0);
                        keybd_event('7', 0, KEYEVENTF_KEYUP, 0);
                    }

                    if (words[0] == 3 && words[1] == 1 && words[2] == 1 && words[3] == 3) {
                        keybd_event('E', 0, 0, 0);
                        keybd_event('E', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 4 && words[1] == 2 && words[2] == 1 && words[3] == 3) {
                        keybd_event('A', 0, 0, 0);
                        keybd_event('A', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 5 && words[1] == 3 && words[2] == 1 && words[3] == 3) {
                        keybd_event('I', 0, 0, 0);
                        keybd_event('I', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 4 && words[1] == 4 && words[2] == 1 && words[3] == 3) {
                        keybd_event('O', 0, 0, 0);
                        keybd_event('O', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 3 && words[1] == 5 && words[2] == 1 && words[3] == 3) {
                        keybd_event('U', 0, 0, 0);
                        keybd_event('U', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 2 && words[1] == 4 && words[2] == 1 && words[3] == 3) {
                        keybd_event(VK_CONTROL, 0, 0, 0);
                        keybd_event('C', 0, 0, 0);
                        keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event('C', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 1 && words[1] == 3 && words[2] == 1 && words[3] == 3) {
                        keybd_event('3', 0, 0, 0);
                        keybd_event('3', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 2 && words[1] == 2 && words[2] == 1 && words[3] == 3) {
                        keybd_event('6', 0, 0, 0);
                        keybd_event('6', 0, KEYEVENTF_KEYUP, 0);
                    }

                    if (words[0] == 3 && words[1] == 1 && words[2] == 2 && words[3] == 4) {
                        keybd_event('R', 0, 0, 0);
                        keybd_event('R', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 4 && words[1] == 2 && words[2] == 2 && words[3] == 4) {
                        keybd_event('P', 0, 0, 0);
                        keybd_event('P', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 5 && words[1] == 3 && words[2] == 2 && words[3] == 4) {
                        keybd_event('Q', 0, 0, 0);
                        keybd_event('Q', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 4 && words[1] == 4 && words[2] == 2 && words[3] == 4) {
                        keybd_event('S', 0, 0, 0);
                        keybd_event('S', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 3 && words[1] == 5 && words[2] == 2 && words[3] == 4) {
                        keybd_event('T', 0, 0, 0);
                        keybd_event('T', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 2 && words[1] == 4 && words[2] == 2 && words[3] == 4) {
                        keybd_event(VK_CONTROL, 0, 0, 0);
                        keybd_event('V', 0, 0, 0);
                        keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 1 && words[1] == 3 && words[2] == 2 && words[3] == 4) {
                        keybd_event('4', 0, 0, 0);
                        keybd_event('4', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 2 && words[1] == 2 && words[2] == 2 && words[3] == 4) {
                        keybd_event('5', 0, 0, 0);
                        keybd_event('5', 0, KEYEVENTF_KEYUP, 0);
                    }

                    if (words[0] == 3 && words[1] == 1 && words[2] == 3 && words[3] == 5) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event('1', 0, 0, 0);
                        keybd_event('1', 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 4 && words[1] == 2 && words[2] == 3 && words[3] == 5) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_2, 0, 0, 0);
                        keybd_event(VK_OEM_2, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 5 && words[1] == 3 && words[2] == 3 && words[3] == 5) {
                        keybd_event(VK_OEM_COMMA, 0, 0, 0);
                        keybd_event(VK_OEM_COMMA, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 4 && words[1] == 4 && words[2] == 3 && words[3] == 5) {
                        keybd_event(VK_OEM_PERIOD, 0, 0, 0);
                        keybd_event(VK_OEM_PERIOD, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 3 && words[1] == 5 && words[2] == 3 && words[3] == 5) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_3, 0, 0, 0);
                        keybd_event(VK_OEM_3, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 2 && words[1] == 4 && words[2] == 3 && words[3] == 5) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_MINUS, 0, 0, 0);
                        keybd_event(VK_OEM_MINUS, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 1 && words[1] == 3 && words[2] == 3 && words[3] == 5) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_1, 0, 0, 0);
                        keybd_event(VK_OEM_1, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 2 && words[1] == 2 && words[2] == 3 && words[3] == 5) {
                        keybd_event(VK_OEM_1, 0, 0, 0);
                        keybd_event(VK_OEM_1, 0, KEYEVENTF_KEYUP, 0);
                    }

                    if (words[0] == 3 && words[1] == 1 && words[2] == 4 && words[3] == 4) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event('9', 0, 0, 0);
                        keybd_event('9', 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 4 && words[1] == 2 && words[2] == 4 && words[3] == 4) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event('0', 0, 0, 0);
                        keybd_event('0', 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 5 && words[1] == 3 && words[2] == 4 && words[3] == 4) {
                        keybd_event(VK_OEM_4, 0, 0, 0);
                        keybd_event(VK_OEM_4, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 4 && words[1] == 4 && words[2] == 4 && words[3] == 4) {
                        keybd_event(VK_OEM_6, 0, 0, 0);
                        keybd_event(VK_OEM_6, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 3 && words[1] == 5 && words[2] == 4 && words[3] == 4) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_4, 0, 0, 0);
                        keybd_event(VK_OEM_4, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 2 && words[1] == 4 && words[2] == 4 && words[3] == 4) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_6, 0, 0, 0);
                        keybd_event(VK_OEM_6, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 1 && words[1] == 3 && words[2] == 4 && words[3] == 4) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_COMMA, 0, 0, 0);
                        keybd_event(VK_OEM_COMMA, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 2 && words[1] == 2 && words[2] == 4 && words[3] == 4) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_PERIOD, 0, 0, 0);
                        keybd_event(VK_OEM_PERIOD, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }

                    if (words[0] == 3 && words[1] == 1 && words[2] == 5 && words[3] == 3) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_PLUS, 0, 0, 0);
                        keybd_event(VK_OEM_PLUS, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 4 && words[1] == 2 && words[2] == 5 && words[3] == 3) {
                        keybd_event(VK_OEM_MINUS, 0, 0, 0);
                        keybd_event(VK_OEM_MINUS, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 5 && words[1] == 3 && words[2] == 5 && words[3] == 3) {
                        keybd_event(VK_MULTIPLY, 0, 0, 0);
                        keybd_event(VK_MULTIPLY, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 4 && words[1] == 4 && words[2] == 5 && words[3] == 3) {
                        keybd_event(VK_OEM_2, 0, 0, 0);
                        keybd_event(VK_OEM_2, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 3 && words[1] == 5 && words[2] == 5 && words[3] == 3) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event('7', 0, 0, 0);
                        keybd_event('7', 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 2 && words[1] == 4 && words[2] == 5 && words[3] == 3) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_5, 0, 0, 0);
                        keybd_event(VK_OEM_5, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 1 && words[1] == 3 && words[2] == 5 && words[3] == 3) {
                        keybd_event(VK_OEM_7, 0, 0, 0);
                        keybd_event(VK_OEM_7, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 2 && words[1] == 2 && words[2] == 5 && words[3] == 3) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_7, 0, 0, 0);
                        keybd_event(VK_OEM_7, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }

                    if (words[0] == 3 && words[1] == 1 && words[2] == 4 && words[3] == 2) {
                        keybd_event('V', 0, 0, 0);
                        keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 4 && words[1] == 2 && words[2] == 4 && words[3] == 2) {
                        keybd_event('W', 0, 0, 0);
                        keybd_event('W', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 5 && words[1] == 3 && words[2] == 4 && words[3] == 2) {
                        keybd_event('X', 0, 0, 0);
                        keybd_event('X', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 4 && words[1] == 4 && words[2] == 4 && words[3] == 2) {
                        keybd_event('Y', 0, 0, 0);
                        keybd_event('Y', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 3 && words[1] == 5 && words[2] == 4 && words[3] == 2) {
                        keybd_event('Z', 0, 0, 0);
                        keybd_event('Z', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 2 && words[1] == 4 && words[2] == 4 && words[3] == 2) {
                        keybd_event('#', 0, 0, 0);
                        keybd_event('#', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 1 && words[1] == 3 && words[2] == 4 && words[3] == 2) {
                        keybd_event('0', 0, 0, 0);
                        keybd_event('0', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (words[0] == 2 && words[1] == 2 && words[2] == 4 && words[3] == 2) {
                        keybd_event('9', 0, 0, 0);
                        keybd_event('9', 0, KEYEVENTF_KEYUP, 0);
                    }


                    if (i == 1)
                    {
                        printf("axis %d\n", i);
                        printf("code: %d\n", words[i]);
                    }
                }
            }
        }
    }
    //視覺頭盔
    if (prevState->pov != joyinfo.dwPOV) {
        printf("POV: %u\n", joyinfo.dwPOV);
        int pov = joyinfo.dwPOV;
        /*if (repov == 0) {
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
            keybd_event('Z', 0, KEYEVENTF_KEYUP, 0);
        }
        keybd_event(VK_RWIN, 0, KEYEVENTF_KEYUP, 0);
        keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
        keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
        if (repov == 18000)
            keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
        */if (pov == 22500) {
            keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
        }

        if (pov == 27000)
            keybd_event(VK_CONTROL, 0, 0, 0);
        else
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);

        if (pov == 0) {
            keybd_event(VK_CONTROL, 0, 0, 0);
            keybd_event('Z', 0, 0, 0);
        }
        else {
            //keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
            keybd_event('Z', 0, KEYEVENTF_KEYUP, 0);
        }
        if (pov == 9000) {
            keybd_event(VK_LWIN, 0, 0, 0);
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
        }
        else
            keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
        if(pov == 13500) {
            keybd_event(VK_LWIN, 0, 0, 0);
            keybd_event(VK_CAPITAL, 0, 0, 0);
        }
        else {
            keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
            keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
        }
        if(pov == 18000)
            keybd_event(VK_CAPITAL, 0, 0, 0);
        else
            keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
        if (pov == 22500) {
            keybd_event(VK_CAPITAL, 0, 0, 0);
            keybd_event(VK_CONTROL, 0, 0, 0);
        }
        else {
            keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
            //keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
        }
        repov = pov;
    }

    //把這個frame的狀態保存
    prevState->button = joyinfo.dwButtons;
    prevState->pov = joyinfo.dwPOV;
    memcpy(prevState->axis, joyinfoAxis, sizeof(DWORD) * 6);
    return 1;
}

int main() {
    setlocale(LC_ALL, "cht"); //讓printf可以印出中文
    int joyId = -1;
    int buttonNum;
    for (;; Sleep(SLEEP_INTERVAL)) {
        //一、尋找手把
        joyId = findJoystick(&buttonNum);
        if (joyId == -1) { //找不到手把
            continue;
        }

        //二、讀取手把輸入
        JoystickState prevState; //用來記錄前一個frame的狀態
        ZeroMemory(&prevState, sizeof(JoystickState));
        //手把拔出才離開這個迴圈
        while (readJoystickState(joyId, buttonNum, &prevState)) {
            Sleep(SLEEP_INTERVAL);
        }
        joyId = -1;
    }
    return 0;
}
