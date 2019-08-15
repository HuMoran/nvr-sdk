   /*
 * Copyright(C) 2010,Custom Co., Ltd 
 *    FileName: playback.cpp
 * Description: 
 *     Version: 1.0
 *      Author: panyadong
 * Create Date: 2010-4-24
 * Modification History��
 */

#include "public.h"
#include <stdio.h>
#include <time.h>
#include "playback.h"
#include <stdlib.h>
#include <memory.h>

#ifdef _WIN32
#else
#include   <unistd.h> 

#endif

int saveFile(int userId,char * srcfile,char * destfile);

int g_hPlayback=0;

void CALLBACK fPlayDataCallBack(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer,DWORD dwBufSize,DWORD dwUser)
{
    int pos = NET_DVR_GetDownloadPos(g_hPlayback);
    if (pos > 98)
    {
        fprintf(stderr, "pyd---Current Time:%d, data size:%d\n", time(NULL), dwBufSize);
        printf("Play over!%d\n", pos);
    }
}

int Demo_PlayBack(LONG lUserID)
{
    //Demo_PlayBackByTime(lUserID);
	//Demo_PlayBackByName();
	//Demo_FindEvent(lUserID);
    Demo_FindFile(lUserID);
    return HPR_OK;
}

int Demo_FindFile(LONG lUserID)
{
    if(lUserID >= 0)
    {
        //find file
      //  NET_DVR_TIME struStartTime, struStopTime;
       // struStartTime.dwYear	= 2010;
//     struStartTime.dwMonth	= 6;
      //  struStartTime.dwDay		= 1;
      //  struStartTime.dwHour	= 0;
        //struStartTime.dwMinute	= 0;
       // struStartTime.dwSecond	=0;

       // struStopTime.dwYear		= 2010;
       // struStopTime.dwMonth	= 6;
       // struStopTime.dwDay		= 1;
       // struStopTime.dwHour		= 1;
       // struStopTime.dwMinute	= 0;
       // struStopTime.dwSecond	= 0;


        //int lFindHandle = NET_DVR_FindFile(lUserID, 1, 0xff, &struStartTime, &struStopTime);

       NET_DVR_FILECOND_V50 m_struFileCondV50;
       memset(&m_struFileCondV50, 0, sizeof(NET_DVR_FILECOND_V50));
       m_struFileCondV50.struStreamID.dwChannel = 1;
       m_struFileCondV50.dwFileType = 0xff;
            m_struFileCondV50.dwFileType = 255;
            m_struFileCondV50.struStartTime.wYear = 2018;
            m_struFileCondV50.struStartTime.byMonth = 10;
            m_struFileCondV50.struStartTime.byDay = 22;
            m_struFileCondV50.struStartTime.byHour = 0;
            m_struFileCondV50.struStartTime.byMinute = 0;
            m_struFileCondV50.struStartTime.bySecond = 0;
            m_struFileCondV50.struStopTime.wYear = 2018;
            m_struFileCondV50.struStopTime.byMonth = 10;
            m_struFileCondV50.struStopTime.byDay = 22;
            m_struFileCondV50.struStopTime.byHour = 23;
            m_struFileCondV50.struStopTime.byMinute = 59;
            m_struFileCondV50.struStopTime.bySecond = 59;
int lFindHandle = NET_DVR_FindFile_V50(lUserID, &m_struFileCondV50);


        if(lFindHandle < 0)
        {
            printf("find file fail,last error %d\n",NET_DVR_GetLastError());
            return 1;
        }
        NET_DVR_FINDDATA_V50 struFileData;

        while(true)
        {
            printf("NET_DVR_FindNextFile_V50\n");
            int result = NET_DVR_FindNextFile_V50(lFindHandle, &struFileData);
            if(result == NET_DVR_ISFINDING)
            {
                //nsleep(0, 20000000L); // sleep for 2/100 second;  printf("pyd---NET_DVR_RealPlay_V40  error, %d\n", NET_DVR_GetLastError());
                continue;
            }
            else if(result == NET_DVR_FILE_SUCCESS)
            {
                //char strFileName[256] = {0};
                //sprintf(strFileName, "./%s", struFileData.sFileName);
                printf("%s, %d, %d, %d-----%d,%d,%d.\n", struFileData.sFileName, 
                    struFileData.struStartTime.wYear, struFileData.struStartTime.byMonth,  struFileData.struStartTime.byDay,
                    struFileData.struStopTime.wYear, struFileData.struStopTime.byMonth,  struFileData.struStopTime.byDay);
                //saveFile(lUserID, struFileData.sFileName,  strFileName);
                break;
            }
            else if(result == NET_DVR_FILE_NOFIND || result == NET_DVR_NOMOREFILE)
            {	
                break;
            }
            else
            {
                printf("find file fail for illegal get file state\n");
                break;
            }

        }

        NET_DVR_FindClose_V30(lFindHandle);
    }

    return HPR_OK;
}

