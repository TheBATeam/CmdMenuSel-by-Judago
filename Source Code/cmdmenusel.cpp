#define WIN32_WINNT 0x500
#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>



using namespace std;


bool HexToint(string sToConvert, int& iNout)
{
    stringstream ssConverter;
    ssConverter << hex << sToConvert;
    if ( ssConverter >> iNout )
    {
        return true;
    } else {
        return false;
    }
}



int main(int argc, char *argv[])
{
    if (argc <= 2)
    {
        cout << "CmdMenuSel (V1.4)" << endl << endl
             << "CmdMenuSel displays a line based menu in the console allowing the user" << endl
             << "to select a single option. The menu can be interacted with via the mouse" << endl
             << "or keyboard. The menu colo[u]rs can be choosen or the defaults used(E2F4)." << endl
             << "An exit code(ErrorLevel) is returned denoting the position the selected" << endl
             << "option was specified in." << endl << endl
             << "USAGE:" << endl << endl
             << "CmdMenuSel FBFB {\"Option 1\"} [\"Option 2\"] [....] " << endl << endl << endl
             << "All option string are trimmed to the console width. The number of specified" << endl
             << "options must be less than or equal to the number of lines in the console." << endl << endl
             << "\"FBFB\" denotes the colo[u]rs to be used, they are four hex digits" << endl
             << "(0 - 9 and A - F). The first two digits corrospond to the selected" << endl
             << "menu item and the last two corrospond to the unselected menu items." << endl
             << "\"F\" is the foreground colo[u]r and \"B\" is the background colo[u]r." << endl
             << "See \"COLOR /?\" for colo[u]r code listings." << endl << endl
             << "The default colo[u]rs are used if the:" << endl
             << "  String contains characters outside of 0-9,A-F or a-f." << endl
             << "  String isn't exactly four characters." << endl
             << "  Selected or unselected background and foreground colo[u]rs are the same." << endl
             << "  Selected and unselected background colo[u]rs are the same." << endl << endl

             << "If an error occurs the return code will be zero and a string will be set" << endl
             << "to the console error stream." << endl << endl

             << "Usable input:" << endl << endl
             << "Select:" << endl
             << "SPACE" << endl << "ENTER" << endl << "LEFT_CLICK" << endl << endl
             << "NAVIGATE:" << endl
             << "UP" << endl << "DOWN" << endl << "TAB" << endl << "SHIFT + TAB"
             << endl << "HOME" << endl << "END" << endl << "PAGE_UP" << endl
             << "PAGE_DOWN" << endl << "MOUSE_HOVER" << endl << "SCROLL_WHEEL"
             << endl << endl;
        return 0;
    }

    //variables
    HANDLE hConsoleOutput;
    HANDLE hConsoleInput;
    CONSOLE_SCREEN_BUFFER_INFO sbiConsoleInfo;
    DWORD dwConsoleMode;
    DWORD dwInputEventsRead = 0;
    INPUT_RECORD irUserInput;
    CONSOLE_CURSOR_INFO cciRestore;
    CONSOLE_CURSOR_INFO cciChange;
    COORD coMenuPos;
    coMenuPos.X = 0;
    coMenuPos.Y = 0;
    COORD coWritePos;
    coWritePos.X = 0;
    WORD wSelected = 0;
    WORD wStandard = 0;
    WORD retcode = 10000;
    string sChopper;
    int iHexVal;
    bool bRedraw = true;
    bool bMouseDown = false;
    WORD wMouseDownPos = 0;
    bool bKeyDown = false;
    bool bMouseUntilUp = false;
    WORD wLastMousePos = 0;

    WORD wRectYArea = 0;
    WORD wMenuItems = 0;
    WORD wMenuSel = 0;

    SMALL_RECT srScrollRect;
    COORD coScrollTop;
    coScrollTop.X = 0;
    coScrollTop.Y = 0;
    CHAR_INFO ciScrollFill;
    ciScrollFill.Char.AsciiChar = ' ';

    // ************** Setup Environment ******************
    //output handle
    hConsoleOutput = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hConsoleOutput == INVALID_HANDLE_VALUE)
    {
        cerr << "Could not retrieve console output handle with Createfile()"
             << " Sys Error: " << GetLastError() << endl;
        retcode = 0;
    }
    //input handle
    hConsoleInput = CreateFile("CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hConsoleInput == INVALID_HANDLE_VALUE)
    {
        cerr << "Could not retrieve console input handle with Createfile()"
             << " Sys Error: " << GetLastError() << endl;
        retcode = 0;
    }
    //buffer info
    if (!GetConsoleScreenBufferInfo(hConsoleOutput, &sbiConsoleInfo) )
    {
        cerr << "Could not buffer info with GetConsoleScreenBufferInfo()"
             << " Sys Error: " << GetLastError() << endl;
        retcode = 0;
    }

    //get the console mode for reset on exit
    if (!GetConsoleMode(hConsoleInput, &dwConsoleMode))
    {
        cerr << "Could not get console mode with GetConsoleMode()"
             << " Sys Error: " << GetLastError() << endl;
        retcode = 0;
    }

    //Ensure mouse input, disable CTRL C (so I can always restore the mode)
    // to avoid the console inheriting the changes the mode us reset at the end)
    if (!SetConsoleMode(hConsoleInput, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS))
    {
        cerr << "Could not set console mode with SetConsoleMode()"
             << " Sys Error: " << GetLastError() << endl;
        retcode = 0;
    }

    //Get the cursor info
    if (!GetConsoleCursorInfo(hConsoleOutput, &cciRestore))
    {
        cerr << "Could not get console cursor info with GetConsoleCursorInfo()"
             << " Sys Error: " << GetLastError() << endl;
        retcode = 0;
    }

    //chage the cursor
    cciChange.bVisible = FALSE;
    cciChange.dwSize = 25;
    if (!SetConsoleCursorInfo(hConsoleOutput, &cciChange))
    {
        cerr << "Could set get console cursor info with SetConsoleCursorInfo()"
             << " Sys Error: " << GetLastError() << endl;
        retcode = 0;
    }

    // ******************** End Setup *******************************
    if (argc - 2 > sbiConsoleInfo.dwSize.Y)
    {
        cerr << "The console buffer(" << sbiConsoleInfo.dwSize.Y
             << " lines) is to small for " << argc - 2
             << " options." << endl;
        retcode = 0;
    }

    if (retcode != 0)
    {
        ciScrollFill.Attributes = sbiConsoleInfo.wAttributes;
        coMenuPos.Y = sbiConsoleInfo.dwCursorPosition.Y;
        coWritePos.Y = sbiConsoleInfo.dwCursorPosition.Y;
        srScrollRect.Top = 1;
        srScrollRect.Left = 0;
        srScrollRect.Bottom = sbiConsoleInfo.dwSize.Y - 1;
        srScrollRect.Right = sbiConsoleInfo.dwSize.X - 1;
        wSelected = 0x2e;
        wStandard = 0x4f;
        sChopper = argv[1];
        if (
            sChopper.length() == 4
            && sChopper.substr(0,1) != sChopper.substr(1,1)
            && sChopper.substr(2,1) != sChopper.substr(3,1)
            && sChopper.substr(1,1) != sChopper.substr(3,1)
            )
        {
            if (HexToint(sChopper.substr(1,1), iHexVal))
            {
                wSelected = iHexVal;
                if (HexToint(sChopper.substr(0,1), iHexVal))
                {
                    wSelected = iHexVal + (wSelected << 4);
                } else {
                    wSelected = 0x2e;
                    sChopper.replace(3,1,1,'p');
                }

            } else {
                sChopper.replace(3,1,1,'p');
            }
            if (HexToint(sChopper.substr(3,1), iHexVal))
            {
                wStandard = iHexVal;
                if (HexToint(sChopper.substr(2,1), iHexVal))
                {
                    wStandard = iHexVal + (wStandard << 4);
                } else {
                    wStandard = 0x4f;
                    wSelected = 0x2e;
                }

            } else {
                wSelected = 0x2e;
            }

        }

        char * cOutputStr = new char[sbiConsoleInfo.dwSize.X];
        for (int i = 2; i < argc ; i++)
        {
            wMenuItems++;
            for (int j = 0 ; j < sbiConsoleInfo.dwSize.X ; j++)
            {
                cOutputStr[j] = ' ';
            }
            for (int k = 0 ; k < sbiConsoleInfo.dwSize.X ; k++)
            {
                if (argv[i][k] == '\0')
                {
                    break;
                }
                cOutputStr[k] = argv[i][k];
            }
            if (coWritePos.Y == sbiConsoleInfo.dwSize.Y)
            {
                coWritePos.Y--;
                coMenuPos.Y--;
                if (!ScrollConsoleScreenBuffer(hConsoleOutput,
                    &srScrollRect, NULL, coScrollTop,&ciScrollFill ))
                {
                    cerr << "Could not get scroll console buffer with ScrollConsoleCursorScreenBuffer()"
                         << " Sys Error: " << GetLastError() << endl;
                    retcode = 0;
                    break;
                }
            }
            if (!WriteConsoleOutputCharacter(hConsoleOutput, cOutputStr,
                sbiConsoleInfo.dwSize.X, coWritePos, &dwInputEventsRead))
            {
                cerr << "Could write to console with WriteConsoleOutputCharacter()"
                     << " Sys Error: " << GetLastError() << endl;
                retcode = 0;
                break;
            }
            if (!FillConsoleOutputAttribute(hConsoleOutput,wStandard,
                sbiConsoleInfo.dwSize.X, coWritePos, &dwInputEventsRead))
            {
                cerr << "Could not colo[u]r cells in console with FillConsoleOutputAttribute()"
                     << " Sys Error: " << GetLastError() << endl;
                retcode = 0;
                break;
            }
            coWritePos.Y++;
        }
        delete [] cOutputStr;
        coWritePos.Y = coMenuPos.Y;
        FlushConsoleInputBuffer(hConsoleInput);

        //Position the items so as many as possible are displayed on outset
        if ( !SetConsoleCursorPosition(hConsoleOutput, coMenuPos))
        {
            cerr << "Could not set the cursor position with SetConsoleCursorPosition()"
                 << " Sys Error: " << GetLastError() << endl;
            retcode = 0;
        }
        if (coMenuPos.Y + wMenuItems - 1 > sbiConsoleInfo.srWindow.Bottom
            && coMenuPos.Y != sbiConsoleInfo.srWindow.Top)
        {
            wRectYArea = sbiConsoleInfo.srWindow.Bottom
                - sbiConsoleInfo.srWindow.Top;
            if (wRectYArea >= wMenuItems - 1)
            {
                sbiConsoleInfo.srWindow.Bottom = coMenuPos.Y + wMenuItems - 1;
                sbiConsoleInfo.srWindow.Top = sbiConsoleInfo.srWindow.Bottom - wRectYArea;
            } else {
                sbiConsoleInfo.srWindow.Top = coMenuPos.Y;
                sbiConsoleInfo.srWindow.Bottom = sbiConsoleInfo.srWindow.Top + wRectYArea;
            }
            if (!SetConsoleWindowInfo(hConsoleOutput, TRUE, &sbiConsoleInfo.srWindow))
            {
                cerr << "Could not set console window info with SetConsoleWindowInfo()"
                     << " Sys Error: " << GetLastError() << endl;
                retcode = 0;
            }
        }
    } // end if
    while (retcode == 10000)
    {
        if (bRedraw)
        {
            if (!FillConsoleOutputAttribute(hConsoleOutput,wStandard,
                sbiConsoleInfo.dwSize.X, coWritePos, &dwInputEventsRead))
            {
                cerr << "Could not colo[u]r cells in console buffer with FillConsoleOutputAttribute()"
                     << " Sys Error: " << GetLastError() << endl;
                retcode = 0;
                continue;
            }
            coWritePos.Y = coMenuPos.Y + wMenuSel;
            if (!FillConsoleOutputAttribute(hConsoleOutput,wSelected,
                sbiConsoleInfo.dwSize.X, coWritePos, &dwInputEventsRead))
            {
                cerr << "Could not colo[u]r cells in console buffer with FillConsoleOutputAttribute()"
                     << " Sys Error: " << GetLastError() << endl;
                retcode = 0;
                continue;
            }
            if ( !SetConsoleCursorPosition(hConsoleOutput, coWritePos))
            {
                cerr << "Could not set the cursor position with SetConsoleCursorPosition()"
                     << " Sys Error: " << GetLastError() << endl;
                retcode = 0;
                continue;
            }
            bRedraw = false;
        }
        if (!ReadConsoleInput(hConsoleInput,&irUserInput, 1, &dwInputEventsRead))
        {
            cerr << "Could not read the console input buffer with ReadConsoleInput()"
                 << " Sys Error: " << GetLastError() << endl;
            retcode = 0;
            continue;
        }
        if (irUserInput.EventType == MOUSE_EVENT)
        {
            bKeyDown = false;
            if (irUserInput.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
            {

                if (irUserInput.Event.MouseEvent.dwMousePosition.Y >= coMenuPos.Y
                    && irUserInput.Event.MouseEvent.dwMousePosition.Y
                    < coMenuPos.Y + wMenuItems)
                {
                    wLastMousePos = wMenuSel;
                    wMenuSel = irUserInput.Event.MouseEvent.dwMousePosition.Y - coMenuPos.Y;
                    if (!bMouseDown)
                    {
                        wMouseDownPos = wMenuSel;
                    }
                    bMouseDown = true;
                } else {
                    bMouseUntilUp = true;
                }
                if (bMouseDown || bMouseUntilUp)
                {
                    if (wMouseDownPos != wMenuSel)
                    {
                        bMouseUntilUp = true;
                    }
                    if (irUserInput.Event.MouseEvent.dwMousePosition.Y >= coMenuPos.Y
                        && irUserInput.Event.MouseEvent.dwMousePosition.Y
                        < coMenuPos.Y + wMenuItems)
                    {
                        if (wMenuSel != wLastMousePos)
                        {
                            bRedraw = true;
                        }
                    }
                }
                continue;

            } else {
                if (irUserInput.Event.MouseEvent.dwButtonState == 0)
                {
                    if (bMouseUntilUp)
                    {
                        bMouseUntilUp = false;
                        bMouseDown = false;
                        if (wMouseDownPos == wMenuSel)
                        {
                            bMouseDown = true;
                        }
                    }
                    if (bMouseDown)
                    {
                        if (irUserInput.Event.MouseEvent.dwMousePosition.Y
                            - coMenuPos.Y == wMouseDownPos)
                        {
                            retcode = wMouseDownPos + 1;
                            continue;
                        }
                        bMouseDown = false;
                        //bRedraw = true;
                    }
                } else {
                    bMouseDown = false;
                }
            }
            bMouseDown = false;


            if (irUserInput.Event.MouseEvent.dwEventFlags == MOUSE_WHEELED)
            {
                if (irUserInput.Event.MouseEvent.dwButtonState & 0xFF000000)
                {
                    //down
                    wMenuSel = (wMenuSel + 1) % wMenuItems;
                } else {
                    //up
                    if (wMenuSel == 0)
                    {
                        wMenuSel = wMenuItems - 1;
                    } else {
                        wMenuSel--;
                    }
                }
                bRedraw = true;
                continue;
            }

            if (irUserInput.Event.MouseEvent.dwMousePosition.Y >= coMenuPos.Y
                && irUserInput.Event.MouseEvent.dwMousePosition.Y
                < coMenuPos.Y + wMenuItems)
            {
                wMenuSel = irUserInput.Event.MouseEvent.dwMousePosition.Y - coMenuPos.Y;
                bRedraw = true;
                continue;
            }
            continue;
        } else {
            bMouseDown = false;
        }
        if (irUserInput.EventType == KEY_EVENT)
        {
            if ( !irUserInput.Event.KeyEvent.bKeyDown)
            {
                if (bKeyDown)
                {
                    retcode = wMenuSel + 1;
                }
                continue;
            }
            if (irUserInput.Event.KeyEvent.wVirtualKeyCode == VK_SPACE
                || irUserInput.Event.KeyEvent.wVirtualKeyCode == VK_RETURN)
            {
                bKeyDown = true;
                continue;
            }
            bKeyDown = false;
            if (irUserInput.Event.KeyEvent.wVirtualKeyCode == VK_TAB)
            {
                if (irUserInput.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED)
                {
                    //up
                    if (wMenuSel == 0)
                    {
                        wMenuSel = wMenuItems - 1;
                    } else {
                        wMenuSel--;
                    }
                } else {
                    //down
                    wMenuSel = (wMenuSel + 1) % wMenuItems;
                }
                bRedraw = true;
                continue;
            }
            if (irUserInput.Event.KeyEvent.wVirtualKeyCode == VK_PRIOR)
            {
                if (wMenuSel - 5 < 0)
                {
                    wMenuSel = 0;
                } else {
                    wMenuSel -= 5;
                }
                bRedraw = true;
                continue;
            }
            if (irUserInput.Event.KeyEvent.wVirtualKeyCode == VK_NEXT)
            {
                if (wMenuSel + 5 > wMenuItems - 1)
                {
                    wMenuSel = wMenuItems - 1;
                } else {
                    wMenuSel += 5;
                }
                bRedraw = true;
                continue;
            }
            if (irUserInput.Event.KeyEvent.wVirtualKeyCode == VK_HOME)
            {
                wMenuSel = 0;
                bRedraw = true;
                continue;
            }
            if (irUserInput.Event.KeyEvent.wVirtualKeyCode == VK_END)
            {
                wMenuSel = wMenuItems - 1;
                bRedraw = true;
                continue;
            }
            if (irUserInput.Event.KeyEvent.wVirtualKeyCode == VK_UP)
            {
                if (wMenuSel == 0)
                {
                    wMenuSel = wMenuItems - 1;
                } else {
                    wMenuSel--;
                }
                bRedraw = true;
                continue;
            }
            if (irUserInput.Event.KeyEvent.wVirtualKeyCode == VK_DOWN)
            {
                wMenuSel = (wMenuSel + 1) % wMenuItems;
                bRedraw = true;
                continue;
            }

            continue;
        }
        bKeyDown = false;

    }


    // ******************* Clean up ***********************************
    // I don't see any point in error checking it....
    coMenuPos.Y += wMenuItems;
    if (coMenuPos.Y > sbiConsoleInfo.dwSize.Y)
    {
        coMenuPos.Y = sbiConsoleInfo.dwSize.Y;
    }
    if (coMenuPos.Y == sbiConsoleInfo.dwSize.Y)
    {
        coMenuPos.Y--;
        srScrollRect.Top = 1;
        srScrollRect.Left = 0;
        srScrollRect.Bottom = sbiConsoleInfo.dwSize.Y - 1;
        srScrollRect.Right = sbiConsoleInfo.dwSize.X - 1;
        coScrollTop.X = 0;
        coScrollTop.Y = 0;
        ScrollConsoleScreenBuffer(hConsoleOutput,
            &srScrollRect, NULL, coScrollTop,&ciScrollFill );
    }
    if (wMenuItems !=0)
    {
        SetConsoleCursorPosition(hConsoleOutput, coMenuPos);
    }
    SetConsoleMode(hConsoleInput, dwConsoleMode);
    SetConsoleCursorInfo(hConsoleOutput, &cciRestore);
    CloseHandle(hConsoleInput);
    CloseHandle(hConsoleOutput);

    // ********************** End Cleanup **************************
    return retcode;
}
