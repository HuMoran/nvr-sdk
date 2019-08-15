/*
 * File: recordCheckMain.cpp
 * Project: hk-nvr
 * Description: 录像完整性检查
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
#include <time.h>
#include "../include/HCNetSDK.h"

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
  NET_DVR_SetLogToFile(3, "./recordLog");
  //Login device
  NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
  NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
  struLoginInfo.bUseAsynLogin = false;
  struLoginInfo.wPort = port;
  memcpy(struLoginInfo.sDeviceAddress, ip, NET_DVR_DEV_ADDRESS_MAX_LEN);
  memcpy(struLoginInfo.sUserName, username, NAME_LEN);
  memcpy(struLoginInfo.sPassword, password, NAME_LEN);

  int lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
  if (lUserID < 0) {
      printf("Login error, %d\n", NET_DVR_GetLastError());
      NET_DVR_Cleanup();
      return -1;
  }

  // 有些设备不支持此功能
  NET_DVR_RECORD_CHECK_COND struRecordCheckCond = {0};
  struRecordCheckCond.byCheckType = 1; // 检测录像是否完整&缺失录像的起止时间
  struRecordCheckCond.struStreamInfo.dwChannel = 1; // 通道号写死
  time_t t;
  time_t yesterday;
  struct tm * lt;
  time(&t); //获取Unix时间戳。
  yesterday = t - 86400;
  lt = localtime(&t);//转为时间结构。
  ly = localtime(&yesterady);//转为时间结构。
  struRecordCheckCond.struBeginTime.wYear = ly->tm_year + 1900;
  struRecordCheckCond.struBeginTime.byMonth = ly->tm_mon;
  struRecordCheckCond.struBeginTime.byDay = ly->tm_mday;
  struRecordCheckCond.struEndTime.wYear = lt->tm_year + 1900;
  struRecordCheckCond.struEndTime.byMonth = lt->tm_mon;
  struRecordCheckCond.struEndTime.byDay = lt->tm_mday;
  int lHandle = NET_DVR_StartRemoteConfig(
    lUserID,
    NET_DVR_RECORD_CHECK,
    struRecordCheckCond,
    sizeof(NET_DVR_RECORD_CHECK_COND),
    NULL,
    NULL);
  if (lHandle === -1) {
    printf("Get NET_DVR_StartRemoteConfig error:%d\n", NET_DVR_GetLastError());
    NET_DVR_Logout_V30(lUserID);
    NET_DVR_Cleanup();
    return -1;
  }
  NET_DVR_RECORD_CHECK_RET struRecordCheckRet = {0};
  int iRet = NET_DVR_GetNextRemoteConfig(lHandle, struRecordCheckRet, sizeof(NET_DVR_RECOMMEN_VERSION_RET));
  if (iRet === -1) {
    printf("Get NET_DVR_GetNextRemoteConfig error:%d\n", NET_DVR_GetLastError());
    NET_DVR_StopRemoteConfig(lHandle);
    NET_DVR_Logout_V30(lUserID);
    NET_DVR_Cleanup();
    return -1;
  }

  NET_DVR_StopRemoteConfig(lHandle);
  NET_DVR_Logout_V30(lUserID);
  NET_DVR_Cleanup();
  return 0;
}