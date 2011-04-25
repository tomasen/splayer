#include "stdafx.h"
#include "UpdateController.h"

#include "hashcontroller.h"
#include "LazyInstance.h"
#include <Strings.h>
#include "../model/SubTransFormat.h"
#include "../../mplayerc\revision.h"

#include <sinet.h>
#include <windows.h>
#include <fstream>
#include <string>

#include <Softpub.h>
#include <wincrypt.h>
#include <wintrust.h>

// Link with the Wintrust.lib file.
#pragma comment (lib, "wintrust")

using namespace sinet;

BOOL VerifyEmbeddedSignature(LPCWSTR pwszSourceFile)
{
  LONG lStatus;
  DWORD dwLastError;

  // Initialize the WINTRUST_FILE_INFO structure.

  WINTRUST_FILE_INFO FileData;
  memset(&FileData, 0, sizeof(FileData));
  FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
  FileData.pcwszFilePath = pwszSourceFile;
  FileData.hFile = NULL;
  FileData.pgKnownSubject = NULL;

  /*
  WVTPolicyGUID specifies the policy to apply on the file
  WINTRUST_ACTION_GENERIC_VERIFY_V2 policy checks:

  1) The certificate used to sign the file chains up to a root 
  certificate located in the trusted root certificate store. This 
  implies that the identity of the publisher has been verified by 
  a certification authority.

  2) In cases where user interface is displayed (which this example
  does not do), WinVerifyTrust will check for whether the  
  end entity certificate is stored in the trusted publisher store,  
  implying that the user trusts content from this publisher.

  3) The end entity certificate has sufficient permission to sign 
  code, as indicated by the presence of a code signing EKU or no 
  EKU.
  */

  GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
  WINTRUST_DATA WinTrustData;

  // Initialize the WinVerifyTrust input data structure.

  // Default all fields to 0.
  memset(&WinTrustData, 0, sizeof(WinTrustData));

  WinTrustData.cbStruct = sizeof(WinTrustData);

  // Use default code signing EKU.
  WinTrustData.pPolicyCallbackData = NULL;

  // No data to pass to SIP.
  WinTrustData.pSIPClientData = NULL;

  // Disable WVT UI.
  WinTrustData.dwUIChoice = WTD_UI_NONE;

  // No revocation checking.
  WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE; 

  // Verify an embedded signature on a file.
  WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;

  // Default verification.
  WinTrustData.dwStateAction = 0;

  // Not applicable for default verification of embedded signature.
  WinTrustData.hWVTStateData = NULL;

  // Not used.
  WinTrustData.pwszURLReference = NULL;

  // This is not applicable if there is no UI because it changes 
  // the UI to accommodate running applications instead of 
  // installing applications.
  WinTrustData.dwUIContext = 0;

  // Set pFile.
  WinTrustData.pFile = &FileData;

  // WinVerifyTrust verifies signatures as specified by the GUID 
  // and Wintrust_Data.
  lStatus = WinVerifyTrust(
    NULL,
    &WVTPolicyGUID,
    &WinTrustData);

  switch (lStatus) 
  {
  case ERROR_SUCCESS:
    /*
    Signed file:
    - Hash that represents the subject is trusted.

    - Trusted publisher without any verification errors.

    - UI was disabled in dwUIChoice. No publisher or 
    time stamp chain errors.

    - UI was enabled in dwUIChoice and the user clicked 
    "Yes" when asked to install and run the signed 
    subject.
    */
    Logging(L"The file \"%s\" is signed and the signature "
      L"was verified.\n",
      pwszSourceFile);
    return true;
    break;

  case TRUST_E_NOSIGNATURE:
    // The file was not signed or had a signature 
    // that was not valid.

    // Get the reason for no signature.
    dwLastError = GetLastError();
    if (TRUST_E_NOSIGNATURE == dwLastError ||
      TRUST_E_SUBJECT_FORM_UNKNOWN == dwLastError ||
      TRUST_E_PROVIDER_UNKNOWN == dwLastError) 
    {
      // The file was not signed.
      Logging(L"The file \"%s\" is not signed.\n",
        pwszSourceFile);
    } 
    else 
    {
      // The signature was not valid or there was an error 
      // opening the file.
      Logging(L"An unknown error occurred trying to "
        L"verify the signature of the \"%s\" file.\n",
        pwszSourceFile);
    }

    break;

  case TRUST_E_EXPLICIT_DISTRUST:
    // The hash that represents the subject or the publisher 
    // is not allowed by the admin or user.
    Logging(L"The signature is present, but specifically "
      L"disallowed.\n");
    break;

  case TRUST_E_SUBJECT_NOT_TRUSTED:
    // The user clicked "No" when asked to install and run.
    Logging(L"The signature is present, but not "
      L"trusted.\n");
    break;

  case CRYPT_E_SECURITY_SETTINGS:
    /*
    The hash that represents the subject or the publisher 
    was not explicitly trusted by the admin and the 
    admin policy has disabled user trust. No signature, 
    publisher or time stamp errors.
    */
    Logging(L"CRYPT_E_SECURITY_SETTINGS - The hash "
      L"representing the subject or the publisher wasn't "
      L"explicitly trusted by the admin and admin policy "
      L"has disabled user trust. No signature, publisher "
      L"or timestamp errors.\n");
    break;

  default:
    // The UI was disabled in dwUIChoice or the admin policy 
    // has disabled user trust. lStatus contains the 
    // publisher or time stamp chain error.
    Logging(L"Error is: 0x%x.\n",
      lStatus);
    break;
  }

  return false;
}