int Demo_PlayBackByName()
{
    //init
    NET_DVR_Init();
    //login
    long lUserID;
    //login
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
    struLoginInfo.bUseAsynLogin = false;

    struLoginInfo.wPort = 8000;
    memcpy(struLoginInfo.sDeviceAddress,"10.8.98.80", NET_DVR_DEV_ADDRESS_MAX_LEN);
    memcpy(struLoginInfo.sUserName, "admin", NAME_LEN);
    memcpy(struLoginInfo.sPassword, "hik12345", NAME_LEN);
    lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
    //NET_DVR_DEVICEINFO struDeviceInfo;
    //lUserID = NET_DVR_Login("172.9.204.5", 8000, "admin", "12345", &struDeviceInfo);
    if (lUserID < 0)
    {
        printf("pyd1---Login error, %d\n", NET_DVR_GetLastError());
        return HPR_ERROR;
    }

    //for (int i = 0; i < 30; i++)
    {
        //printf("%d.\n", i);
#if defined(_WIN32) || defined(_APPLE_)
        HWND hWnd = NULL;
#else
        HWND hWnd = {0};
#endif

        LONG lFindPicHandle = NET_DVR_PlayBackByName(lUserID, "ch01_02010100141000000", hWnd);
        if (lFindPicHandle < 0)
        {
            printf("pyd---NET_DVR_PlayBackControl fail: %d.\n", NET_DVR_GetLastError()); 
        }
        else
        {
            if(!NET_DVR_SetPlayDataCallBack(lFindPicHandle, fPlayDataCallBack,1))
            {
                printf("NET_DVR_SetPlayDataCallBack fail!\n");
            }

            //if (!NET_DVR_PlayBackControl(lFindPicHandle, NET_DVR_PLAYSTART, 0, NULL))
	     if (!NET_DVR_PlayBackControl_V40(lFindPicHandle, NET_DVR_PLAYSTART,  NULL, 0, NULL, NULL))
            {
                printf("pyd---NET_DVR_PlayBackControl fail.%d\n", NET_DVR_GetLastError());
            }
        }

#ifdef _WIN32
        Sleep(5000);  //millisecond
#elif  defined(__linux__) || defined(__APPLE__)
        sleep(500);   //second
#endif
    }

    NET_DVR_Logout_V30(lUserID);
    //cleanup
    NET_DVR_Cleanup();

    return HPR_OK;
}

