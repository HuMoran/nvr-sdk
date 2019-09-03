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
  int IPChanNum;   // 数字通道最大支持个数
  int IPChanStart; // 数字通道起始号
} DVR_BASE_INFO;

typedef struct
{
  int no;
  unsigned int capacity;
  unsigned int freeSpace;
  short status;
  short type;
  short group;
} DISK_INFO;


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

  struDVRBaseInfo.chanNum = struDeviceInfoV40.struDeviceV30.byChanNum;
  struDVRBaseInfo.chanStart = struDeviceInfoV40.struDeviceV30.byStartChan;

  // 数字通道个数，高8位*256+低8位
  struDVRBaseInfo.IPChanNum = struDeviceInfoV40.struDeviceV30.byHighDChanNum * 256
    + struDeviceInfoV40.struDeviceV30.byIPChanNum;
  // 数字通道起始通道号, 0表示无通道
  struDVRBaseInfo.IPChanStart = struDeviceInfoV40.struDeviceV30.byStartDChan;
  // 模拟通道个数
  struDVRBaseInfo.chanNum = struDeviceInfoV40.struDeviceV30.byChanNum;
  // 模拟通道起始通道号, 从1开始
  struDVRBaseInfo.chanStart = struDeviceInfoV40.struDeviceV30.byStartChan;

  return struDVRBaseInfo;
}

int getDiskInfo(LONG lUserID, DISK_INFO *struDisk)
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
  int diskNum = struDevConfig.dwHDCount;
  if (diskNum <= 0) {
    return 0;
  }
  for(int i = 0; i < diskNum; i++)
  {
    struDisk[i].no = struDevConfig.struHDInfo[i].dwHDNo;
    struDisk[i].capacity = struDevConfig.struHDInfo[i].dwCapacity;
    struDisk[i].freeSpace = struDevConfig.struHDInfo[i].dwFreeSpace;
    struDisk[i].status = struDevConfig.struHDInfo[i].dwHdStatus;
    struDisk[i].type = struDevConfig.struHDInfo[i].byHDType;
    struDisk[i].group = struDevConfig.struHDInfo[i].dwHdGroup;
  }

  return 0;
}

int getChannelInfo(LONG lUserID, int iGroupNO, NET_DVR_IPPARACFG_V40 *IPAccessCfgV40)
{
    //获取IP通道参数信息
    // NET_DVR_IPPARACFG_V40 IPAccessCfgV40 = {0};
    DWORD dwReturned = 0;

    int iRet = NET_DVR_GetDVRConfig(
        lUserID,
        NET_DVR_GET_IPPARACFG_V40,
        iGroupNO,
        IPAccessCfgV40,
        sizeof(NET_DVR_IPPARACFG_V40),
        &dwReturned);
    if (!iRet)
    {
        printf("NET_DVR_GET_IPPARACFG_V40 error, %d\n", NET_DVR_GetLastError());
        return -1;
    }
    return 0;
    #if 0
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
    return 0;
    #endif
}

