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
#include <unistd.h>
#include "../include/HCNetSDK.h"

typedef struct
{
  int diskNum;     // 硬盘个数
  int DVRType;     // 设备类型
  int chanNum;     // 模拟通道个数
  int chanStart;   // 模拟通道起始号
  int IPChanNum;      // 数字通道最大支持个数
  int IPChanStart; // 数字通道起始号
} DVR_BASE_INFO;

typedef struct
{
    unsigned int iGroupNO;  // 组号
    unsigned int iNO;       //通道号
    unsigned char byEnable; // 通道对应的设备状态 0：offline 1：online
    unsigned int IdevID;    // 设备ID
    char sIpV4[16];
    unsigned char sIPV6[128];
} IP_CHAN_INFO;

int login(char *ip, int port, char *username, char *password, LPNET_DVR_DEVICEINFO_V40 struDeviceInfoV40)
{
  NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
  // struLoginInfo.bUseAsynLogin = false;
  struLoginInfo.wPort = port;
  memcpy(struLoginInfo.sDeviceAddress, ip, NET_DVR_DEV_ADDRESS_MAX_LEN);
  memcpy(struLoginInfo.sUserName, username, NAME_LEN);
  memcpy(struLoginInfo.sPassword, password, NAME_LEN);

  int lUserID = NET_DVR_Login_V40(&struLoginInfo, struDeviceInfoV40);
  if (lUserID < 0)
  {
    printf("Login error, %d\n", NET_DVR_GetLastError());
    NET_DVR_Cleanup();
    return -1;
  }
  return lUserID;
}

DVR_BASE_INFO getBaseInfo(NET_DVR_DEVICEINFO_V40 struDeviceInfoV40)
{
  DVR_BASE_INFO struDVRBaseInfo = {0};

  struDVRBaseInfo.diskNum = struDeviceInfoV40.struDeviceV30.byDiskNum;
  struDVRBaseInfo.DVRType = struDeviceInfoV40.struDeviceV30.wDevType;

  struDVRBaseInfo.chanNum;
  struDVRBaseInfo.chanStart = struDeviceInfoV40.struDeviceV30.byDiskNum;

  // 数字通道个数，高8位*256+低8位
  struDVRBaseInfo.IPChanNum = struDeviceInfoV40.struDeviceV30.byHighDChanNum * 256
    + struDeviceInfoV40.struDeviceV30.byIPChanNum;
  // 数字通道起始通道号, 0表示无通道
  struDVRBaseInfo.IPChanStart = struDeviceInfoV40.struDeviceV30.byStartDChan;
  // 模拟通道个数
  struDVRBaseInfo.chanNum = struDeviceInfoV40.struDeviceV30.byChanNum;
  // 模拟通道起始通道号, 从1开始
  struDVRBaseInfo.chanStart = struDeviceInfoV40.struDeviceV30.byStartChan;

  printf("硬盘个数:%d\n", struDVRBaseInfo.diskNum);
  printf("设备类型:%d\n", struDVRBaseInfo.DVRType);
  printf("模拟通道个数:%d, 模拟通道起始通道号:%d\n", struDVRBaseInfo.chanNum, struDVRBaseInfo.chanStart);
  printf("数字通道最大支持个数:%d, 数字通道起始通道号:%d\n", struDVRBaseInfo.IPChanNum, struDVRBaseInfo.IPChanStart);
  return struDVRBaseInfo;
}

int getDiskInfo(LONG lUserID)
{
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
  printf("硬盘个数: %d\n", struDevConfig.dwHDCount);
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
  return 0;
}

