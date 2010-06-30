
#include "DisplaySettingDetector.h"

CDisplaySettingDetector::CDisplaySettingDetector(void)
{
}

CDisplaySettingDetector::~CDisplaySettingDetector(void)
{
}

bool CDisplaySettingDetector::startswith(const char * src, const char * prefix)
{
    return strncmp(src, prefix, strlen(prefix)) == 0;
}

bool CDisplaySettingDetector::startsiwith(const char * src, const char * prefix)
{
    return strnicmp(src, prefix, strlen(prefix)) == 0;
}

void CDisplaySettingDetector::init()
{
    HKEY hk = 0;
    DWORD pt;
    int device = -1;
    Video0Name[0] = 0;

    DISPLAY_DEVICEA dd;
    ZeroMemory(&dd,sizeof(dd));
    dd.cb = sizeof(dd);
    for(int i = 0; EnumDisplayDevicesA(NULL,i,&dd,0); i++)
    {
        //printf("%d %s %s %s %s\n",i,dd.DeviceName,dd.DeviceString,dd.DeviceID,dd.DeviceKey);
        if((dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) != 0)
        {
            strcpy(Video0Name,dd.DeviceKey);
            //printf("%s\n",Video0Name);
            break;
        }
    }

    // manual search
    if(Video0Name[0] == 0)
    {
        RegOpenKeyA(HKEY_LOCAL_MACHINE,"HARDWARE\\DEVICEMAP\\VIDEO",&hk);
        if(hk != 0)
        {
            DWORD pdn = sizeof(Video0Name);
            // TODO: use MaxObjectNumber
            if(device == -1)
            {
                for(int i = 0; i < 10; i++)
                {
                    char buf[128];
                    pdn = sizeof(Video0Name);
                    sprintf(buf,"\\Device\\Video%d",i);
                    RegQueryValueExA(hk,buf,0, &pt,(LPBYTE)Video0Name,&pdn);
                    /* Make sure that it isn't a pseudo-device, e.g. smsmdd */
                    if(strstr(Video0Name, "\\Device") == 0 && strstr(Video0Name, "VgaSave") == 0 && startswith(Video0Name,"\\Registry\\Machine\\"))
                        break;
                    else
                        Video0Name[0] = 0;
                }
            }
            else
            {
                char buf[128];
                pdn = sizeof(Video0Name);
                sprintf(buf,"\\Device\\Video%d",device);
                RegQueryValueExA(hk,buf,0, &pt,(LPBYTE)Video0Name,&pdn);
            }
            /*if(Video0Name[0] == 0)
                exitError("Cannot find Video Device in Registry");
            else
                printf("Selected video %s\n",Video0Name);
                */
        }
        RegCloseKey(hk);
    }

}
int CDisplaySettingDetector::GetVideoAccelLevel()
{
    //0 == fully opened
    //5 == disabled
    HKEY hk = 0;
    DWORD level = 0;
//    char resultMessage[100];
    //printf("Video0 Device is %s\n",Video0Name);
    RegOpenKeyA(HKEY_LOCAL_MACHINE,Video0Name+strlen("\\Registry\\Machine\\"),&hk);
    if(hk != 0)
    {
        DWORD pdn = sizeof(level);
        DWORD pt;
        RegQueryValueExA(hk,"Acceleration.Level",0,&pt,(LPBYTE)&level,&pdn);
        
        RegCloseKey(hk);
        return level;
    }
    else
        return 0xff;
}
void CDisplaySettingDetector::SetVideoAccelLevel(int level)
{
    HKEY hk = 0;
    DWORD pt;
    //char resultMessage[100];
    //printf("Video0 Device is %s\n",Video0Name);
    RegOpenKeyA(HKEY_LOCAL_MACHINE,Video0Name+strlen("\\Registry\\Machine\\"),&hk);
    if(hk != 0)
    {
        DWORD pdn = sizeof(level);
        DWORD pt = 0;
       
        RegSetValueExA(hk,"Acceleration.Level",0,pt, (LPBYTE)&level,sizeof(level));
        //sprintf(resultMessage,"Acceleration is %s (%d)\n",level == 0 ? "on" : level == 5 ? "off" : "partial", level);
        RegCloseKey(hk);
    }
    else
        return ;//exitError("cannot open device Registry Key");

    char * name = strrchr(Video0Name,'\\');
    if(name != 0)
    {
        *name = 0;
        strcat(Video0Name,"\\Video");
    }
    else
        return;// ("cannot get Video Service Name");

    RegOpenKeyA(HKEY_LOCAL_MACHINE,Video0Name+strlen("\\Registry\\Machine\\"),&hk);
    if(hk != 0)
    {
        char ServiceName[100];
        DWORD pdn = sizeof(ServiceName);
        RegQueryValueExA(hk,"Service",0,&pt,(LPBYTE)&ServiceName,&pdn);
        RegCloseKey(hk);
        //printf("Service is %s\n",ServiceName);
        SC_HANDLE sc = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
        if(sc == 0)
            return; //exitError("cannot open SC Manager");

        SC_HANDLE scs = OpenServiceA(sc,ServiceName,SC_MANAGER_ALL_ACCESS);
        if(scs == 0)
            return; //exitError("cannot open Video Service");

        SERVICE_STATUS status;
        ControlService(scs, SERVICE_CONTROL_PARAMCHANGE,&status);
        CloseServiceHandle(scs);
        CloseServiceHandle(sc);

        ChangeDisplaySettings(NULL, 0); 
        //MessageBox(0,resultMessage,APPNAME,MB_OK);
        return ;
    }
    else
        return; //exitError("Cannot get the name of the Video Service");    return 0;

}