int getRecordCfg(LONG lUserID, LONG lChannel)
{
  NET_DVR_RECORD_V40 struRecordCfg = {0};
  DWORD uiReturnLen;
  int iRet = NET_DVR_GetDVRConfig(
      lUserID,
      NET_DVR_GET_RECORDCFG_V40,
      lChannel,
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
  printf("\"chanNo\": %d,\n", lChannel);
  printf("\"isRecord\": %s,\n", struRecordCfg.dwRecord == 0 ? "false" : "true");
  printf("\"details\": [\n");
  // printf("是否启用计划录像配置: %s\n", struRecordCfg.dwRecord == 0 ? "否" : "是");
  for (int i = 0; i < MAX_DAYS; i++)
  {
    printf("{\n");
    printf("\"isAllDayRecord\": %s,\n", struRecordCfg.struRecAllDay[i].byAllDayRecord == 0 ? "false" : "true");
    printf("\"allDayRecordType\": %d,\n", struRecordCfg.struRecAllDay[i].byRecordType);
    if (struRecordCfg.struRecAllDay[i].byAllDayRecord == 0)
    {
      printf("\"recordSchedule\": [\n");
      for (int j = 0; j < MAX_TIMESEGMENT_V30; j++)
      {
        printf("{\n");
        printf("\"recordType\": %d,\n", struRecordCfg.struRecordSched[i][j].byRecordType);
        printf("\"startHour\": %d,\n", struRecordCfg.struRecordSched[i][j].struRecordTime.byStartHour);
        printf("\"startMin\": %d,\n", struRecordCfg.struRecordSched[i][j].struRecordTime.byStartMin);
        printf("\"stopHour\": %d,\n", struRecordCfg.struRecordSched[i][j].struRecordTime.byStopHour);
        printf("\"stopMin\": %d\n", struRecordCfg.struRecordSched[i][j].struRecordTime.byStopMin);
        if (j == MAX_TIMESEGMENT_V30 - 1)
        {
          printf("}\n");
        } else {
          printf("},\n");
        }
      }
      if (i == MAX_DAYS - 1)
      {
        printf("]}\n");
      } else {
        printf("]},\n");
      }

    } else {
      if (i == MAX_DAYS - 1)
      {
        printf("\"recordSchedule\": []}\n");
      } else {
        printf("\"recordSchedule\": []},\n");
      }
    }

  }
  printf("]\n");
  return 0;
}

int printChansInfo(NET_DVR_IPPARACFG_V40 IPAccessCfgV40) {
    BYTE byEnable, byIPID, byIPIDHigh;
    int iDevInfoIndex;
    int dwDChanNum = IPAccessCfgV40.dwDChanNum;
    // printf("数字通道个数: %d\n", dwDChanNum);
    if (0 == dwDChanNum) return -1;

    IP_CHAN_INFO struIPChanInfo[dwDChanNum] = {0};
    int iChanNum = 0;
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
                // iDevInfoIndex = byIPIDHigh * 256 + byIPID - 1 - i * 64;
                iDevInfoIndex = byIPIDHigh * 256 + byIPID - 1;
                struIPChanInfo[iChanNum].iGroupNO = 0;
                struIPChanInfo[iChanNum].iNO = i + 1;
                struIPChanInfo[iChanNum].byEnable = byEnable;
                memcpy(struIPChanInfo[iChanNum].sIpV4, IPAccessCfgV40.struIPDevInfo[iDevInfoIndex].struIP.sIpV4, 16);
                iChanNum++;
            }
            break;
        default:
            break;
        }
    }
    for (int i = 0; i < iChanNum; i++)
    {
      printf("{\n");
      printf("\"no\": %d,\n", struIPChanInfo[i].iNO);
      printf("\"isOnline\": %s,\n", struIPChanInfo[i].byEnable == 0 ? "false" : "true");
      printf("\"ip\": \"%s\"\n", struIPChanInfo[i].sIpV4);
      if (i == iChanNum - 1) {
        printf("}\n");
      } else {
        printf("},\n");
      }
    }
    return iChanNum;
}

