!include "inc.Var.nsi"

Function .onInit
 !include "inc.OnInt.nsi"

FunctionEnd

OutFile "..\..\allSetup.exe"

SectionGroup /e "影音解码包"
; The stuff to install
Section  "FFDShow"  ffdshow
    UnRegDLL $INSTDIR\codecs\ffdshow.ax
    Delete /REBOOTOK $INSTDIR\codecs\ffdshow.ax

    SetOutPath $INSTDIR\codecs
    File ..\..\svplayer.bin\ffdshow\*.*
    SetOutPath $INSTDIR\codecs\languages
    File ..\..\svplayer.bin\ffdshow\languages\*.*
    SetOutPath $SYSDIR
    File ..\..\svplayer.bin\ffdshow\sys\ff_acm.acm
    File ..\..\svplayer.bin\ffdshow\sys\ff_vfw.dll
    File ..\..\svplayer.bin\ffdshow\sys\ff_vfw.dll.manifest
    File ..\..\svplayer.bin\ffdshow\sys\pthreadGC2.dll
    
    RegDLL $INSTDIR\codecs\ffdshow.ax

    ;写入注册表
	WriteRegStr HKCU "Software\GNU\ffdshow" "lang" "SC"
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "isBlacklist" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "lastPage" 0x00000073
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "trayIcon" 0x00000000
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "trayIconExt" 0x00000000
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "OSDfontFast" 0x00000000
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "xvid" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "div3" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "divx" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "dx50" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "mp43" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "mp42" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "mp41" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "_3iv" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "ffv1" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "fvfw" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "hfyu" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "iv32" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "png1" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "zlib" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "cvid" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "h261" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "h263" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "h264" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "fastH264" 0x00000000
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "theo" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "tscc" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "vp3" 0x00000001

	WriteRegDWORD HKCU "Software\GNU\ffdshow" "mjpg" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "dvsd" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "cyuv" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "asv1" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "vcr1" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "svq1" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "svq3" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "cram" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "rv10" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "rle" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "mszh" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "flv1" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "8bps" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "qtrle" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "duck" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "qpeg" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "loco" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "zmbv" 0x00000000

	WriteRegDWORD HKCU "Software\GNU\ffdshow" "mpg1" 0x00000005
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "mpg2" 0x00000005
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "mpegAVI" 0x00000005

	WriteRegDWORD HKCU "Software\GNU\ffdshow" "wmv1" 0x00000000
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "wmv2" 0x00000000
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "wmv3" 0x00000000

	WriteRegDWORD HKCU "Software\GNU\ffdshow" "avis" 0x00000000
	WriteRegDWORD HKCU "Software\GNU\ffdshow" "rawv" 0x00000000

	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio" "streamsOptionsMenu" 0x00000000
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio" "trayIcon" 0x00000000
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio" "trayIconExt" 0x00000000
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio" "isAudioSwitcher" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio" "isBlacklist" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio" "lastPage" 0x00000074
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio" "iadpcm" 0x00000000
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio" "tta" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio" "aac" 0x00000008
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio" "mp2" 0x00000000
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio" "mp3" 0x00000006
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio" "ac3" 0x00000000
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio" "dts" 0x00000000
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio" "lpcm" 0x00000004
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio" "vorbis" 0x00000012
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio" "amr" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio\default" "isMixer" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio\default" "mixerNormalizeMatrix" 0x00000000
	WriteRegDWORD HKCU "Software\GNU\ffdshow_audio\default" "mixerOut" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_enc" "lastPage" 0x00000066
	WriteRegStr HKCU "Software\GNU\ffdshow_vfw" "pth" "$SYSDIR"
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "lastPage" 0x00000073
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "_3iv" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "div3" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "divx" 0x00000009
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "duck" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "dvsd" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "dx50" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "ffv1" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "fvfw" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "h263" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "h264" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "hfyu" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "iv32" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "mjpg" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "png1" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "qtrle" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "svq1" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "svq3" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "theo" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "tscc" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "vp3" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "vcr1" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "xvid" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "zlib" 0x00000001
	WriteRegDWORD HKCU "Software\GNU\ffdshow_vfw" "cvid" 0x00000001
	WriteRegStr HKLM "SOFTWARE\GNU\ffdshow" "pth" "$INSTDIR\codecs"

SectionEnd

