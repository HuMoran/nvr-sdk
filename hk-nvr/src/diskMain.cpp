/*
 * File: diskMain.cpp
 * Project: hk-nvr
 * Description: 硬盘状态检查
 * Created By: Tao.Hu 2019-08-12
 * -----
 * Last Modified: 2019-08-12 01:48:50 am
 * Modified By: Tao.Hu
 * -----
 * Copyright (c) 2019 Kideasoft Tech Co.,Ltd
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/HCNetSDK.h"
using namespace std;

int main(int argc, char *argv[]) {
  if (argc < 5) {
    printf("please run hk-nvr as:\n");
    printf("./hk-nvr ip port username password\n");
    return -1;
  }
  char *ip = argv[1];
  int port = atoi(argv[2]);
  char *username = argv[3];
  char *password = argv[4];
  // sdk init
  NET_DVR_Init();
  // log init
  NET_DVR_SetLogToFile(3, "./diskLog");
  //Login device
  NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
  NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
  struLoginInfo.bUseAsynLogin = false;
  struLoginInfo.wPort = port;
  memcpy(struLoginInfo.sDeviceAddress, ip, NET_DVR_DEV_ADDRESS_MAX_LEN);
  memcpy(struLoginInfo.sUserName, username, NAME_LEN);
  memcpy(struLoginInfo.sPassword, password, NAME_LEN);
  // struLoginInfo.wPort = 8000;
  // memcpy(struLoginInfo.sDeviceAddress, "192.168.1.106", NET_DVR_DEV_ADDRESS_MAX_LEN);
  // memcpy(struLoginInfo.sUserName, "admin", NAME_LEN);
  // memcpy(struLoginInfo.sPassword, "admin123", NAME_LEN);

  int lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
  printf("lUserID:%d\n", lUserID);

  if (lUserID < 0) {
      printf("Login error, %d\n", NET_DVR_GetLastError());
      NET_DVR_Cleanup();
      return -1;
  }

  NET_DVR_HDCFG struDevConfig = {0};
  DWORD uiReturnLen;
  int iRet = NET_DVR_GetDVRConfig(
    lUserID,
    NET_DVR_GET_HDCFG,
    0xFFFFFFFF,
    &struDevConfig,
    sizeof(NET_DVR_HDCFG),
    &uiReturnLen
  );
  if (!iRet) {
    printf("Get NET_DVR_GetDVRConfig error:%d\n", NET_DVR_GetLastError());
    NET_DVR_Logout_V30(lUserID);
    NET_DVR_Cleanup();
    return -1;
  }
  printf("HDCount: %d\n", struDevConfig.dwHDCount);
  for(int i = 0; i < (int)struDevConfig.dwHDCount; i++) {
    printf("No: %d, Capacity: %d, FreeSpace: %d, Status: %d, Type: %d, GRoup: %d\n",
      struDevConfig.struHDInfo[i].dwHDNo,
      struDevConfig.struHDInfo[i].dwCapacity,
      struDevConfig.struHDInfo[i].dwFreeSpace,
      struDevConfig.struHDInfo[i].dwHdStatus,
      struDevConfig.struHDInfo[i].byHDType,
      struDevConfig.struHDInfo[i].dwHdGroup
    );
  }
  NET_DVR_Logout_V30(lUserID);
  NET_DVR_Cleanup();
  return 0;
}