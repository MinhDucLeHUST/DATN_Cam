#ifndef DEVICE_STATUS_H_
#define DEVICE_STATUS_H_

enum UnlockOption
{
    NONE = 0,
    Manual = 1,
    KeyPad = 2,
    Finger = 3,
    MobileApp = 4,
};
class DeviceStatus
{
private:
    /* data */
public:
    bool statusChangeFinger;
    bool keyPress;
    bool checkPassWord;
    bool openDoor;
    bool warning;
    UnlockOption unlockOption = NONE;
};

#endif