UpdateController::UpdateController(void)
{
  m_localversion = SVP_REV_NUMBER;
}

void UpdateController::Start()
{
  _Stop(300,50);
  _Start();
}

void UpdateController::_Thread()
{
  CheckUpdateEXEUpdate();
}

bool UpdateController::CheckUpdateEXEUpdate()
{
  std::wstring ftmp, pwd, updater_path;

  wchar_t path[MAX_PATH];
  ::GetModuleFileName(NULL, path, MAX_PATH);
  pwd = path;
  pwd = pwd.substr(0, pwd.find_last_of(L'\\')+1);

  updater_path = pwd + L"Updater.exe";

  std::wstring updater_version_hash = 
                HashController::GetInstance()->GetVersionHash(updater_path.c_str());

  // if dir not writable, return 0
  if(SubTransFormat::IfDirWritable_STL(pwd) == false)
    return false;

  if(::GetTempPath(MAX_PATH, path) == 0 )
    return false;
  ftmp = path;
  ftmp += L"sptmpupdater.tmp";

  int tryid = rand()%2 + 1;

  sinet::refptr<sinet::pool>     net_pool = sinet::pool::create_instance();
  sinet::refptr<sinet::task>     net_task = sinet::task::create_instance();
  sinet::refptr<sinet::request>  net_rqst = sinet::request::create_instance();
  sinet::refptr<sinet::postdata> net_pd   = sinet::postdata::create_instance();
  
  refptr<config> cfg = config::create_instance();
  
  // string package format: "branch=updater%s&current=%s&updaterhash=%s", BRANCHVER, fileversion and length
  // 将文件版本号+文件长度hash 发送给 http://svplayer.shooter.cn/api/updater.php 
  sinet::refptr<sinet::postdataelem> net_pelem1 = sinet::postdataelem::create_instance();
  net_pelem1->set_name(L"branch");
  std::wstring str = L"updater";
  str.append(BRANCHVER);
  net_pelem1->setto_text(str.c_str());
  net_pd->add_elem(net_pelem1);

  sinet::refptr<sinet::postdataelem> net_pelem2 = sinet::postdataelem::create_instance();
  net_pelem2->set_name(L"current");
  net_pelem2->setto_text(updater_version_hash.c_str());
  net_pd->add_elem(net_pelem2);

  std::wstring url = GetServerUrl('upda', 0);

  net_rqst->set_request_url(url.c_str());
  net_rqst->set_request_method(REQ_POST);
  net_rqst->set_request_outmode(REQ_OUTFILE);
  net_rqst->set_postdata(net_pd);
  net_rqst->set_outfile(ftmp.c_str());

  SinetConfig(cfg, tryid);
  net_task->use_config(cfg);
  net_task->append_request(net_rqst);
  net_pool->execute(net_task);

  while (net_pool->is_running_or_queued(net_task))
  {
    if (_Exit_state(400))
      return 0;
  }
  
  bool ret = false;
  int err = net_rqst->get_response_errcode();
  if (err == 0) // successed
  {
    // unpackGz the tmp file to updater.exe
    if (0 == SubTransFormat::UnpackGZFile(ftmp, updater_path)) // successed
      ret = true;
  }
  if (!updater_version_hash.empty() && (ret == true || err == 404))
  {
    if (VerifyEmbeddedSignature(updater_path.c_str()))
      ::ShellExecute(NULL, L"open", updater_path.c_str(), L" /hide ", NULL, SW_SHOWMINNOACTIVE);
  }
  return ret;
}

