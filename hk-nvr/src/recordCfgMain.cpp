/*
 * File: recordCfgMain.cpp
 * Project: hk-nvr
 * Description: 录像计划查询
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

  int lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
  if (lUserID < 0) {
    printf("Login error, %d\n", NET_DVR_GetLastError());
    NET_DVR_Cleanup();
    return -1;
  }

  // 数字通道个数，高8位*256+低8位
  int iIpChanNum = struDeviceInfoV40.struDeviceV30.byHighDChanNum * 256 + struDeviceInfoV40.struDeviceV30.byIPChanNum;
  // 数字通道起始通道号, 0表示无通道
  int iIpChanStart = struDeviceInfoV40.struDeviceV30.byStartDChan;
  // 模拟通道个数
  int iChanNum = struDeviceInfoV40.struDeviceV30.byChanNum;
  // 模拟通道起始通道号, 从1开始
  int iChanStart = struDeviceInfoV40.struDeviceV30.byStartChan;
  printf("硬盘个数:%d\n", struDeviceInfoV40.struDeviceV30.byDiskNum);
  printf("设备类型:%d\n", struDeviceInfoV40.struDeviceV30.byDVRType);
  printf("模拟通道个数:%d, 模拟通道起始通道号:%d\n", iChanNum, iChanStart);
  printf("数字通道个数:%d, 数字通道起始通道号:%d\n", iIpChanNum, iIpChanStart);

  NET_DVR_RECORD_V40 struRecordCfg = {0};
  DWORD uiReturnLen;
  int iRet = NET_DVR_GetDVRConfig(
    lUserID,
    NET_DVR_GET_RECORDCFG_V40,
    34,
    &struRecordCfg,
    sizeof(NET_DVR_RECORD_V40),
    &uiReturnLen
  );
  if (!iRet) {
    printf("Get NET_DVR_GetDVRConfig error:%d\n", NET_DVR_GetLastError());
    NET_DVR_Logout_V30(lUserID);
    NET_DVR_Cleanup();
    return -1;
  }
  printf("是否启用计划录像配置: %s\n", struRecordCfg.dwRecord == 0 ? "否" : "是");
  for (int i = 0; i < 7; i++) {
    printf("星期[%d]是否全天录像: %s, 录像类型: %d\n",
      i + 1,
      struRecordCfg.struRecAllDay[i].byAllDayRecord == 0 ? "否" : "是",
      struRecordCfg.struRecAllDay[i].byRecordType
    );
  }
  NET_DVR_Logout_V30(lUserID);
  NET_DVR_Cleanup();
  return 0;
}