IP_CHAN_INFO *getChannelInfo(LONG lUserID, int iGroupNO)
{
    //获取IP通道参数信息
    NET_DVR_IPPARACFG_V40 IPAccessCfgV40 = {0};
    DWORD dwReturned = 0;
    BYTE byEnable, byIPID, byIPIDHigh;
    int iDevInfoIndex;

    int iRet = NET_DVR_GetDVRConfig(
        lUserID,
        NET_DVR_GET_IPPARACFG_V40,
        iGroupNO,
        &IPAccessCfgV40,
        sizeof(NET_DVR_IPPARACFG_V40),
        &dwReturned);
    if (!iRet)
    {
        printf("NET_DVR_GET_IPPARACFG_V40 error, %d\n", NET_DVR_GetLastError());
        return NULL;
    }
    int dwDChanNum = IPAccessCfgV40.dwDChanNum;
    printf("数字通道个数: %d\n", dwDChanNum);
    if (0 == dwDChanNum)
        return NULL;

    IP_CHAN_INFO struIPChanInfo[dwDChanNum] = {0};

    for (int i = 0; i < dwDChanNum; i++)
    {
        switch (IPAccessCfgV40.struStreamMode[i].byGetStreamType)
        {
        case 0: //直接从设备取流
            if (IPAccessCfgV40.struStreamMode[i].uGetStream.struChanInfo.byEnable)
            {
                byEnable = IPAccessCfgV40.struStreamMode[i].uGetStream.struChanInfo.byEnable;
                byIPID = IPAccessCfgV40.struStreamMode[i].uGetStream.struChanInfo.byIPID;
                byIPIDHigh = IPAccessCfgV40.struStreamMode[i].uGetStream.struChanInfo.byIPIDHigh;
                iDevInfoIndex = byIPIDHigh * 256 + byIPID - 1 - iGroupNO * 64;
                struIPChanInfo[i].iGroupNO = 0;
                struIPChanInfo[i].iNO = i + 1;
                struIPChanInfo[i].byEnable = byEnable;
                memcpy(struIPChanInfo[i].sIpV4, IPAccessCfgV40.struIPDevInfo[iDevInfoIndex].struIP.sIpV4, 16);
                printf("IP channel no.%d is %s, IP: %s\n",
                      i + 1,
                      byEnable == 0 ? "offline" : "online",
                      // IPAccessCfgV40.struIPDevInfo[iDevInfoIndex].struIP.byEnable,
                      IPAccessCfgV40.struIPDevInfo[iDevInfoIndex].struIP.sIpV4);
            }
            break;
        default:
            break;
        }
    }
    return struIPChanInfo;
}

int getRecordCfg(LONG lUserID)
{
  NET_DVR_RECORD_V40 struRecordCfg = {0};
  DWORD uiReturnLen;
  int iRet = NET_DVR_GetDVRConfig(
      lUserID,
      NET_DVR_GET_RECORDCFG_V40,
      34,
      &struRecordCfg,
      sizeof(NET_DVR_RECORD_V40),
      &uiReturnLen);
  if (!iRet)
  {
    printf("Get NET_DVR_GetDVRConfig error:%d\n", NET_DVR_GetLastError());
    NET_DVR_Logout_V30(lUserID);
    NET_DVR_Cleanup();
    return -1;
  }
  printf("是否启用计划录像配置: %s\n", struRecordCfg.dwRecord == 0 ? "否" : "是");
  for (int i = 0; i < 7; i++)
  {
    printf("星期[%d]是否全天录像: %s, 录像类型: %d\n",
           i + 1,
           struRecordCfg.struRecAllDay[i].byAllDayRecord == 0 ? "否" : "是",
           struRecordCfg.struRecAllDay[i].byRecordType);
  }
}