int Demo_PlayBackByTime(LONG lUserID)
{
	int hPlayback;

		//find file
#if defined(__linux__)
		HWND hWnd = {0};
#else
		HWND hWnd = NULL;
#endif
		NET_DVR_VOD_PARA struVodPara = {0};
        struVodPara.struBeginTime.dwYear = 2018;
        struVodPara.struBeginTime.dwMonth = 10;
        struVodPara.struBeginTime.dwDay = 22;
        struVodPara.struBeginTime.dwHour = 0;
        struVodPara.struBeginTime.dwMinute = 0;
        struVodPara.struBeginTime.dwSecond = 0;
        struVodPara.struEndTime.dwYear = 2018;
        struVodPara.struEndTime.dwMonth = 10;
        struVodPara.struEndTime.dwDay = 22;
        struVodPara.struEndTime.dwHour = 23;
        struVodPara.struEndTime.dwMinute = 59;
        struVodPara.struEndTime.dwSecond = 59;
        struVodPara.struIDInfo.dwChannel = 1;
        struVodPara.hWnd = hWnd;


		//��ʱ������
		hPlayback = NET_DVR_PlayBackByTime_V40(lUserID, &struVodPara);
		//HWND struPlayRect = {0};//!!!!must init.
		//hPlayback = NET_DVR_PlayBackByTime(lUserID, 1, &struStartTime, &struStopTime, struPlayRect);
		if(hPlayback < 0)
		{
			printf("NET_DVR_PlayBackByTime_V40 fail,last error %d\n",NET_DVR_GetLastError());
			return 1;
		} 

	g_hPlayback = hPlayback;

	//����ȡ���ص�
	if(!NET_DVR_SetPlayDataCallBack(hPlayback, fPlayDataCallBack,1))
	{
		printf("NET_DVR_SetPlayDataCallBack fail!\n");
	}

	if(!NET_DVR_PlayBackControl(hPlayback, NET_DVR_PLAYSTART, 0, NULL))
	{
		printf("play back control failed [%d]\n",NET_DVR_GetLastError());
		return HPR_ERROR;
	}

#if defined(_WIN32)
	Sleep(1000);
#else
	sleep(5);
#endif

	//sleep(180); //˯3����

	//if(!NET_DVR_StopPlayBack(hPlayback))
	//{
	//	printf("failed to stop get file [%d]\n",NET_DVR_GetLastError());
	//	return HPR_ERROR;
	//}

	//NET_DVR_Logout(lUserID);
	//NET_DVR_Logout_V30(lUserID);
	//cleanup
	NET_DVR_Cleanup();

	return HPR_OK;
}

int Demo_GetFileByTime()
{
    //init
    NET_DVR_Init();
    //login
    long lUserID;
    //login
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
    struLoginInfo.bUseAsynLogin = false;

    struLoginInfo.wPort = 8000;
    memcpy(struLoginInfo.sDeviceAddress, "10.8.98.80", NET_DVR_DEV_ADDRESS_MAX_LEN);
    memcpy(struLoginInfo.sUserName, "admin", NAME_LEN);
    memcpy(struLoginInfo.sPassword, "hik12345", NAME_LEN);
    lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);

    if (lUserID < 0)
    {
        printf("pyd1---Login error, %d\n", NET_DVR_GetLastError());
        return HPR_ERROR;
    }

    int hPlayback;

    if(lUserID >= 0)
    {
        //find file
		NET_DVR_PLAYCOND struDownloadCond = {0};
		struDownloadCond.struStartTime.dwYear = 2018;
		struDownloadCond.struStartTime.dwMonth = 10;
		struDownloadCond.struStartTime.dwDay = 22;
		struDownloadCond.struStartTime.dwHour = 0;
		struDownloadCond.struStartTime.dwMinute = 0;
		struDownloadCond.struStartTime.dwSecond = 0;
		struDownloadCond.struStopTime.dwYear = 2018;
		struDownloadCond.struStopTime.dwMonth = 10;
		struDownloadCond.struStopTime.dwDay = 22;
		struDownloadCond.struStopTime.dwHour = 23;
		struDownloadCond.struStopTime.dwMinute = 59;
		struDownloadCond.struStopTime.dwSecond = 59;  
		struDownloadCond.dwChannel = 1;

	   hPlayback = NET_DVR_GetFileByTime_V40(lUserID, "./test.mp4", &struDownloadCond);


        //HWND struPlayRect = {0};//!!!!must init.
        //hPlayback = NET_DVR_PlayBackByTime(lUserID, 1, &struStartTime, &struStopTime, struPlayRect);
        if(hPlayback < 0)
        {
            printf("NET_DVR_GetFileByTime_V40 fail,last error %d\n",NET_DVR_GetLastError());
            return 1;
        } 
    }

    g_hPlayback = hPlayback;

    //����ȡ���ص�
    if(!NET_DVR_SetPlayDataCallBack(hPlayback, fPlayDataCallBack,1))
    {
        printf("NET_DVR_SetPlayDataCallBack fail!\n");
    }

    if(!NET_DVR_PlayBackControl(hPlayback, NET_DVR_PLAYSTART, 0, NULL))
    {
        printf("play back control failed [%d]\n",NET_DVR_GetLastError());
        return HPR_ERROR;
    }

    int pos=0;
    for(pos = 0;  pos < 100&&pos>=0; pos = NET_DVR_GetDownloadPos(hPlayback))
    {
#ifdef _WIN32
        Sleep(5000);  //millisecond
#elif  defined(__linux__)
        sleep(500);   //second
#endif
    }

    printf("have got %d\n", pos);
    if(!NET_DVR_StopGetFile(hPlayback))
    {
        printf("failed to stop get file [%d]\n",NET_DVR_GetLastError());
        return HPR_ERROR;
    }

    printf("StopGetFile\n");
    if(pos<0||pos>100)
    {
        printf("download err [%d]\n",NET_DVR_GetLastError());
        return HPR_ERROR;

    }else
    {
        return 0;
    }


    //sleep(180); //˯3����
    NET_DVR_Logout_V30(lUserID);
    //cleanup
    NET_DVR_Cleanup();

    return HPR_OK;
}


