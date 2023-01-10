#ifndef DEVICE_STATUS_H_
#define DEVICE_STATUS_H_

struct DeviceStatus
{
    bool statusChangeFinger = false;
    bool keyPress = false;
    bool openDoor = false;
    bool checkPassWord = false;
    bool warning = false;
};

#endif