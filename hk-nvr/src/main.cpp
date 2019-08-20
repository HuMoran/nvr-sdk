/*
 * File: main.cpp
 * Project: src
 * Description:
 * Created By: Tao.Hu 2019-08-20
 * -----
 * Last Modified: 2019-08-20 11:14:34 am
 * Modified By: Tao.Hu
 * -----
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/HCNetSDK.h"

int login(char *ip, int port, char *username, char *password)
{
  //Login device
  NET_DVR_USER_LOGIN_INFO struLoginInfo = {1};
  NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
  struLoginInfo.bUseAsynLogin = false;
  struLoginInfo.wPort = port;
  memcpy(struLoginInfo.sDeviceAddress, ip, NET_DVR_DEV_ADDRESS_MAX_LEN);
  memcpy(struLoginInfo.sUserName, username, NAME_LEN);
  memcpy(struLoginInfo.sPassword, password, NAME_LEN);

  int lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
  if (lUserID < 0)
  {
    printf("Login error, %d\n", NET_DVR_GetLastError());
    NET_DVR_Cleanup();
    return -1;
  }
  return 0;
}

int main(int argc, char *argv[])
{
  if (argc < 5)
  {
    printf("please run hk-nvr as:\n ./hk-nvr ip port username password\n");
    return -1;
  }
  int iRet, iGroupSum;
  char *ip = argv[1];
  int port = atoi(argv[2]);
  char *username = argv[3];
  char *password = argv[4];
  // sdk init
  NET_DVR_Init();
  // log init
  NET_DVR_SetLogToFile(3, "./diskLog");
  iRet = login(ip, port, username, password);
  if (0 != iRet)
    return -1;
}