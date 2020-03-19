/*

  a test for Sound Library for "Virtuality" project

  author : Klimov Alexander aka Dispell

*/

#include "abnormal.h"

#include "DSDevice.h"
#include "A3DDevice.h"
#include <conio.h>

DSDevice    SndDevice;

void main()
{
    SndDevice.InitializeSound(GetForegroundWindow());
    point3 pos(-40, 0, 0), vel(0, 0, 0), front(0, 1, 0), up(0, 0, 1);
    //SndDevice.UpdateListenerXYZ(pos, vel, front, up);
    //SndDevice.UpdateListenerPos(pos);
    SndDevice.Play("file0038.wav", "no_desc");
    SndDevice.UpdateListenerPos(pos);
    _getch();
    SndDevice.CloseSound();
}