int getRecordFile(LONG lUserID, LONG lChannel, NET_DVR_TIME struStartTime, NET_DVR_TIME struStopTime)
{
  // 查找录像文件
  int iRet;
  NET_DVR_FILECOND_V40 pFindCond = {0};
  pFindCond.lChannel = lChannel;
  pFindCond.dwFileType = 0xff;
  pFindCond.dwIsLocked = 0xff;
  pFindCond.struStartTime = struStartTime;
  pFindCond.struStopTime = struStopTime;
  // pFindCond.struStartTime.dwYear = struStartTime.dwYear;
  // pFindCond.struStartTime.dwMonth = struStartTime.dwMonth;
  // pFindCond.struStartTime.dwDay = struStartTime.dwDay;
  // pFindCond.struStartTime.dwHour = struStartTime.dwHour;
  // pFindCond.struStartTime.dwMinute = struStartTime.dwMinute;
  // pFindCond.struStartTime.dwSecond = struStartTime.dwSecond;
  // pFindCond.struStopTime.dwYear = struStopTime.dwYear;
  // pFindCond.struStopTime.dwMonth = struStopTime.dwMonth;
  // pFindCond.struStopTime.dwDay = struStopTime.dwDay;
  // pFindCond.struStopTime.dwHour = struStopTime.dwHour;
  // pFindCond.struStopTime.dwMinute = struStopTime.dwMinute;
  // pFindCond.struStopTime.dwSecond = struStopTime.dwSecond;

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
  printf("\"chanNo\": %d,\n", lChannel);
  printf("\"startTime\": \"%d-%d-%d %d:%d:%d\",\n",
    struStartTime.dwYear,
    struStartTime.dwMonth,
    struStartTime.dwDay,
    struStartTime.dwHour,
    struStartTime.dwMinute,
    struStartTime.dwSecond
  );
  printf("\"stopTime\": \"%d-%d-%d %d:%d:%d\",\n",
    struStopTime.dwYear,
    struStopTime.dwMonth,
    struStopTime.dwDay,
    struStopTime.dwHour,
    struStopTime.dwMinute,
    struStopTime.dwSecond
  );
  printf("\"files\":[\n");
  for (int i = 0; i < count; i++)
  {
    printf("{\n");
    printf("\"filename\":\"%s\",\n", lpFindData[i].sFileName);
    printf("\"startTime\":\"%d-%d-%d %d:%d:%d\",\n",
      lpFindData[i].struStartTime.dwYear,
      lpFindData[i].struStartTime.dwMonth,
      lpFindData[i].struStartTime.dwDay,
      lpFindData[i].struStartTime.dwHour,
      lpFindData[i].struStartTime.dwMinute,
      lpFindData[i].struStartTime.dwSecond
    );
    printf("\"stopTime\":\"%d-%d-%d %d:%d:%d\",\n",
      lpFindData[i].struStopTime.dwYear,
      lpFindData[i].struStopTime.dwMonth,
      lpFindData[i].struStopTime.dwDay,
      lpFindData[i].struStopTime.dwHour,
      lpFindData[i].struStopTime.dwMinute,
      lpFindData[i].struStopTime.dwSecond
    );
    printf("\"size\":%d,\n", lpFindData[i].dwFileSize);
    printf("\"fileType\":%d\n", lpFindData[i].byFileType);
    if (i == count - 1)
    {
      printf("}\n");
    } else
    {
      printf("},\n");
    }

    // printf("文件名：%s, 大小：%dM ", lpFindData[i].sFileName, lpFindData[i].dwFileSize / (1024 * 1024));
    // printf("开始时间：%d/%d %d:%d:%d，结束时间：%d/%d %d:%d:%d\n",
    //   lpFindData[i].struStartTime.dwMonth,
    //   lpFindData[i].struStartTime.dwDay,
    //   lpFindData[i].struStartTime.dwHour,
    //   lpFindData[i].struStartTime.dwMinute,
    //   lpFindData[i].struStartTime.dwSecond,
    //   lpFindData[i].struStopTime.dwMonth,
    //   lpFindData[i].struStopTime.dwDay,
    //   lpFindData[i].struStopTime.dwHour,
    //   lpFindData[i].struStopTime.dwMinute,
    //   lpFindData[i].struStopTime.dwSecond
    // );
  }
  printf("]\n");
  NET_DVR_FindClose_V30(lFindHandle);
  return 0;
}

