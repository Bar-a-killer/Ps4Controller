#ifndef UNICODE
#define UNICODE
#endif
#include<windows.h>
#include<stdio.h>
#include<locale.h>
#include<cmath>
#include<xinput.h>
const int SLEEP_INTERVAL = 13; //75+ FPS

typedef struct {
    DWORD button;
    DWORD axis[6];
    DWORD pov;
} JoystickState;

//函式傳回手把編號
//buttonNum傳回按鈕數，讀取手把輸入時會用到

//這個function用於check手把有沒有接上
static int findJoystick(int* buttonNum) {
    int joyId = -1;
    JOYCAPS joycaps;
    MMRESULT ret;

    //XINPUT_STATE controllerState;
    /*for (int i = 0; i < 20; i++) {
        DWORD result = XInputGetState(i, &controllerState); // 檢查第一個控制器的狀態
        if (result == ERROR_SUCCESS) {
            // 控制器已連接
            XINPUT_VIBRATION vibration;
            vibration.wLeftMotorSpeed = 65535; // 左震動馬達的強度
            vibration.wRightMotorSpeed = 65535; // 右震動馬達的強度

            DWORD result = XInputSetState(0, &vibration);
            printf("Connect!!");
        }
        else
            printf("NO!%d",i);
    }*/
    /*上面那串註解希望以後能用*/

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


//-----------下面會用到的變數們-------------//
int repov = 0; //其實應該沒什麼用了 但俗話說:「程式沒bug就不要動它」
POINT point;  //滑鼠操作會用來裝滑鼠狀態的變數
bool msmode = 0; //分辨是不是滑鼠模式
int words[7] = { 0 }; //其實應該沒什麼用了 但俗話說:「程式沒bug就不要動它」
int Lmode = 0,Rmode = 0; //分辨左右的方向
//-----------下面會用到的變數們-------------//

//---------------------主要函式(我的工作區域，其他幾乎都是抄來的)-------------------//
//手把拔出則傳回0，此時要重新尋找手把
static int readJoystickState(int joyId, int buttonNum, JoystickState* prevState) {

    //////////////////////////////////////////////////////(等等出現一長串/就是我也懶得看懂)
    MMRESULT ret;
    JOYINFOEX joyinfo;
    joyinfo.dwSize = sizeof(JOYINFOEX);
    joyinfo.dwFlags = JOY_RETURNALL;
    ret = joyGetPosEx(joyId, &joyinfo);
    if (ret != JOYERR_NOERROR) { //手把拔出，重新搜尋
        printf("--joystick is removed\n");
        return 0;
    }
    /////////////////////////////////////////////////////
    
    //跟前一個frame的狀態比對，找出有變化的輸入
    
    
    //++++++++++++++++++++++按鈕+++++++++++++++++++++++//
    DWORD bitFlag = 1;
    DWORD changedButtons = prevState->button ^ joyinfo.dwButtons;
    //每個不同的按鍵的i都不同
    for (int i = 1; i <= buttonNum; i++, bitFlag = bitFlag << 1) {
        if (!(changedButtons & bitFlag)) { continue; }
        if (joyinfo.dwButtons & bitFlag) {
            printf("button %d: press\n", i);
            if (i == 1)
                keybd_event(VK_MENU, 0, 0, 0);// Alt键
            if (i == 2)
                keybd_event(VK_ESCAPE, 0, 0, 0);// Escape键
            if (i == 3)
                keybd_event(VK_TAB, 0, 0, 0);// Tab键
            if (i == 4) 
                msmode = !msmode;
            if (i == 5) {
                if(msmode)
                    ::mouse_event(MOUSEEVENTF_LEFTDOWN, point.x, point.y, 0, 0);
                else
                    keybd_event(VK_SHIFT, 0, 0, 0);//shift鍵
            }
                
            if (i == 6) {
                if(msmode)
                    ::mouse_event(MOUSEEVENTF_RIGHTDOWN, point.x, point.y, 0, 0);
                else
                    keybd_event(VK_BACK, 0, 0, 0);//back鍵
            }
                
            if (i == 7)
                keybd_event(VK_SPACE, 0, 0, 0);//space鍵
            if (i == 8)
                keybd_event(VK_RETURN, 0, 0, 0);//enter鍵    
            if (i == 11)
                ::mouse_event(MOUSEEVENTF_LEFTDOWN, point.x, point.y, 0, 0);
            if (i == 12)
                ::mouse_event(MOUSEEVENTF_RIGHTDOWN, point.x, point.y, 0, 0);
        }
        else {
            printf("button %d: release\n", i);
            if (i == 1)
                keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);// Alt键
            if (i == 2)
                keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);// Escape键
            if (i == 3)
                keybd_event(VK_TAB, 0, KEYEVENTF_KEYUP, 0);// Tab键
            if (i == 5) {
                if(msmode)
                    ::mouse_event(MOUSEEVENTF_LEFTUP, point.x, point.y, 0, 0);
                else
                    keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);//shift鍵
            }
                
            if (i == 6) {
                if(msmode)
                    ::mouse_event(MOUSEEVENTF_RIGHTUP, point.x, point.y, 0, 0);
                else
                    keybd_event(VK_BACK, 0, KEYEVENTF_KEYUP, 0);//back鍵
            }
                
            if (i == 7)
                keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);//space鍵
            if (i == 8)
                keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);//enter鍵  
            if (i == 11)
                ::mouse_event(MOUSEEVENTF_LEFTUP, point.x, point.y, 0, 0);
            if (i == 12)
                ::mouse_event(MOUSEEVENTF_RIGHTUP, point.x, point.y, 0, 0);
        }
    }
    //++++++++++++++++++++++按鈕+++++++++++++++++++++++//
    


    //++++++++++++++++++++++軸+++++++++++++++++++++++++//
    DWORD* joyinfoAxis = &joyinfo.dwXpos;
    if (msmode) {
        int x, y;
        int Ynegative = 1,Xnegative = 1;
        ::GetCursorPos(&point);
        x = joyinfoAxis[2] - 32767;
        y = joyinfoAxis[3] - 32767;
        if (x < 0) {
            Xnegative = -1;
            x *= -1;
        }
        if (y < 0) {
            Ynegative = -1;
            y *= -1;
        }
        if (x < 13683 && x > -1 && y < 13683 && y > -1)
        {
            x = x / 3200; x*= Xnegative;
            y = y / 3200; y*= Ynegative;
        }
        else
        {
            x = x / 1600; x*= Xnegative;
            y = y / 1600; y*= Ynegative;
        }
        point.x += x;
        point.y += y;
        //printf("mos: %d %d\n", x, y);
        ::SetCursorPos(point.x, point.y);
        int Znegetive = 1;
        int wheel = joyinfoAxis[1] - 32767;

        if (wheel < 0) {
            Znegetive = -1;
            wheel *= -1;
        }
        
        wheel /= 400; wheel *= Znegetive;
        if (wheel > -10 && wheel < 10)
            wheel = 0;
        ::mouse_event(MOUSEEVENTF_WHEEL, point.x, point.y, wheel, 0);
    }
    for (int i = 0; i < 6; i+=2) {
        if (prevState->axis[i] != joyinfoAxis[i]) {
            //printf("axis X: %d\n", i, 2 * static_cast<long long>(joyinfoAxis[i]) - 65535);
            //printf("axis Y: %d\n", -2 * static_cast<long long>(joyinfoAxis[i + 1]) + 65535);
            //printf("moved");
            int axis = joyinfoAxis[i];
            if (!msmode)
            {
                bool change = 0;
                long long Xaxis = 2*static_cast<long long>(joyinfoAxis[i]) - 65535;
                long long Yaxis = 2*static_cast<long long>(joyinfoAxis[i + 1]) - 65535;
                Yaxis *= -1;
                long long moverange = 66000;
                long long moverange2 = 50000;
                int lr = 1;
                //printf("axis Y: %d\n", Xaxis * Xaxis + Yaxis * Yaxis);
                //printf("X: %d Y: %d\n", Xaxis, Yaxis);
                if (Xaxis * 2   <= 5 * Yaxis && Xaxis * 12  >  5 * Yaxis) {
                    if (i == 0 && Lmode != 2) {
                        Lmode = 2;
                        change = 1;
                        //printf("Lmode 2");
                    }
                    if (i == 2 && Rmode != 2) {
                        Rmode = 2;
                        change = 1;
                        //printf("Rmode 2");
                    }
                }
                if (Xaxis * -2  <= 5 * Yaxis && Xaxis * 2   >  5 * Yaxis) {
                    if (i == 0 && Lmode != 3) {
                        Lmode = 3;
                        change = 1;
                        //printf("Lmode 3");
                    }
                    if (i == 2 && Rmode != 3) {
                        Rmode = 3;
                        change = 1;
                        //printf("Rmode 3");
                    }
                    moverange = 200;
                    lr = 2;
                }
                if (Xaxis * -12 <= 5 * Yaxis && Xaxis * -2  >  5 * Yaxis) {
                    if (i == 0 && Lmode != 4) {
                        Lmode = 4;
                        change = 1;
                        //printf("Lmode 4");
                    }
                    if (i == 2 && Rmode != 4) {
                        Rmode = 4;
                        change = 1;
                        //printf("Rmode 4");
                    }
                }
                
                if (Xaxis * 12  <= 5 * Yaxis && Xaxis * 2   >  5 * Yaxis) {
                    if (i == 0 && Lmode != 6) {
                        Lmode = 6;
                        change = 1;
                        //printf("Lmode 6");
                    }
                    if (i == 2 && Rmode != 6) {
                        Rmode = 6;
                        change = 1;
                        //printf("Rmode 6");
                    }
                }
                if (Xaxis * 2   <= 5 * Yaxis && Xaxis * -2  >  5 * Yaxis) {
                    if (i == 0 && Lmode != 7) {
                        Lmode = 7;
                        change = 1;
                        //printf("Lmode 7");
                    }
                    if (i == 2 && Rmode != 7) {
                        Rmode = 7;
                        change = 1;
                        //printf("Rmode 7");
                    }
                    moverange = moverange2;
                    lr = 2;
                }
                if (Xaxis * -2  <= 5 * Yaxis && Xaxis * -12 >  5 * Yaxis) {
                    if (i == 0 && Lmode != 8) {
                        Lmode = 8;
                        change = 1;
                        //printf("Lmode 8");
                    }
                    if (i == 2 && Rmode != 8) {
                        Rmode = 8;
                        change = 1;
                        //printf("Rmode 8");
                    }
                }
                
                if (Xaxis < 27000 && Xaxis > -27000 && Yaxis > 0) {
                    if (i == 0 && Lmode != 1) {
                        Lmode = 1;
                        change = 1;
                        //printf("Lmode 1");
                    }
                    if (i == 2 && Rmode != 1) {
                        Rmode = 1;
                        change = 1;
                        //printf("Rmode 1");
                    }
                    lr = 3;
                }
                if (Xaxis < 27000 && Xaxis > -27000 && Yaxis < 0) {
                    if (i == 0 && Lmode != 5) {
                        Lmode = 5;
                        change = 1;
                        //printf("Lmode 5");
                    }
                    if (i == 2 && Rmode != 5) {
                        Rmode = 5;
                        change = 1;
                        //printf("Rmode 5");
                    }
                    lr = 3;
                }

                if (lr == 1 && Xaxis * Xaxis + Yaxis * Yaxis < static_cast<long long>(moverange) * moverange) {
                    if (i == 0) {
                        Lmode = 0;
                    }
                    if (i == 2 && Rmode != 3) {
                        Rmode = 0;
                    }
                    //printf("X\n");
                    continue;
                }
                if (lr == 2 && Xaxis < 50000 && Xaxis > -50000) {
                    if (i == 0) {
                        Lmode = 0;
                        //printf("X\n");
                        continue;
                    }
                    if (i == 2) {
                        Rmode = 0;
                        //printf("X\n");
                        continue;
                    }
                }
                if (lr == 3 && Yaxis < 50000 && Yaxis > -50000) {
                    if (i == 0) {
                        Lmode = 0;
                        //printf("X\n");
                        continue;
                    }
                    if (i == 2) {
                        Rmode = 0;
                        //printf("X\n");
                        continue;
                    }
                }
                
                /*if (axis < 2494 && words[i] != 1)
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
                */
                
                
                if (change) {
                    
                    if (i == 0) {
                        printf("\nLmode %d\n", Lmode);
                    }
                    if (i == 2 ) {
                        printf("\nRmode %d\n", Rmode);
                    }

                    //printf("\n");

                    if (Lmode == 1 && Rmode == 1) {
                        keybd_event('T', 0, 0, 0);
                        keybd_event('T', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 2 && Rmode == 1) {
                        keybd_event('B', 0, 0, 0);
                        keybd_event('B', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 3 && Rmode == 1) {
                        keybd_event('C', 0, 0, 0);
                        keybd_event('C', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 4 && Rmode == 1) {
                        keybd_event('D', 0, 0, 0);
                        keybd_event('D', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 5 && Rmode == 1) {
                        keybd_event('F', 0, 0, 0);
                        keybd_event('F', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 6 && Rmode == 1) {
                        keybd_event('G', 0, 0, 0);
                        keybd_event('G', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 7 && Rmode == 1) {
                        keybd_event('1', 0, 0, 0);
                        keybd_event('1', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 8 && Rmode == 1) {
                        keybd_event('8', 0, 0, 0);
                        keybd_event('8', 0, KEYEVENTF_KEYUP, 0);
                    }

                    if (Lmode == 1 && Rmode == 8) {
                        keybd_event('H', 0, 0, 0);
                        keybd_event('H', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 2 && Rmode == 8) {
                        keybd_event('J', 0, 0, 0);
                        keybd_event('J', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 3 && Rmode == 8) {
                        keybd_event('K', 0, 0, 0);
                        keybd_event('K', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 4 && Rmode == 8) {
                        keybd_event('L', 0, 0, 0);
                        keybd_event('L', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 5 && Rmode == 8) {
                        keybd_event('M', 0, 0, 0);
                        keybd_event('M', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 6 && Rmode == 8) {
                        keybd_event('N', 0, 0, 0);
                        keybd_event('N', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 7 && Rmode == 8) {
                        keybd_event('2', 0, 0, 0);
                        keybd_event('2', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 8 && Rmode == 8) {
                        keybd_event('7', 0, 0, 0);
                        keybd_event('7', 0, KEYEVENTF_KEYUP, 0);
                    }

                    if (Lmode == 1 && Rmode == 7) {
                        keybd_event('E', 0, 0, 0);
                        keybd_event('E', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 2 && Rmode == 7) {
                        keybd_event('A', 0, 0, 0);
                        keybd_event('A', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 3 && Rmode == 7) {
                        keybd_event('I', 0, 0, 0);
                        keybd_event('I', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 4 && Rmode == 7) {
                        keybd_event('O', 0, 0, 0);
                        keybd_event('O', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 5 && Rmode == 7) {
                        keybd_event('U', 0, 0, 0);
                        keybd_event('U', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 6 && Rmode == 7) {
                        keybd_event(VK_CONTROL, 0, 0, 0);
                        keybd_event('C', 0, 0, 0);
                        keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event('C', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 7 && Rmode == 7) {
                        keybd_event('3', 0, 0, 0);
                        keybd_event('3', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 8 && Rmode == 7) {
                        keybd_event('6', 0, 0, 0);
                        keybd_event('6', 0, KEYEVENTF_KEYUP, 0);
                    }

                    if (Lmode == 1 && Rmode == 6) {
                        keybd_event('R', 0, 0, 0);
                        keybd_event('R', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 2 && Rmode == 6) {
                        keybd_event('P', 0, 0, 0);
                        keybd_event('P', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 3 && Rmode == 6) {
                        keybd_event('Q', 0, 0, 0);
                        keybd_event('Q', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 4 && Rmode == 6) {
                        keybd_event('S', 0, 0, 0);
                        keybd_event('S', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 5 && Rmode == 6) {
                        keybd_event('T', 0, 0, 0);
                        keybd_event('T', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 6 && Rmode == 6) {
                        keybd_event(VK_CONTROL, 0, 0, 0);
                        keybd_event('V', 0, 0, 0);
                        keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 7 && Rmode == 6) {
                        keybd_event('4', 0, 0, 0);
                        keybd_event('4', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 8 && Rmode == 6) {
                        keybd_event('5', 0, 0, 0);
                        keybd_event('5', 0, KEYEVENTF_KEYUP, 0);
                    }

                    if (Lmode == 1 && Rmode == 5) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event('1', 0, 0, 0);
                        keybd_event('1', 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 2 && Rmode == 5) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_2, 0, 0, 0);
                        keybd_event(VK_OEM_2, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 3 && Rmode == 5) {
                        keybd_event(VK_OEM_COMMA, 0, 0, 0);
                        keybd_event(VK_OEM_COMMA, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 4 && Rmode == 5) {
                        keybd_event(VK_OEM_PERIOD, 0, 0, 0);
                        keybd_event(VK_OEM_PERIOD, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 5 && Rmode == 5) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_3, 0, 0, 0);
                        keybd_event(VK_OEM_3, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 6 && Rmode == 5) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_MINUS, 0, 0, 0);
                        keybd_event(VK_OEM_MINUS, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 7 && Rmode == 5) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_1, 0, 0, 0);
                        keybd_event(VK_OEM_1, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 8 && Rmode == 5) {
                        keybd_event(VK_OEM_1, 0, 0, 0);
                        keybd_event(VK_OEM_1, 0, KEYEVENTF_KEYUP, 0);
                    }

                    if (Lmode == 1 && Rmode == 4) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event('9', 0, 0, 0);
                        keybd_event('9', 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 2 && Rmode == 4) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event('0', 0, 0, 0);
                        keybd_event('0', 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 3 && Rmode == 4) {
                        keybd_event(VK_OEM_4, 0, 0, 0);
                        keybd_event(VK_OEM_4, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 4 && Rmode == 4) {
                        keybd_event(VK_OEM_6, 0, 0, 0);
                        keybd_event(VK_OEM_6, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 5 && Rmode == 4) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_4, 0, 0, 0);
                        keybd_event(VK_OEM_4, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 6 && Rmode == 4) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_6, 0, 0, 0);
                        keybd_event(VK_OEM_6, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 7 && Rmode == 4) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_COMMA, 0, 0, 0);
                        keybd_event(VK_OEM_COMMA, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 8 && Rmode == 4) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_PERIOD, 0, 0, 0);
                        keybd_event(VK_OEM_PERIOD, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }

                    if (Lmode == 1 && Rmode == 3) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_PLUS, 0, 0, 0);
                        keybd_event(VK_OEM_PLUS, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 2 && Rmode == 3) {
                        keybd_event(VK_OEM_MINUS, 0, 0, 0);
                        keybd_event(VK_OEM_MINUS, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 3 && Rmode == 3) {
                        keybd_event(VK_MULTIPLY, 0, 0, 0);
                        keybd_event(VK_MULTIPLY, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 4 && Rmode == 3) {
                        keybd_event(VK_OEM_2, 0, 0, 0);
                        keybd_event(VK_OEM_2, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 5 && Rmode == 3) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event('7', 0, 0, 0);
                        keybd_event('7', 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 6 && Rmode == 3) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_5, 0, 0, 0);
                        keybd_event(VK_OEM_5, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 7 && Rmode == 3) {
                        keybd_event(VK_OEM_7, 0, 0, 0);
                        keybd_event(VK_OEM_7, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 8 && Rmode == 3) {
                        keybd_event(VK_SHIFT, 0, 0, 0);
                        keybd_event(VK_OEM_7, 0, 0, 0);
                        keybd_event(VK_OEM_7, 0, KEYEVENTF_KEYUP, 0);
                        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
                    }

                    if (Lmode == 1 && Rmode == 2) {
                        keybd_event('V', 0, 0, 0);
                        keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 2 && Rmode == 2) {
                        keybd_event('W', 0, 0, 0);
                        keybd_event('W', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 3 && Rmode == 2) {
                        keybd_event('X', 0, 0, 0);
                        keybd_event('X', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 4 && Rmode == 2) {
                        keybd_event('Y', 0, 0, 0);
                        keybd_event('Y', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 5 && Rmode == 2) {
                        keybd_event('Z', 0, 0, 0);
                        keybd_event('Z', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 6 && Rmode == 2) {
                        keybd_event(VK_OEM_3, 0, 0, 0);
                        keybd_event(VK_OEM_3, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 7 && Rmode == 2) {
                        keybd_event('0', 0, 0, 0);
                        keybd_event('0', 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (Lmode == 8 && Rmode == 2) {
                        keybd_event('9', 0, 0, 0);
                        keybd_event('9', 0, KEYEVENTF_KEYUP, 0);
                    }

                    
                    /*if (i == 1)
                    {
                        printf("axis %d\n", i);
                        printf("code: %d\n", words[i]);
                    }*/
                }
            }
        }
    }
    //++++++++++++++++++++++軸+++++++++++++++++++++++++//


    //++++++++++++++++++++++視覺頭盔+++++++++++++++++++//
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
        if (pov == 13500) {
            keybd_event(VK_LWIN, 0, 0, 0);
            keybd_event(VK_CAPITAL, 0, 0, 0);
        }
        else {
            keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
            keybd_event(VK_CAPITAL, 0, KEYEVENTF_KEYUP, 0);
        }
        if (pov == 18000)
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
    //++++++++++++++++++++++視覺頭盔+++++++++++++++++++//
    
   
    //把這個frame的狀態保存
    prevState->button = joyinfo.dwButtons;
    prevState->pov = joyinfo.dwPOV;
    memcpy(prevState->axis, joyinfoAxis, sizeof(DWORD) * 6);
    return 1;
}

////////////////////////////////////////////////////////
int main() {
    setlocale(LC_ALL, "cht"); //讓printf可以印出中文
    int joyId = -1;
    int buttonNum;
    point.x = 0;
    point.y = 0;
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
////////////////////////////////////////////////////////////