Section  "H.264解码"  coreavc
    SetOutPath $INSTDIR\codecs
    File ..\..\svplayer.bin\coreavc\CoreAVCDecoder.ax
    WriteRegStr HKLM "SOFTWARE\CoreCodec\CoreAVC Pro" "Serial" "03JUN-10K9Y-CORE-0CLQV-JOTFL"
    WriteRegStr HKLM "SOFTWARE\CoreCodec\CoreAVC Pro" "User" "Registered User"
    RegDLL $INSTDIR\codecs\CoreAVCDecoder.ax
    WriteRegDWORD HKCU "Software\GNU\ffdshow" "h264" 0x00000000
SectionEnd

/*
Section  "Haali 媒体切分器" haali
    SetOutPath $INSTDIR\codecs\haali
    File ..\..\svplayer.bin\MatroskaSplitter\*.*
    RegDLL $INSTDIR\codecs\haali\splitter.ax
SectionEnd
*/

Section  "Real Media"  realcodec

SetOutPath $INSTDIR
    File ..\..\svplayer.bin\Real\keys.dat
    SetOutPath $INSTDIR\codecs
    File ..\..\svplayer.bin\Real\realcfg.exe
    File ..\..\svplayer.bin\Real\realcfg.ini
    File ..\..\svplayer.bin\Real\embed_cn.dll
    File ..\..\svplayer.bin\Real\rpclsvc_cn.dll
    File ..\..\svplayer.bin\Real\rpclutil_cn.dll
    File ..\..\svplayer.bin\Real\rpgutil_cn.dll
    File ..\..\svplayer.bin\Real\RealMediaSplitter2000.ax
    RegDLL $INSTDIR\codecs\RealMediaSplitter2000.ax
    SetOutPath $SYSDIR
    File ..\..\svplayer.bin\Real\pncrt.dll
    File ..\..\svplayer.bin\Real\rmoc3260.dll
    RegDLL $SYSDIR\rmoc3260.dll
    File ..\..\svplayer.bin\Real\pndx5016.dll
    File ..\..\svplayer.bin\Real\pndx5032.dll
    SetOutPath $INSTDIR\rpplugins
    File ..\..\svplayer.bin\Real\rpplugins\*.*
    SetOutPath $COMMONFILES\Real\codecs
    File ..\..\svplayer.bin\Real\codecs\*.*
    SetOutPath $COMMONFILES\Real\Common
    File ..\..\svplayer.bin\Real\Common\*.*
    SetOutPath $COMMONFILES\Real\Plugins
    File ..\..\svplayer.bin\Real\Plugins\*.*
    SetOutPath $COMMONFILES\Real\Plugins\ExtResources
    File ..\..\svplayer.bin\Real\Plugins\ExtResources\*.*

    DeleteRegKey HKLM "SOFTWARE\Microsoft\Internet Explorer\ActiveX Compatibility\{CFCDAA03-8BE4-11CF-B84B-0020AFBBCCFA}"


	WriteRegStr HKCR "rtsp" "" "Real-Time Streaming Protocol"
	WriteRegBin HKCR "rtsp" "EditFlags" 02000000
	WriteRegStr HKCR "rtsp" "URL Protocol" ""
	WriteRegStr HKCR "rtsp\DefaultIcon" "" '"$INSTDIR\mplayerc.exe"'
	WriteRegStr HKCR "rtsp\shell\open\command" "" '"$INSTDIR\mplayerc.exe" "%L"'
	WriteRegStr HKCR "pnm" "" "RealNetworks Streaming Protocol"
	WriteRegBin HKCR "pnm" "EditFlags" 03000000
	WriteRegStr HKCR "pnm" "URL Protocol" ""
	WriteRegStr HKCR "pnm\DefaultIcon" "" '"$INSTDIR\mplayerc.exe"'
	WriteRegStr HKCR "pnm\shell\open\command" "" '"$INSTDIR\mplayerc.exe" "%L"'
	WriteRegStr HKCR "pnm\shellex\ContextMenuHandlers\RealPlayerHandler" "" "{F0CB00CD-5A07-4D91-97F5-A8C92CDA93E4}"
	WriteRegStr HKCR "MIME\Database\Content Type\application/sdp" "Extension" ".sdp"
	WriteRegStr HKCR "MIME\Database\Content Type\application/smil" "Extension" ".smil"
	WriteRegStr HKCR "MIME\Database\Content Type\application/streamingmedia" "Extension" ".ssm"
	WriteRegStr HKCR "MIME\Database\Content Type\application/vnd.rn-realmedia" "CLSID" "{CFCDAA03-8BE4-11CF-B84B-0020AFBBCCFA}"
	WriteRegStr HKCR "MIME\Database\Content Type\application/vnd.rn-realmedia" "Extension" ".rm"
	WriteRegStr HKCR "MIME\Database\Content Type\application/vnd.rn-realplayer" "Extension" ".rnx"
	WriteRegStr HKCR "MIME\Database\Content Type\application/vnd.rn-rsml" "Extension" ".rsml"
	WriteRegStr HKCR "MIME\Database\Content Type\application/x-rtsp" "CLSID" "{02BF25D5-8C17-4B23-BC80-D3488ABDDC6B}"
	WriteRegStr HKCR "MIME\Database\Content Type\application/x-rtsp" "Extension" ".rtsp"
	WriteRegStr HKCR "MIME\Database\Content Type\audio/vnd.rn-realaudio" "Extension" ".ra"
	WriteRegStr HKCR "MIME\Database\Content Type\audio/x-pn-realaudio-plugin" "CLSID" "{CFCDAA03-8BE4-11CF-B84B-0020AFBBCCFA}"
	WriteRegStr HKCR "MIME\Database\Content Type\audio/x-realaudio" "CLSID" "{CFCDAA03-8BE4-11CF-B84B-0020AFBBCCFA}"
	WriteRegStr HKCR "MIME\Database\Content Type\audio/x-realaudio" "Extension" ".ra"
	WriteRegStr HKCR "MIME\Database\Content Type\image/vnd.rn-realpix" "Extension" ".rp"
	WriteRegStr HKCR "MIME\Database\Content Type\text/vnd.rn-realtext" "Extension" ".rt"
	WriteRegStr HKCR "MIME\Database\Content Type\video/vnd.rn-realvideo" "CLSID" "{CFCDAA03-8BE4-11CF-B84B-0020AFBBCCFA}"
	WriteRegStr HKCR "MIME\Database\Content Type\video/vnd.rn-realvideo" "Extension" ".rv"
	WriteRegStr HKCR "Software\RealNetworks\Preferences\DT_Codecs" "" "$COMMONFILES\Real\Codecs\"
	WriteRegStr HKCR "Software\RealNetworks\Preferences\DT_Common" "" "$COMMONFILES\Real\Common\"
	WriteRegStr HKCR "Software\RealNetworks\Preferences\DT_Objbrokr" "" "$COMMONFILES\Real\Common\"
	WriteRegStr HKCR "Software\RealNetworks\Preferences\DT_Plugins" "" "$COMMONFILES\Real\Plugins\"
	WriteRegStr HKCR "Software\RealNetworks\Preferences\DT_Update_OB" "" "$COMMONFILES\Real\Update_OB\"
	WriteRegStr HKCR "Software\RealNetworks\RealMediaSDK\6.0\Preferences\Bandwidth" "" "10485800"
	WriteRegStr HKCR "Software\RealNetworks\RealMediaSDK\6.0\Preferences\BandwidthNotKnown" "" "0"
	WriteRegStr HKCR "Software\RealNetworks\RealMediaSDK\6.0\Preferences\BufferedPlayTime" "" "30"
	WriteRegStr HKCR "Software\RealNetworks\RealMediaSDK\6.0\Preferences\CacheDefaultTTL" "" "3600"
	WriteRegStr HKCR "Software\RealNetworks\RealMediaSDK\6.0\Preferences\HTTPProxyAutoConfig" "" "0"
	WriteRegStr HKCR "Software\RealNetworks\RealMediaSDK\6.0\Preferences\HTTPProxySupport" "" "0"
	WriteRegStr HKCR "Software\RealNetworks\RealMediaSDK\6.0\Preferences\PerfectPlay" "" "0"
	WriteRegStr HKCR "Software\RealNetworks\RealMediaSDK\6.0\Preferences\PerfectPlayTime" "" "30"
	WriteRegStr HKCR "Software\RealNetworks\RealMediaSDK\6.0\Preferences\PerfPlayEntireClip" "" "0"
	WriteRegStr HKCR "Software\RealNetworks\RealMediaSDK\6.0\Preferences\Rotuma" "" "efggdhhinjjfhlcmeonrkuktlrjrptqufkrgkioiijdkhlpljnorgpptloknqtmpdfhjdmfi"
	WriteRegStr HKCR "Software\RealNetworks\RealMediaSDK\6.0\Preferences\UseSystemProxySettings" "" "1"
	WriteRegStr HKCR "Software\RealNetworks\RealPlayer\6.0\Preferences\ApplicationName" "" "RealPlayer"
	WriteRegStr HKCR "Software\RealNetworks\RealPlayer\6.0\Preferences\ClassName" "" "GeminiWindowClass|The RNWK AppBar"
	WriteRegStr HKCR "Software\RealNetworks\RealPlayer\6.0\Preferences\ClientLicenseKey" "" "0000000000006000C6A2000000007FF7FF00"
	WriteRegStr HKCR "Software\RealNetworks\RealPlayer\6.0\Preferences\DisplayName" "" "RealPlayer"
	WriteRegStr HKCR "Software\RealNetworks\RealPlayer\6.0\Preferences\DistCode" "" "RN30RD"
	WriteRegStr HKCR "Software\RealNetworks\RealPlayer\6.0\Preferences\LangID" "" "zh-cn"
	WriteRegStr HKCR "Software\RealNetworks\RealPlayer\6.0\Preferences\MainApp" "" "$INSTDIR\mplayerc.exe"
	WriteRegStr HKCR "Software\RealNetworks\RealPlayer\6.0\Preferences\OrigCode" "" "RN30RD"
	WriteRegStr HKCR "Software\RealNetworks\RealPlayer\6.0\Preferences\PluginFilePath" "" "$INSTDIR\rpplugins"
	WriteRegStr HKCR "Software\RealNetworks\RealPlayer\6.0\Preferences\Satellite3" "" "$INSTDIR\rpplugins\cn\embed_cn.dll"
	WriteRegStr HKCR "Software\RealNetworks\RealPlayer\6.0\Preferences\Satellite17" "" "$INSTDIR\rpplugins\cn\rpclsvc_cn.dll"
	WriteRegStr HKCR "Software\RealNetworks\RealPlayer\6.0\Preferences\Title" "" "RealPlayer"
	WriteRegStr HKCR "Software\RealNetworks\RealPlayer\6.0\Preferences\TriedAutoConfig" "" "0"
	WriteRegStr HKCR "Software\RealNetworks\RealPlayer\CurrentVersion" "" "6.0"
	WriteRegStr HKCU "Software\RealNetworks\RealPlayer\6.0\Preferences\EnableInstantPlayback" "" "1"
	WriteRegStr HKCU "Software\RealNetworks\RealPlayer\6.0\Preferences\GetHTTPProxyFromBrowser" "" "0"
	WriteRegStr HKCR "Software\RealNetworks\RealPlayer\6.0\Preferences\InitialVolume" "" "50"
	WriteRegStr HKCR "Software\RealNetworks\RealPlayer\6.0\Preferences\Language" "" "zh-cn"
	WriteRegStr HKCU "Software\RealNetworks\RealPlayer\6.0\Preferences\NetdetectOptions" "" "1"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\Bandwidth" "" "10485800"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\TurboPlay" "" "1"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\AllowAuthID" "" "0"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\BufferedPlayTime" "" "30"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\ConnectionTimeout" "" "20"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\CookiesEnabled" "" "1"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\HTTPProxyHost" "" ""
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\HTTPProxyPort" "" "80"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\HTTPProxySupport" "" "0"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\MaxBandwidth" "" "10485800"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\NoProxyFor" "" ""
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\PNAProxyHost" "" ""
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\PNAProxyPort" "" ""
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\PNAProxySupport" "" "0"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\Quality" "" "4"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\RTSPProxyHost" "" ""
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\RTSPProxyPort" "" ""
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\RTSPProxySupport" "" "0"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\SendStatistics" "" "1"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\ServerTimeOut" "" "90"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\TurboPlay" "" "1"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\UseUDPPort" "" "0"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\6.0\Preferences\Volume" "" "-1"
	WriteRegStr HKCU "Software\RealNetworks\RealMediaSDK\10.0\Preferences\Volume" "" "-1"
SectionEnd
;*/
;--------------------------------
SectionGroupEnd


Section "Uninstall"

  !include "inc.uninstall.nsi"

SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${mplayerc} "播放器主程序，必须安装"
!insertmacro MUI_DESCRIPTION_TEXT ${ffdshow} "FFdshow解码器，满足大部分视频播放的要求"
!insertmacro MUI_DESCRIPTION_TEXT ${realcodec}  "Real解码器，如果您已安装过realone或其他real解码器，则无须再次安装"
!insertmacro MUI_DESCRIPTION_TEXT ${coreavc}  "高画质的H.264解码器"
;!insertmacro MUI_DESCRIPTION_TEXT ${haali} "支持包括mkv、ts在内的多种高清文件格式"
!insertmacro MUI_FUNCTION_DESCRIPTION_END
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