int main(int argc, char *argv[])
{
  if (argc < 5)
  {
    printf("please run hk-nvr as:\n ./hk-nvr ip port username password\n");
    return -1;
  }
  int iRet, iGroupSum, lUserID;
  char *ip = argv[1];
  int port = atoi(argv[2]);
  char *username = argv[3];
  char *password = argv[4];
  // sdk init
  NET_DVR_Init();
  // log init
  NET_DVR_SetLogToFile(0, "./log");
  // login
  NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
  lUserID = login(ip, port, username, password, &struDeviceInfoV40);
  if (-1 == lUserID) return -1;

  // 获取设备基本信息
  DVR_BASE_INFO struDVRBaseInfo;
  struDVRBaseInfo = getBaseInfo(struDeviceInfoV40);
  // 获取硬盘信息
  getDiskInfo(lUserID);
  // 获取通道信息
  // 计算组数量
  iGroupSum = struDVRBaseInfo.IPChanNum / 64;
  IP_CHAN_INFO *lpIPChanInfo[iGroupSum];
  for (int i = 0; i <= iGroupSum; i++)
  {
    lpIPChanInfo[i] = getChannelInfo(lUserID, i);
  }
  // 设备抓图
  NET_DVR_JPEGPARA lpJpegPara = {0};
  lpJpegPara.wPicSize = 0xff;
  lpJpegPara.wPicQuality = 2; // 图片质量，0：最好，1：较好，2：一般
  for (int i = 0; i <= struDVRBaseInfo.IPChanNum; i++) {
    char str[25] = {0};
    snprintf(str, sizeof(i), "%d", i);
    char *filename = strcat(str, ".jpg");
    int iRet = NET_DVR_CaptureJPEGPicture(lUserID, struDVRBaseInfo.IPChanStart + i, &lpJpegPara, filename);
    // printf("文件名：%s，返回值：%d， 错误码：%d\n", filename, iRet, NET_DVR_GetLastError());
  }

  // 获取录像计划
  getRecordCfg(lUserID);

  // 查找录像文件
  NET_DVR_FILECOND_V40 pFindCond = {0};
  NET_DVR_TIME startTime = {
    dwYear: 2019,
    dwMonth: 8,
    dwDay: 16,
  };
  NET_DVR_TIME endTime = {
    dwYear: 2019,
    dwMonth: 8,
    dwDay: 17,
  };
  pFindCond.lChannel = 34;
  pFindCond.dwFileType = 0;
  pFindCond.dwIsLocked = 0xff;
  pFindCond.struStartTime.dwYear = 2019;
  pFindCond.struStartTime.dwMonth = 1;
  pFindCond.struStartTime.dwDay = 1;
  pFindCond.struStopTime.dwYear = 2019;
  pFindCond.struStopTime.dwMonth = 12;
  pFindCond.struStopTime.dwDay = 30;

  int lFindHandle = NET_DVR_FindFile_V40(lUserID, &pFindCond);
  if (-1 == lFindHandle) {
    NET_DVR_Logout_V30(lUserID);
    NET_DVR_Cleanup();
    return -1;
  }
  NET_DVR_FINDDATA_V40 lpFindData[4000] = {0};
  int count = 0;
  while(true)
  {
    iRet = NET_DVR_FindNextFile_V40(lFindHandle, &lpFindData[count]);
    // printf("返回值：%d， 错误码：%d\n", iRet, NET_DVR_GetLastError());
    if (-1 == iRet) {
      NET_DVR_Logout_V30(lUserID);
      NET_DVR_Cleanup();
      return -1;
    }
    if (iRet == NET_DVR_ISFINDING)
    {
      sleep(1);
      continue;
    }
    if (iRet == NET_DVR_FILE_SUCCESS)
    {
      count += 1;
      continue;
    }
    break;
  }
  for (int i = 0; i < count; i++)
  {
    printf("文件名：%s, 大小：%dM ", lpFindData[i].sFileName, lpFindData[i].dwFileSize / (1024 * 1024));
    printf("开始时间：%d/%d %d:%d:%d，结束时间：%d/%d %d:%d:%d\n",
      lpFindData[i].struStartTime.dwMonth,
      lpFindData[i].struStartTime.dwDay,
      lpFindData[i].struStartTime.dwHour,
      lpFindData[i].struStartTime.dwMinute,
      lpFindData[i].struStartTime.dwSecond,
      lpFindData[i].struStopTime.dwMonth,
      lpFindData[i].struStopTime.dwDay,
      lpFindData[i].struStopTime.dwHour,
      lpFindData[i].struStopTime.dwMinute,
      lpFindData[i].struStopTime.dwSecond
    );
  }
  NET_DVR_FindClose_V30(lFindHandle);

  NET_DVR_MRD_SEARCH_PARAM lpInBuffer[1] = {0};
  lpInBuffer[0].dwSize = sizeof(NET_DVR_MRD_SEARCH_PARAM);
  lpInBuffer[0].struStreamInfo.dwSize = sizeof(NET_DVR_STREAM_INFO);
  lpInBuffer[0].struStreamInfo.dwChannel = 33;
  lpInBuffer[0].wYear = 2019;
  int month = 8;
  lpInBuffer[0].byMonth = month;

  int lpStatusList[2] = {0};
  NET_DVR_MRD_SEARCH_RESULT lpOutBuffer[1] = {0};
  int dwOutBufferSize = sizeof(NET_DVR_MRD_SEARCH_RESULT) * 1;

  iRet = NET_DVR_GetDeviceConfig(
    lUserID,
    NET_DVR_GET_MONTHLY_RECORD_DISTRIBUTION,
    0,
    lpInBuffer,
    sizeof(NET_DVR_MRD_SEARCH_PARAM) * 1,
    lpStatusList,
    lpOutBuffer,
    dwOutBufferSize
  );
  printf("返回值：%d， 错误码：%d %d %d\n", iRet, NET_DVR_GetLastError(), lpStatusList[0], lpStatusList[1]);
  // for (int i = 0; i < 30; i++) {
  //   printf("%d月%d号：%d\n", month, i + 1, lpOutBuffer[0].byHasEventRecode[i]);
  //   printf("%d月%d号事件录像：%d\n", month, i + 1, lpOutBuffer[0].byHasEventRecode[i]);
  // }

  NET_DVR_Logout_V30(lUserID);
  NET_DVR_Cleanup();
  return 0;
}