int main(int argc, char *argv[])
{
  if (argc < 5)
  {
    printf("please run hk-nvr as:\n ./hk-nvr ip port username password\n");
    return -1;
  }
  int iGroupSum, lUserID;
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
  printf("{\n");
  printf("\"devType\": %d,\n", struDVRBaseInfo.DVRType);
  printf("\"diskNum\": %d,\n", struDVRBaseInfo.diskNum);
  printf("\"chanMax\": %d, \n", struDVRBaseInfo.chanNum);
  printf("\"IPChanMax\": %d,\n", struDVRBaseInfo.IPChanNum);
  // 获取硬盘信息
  if (struDVRBaseInfo.diskNum > 0) {
    DISK_INFO struDisk[struDVRBaseInfo.diskNum] = {0};
    getDiskInfo(lUserID, struDisk);
    printf("\"disks\":[\n");
    for (int i = 0; i < struDVRBaseInfo.diskNum; i++)
    {
      printf("{\n");
      printf("\"no\": %d,\n", struDisk[i].no);
      printf("\"capacity\": %d,\n", struDisk[i].capacity);
      printf("\"freeSpace\": %d,\n", struDisk[i].freeSpace);
      printf("\"status\": %d,\n", struDisk[i].status);
      printf("\"type\": %d,\n", struDisk[i].type);
      printf("\"group\": %d\n", struDisk[i].group);
      if (i == struDVRBaseInfo.diskNum - 1) {
        printf("}\n");
      } else {
        printf("},\n");
      }
    }
    printf("],\n");
  }
  // 获取通道信息
  // 计算组数量
  iGroupSum = struDVRBaseInfo.IPChanNum / 64;
  int iChanNum = 0;
  for (int i = 0; i <= iGroupSum; i++)
  {
    NET_DVR_IPPARACFG_V40 IPAccessCfgV40 = {0};
    // lpIPChanInfo[i] = getChannelInfo(lUserID, i);
    getChannelInfo(lUserID, i, &IPAccessCfgV40);
    printf("\"IPChans\": [\n");
    int iRet = printChansInfo(IPAccessCfgV40);
    iChanNum = iRet + iChanNum;
    printf("],\n");
  }
  // 设备抓图
  NET_DVR_JPEGPARA lpJpegPara = {0};
  lpJpegPara.wPicSize = 0xff;
  lpJpegPara.wPicQuality = 2; // 图片质量，0：最好，1：较好，2：一般
  for (int i = 0; i <= struDVRBaseInfo.IPChanNum; i++) {
    char str[25] = {0};
    snprintf(str, sizeof(i), "%d", i);
    char *filename = strcat(str, ".jpg");
    NET_DVR_CaptureJPEGPicture(lUserID, struDVRBaseInfo.IPChanStart + i, &lpJpegPara, filename);
    // int iRet = NET_DVR_CaptureJPEGPicture(lUserID, struDVRBaseInfo.IPChanStart + i, &lpJpegPara, filename);
    // printf("文件名：%s，返回值：%d， 错误码：%d\n", filename, iRet, NET_DVR_GetLastError());
  }

  // 获取录像计划

  printf("\"recordCfg\": [\n");
  for (int i = 0; i < struDVRBaseInfo.IPChanNum; i++)
  {
    printf("{\n");
    getRecordCfg(lUserID, i + struDVRBaseInfo.IPChanStart);
    if (i == struDVRBaseInfo.IPChanNum - 1)
    {
      printf("}\n");
    } else
    {
      printf("},\n");
    }

  }
  printf("],\n");

  // 查找录像文件
  NET_DVR_TIME struStartTime = {
    dwYear: 2019,
    dwMonth: 9,
    dwDay: 2,
    dwHour: 0,
    dwMinute: 0,
    dwSecond: 0
  };
  NET_DVR_TIME struStopTime = {
    dwYear: 2019,
    dwMonth: 9,
    dwDay: 3,
    dwHour: 0,
    dwMinute: 0,
    dwSecond: 0
  };
  printf("\"records\":[\n");
  for (int i = 0; i < iChanNum; i++)
  {
    printf("{\n");
    getRecordFile(lUserID, i + struDVRBaseInfo.IPChanStart, struStartTime, struStopTime);
    if (i == iChanNum - 1)
    {
      printf("}\n");
    } else {
      printf("},\n");
    }
  }
  printf("]\n");
  printf("}\n");
  NET_DVR_Logout_V30(lUserID);
  NET_DVR_Cleanup();
  return 0;
}