int saveFile(int userId,char * srcfile,char * destfile)
{

    int bRes = 1;

    int hPlayback=0;
    if( (hPlayback = NET_DVR_GetFileByName(userId, srcfile, destfile)) < 0 )
    {
        printf( "GetFileByName failed. error[%d]\n", NET_DVR_GetLastError());
        bRes= -1;
        return bRes;
    }



    if(!NET_DVR_PlayBackControl(hPlayback, NET_DVR_PLAYSTART, 0, NULL))
    {

        printf("play back control failed [%d]\n",NET_DVR_GetLastError());

        bRes=-1;

        return bRes;

    }

    int pos=0;

    for(pos = 0;  pos < 100&&pos>=0; pos = NET_DVR_GetDownloadPos(hPlayback))
    {
#ifdef _WIN32
        Sleep(5000);  //millisecond
#elif  defined(__linux__)
        sleep(500);   //second
#endif
    }

    printf("have got %d\n", pos);

    if(!NET_DVR_StopGetFile(hPlayback))
    {
        printf("failed to stop get file [%d]\n",NET_DVR_GetLastError());
        bRes = -1;
        return bRes;
    }

    printf("StopGetFile\n");

    printf("%s\n",srcfile);

    if(pos<0||pos>100){

        printf("download err [%d]\n",NET_DVR_GetLastError());

        bRes=-1;

        return bRes;

    }else{

        return 0;

    }

}


//Find event
int Demo_FindEvent(LONG lUserID)
{
	NET_DVR_SEARCH_EVENT_PARAM struEventParams;
	memset(&struEventParams, 0, sizeof(struEventParams));
	struEventParams.wMajorType = 0;
	struEventParams.wMinorType = 0xffff;
	
	struEventParams.uSeniorParam.struMotionParam.byMotDetChanNo[0] = 1;
	
	
	int lFindHandle = NET_DVR_FindFileByEvent(lUserID,  &struEventParams);
	if(lFindHandle < 0)
	{
		printf("find file fail,last error %d\n",NET_DVR_GetLastError());
		return 1;
	}
	
	NET_DVR_SEARCH_EVENT_RET struEventInfo;
	while(true)
	{
		memset(&struEventInfo, 0, sizeof(NET_DVR_SEARCH_EVENT_RET));
		int result = NET_DVR_FindNextEvent(lFindHandle, &struEventInfo);
		if(result == NET_DVR_ISFINDING)
		{
			//nsleep(0, 20000000L); // sleep for 2/100 second;
			continue;
		}
		else if(result == NET_DVR_FILE_SUCCESS)
		{
			printf("Get a event!\n");
			continue;
		}
		else if(result == NET_DVR_FILE_NOFIND || result == NET_DVR_NOMOREFILE)
		{	
			break;
		}
		else
		{
			printf("find file fail for illegal get file state");
			break;
		}
		
	}
	
	NET_DVR_FindClose(lFindHandle);
	return 0;
}


