#include "StdAfx.h"
#include "vpversion.h"
#include "buildnumber.h"
#include "resource.h"
#include "hash.h"
#include <algorithm>

#define HASHLENGTH 16

#if _MSC_VER <= 1310 // VC 2003 and before
 #define _itoa_s(a,b,c,d) _itoa(a,b,d)
 #define _itow_s(a,b,c,d) _itow(a,b,d)
#endif

const unsigned char TABLE_KEY[] = "Visual Pinball";
const unsigned char PARAPHRASE_KEY[] = { 0xB4, 0x0B, 0xBE, 0x37, 0xC3, 0x0C, 0x8E, 0xA1, 0x5A, 0x05, 0xDF, 0x1B, 0x2D, 0x02, 0xEF, 0x8D };

INT_PTR CALLBACK ProgressProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK TableWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define TIMER_ID_AUTOSAVE 12345
#define TIMER_ID_CLOSE_TABLE 12346


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void ScriptGlobalTable::Init(PinTable *pt)
{
   m_pt = pt;
}

STDMETHODIMP ScriptGlobalTable::Nudge(float Angle, float Force)
{
   if (g_pplayer)
   {
      const float sn = sinf(ANGTORAD(Angle));
      const float cs = cosf(ANGTORAD(Angle));

      g_pplayer->m_tableVel.x +=  sn * Force;
      g_pplayer->m_tableVel.y += -cs * Force;
   }

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::PlaySound(BSTR bstr, long LoopCount, float volume, float pan, float randompitch, long pitch, VARIANT_BOOL usesame, VARIANT_BOOL restart)
{
   if (g_pplayer && g_pplayer->m_fPlaySound) m_pt->PlaySound(bstr, LoopCount, volume, pan, randompitch, pitch, usesame, restart);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::FireKnocker(int Count)
{
   if (g_pplayer) m_pt->FireKnocker(Count);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::QuitPlayer(int CloseType)
{
   if (g_pplayer) m_pt->QuitPlayer(CloseType);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::StartShake()
{
   m_pt->StartShake();

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::StopShake()
{
   m_pt->StopShake();

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::StopSound(BSTR Sound)
{
   if (g_pplayer && g_pplayer->m_fPlaySound)
      m_pt->StopSound(Sound);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::PlayMusic(BSTR str)
{
   if (g_pplayer && g_pplayer->m_fPlayMusic)
   {
      if (g_pplayer->m_pxap)
         EndMusic();

      char szT[512];
      char szPath[MAX_PATH+512];

      WideCharToMultiByte(CP_ACP, 0, g_pvp->m_wzMyPath, -1, szPath, MAX_PATH, NULL, NULL);
      WideCharToMultiByte(CP_ACP, 0, str, -1, szT, 512, NULL, NULL);

      char szextension[MAX_PATH];
      ExtensionFromFilename(szT, szextension);

      //ppi->m_ppb;// = new PinBinary();

      lstrcat(szPath, "Music\\");

      //WideCharToMultiByte(CP_ACP, 0, str, -1, szT, 512, NULL, NULL);

      // We know that szT can't be more than 512 characters as this point, and that szPath can't be more than MAX_PATH
      lstrcat(szPath, szT);

      g_pplayer->m_pxap = new XAudPlayer();

      const float MusicVolumef = max(min((float)g_pplayer->m_MusicVolume*m_pt->m_TableMusicVolume,100.0f),0.0f);
      const int MusicVolume = (MusicVolumef == 0.0f) ? DSBVOLUME_MIN : (int)(logf(MusicVolumef)*(float)(1000.0/log(10.0)) - 2000.0f); // 10 volume = -10Db

      if (!g_pplayer->m_pxap->Init(szPath, MusicVolume))
      {
         delete g_pplayer->m_pxap;
         g_pplayer->m_pxap = NULL;
      }
   }

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::EndMusic()
{
   if (g_pplayer && g_pplayer->m_fPlayMusic)
   {
      if (g_pplayer->m_pxap)
      {
         g_pplayer->m_pxap->End();
         delete g_pplayer->m_pxap;
         g_pplayer->m_pxap = NULL;
      }
   }

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_Name(BSTR *pVal)
{
   *pVal = SysAllocString(L"Global");

   return S_OK;
}

IDispatch *ScriptGlobalTable::GetDispatch()
{
   return (IDispatch *)this;
}

STDMETHODIMP ScriptGlobalTable::get_LeftFlipperKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eLeftFlipperKey];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_RightFlipperKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eRightFlipperKey];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_LeftTiltKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eLeftTiltKey];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_RightTiltKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eRightTiltKey];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_CenterTiltKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eCenterTiltKey];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_PlungerKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[ePlungerKey];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_StartGameKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eStartGameKey];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_AddCreditKey(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eAddCreditKey];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_AddCreditKey2(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eAddCreditKey2];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_MechanicalTilt(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eMechanicalTilt];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_LeftMagnaSave(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eLeftMagnaSave];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_RightMagnaSave(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eRightMagnaSave];

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_ExitGame(long *pVal)
{
   *pVal = g_pplayer->m_rgKeys[eExitGame];

   return S_OK;
}

BOOL ScriptGlobalTable::GetTextFileFromDirectory(char *szfilename, char *dirname, BSTR *pContents)
{
   char *szPath;
   szPath = new char[MAX_PATH + lstrlen(szfilename)];
   BOOL fSuccess = fFalse;

   if (dirname != NULL)
   {
      lstrcpy(szPath, g_pvp->m_szMyPath);
      lstrcat(szPath, dirname);
   }
   else
   {
      // Current directory
      szPath[0] = '\0';
   }
   lstrcat(szPath, szfilename);

   int len;
   BYTE *szContents;

   if (RawReadFromFile(szPath, &len, (char **)&szContents))
   {
      BYTE *szDataStart = szContents;
      int encoding = CP_ACP;

      if (szContents[0] == 0xEF && szContents[1] == 0xBB && szContents[2] == 0xBF)
      {
         encoding = CP_UTF8;
         szDataStart += 3;
         len -= 3;
      }

      if (szContents[0] == 0xFF && szContents[1] == 0xFE)
      {
         // Unicode
         szDataStart += 2;
         len -= 2;
         *pContents = SysAllocString((WCHAR *)szDataStart);
      }
      else
      {
         WCHAR *wzContents = new WCHAR[len+1];

         MultiByteToWideChar(encoding, 0, (char *)szDataStart, len, wzContents, len+1);
         wzContents[len] = L'\0';

         *pContents = SysAllocString(wzContents);
         delete [] wzContents;
      }

      delete [] szContents;

      fSuccess = fTrue;
   }

   delete [] szPath;

   return fSuccess;
}

STDMETHODIMP ScriptGlobalTable::GetTextFile(BSTR FileName, BSTR *pContents)
{
   BOOL fSuccess;
   char szFileName[MAX_PATH];

   WideCharToMultiByte(CP_ACP, 0, FileName, -1, szFileName, MAX_PATH, NULL, NULL);

   // try to load the scripts from the current directory
   fSuccess = GetTextFileFromDirectory(szFileName, NULL, pContents);

   // if that fails, try the User, Scripts and Tables sub-directorys under where VP was loaded from
   if (!fSuccess)
      fSuccess = GetTextFileFromDirectory(szFileName, "User\\", pContents);

   if (!fSuccess)
      fSuccess = GetTextFileFromDirectory(szFileName, "Scripts\\", pContents);

   if (!fSuccess)
      fSuccess = GetTextFileFromDirectory(szFileName, "Tables\\", pContents);

   return (fSuccess) ? S_OK : E_FAIL;
}

STDMETHODIMP ScriptGlobalTable::get_UserDirectory(BSTR *pVal)
{
   WCHAR wzPath[MAX_PATH];
   WideStrCopy(g_pvp->m_wzMyPath, wzPath);
   WideStrCat(L"User\\", wzPath);
   *pVal = SysAllocString(wzPath);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_GetPlayerHWnd(long *pVal)
{
   if (!g_pplayer)
   {
      *pVal = NULL;
      return E_FAIL;
   }
   else
   {
      *pVal = (long)g_pplayer->m_hwnd;
   }

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::AddObject(BSTR Name, IDispatch *pdisp)
{
   if (!g_pplayer)
      return E_FAIL;

   g_pplayer->m_ptable->m_pcv->AddTemporaryItem(Name, pdisp);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::SaveValue(BSTR TableName, BSTR ValueName, VARIANT Value)
{
   IStorage* pstgRoot;
   IStorage* pstgTable;
   IStream* pstmValue;

   HRESULT hr = S_OK;

   WCHAR wzPath[MAX_PATH];
   WideStrCopy(g_pvp->m_wzMyPath, wzPath);
   WideStrCat(L"User\\VPReg.stg", wzPath);

   if (FAILED(hr = StgOpenStorage(wzPath, NULL, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgRoot)))
   {
      // Registry file does not exist - create it
      if (FAILED(hr = StgCreateDocfile(wzPath, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, &pstgRoot)))
      {
         WCHAR wzMkPath[MAX_PATH];
         WideStrCopy(g_pvp->m_wzMyPath, wzMkPath);
         WideStrCat(L"User", wzMkPath);
         if (_wmkdir(wzMkPath) != 0) 
            return hr;

         if (FAILED(hr = StgCreateDocfile(wzPath, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, &pstgRoot)))
            return hr;
      }
   }

   if (FAILED(hr = pstgRoot->OpenStorage(TableName, NULL, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgTable)))
   {
      // Table file does not exist
      if (FAILED(hr = pstgRoot->CreateStorage(TableName, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstgTable)))
      {
         pstgRoot->Release();
         return hr;
      }
   }

   if (FAILED(hr = pstgTable->CreateStream(ValueName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmValue)))
   {
      pstgTable->Release();
      pstgRoot->Release();
      return hr;
   }

   BSTR bstr = BstrFromVariant(&Value, 0x409);

   DWORD writ;

   hr = pstmValue->Write((WCHAR *)bstr, lstrlenW((WCHAR *)bstr) * sizeof(WCHAR), &writ);

   SysFreeString(bstr);

   pstmValue->Release();

   pstgTable->Commit(STGC_DEFAULT);
   pstgTable->Release();

   pstgRoot->Commit(STGC_DEFAULT);
   pstgRoot->Release();

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::LoadValue(BSTR TableName, BSTR ValueName, VARIANT *Value)
{
   IStorage* pstgRoot;
   IStorage* pstgTable;
   IStream* pstmValue;

   HRESULT hr = S_OK;

   WCHAR wzPath[MAX_PATH];
   WideStrCopy(g_pvp->m_wzMyPath, wzPath);
   WideStrCat(L"User\\VPReg.stg", wzPath);

   if (FAILED(hr = StgOpenStorage(wzPath, NULL, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgRoot)))
   {
      SetVarBstr(Value, SysAllocString(L""));
      return S_OK;
   }

   if (FAILED(hr = pstgRoot->OpenStorage(TableName, NULL, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgTable)))
   {
      SetVarBstr(Value, SysAllocString(L""));
      pstgRoot->Release();
      return S_OK;
   }

   if (FAILED(hr = pstgTable->OpenStream(ValueName, 0, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmValue)))
   {
      SetVarBstr(Value, SysAllocString(L""));
      pstgTable->Release();
      pstgRoot->Release();
      return S_OK;
   }

   STATSTG statstg;

   pstmValue->Stat(&statstg, STATFLAG_NONAME);

   WCHAR *wzT;

   int size = statstg.cbSize.LowPart / sizeof(WCHAR);

   wzT = new WCHAR[size + 1];

   DWORD read;

   hr = pstmValue->Read(wzT, size * sizeof(WCHAR), &read);
   wzT[size] = L'\0';

   pstmValue->Release();

   pstgTable->Commit(STGC_DEFAULT);
   pstgTable->Release();

   pstgRoot->Commit(STGC_DEFAULT);
   pstgRoot->Release();

   SetVarBstr(Value, SysAllocString(wzT));

   delete [] wzT;

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_ActiveBall(IBall **pVal)
{
   if (!pVal || !g_pplayer || !g_pplayer->m_pactiveball)
      return E_POINTER;

   BallEx *pballex = g_pplayer->m_pactiveball->m_pballex;

   if (!pballex)
	  return E_POINTER;

   pballex->QueryInterface(IID_IBall, (void **)pVal);

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_GameTime(long *pVal)
{
   if (!g_pplayer)
      return E_POINTER;

   *pVal = g_pplayer->m_time_msec;

   return S_OK;
}

STDMETHODIMP ScriptGlobalTable::get_VPBuildVersion(long *pVal)
{
   if (!g_pplayer)
      return E_POINTER;

   *pVal = BUILD_NUMBER;
   return S_OK;
}

PinTable::PinTable()
{
   for( int i=0;i<8;i++ )
      m_activeLayers[i]=true;
   m_toggleAllLayers=false;
   m_savingActive=false;

   m_vmultisel.AddElement((ISelect *)this);
   m_undo.m_ptable = this;
   m_fGrid = fTrue;
   m_fBackdrop = fTrue;
   m_fRenderShadows = fTrue;

   m_fRenderDecals = fTrue;
   m_fRenderEMReels = fTrue;

   m_fOverridePhysics = 0;

   m_Gravity = GRAVITYCONST;
   m_hardFriction = C_FRICTIONCONST;
   m_hardScatter = 0;
   m_nudgeTime = 5.0f;

   m_plungerNormalize = 100;  //Mech-Plunger component adjustment or weak spring, aging
   m_plungerFilter = fFalse;
   m_PhysicsMaxLoops = 0xFFFFFFFF;
   /*const HRESULT hr =*/ GetRegInt("Player", "PhysicsMaxLoops", (int*)&m_PhysicsMaxLoops);

   m_right = 0.0f;
   m_bottom = 0.0f;

   m_rotation = 0;
   m_layback = 0;

   m_glassheight = 210;
   m_tableheight = 0;

   m_scalex = 1.0f;
   m_scaley = 1.0f;

   m_xlatex = 0.0f;
   m_xlatey = 0.0f;

   m_shadowDirX= 1.0f;
   m_shadowDirY=-1.0f;

   m_zScale = 1.0f;
   m_xlatez = 0.0f;

   CComObject<CodeViewer>::CreateInstance(&m_pcv);
   m_pcv->AddRef();
   m_pcv->Init((IScriptableHost*)this);
   m_pcv->Create();

   CComObject<ScriptGlobalTable>::CreateInstance(&m_psgt);
   m_psgt->AddRef();
   m_psgt->Init(this);

   m_sdsDirtyProp = eSaveClean;
   m_sdsDirtyScript = eSaveClean;
   m_sdsNonUndoableDirty = eSaveClean;
   m_sdsCurrentDirtyState = eSaveClean;

   // set up default protection security descripter
   ResetProtectionBlock();

   m_szTableName = NULL;
   m_szAuthor = NULL;
   m_szVersion = NULL;
   m_szReleaseDate = NULL;
   m_szAuthorEMail = NULL;
   m_szWebSite = NULL;
   m_szBlurb = NULL;
   m_szDescription = NULL;
   m_szRules = NULL;

   m_pbTempScreenshot = NULL;

   HRESULT hr;
   int tmp;

   F32 tiltsens = 0.40f;
   hr = GetRegInt("Player", "TiltSensitivity", &tmp);
   if (hr == S_OK)
	   tiltsens = (float)tmp*(float)(1.0/1000.0);	
   plumb_set_sensitivity( tiltsens );

   F32 nudgesens = 0.50f;
   hr = GetRegInt("Player", "NudgeSensitivity", &tmp);
   if (hr == S_OK)
	   nudgesens = (float)tmp*(float)(1.0/1000.0);	
   nudge_set_sensitivity( nudgesens );

   m_globalDifficulty = 0;						// easy by default
   hr = GetRegInt("Player", "GlobalDifficulty", &tmp);
   if (hr == S_OK)
	   m_globalDifficulty = (float)tmp*(float)(1.0/100.0);

#ifdef ULTRAPIN
   m_timeout = 0;								// easy by default
   hr = GetRegInt("Player", "Timeout", &tmp);
   if (hr == S_OK)
	   m_timeout = tmp*1000/60;
#endif

   m_tblAccelerometer = fTrue;							// true if electronic accelerometer enabled
   hr = GetRegInt("Player", "PBWEnabled", &m_tblAccelerometer);
   m_tblAccelerometer = m_tblAccelerometer != fFalse;

   m_tblAccelNormalMount = fTrue;						// true is normal mounting (left hand coordinates)
   hr = GetRegInt("Player", "PBWNormalMount", &m_tblAccelNormalMount);
   m_tblAccelNormalMount = m_tblAccelNormalMount != fFalse;

   m_tblAccelAngle = 0.0f;			// 0 degrees rotated counterclockwise (GUI is lefthand coordinates)
   hr = GetRegInt("Player", "PBWRotation", &tmp);
   if (hr == S_OK)
	   m_tblAccelAngle = (float)tmp;

   m_tblAccelAmp = 1.5f;								// Accelerometer gain 
   hr = GetRegInt("Player", "PBWAccelGain", &tmp);
   if (hr == S_OK)
	   m_tblAccelAmp = (float)tmp*(float)(1.0/100.0);

   // X and Y accelerometer gain implemented as such that if it doesn't exist, use the
   // PBWAccelGain value as default.
   m_tblAccelAmpX = 1.5f; //m_tblAccelAmpX = m_tblAccelAmp;
   hr = GetRegInt("Player", "PBWAccelGainX", &tmp);
   if (hr == S_OK)
	   m_tblAccelAmpX = (float)tmp*(float)(1.0/100.0);

   m_tblAccelAmpY = 1.5f; //m_tblAccelAmpY = m_tblAccelAmp;
   hr = GetRegInt("Player", "PBWAccelGainY", &tmp);
   if (hr == S_OK)
	   m_tblAccelAmpY = (float)tmp*(float)(1.0/100.0);

   m_tblAccelManualAmp = 3.5f;							// manual input gain, generally from joysticks
   hr = GetRegInt("Player", "JoystickGain", &tmp);
   if (hr == S_OK)
	   m_tblAccelManualAmp = (float)tmp*(float)(1.0/100.0);

   m_tblAutoStart = 0;
   hr = GetRegInt("Player", "Autostart", &tmp);
   if( hr == S_OK )
	   m_tblAutoStart = tmp*10;

   m_tblAutoStartRetry = 0;
   hr = GetRegInt("Player", "AutostartRetry", &tmp);
   if( hr == S_OK )
	   m_tblAutoStartRetry = tmp*10;

   m_tblAutoStartEnabled = 0;
   hr = GetRegInt("Player", "asenable", &tmp);
   if( hr == S_OK )
	   m_tblAutoStartEnabled = ( tmp != 0 );

   m_tblVolmod = 1.0f;
   hr = GetRegInt("Player", "Volmod", &tmp);
   if( hr == S_OK )
	   m_tblVolmod = (float)tmp*(float)(1.0/1000.0);

   m_tblMirrorEnabled = 0;
   hr = GetRegInt("Player", "mirror", &tmp);
   if( hr == S_OK )
	   m_tblMirrorEnabled = ( tmp != 0 );

   m_tblExitConfirm = 2000;
   hr = GetRegInt("Player", "Exitconfirm", &tmp);
   if( hr == S_OK )
	   m_tblExitConfirm = tmp*1000/60;

   // Write the version of this exe to the registry.  
   // This will be read later by the front end.
   char Version[64];
   sprintf_s( Version, "%d", BUILD_NUMBER );
   SetRegValue ( "Version", "VPinball", REG_SZ, Version, strlen(Version) );

   if ( FAILED(GetRegInt("Player", "AlphaRampAccuracy", &m_globalAlphaRampsAccuracy) ) )
   {
      m_globalAlphaRampsAccuracy = 5;
   }
   m_userAlphaRampsAccuracy=5;
   m_overwriteGlobalAlphaRampsAccuracy = fFalse;

   if ( FAILED(GetRegStringAsFloat("Player", "Stereo3DZPD", &m_globalZPD) ) )
   {
      m_globalZPD = 0.5f;
   }
   m_ZPD = 0.5f;
   if ( FAILED(GetRegStringAsFloat("Player", "Stereo3DMaxSeparation", &m_globalMaxSeparation) ) )
   {
      m_globalMaxSeparation = 0.03f;
   }
   m_maxSeparation = 0.03f;
   m_overwriteGlobalStereo3D = fFalse;

   m_jolt_amount = 500;
   m_tilt_amount = 950;
   m_jolt_trigger_time = 1000;
   m_tilt_trigger_time = 10000;

   m_Shake = false;
}

PinTable::~PinTable()
{
   for (int i=0;i<m_vedit.Size();i++)
      m_vedit.ElementAt(i)->Release();

   ClearOldSounds();

   for (int i=0;i<m_vsound.Size();i++)
   {
      //m_vsound.ElementAt(i)->m_pDSBuffer->Release();
      delete m_vsound.ElementAt(i);
   }

   for (int i=0;i<m_vimage.Size();i++)
      delete m_vimage.ElementAt(i);

   for (int i=0;i<m_vfont.Size();i++)
   {
      m_vfont.ElementAt(i)->UnRegister();
      delete m_vfont.ElementAt(i);
   }

   for (int i=0;i<m_vcollection.Size();i++)
      m_vcollection.ElementAt(i)->Release();

   for (int i=0;i<m_vCustomInfoTag.Size();i++)
   {
      delete m_vCustomInfoTag.ElementAt(i);
      delete m_vCustomInfoContent.ElementAt(i);
   }

   m_pcv->Release();

   m_psgt->Release();

   if (IsWindow(m_hwnd))
   {
      //DestroyWindow(m_hwnd);
      SendMessage(g_pvp->m_hwndWork, WM_MDIDESTROY, (DWORD)m_hwnd, 0);
   }

   if (m_hbmOffScreen)
      DeleteObject(m_hbmOffScreen);

   SAFE_VECTOR_DELETE(m_szTableName);
   SAFE_VECTOR_DELETE(m_szAuthor);
   SAFE_VECTOR_DELETE(m_szVersion);
   SAFE_VECTOR_DELETE(m_szReleaseDate);
   SAFE_VECTOR_DELETE(m_szAuthorEMail);
   SAFE_VECTOR_DELETE(m_szWebSite);
   SAFE_VECTOR_DELETE(m_szBlurb);
   SAFE_VECTOR_DELETE(m_szDescription);
   SAFE_VECTOR_DELETE(m_szRules);
}

BOOL PinTable::FVerifySaveToClose()
{
   if (m_vAsyncHandles.Size() > 0)
   {
      /*const DWORD wait =*/ WaitForMultipleObjects(m_vAsyncHandles.Size(), (HANDLE *)m_vAsyncHandles.GetArray(), TRUE, INFINITE);
      //MessageBox(NULL, "Async work items not done", NULL, 0);

      // Close the remaining handles here, since the window messages will never be processed
      for (int i=0;i<m_vAsyncHandles.Size();i++)
         CloseHandle(m_vAsyncHandles.ElementAt(i));

      g_pvp->SetActionCur("");
   }

   return fTrue;
}

BOOL PinTable::CheckPermissions(unsigned long flag)
{
   return ( ((m_protectionData.flags & DISABLE_EVERYTHING) == DISABLE_EVERYTHING) ||
      ((m_protectionData.flags & flag) == flag)	);
}

BOOL PinTable::IsTableProtected()
{
   return (m_protectionData.flags != 0);
}

void PinTable::ResetProtectionBlock()
{
   // set up default protection security descripter
   ZeroMemory (&m_protectionData, sizeof(m_protectionData));
   m_protectionData.fileversion = PROT_DATA_VERSION;
   m_protectionData.size = sizeof(m_protectionData);
}

BOOL PinTable::SetupProtectionBlock(unsigned char *pPassword, unsigned long flags)
{
   int foo;
   HCRYPTPROV   hcp		= NULL;
   HCRYPTKEY  	hkey	= NULL;
   HCRYPTHASH 	hchkey	= NULL;

   _protectionData *pProtectionData = &m_protectionData;

   // acquire a crypto context
   foo = CryptAcquireContext(&hcp, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET/* | CRYPT_SILENT*/);
   foo = GetLastError();
   // create a hash
   foo = CryptCreateHash(hcp, CALG_MD5, NULL, 0, &hchkey);
   foo = GetLastError();
   // hash the password
   foo = CryptHashData(hchkey, pPassword, strlen((char *)pPassword), 0);
   foo = GetLastError();
   // Create a block cipher session key based on the hash of the password.
   foo = CryptDeriveKey(hcp, CALG_RC2, hchkey, CRYPT_EXPORTABLE, &hkey);
   foo = GetLastError();

   // copy our paraphrase key into the key field
   memcpy(pProtectionData->paraphrase, PARAPHRASE_KEY, sizeof(PARAPHRASE_KEY));

   // encypt this with the key generated with the password
   DWORD cryptlen;
   // get the size of the data to encrypt
   cryptlen = PROT_PASSWORD_LENGTH;

   // encrypt the paraphrase
   foo = CryptEncrypt( hkey,									// key to use
      0, 										// not hashing data at the same time
      TRUE, 									// last block (or only block)
      0, 										// no flags
      (BYTE *)pProtectionData->paraphrase,	// buffer to encrypt
      &cryptlen,								// size of data to encrypt
      sizeof(pProtectionData->paraphrase));	// maximum size of buffer (includes any padding)

   foo = GetLastError();		// purge any errors

   // set up the flags
   pProtectionData->flags = flags;

   // destroy our cryto
   foo = CryptDestroyHash(hchkey);
   foo = CryptDestroyKey(hkey);
   foo = CryptReleaseContext(hcp, 0);

   return fTrue;
}

BOOL PinTable::UnlockProtectionBlock(unsigned char *pPassword)
{
   char secret1[]= "Could not create";
   if ((memcmp(pPassword, &secret1, sizeof(secret1)) == 0))
   {
      ResetProtectionBlock();
      return fTrue;
   }

   int foo;
   HCRYPTPROV  	    hcp		= NULL;
   HCRYPTKEY  		hkey	= NULL;
   HCRYPTHASH 		hchkey	= NULL;
   unsigned char	paraphrase[PROT_CIPHER_LENGTH];
   _protectionData *pProtectionData = &m_protectionData;

   // acquire a crypto context
   foo = CryptAcquireContext(&hcp, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET/* | CRYPT_SILENT*/);
   foo = GetLastError();
   // create a hash
   foo = CryptCreateHash(hcp, CALG_MD5, NULL, 0, &hchkey);
   foo = GetLastError();
   // hash the password
   foo = CryptHashData(hchkey, pPassword, strlen((char *)pPassword), 0);
   foo = GetLastError();
   // Create a block cipher session key based on the hash of the password.
   foo = CryptDeriveKey(hcp, CALG_RC2, hchkey, CRYPT_EXPORTABLE, &hkey);
   foo = GetLastError();

   // copy the paraphase from the protection block to some local memory
   memcpy(paraphrase, pProtectionData->paraphrase, sizeof(paraphrase));

   // decypt this with the key generated with the password
   DWORD cryptlen;
   // get the size of the data to encrypt
   cryptlen = PROT_CIPHER_LENGTH;

   // encrypt the paraphrase
   foo = CryptDecrypt( hkey,									// key to use
      0, 										// not hashing data at the same time
      TRUE, 									// last block (or only block)
      0, 										// no flags
      (BYTE *)paraphrase,						// buffer to encrypt
      &cryptlen);								// size of data to decrypt

   // destroy our cryto
   foo = CryptDestroyHash(hchkey);
   foo = CryptDestroyKey(hkey);
   foo = CryptReleaseContext(hcp, 0);

   //	if the decrypted data matches the original paraphrase then unlock the table
   if ( (cryptlen == PROT_PASSWORD_LENGTH) &&
      (memcmp(paraphrase, PARAPHRASE_KEY, sizeof(PARAPHRASE_KEY)) == 0) )
   {
      ResetProtectionBlock();
      return fTrue;
   }
   return fFalse;
}

void PinTable::SwitchToLayer(int layerNumber )
{
   // scan through all layers if all elements are already stored to a layer
   // if not new elements will be stored in layer1
   for( int t=0;t<m_vedit.Size();t++ )
   {
      IEditable *piedit = m_vedit.ElementAt(t);
      bool alreadyIn=false;
      for( int i=0;i<8;i++ )
      {
         if( m_layer[i].IndexOf(piedit)!=-1 )
            alreadyIn=true;
      }
      if( !alreadyIn )
      {
         piedit->GetISelect()->layerIndex=0;
         m_layer[0].AddElement(piedit);
      }
   }
   //toggle layer
   m_activeLayers[layerNumber] ^= true;

   // now set all elements to visible if their layer is active, otherwise hide them
   for( int i=0;i<8;i++ )
   {
      if( m_activeLayers[i] )
      {         
         for( int t=0;t<m_layer[i].Size();t++ )
         {
            IEditable *piedit = m_layer[i].ElementAt(t);
            piedit->isVisible=true;
         }
      }
      else
      {
         for( int t=0;t<m_layer[i].Size();t++ )
         {
            IEditable *piedit = m_layer[i].ElementAt(t);
            piedit->isVisible=false;
         }
      }
   }

   SetDirtyDraw();
}

void PinTable::AssignToLayer(IEditable *obj, int layerNumber )
{
   if( !m_activeLayers[layerNumber] )
      obj->isVisible=false;
   m_layer[obj->GetISelect()->layerIndex].RemoveElement(obj);
   obj->GetISelect()->layerIndex=layerNumber;
   m_layer[layerNumber].InsertElementAt(obj,0);
   SetDirtyDraw();
}

void PinTable::MergeAllLayers()
{
   for( int t=1;t<8;t++)
   {
      for( int i=m_layer[t].Size()-1;i>=0;i-- )
      {
         IEditable *piedit = m_layer[t].ElementAt(i);
         piedit->GetISelect()->layerIndex=0;
         m_layer[0].AddElement(piedit);
      }
      m_layer[t].RemoveAllElements();
   }
   m_layer[0].Clone( &m_vedit );

   SetDirtyDraw();
}

void PinTable::BackupLayers()
{
   // scan through all layers if all elements are already stored to a layer
   // if not new elements will be stored in layer1
   for( int t=0;t<m_vedit.Size();t++ )
   {
      IEditable *piedit = m_vedit.ElementAt(t);
      bool alreadyIn=false;
      for( int i=0;i<8;i++ )
      {
         if( m_layer[i].IndexOf(piedit)!=-1 )
            alreadyIn=true;
      }
      if( !alreadyIn )
      {
         piedit->GetISelect()->layerIndex=0;
         m_layer[0].AddElement(piedit);
      }
   }
   // make all elements visible again
   for( int t=0;t<8;t++)
   {
      //      for( int i=m_layer[t].Size()-1;i>=0;i-- )
      for( int i=0;i<m_layer[t].Size();i++ )
      {
         IEditable *piedit = m_layer[t].ElementAt(i);
         piedit->isVisible=true;
      }
   }
}

void PinTable::RestoreLayers()
{
   for( int i=0;i<8;i++ )
   {
      if( m_activeLayers[i] )
      {         
         for( int t=0;t<m_layer[i].Size();t++ )
         {
            IEditable *piedit = m_layer[i].ElementAt(t);
            piedit->isVisible=true;
         }
      }
      else
      {
         for( int t=0;t<m_layer[i].Size();t++ )
         {
            IEditable *piedit = m_layer[i].ElementAt(t);
            piedit->isVisible=false;
         }
      }
   }
}

void PinTable::DeleteFromLayer( IEditable *obj )
{
   for( int i=0;i<8;i++ )
   {
      if( m_layer[i].IndexOf(obj)!=-1 )
      {
         m_layer[i].RemoveElement(obj);
         break;
      }
   }
}

#define NEWFROMRES 1

void PinTable::Init(VPinball *pvp)
{
   m_pvp = pvp;

#ifdef NEWFROMRES
   // Get our new table resource, get it to be opened as a storage, and open it like a normal file
   HRSRC hrsrc = FindResource(NULL, MAKEINTRESOURCE(IDR_TABLE), "TABLE");
   HGLOBAL hglobal = LoadResource(NULL, hrsrc);
   char *pchar = (char *)LockResource(hglobal);
   DWORD size = SizeofResource(NULL, hrsrc);
   HGLOBAL hcopiedmem = GlobalAlloc(GMEM_MOVEABLE, size);
   char *pcopied = (char *)GlobalLock(hcopiedmem);
   memcpy(pcopied, pchar, size);
   GlobalUnlock(hcopiedmem);

   ILockBytes *pilb;
   CreateILockBytesOnHGlobal(hcopiedmem, TRUE, &pilb); // "TRUE" parm gives ownership of hcopiedmem to Global Object

   IStorage *pis;
   StgOpenStorageOnILockBytes(pilb,NULL,STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,NULL,0,&pis);
   pilb->Release();	// free pilb and hcopiedmem

   m_glassheight = 210;
   m_tableheight = 0;

   for (int i=0;i<16;i++)
   {
      m_rgcolorcustom[i] = RGB(0,0,0);
   }

#ifdef VBA
   m_pStg = pis;
#endif

   //pilb->Release();

   //LoadGameFromFilename("d:\\gdk\\data\\tables\\newsave\\basetable6.vpt");

   char szSuffix[32];

   LocalString ls(IDS_TABLE);
   lstrcpy(m_szTitle, ls.m_szbuffer/*"Table"*/);
   _itoa_s(g_pvp->m_NextTableID, szSuffix, sizeof(szSuffix), 10);
   lstrcat(m_szTitle, szSuffix);
   g_pvp->m_NextTableID++;
   m_szFileName[0] = '\0';
   //lstrcpy(m_szFileName, m_szFileName);

   LoadGameFromStorage(pis);

   m_angletiltMin = 6;
   m_angletiltMax = 726;

   //MAKE_WIDEPTR_FROMANSI(wszFileName, m_szFileName);
   //ApcProject->APC_PUT(DisplayName)(wszFileName);

   InitPostLoad(pvp);

   SetCaption(m_szTitle);
   pvp->InitLayerMenu();

#else

   m_szFileName[0] = 0;
   m_szBlueprintFileName[0] = 0;
   m_gridsize = 50;

   //m_ptinfoCls = NULL;
   //m_ptinfoInt = NULL;

   m_left = 0;
   m_top = 0;
   m_right = EDITOR_BG_WIDTH;
   m_bottom = EDITOR_BG_WIDTH*2;

   m_scalex = 1.0f;
   m_scaley = 1.0f;
   m_zScale = 1.0f;

   m_xlatex = 0.0f;
   m_xlatey = 0.0f;
   m_xlatez = 0.0f;

   m_inclination = 43;
   m_layback = 0;
   m_FOV = 45;

   m_maxSeparation = 0.03f;
   m_ZPD = 0.5f;

   SetDefaultView();

   m_szImage[0] = 0;
   m_szImageBackdrop[0] = 0;

   m_colorplayfield = RGB(128,128,128);
   m_colorbackdrop = RGB(128,128,128);

   InitVBA();

   CreateTableWindow();

   SetMyScrollInfo();
#endif
}

void PinTable::SetDefaultView()
{
   FRect frect;
   GetViewRect(&frect);

   m_offsetx = (frect.left+frect.right) * 0.5f;
   m_offsety = (frect.top+frect.bottom) * 0.5f;
   m_zoom = 0.5f;
}

void PinTable::SetCaption(char *szCaption)
{
   SetWindowText(m_hwnd, szCaption);
   m_pcv->SetCaption(szCaption);
}


void PinTable::InitPostLoad(VPinball *pvp)
{
   m_pvp = pvp;

   m_hbmOffScreen = NULL;
   m_fDirtyDraw = fTrue;

   m_left = 0;
   m_top = 0;

   SetDefaultView();

   m_szBlueprintFileName[0] = 0;

   CreateGDIBackdrop();

   //InitVBA();
   m_pcv->AddItem(this, fFalse);
   m_pcv->AddItem(m_psgt, fTrue);
   m_pcv->AddItem(m_pcv->m_pdm, fFalse);

   CreateTableWindow();

   SetMyScrollInfo();
}


BOOL FWzEqual(const WCHAR *wz1, const WCHAR *wz2)
{
   while (*wz1 != 0 || *wz2 != 0)
   {
      if (*wz1++ != *wz2++)
      {
         return fFalse;
      }
   }
   return fTrue;
}


bool PinTable::IsNameUnique(WCHAR *wzName)
{
   return m_pcv->m_vcvd.GetSortedIndex(wzName) == -1;
}


void PinTable::GetUniqueName(int type, WCHAR *wzUniqueName)
{
   int suffix = 1;
   BOOL fFound = fFalse;
   WCHAR wzRoot[128];
   WCHAR wzName[128];
   WCHAR wzSuffix[10];

   LocalString ls(rgTypeStringIndex[type]);

   MultiByteToWideChar(CP_ACP, 0, ls.m_szbuffer, -1, wzRoot, 128);

   while (!fFound)
   {
      WideStrCopy(wzRoot, wzName);
      _itow_s(suffix, wzSuffix, sizeof(wzSuffix)/sizeof(WCHAR), 10);
      WideStrCat(wzSuffix, wzName);

      if (m_pcv->m_vcvd.GetSortedIndex(wzName) == -1)
      {
         fFound = fTrue;
      }
      else
      {
         suffix += 1;
      }
   }

   WideStrCopy(wzName, wzUniqueName);
}

void PinTable::GetUniqueNamePasting(int type, WCHAR *wzUniqueName)
{
   int suffix = 1;
   BOOL fFound = fFalse;
   WCHAR wzName[MAXNAMEBUFFER];
   WCHAR wzSuffix[10];

   //if the original name is not yet used, use that one (so there's nothing we have to do) 
   //otherwise add/increase the suffix untill we find a name that's not used yet
   if (m_pcv->m_vcvd.GetSortedIndex(wzUniqueName) != -1)
   {
      //first remove the existing suffix
      while (iswdigit(wzUniqueName[wcslen(wzUniqueName)-1]))
      {
         wzUniqueName[wcslen(wzUniqueName)-1] = L'\0';
      }

      while (!fFound)
      {
         WideStrCopy(wzUniqueName, wzName);
         _itow_s(suffix, wzSuffix, sizeof(wzSuffix)/sizeof(WCHAR), 10);
         WideStrCat(wzSuffix, wzName);

         if (m_pcv->m_vcvd.GetSortedIndex(wzName) == -1)
         {
            fFound = fTrue;
         }
         else
         {
            suffix += 1;
         }
      }
      WideStrCopy(wzName, wzUniqueName);
   }
}

void PinTable::Render(Sur * const psur)
{
   RECT rc;
   GetClientRect(m_hwnd, &rc);
   psur->SetFillColor(RGB(255,255,255));
   psur->SetBorderColor(-1,false,0);

   FRect frect;
   GetViewRect(&frect);

   psur->Rectangle2(rc.left, rc.top, rc.right, rc.bottom);

   // can we view the table elements?? if not then draw a box for the table outline
   // got to give the punters at least something to know that the table has loaded
   if (CheckPermissions(DISABLE_TABLEVIEW))
   {
      Vertex2D rlt = psur->ScreenToSurface(rc.left, rc.top);
      Vertex2D rrb = psur->ScreenToSurface(rc.right, rc.bottom);
      rlt.x = max(rlt.x, frect.left);
      rlt.y = max(rlt.y, frect.top);
      rrb.x = min(rrb.x, frect.right);
      rrb.y = min(rrb.y, frect.bottom);

      psur->SetObject(NULL); 						// Don't hit test edgelines

      psur->SetLineColor(RGB(0,0,0), false, 0);		// black outline

      psur->Line(rlt.x, rlt.y, rrb.x, rlt.y);
      psur->Line(rrb.x, rlt.y, rrb.x, rrb.y);
      psur->Line(rlt.x, rrb.y, rrb.x, rrb.y);
      psur->Line(rlt.x, rlt.y, rlt.x, rrb.y);

      return;
   }

   if (m_fBackdrop)
   {
      Texture * const ppi = GetImage((!g_pvp->m_fBackglassView) ? m_szImage : m_szImageBackdrop);

      if (ppi)
      {
         ppi->EnsureHBitmap();
         if (ppi->m_hbmGDIVersion)
         {
            HDC hdcScreen = GetDC(NULL);
            HDC hdcNew = CreateCompatibleDC(hdcScreen);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcNew, ppi->m_hbmGDIVersion);

            psur->Image(frect.left, frect.top, frect.right, frect.bottom, hdcNew, ppi->m_width, ppi->m_height);

            SelectObject(hdcNew, hbmOld);
            DeleteDC(hdcNew);
            ReleaseDC(NULL, hdcScreen);
         }
      }
   }

   if (g_pvp->m_fBackglassView)
   {
      Render3DProjection(psur);
   }


   for (int i=0;i<m_vedit.Size();i++)
   {
      IEditable *ptr = m_vedit.ElementAt(i);
      if (ptr->m_fBackglass == g_pvp->m_fBackglassView)
      {
         if ( ptr->isVisible )
            ptr->PreRender(psur);
      }
   }

   if (m_fGrid)
   {
      Vertex2D rlt = psur->ScreenToSurface(rc.left, rc.top);
      Vertex2D rrb = psur->ScreenToSurface(rc.right, rc.bottom);
      rlt.x = max(rlt.x, frect.left);
      rlt.y = max(rlt.y, frect.top);
      rrb.x = min(rrb.x, frect.right);
      rrb.y = min(rrb.y, frect.bottom);

      const int beginx = (int)(rlt.x / m_gridsize);
      const float lenx = (rrb.x - rlt.x) / m_gridsize;//(((rc.right - rc.left)/m_zoom));
      const int beginy = (int)(rlt.y / m_gridsize);
      const float leny = (rrb.y - rlt.y) / m_gridsize;//(((rc.bottom - rc.top)/m_zoom));

      psur->SetObject(NULL); // Don't hit test gridlines

      psur->SetLineColor(RGB(190,220,240), false, 0);
      for (int i=0;i<(lenx+1);i++)
      {
         const float x = (beginx+i)*m_gridsize;
         psur->Line(x, rlt.y, x, rrb.y);
      }

      for (int i=0;i<(leny+1);i++)
      {
         const float y = (beginy+i)*m_gridsize;
         psur->Line(rlt.x, y, rrb.x, y);
      }
   }

   for (int i=0;i<m_vedit.Size();i++)
   {
      if (m_vedit.ElementAt(i)->m_fBackglass == g_pvp->m_fBackglassView)
      {
         if( m_vedit.ElementAt(i)->isVisible )
            m_vedit.ElementAt(i)->Render(psur);
      }
   }

   if (g_pvp->m_fBackglassView) // Outline of the view, for when the grid is off
   {
      psur->SetObject(NULL);
      psur->SetFillColor(-1);
      psur->SetBorderColor(RGB(0,0,0), false, 1);
      psur->Rectangle(0,0,EDITOR_BG_WIDTH,EDITOR_BG_HEIGHT);
   }

   if (m_fDragging)
   {
      psur->SetFillColor(-1);
      psur->SetBorderColor(RGB(0,0,0),true,0);
      psur->Rectangle(m_rcDragRect.left, m_rcDragRect.top, m_rcDragRect.right, m_rcDragRect.bottom);
   }

   // display the layer string
   //    psur->SetObject(NULL);
   //    SetTextColor( psur->m_hdc,RGB(180,180,180));
   //    char text[64];
   //    char number[8];
   //    strcpy_s( text,"Layer_");
   //    _itoa_s(activeLayer+1, number, 10 );
   //    strcat_s( text, number);
   //    RECT textRect;
   //    SetRect( &textRect, rc.right-60,rc.top, rc.right, rc.top+30 );
   //    DrawText( psur->m_hdc, text, -1, &textRect, DT_LEFT);
   // 
   //    SetTextColor( psur->m_hdc,RGB(0,0,0));

   //   psur->DrawText( text,rc.left+10, rc.top, 90,20);
}


void PinTable::Render3DProjection(Sur * const psur)
{
   const float rotation = ANGTORAD(m_rotation);
   const float inclination = ANGTORAD(m_inclination);

   Vector<Vertex3Ds> vvertex3D;

   for (int i=0;i<m_vedit.Size();i++)
   {
      m_vedit.ElementAt(i)->GetBoundingVertices(&vvertex3D);
   }

   // dummy coordinate system for backdrop view
   PinProjection pinproj;
   pinproj.m_rcviewport.left = 0;
   pinproj.m_rcviewport.top = 0;
   pinproj.m_rcviewport.right = EDITOR_BG_WIDTH;
   pinproj.m_rcviewport.bottom = EDITOR_BG_HEIGHT;

   const GPINFLOAT aspect = 4.0/3.0;

   const float realFOV = (m_FOV < 1.0f) ? 1.0f : m_FOV; // Can't have a real zero FOV, but this will look almost the same

   pinproj.FitCameraToVertices(&vvertex3D/*rgv*/, aspect, rotation, inclination, realFOV, m_xlatez);
   pinproj.SetFieldOfView(realFOV, aspect, pinproj.m_rznear, pinproj.m_rzfar);


   const float skew = -tanf(0.5f*ANGTORAD(m_layback));
   const float skewX = -sinf(rotation)*skew;
   const float skewY =  cosf(rotation)*skew;
   // Skew for FOV of 0 Deg. is not supported. so change it a little bit.
   const float skewFOV = (realFOV < 0.01f) ? 0.01f : realFOV;
   // create skew the z axis to x and y direction.
   const float skewtan = tanf(ANGTORAD((180.0f-skewFOV)*0.5f))*pinproj.m_vertexcamera.y;
   Matrix3D matTrans;
   matTrans.SetIdentity();
   matTrans._31 = skewX;
   matTrans._32 = skewY;
   matTrans._41 = skewtan*skewX;
   matTrans._42 = skewtan*skewY;
   pinproj.Multiply(matTrans);

   pinproj.Scale(m_scalex != 0.0f ? m_scalex : 1.0f, m_scaley != 0.0f ? m_scaley : 1.0f, 1.0f);
#ifdef VP10
   pinproj.Translate(m_xlatex-pinproj.m_vertexcamera.x, m_xlatey-pinproj.m_vertexcamera.y, -pinproj.m_vertexcamera.z);
   pinproj.Rotate( 0, 0, rotation );
#else
   pinproj.Rotate( 0, 0, rotation );
   pinproj.Translate(m_xlatex-pinproj.m_vertexcamera.x, m_xlatey-pinproj.m_vertexcamera.y, -pinproj.m_vertexcamera.z);
#endif
   pinproj.Rotate(inclination, 0, 0);

   pinproj.CacheTransform();

   for (int i=0;i<vvertex3D.Size();i++)
   {
      delete vvertex3D.ElementAt(i);
   }

   psur->SetFillColor(RGB(200,200,200));
   psur->SetBorderColor(-1,false,0);

   Vertex3D rgvIn[8];
   rgvIn[0].x = m_left;    rgvIn[0].y=m_top;    rgvIn[0].z=50.0f;
   rgvIn[1].x = m_left;    rgvIn[1].y=m_top;    rgvIn[1].z=m_glassheight;
   rgvIn[2].x = m_right;   rgvIn[2].y=m_top;    rgvIn[2].z=m_glassheight;
   rgvIn[3].x = m_right;   rgvIn[3].y=m_top;    rgvIn[3].z=50.0f;
   rgvIn[4].x = m_right;   rgvIn[4].y=m_bottom; rgvIn[4].z=50.0f;
   rgvIn[5].x = m_right;   rgvIn[5].y=m_bottom; rgvIn[5].z=0.0f;
   rgvIn[6].x = m_left;    rgvIn[6].y=m_bottom; rgvIn[6].z=0.0f;
   rgvIn[7].x = m_left;    rgvIn[7].y=m_bottom; rgvIn[7].z=50.0f;

   Vertex2D rgvOut[8];
   pinproj.TransformVertices(rgvIn, NULL, 8, rgvOut);
   psur->Polygon(rgvOut, 8);
}


BOOL PinTable::GetDecalsEnabled() const
{
   return m_fRenderDecals;
}


BOOL PinTable::GetEMReelsEnabled() const
{
   return m_fRenderEMReels;
}


void PinTable::Paint(HDC hdc)
{
   //HBITMAP hbmOffScreen;

   RECT rc;
   GetClientRect(m_hwnd, &rc);

   if (m_fDirtyDraw)
   {
      if (m_hbmOffScreen)
      {
         DeleteObject(m_hbmOffScreen);
      }
      m_hbmOffScreen = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
   }

   HDC hdc2 = CreateCompatibleDC(hdc);

   HBITMAP hbmOld = (HBITMAP)SelectObject(hdc2, m_hbmOffScreen);

   if (m_fDirtyDraw)
   {
      Sur * const psur = new PaintSur(hdc2, m_zoom, m_offsetx, m_offsety, rc.right - rc.left, rc.bottom - rc.top, m_vmultisel.ElementAt(0)/*m_pselcur*/);
      Render(psur);

      delete psur;
   }

   BitBlt(hdc, rc.left, rc.top, rc.right, rc.bottom, hdc2, 0, 0, SRCCOPY);

   SelectObject (hdc2, hbmOld);

   DeleteDC(hdc2);

   m_fDirtyDraw = fFalse;
   //DeleteObject(hbmOffScreen);
}

ISelect *PinTable::HitTest(const int x, const int y)
{
   HDC hdc = GetDC(m_hwnd);
   HWND listHwnd = NULL;

   RECT rc;
   GetClientRect(m_hwnd, &rc);

   HitSur * const phs = new HitSur(hdc, m_zoom, m_offsetx, m_offsety, rc.right - rc.left, rc.bottom - rc.top, x, y, this);
   HitSur * const phs2 = new HitSur(hdc, m_zoom, m_offsetx, m_offsety, rc.right - rc.left, rc.bottom - rc.top, x, y, this);

   m_allHitElements.RemoveAllElements();

   Render(phs);

   for (int i=0;i<m_vedit.Size();i++)
   {
      IEditable *ptr = m_vedit.ElementAt(i);
      if (ptr->m_fBackglass == g_pvp->m_fBackglassView)
      {
         ptr->PreRender(phs2);
         ISelect* tmp = phs2->m_pselected;
         if ( m_allHitElements.IndexOf(tmp)==-1 && tmp!=NULL && tmp != this ) 
         {
            m_allHitElements.AddElement(tmp);
         }
      }
   }
   // it's possible that PreRender doesn't find all elements  (gates,plunger)
   // check here if everything was already stored in the list
   if( m_allHitElements.IndexOf(phs->m_pselected)==-1 )
   {
      m_allHitElements.AddElement(phs->m_pselected);
   }
   delete phs2;

   Vector<ISelect> tmpBuffer;
   for( int i=m_allHitElements.Size()-1; i>=0; i-- )
   {
      tmpBuffer.AddElement( m_allHitElements.ElementAt(i) );
   }
   m_allHitElements.RemoveAllElements();
   for(int i=0;i<tmpBuffer.Size();i++)
   {
      m_allHitElements.AddElement( tmpBuffer.ElementAt(i) );
   }
   tmpBuffer.RemoveAllElements();

   ISelect * const pisel = phs->m_pselected;
   delete phs;

   ReleaseDC(m_hwnd, hdc);

   return pisel;
}

void PinTable::SetDirtyDraw()
{
   m_fDirtyDraw = fTrue;
   InvalidateRect(m_hwnd, NULL, fFalse);
}


#include <cmath>

// we want: exp( -1.0 * coeff) == fric
inline float frictionToCoeff(double fric)
{
    return (float)(-std::log(fric));
}


void PinTable::Play()
{
   if (g_pplayer)
      return; // Can't play twice

   mixer_get_volume();

   EndAutoSaveCounter();

   // get the load path from the table filename
   char szLoadDir[MAX_PATH];
   PathFromFilename(m_szFileName, szLoadDir);
   // make sure the load directory is the active directory
   DWORD err = SetCurrentDirectory(szLoadDir);
   if (err == 0)
      err = GetLastError();

   BackupLayers();

   HWND hwndProgressDialog = CreateDialog(g_hinst, MAKEINTRESOURCE(IDD_PROGRESS), g_pvp->m_hwnd, ProgressProc);
   // TEXT
   ShowWindow(hwndProgressDialog, SW_SHOW);

   HWND hwndProgressBar = GetDlgItem(hwndProgressDialog, IDC_PROGRESS2);
   HWND hwndStatusName = GetDlgItem(hwndProgressDialog, IDC_STATUSNAME);

   SendMessage(hwndProgressBar, PBM_SETPOS, 0, 0);
   // TEXT
   SetWindowText(hwndStatusName, "Compiling Script...");

#ifdef VBA
   ApcProject.GetApcProject()->Compile(&foo);
#endif


   SendMessage(hwndProgressBar, PBM_SETPOS, 20, 0);
   SetWindowText(hwndStatusName, "Backing Up Table State...");
   BackupForPlay();

   g_fKeepUndoRecords = fFalse;

   m_pcv->m_fScriptError = fFalse;
   m_pcv->Compile();

   if (!m_pcv->m_fScriptError)
   {
      // set up the texture hashtable for fast access
      m_textureMap.clear();
      for (int i=0;i<m_vimage.Size();i++)
      {
          m_textureMap[ m_vimage.ElementAt(i)->m_szInternalName ] = m_vimage.ElementAt(i);
      }

      g_pplayer = new Player();
      HRESULT hr = g_pplayer->Init(this, hwndProgressBar, hwndStatusName);
      if (!m_pcv->m_fScriptError) 
      {
		 float m_fOverrideContactFriction;
		 float m_fOverrideContactScatterAngle;
		 
		 if(m_fOverridePhysics)
		 {
			 char tmp[256];

			 m_fOverrideGravityConstant = DEFAULT_TABLE_GRAVITY;
		     sprintf_s(tmp,256,"TablePhysicsGravityConstant%d",m_fOverridePhysics-1);
			 hr = GetRegStringAsFloat("Player", tmp, &m_fOverrideGravityConstant);
			 if (hr != S_OK)
				m_fOverrideGravityConstant = DEFAULT_TABLE_GRAVITY;
			 m_fOverrideGravityConstant *= GRAVITYCONST;

			 m_fOverrideContactFriction = DEFAULT_TABLE_CONTACTFRICTION;
		     sprintf_s(tmp,256,"TablePhysicsContactFriction%d",m_fOverridePhysics-1);
			 hr = GetRegStringAsFloat("Player", tmp, &m_fOverrideContactFriction);
			 if (hr != S_OK)
				m_fOverrideContactFriction = DEFAULT_TABLE_CONTACTFRICTION;

			 m_fOverrideContactScatterAngle = DEFAULT_TABLE_SCATTERANGLE;
		     sprintf_s(tmp,256,"TablePhysicsContactScatterAngle%d",m_fOverridePhysics-1);
			 hr = GetRegStringAsFloat("Player", tmp, &m_fOverrideContactScatterAngle);
			 if (hr != S_OK)
				m_fOverrideContactScatterAngle = DEFAULT_TABLE_SCATTERANGLE;
			 m_fOverrideContactScatterAngle = ANGTORAD(m_fOverrideContactScatterAngle);
		 }

         c_hardScatter = (m_fOverridePhysics ? m_fOverrideContactScatterAngle : m_hardScatter);
         c_plungerNormalize = m_plungerNormalize*(float)(1.0/1300.0);
         c_plungerFilter = (m_plungerFilter != 0); 

         const float slope = m_angletiltMin + (m_angletiltMax - m_angletiltMin)* m_globalDifficulty;

         g_pplayer->SetGravity(slope, m_fOverridePhysics ? m_fOverrideGravityConstant : m_Gravity);

         m_pcv->SetEnabled(fFalse); // Can't edit script while playing

         g_pvp->SetEnableToolbar();
         g_pvp->SetEnableMenuItems();	//>>> added as part of table protection

         if (!m_pcv->m_fScriptError && (hr == S_OK))
         {
            ShowWindow(g_pvp->m_hwndWork, SW_HIDE);
         }
         else
         {
            SendMessage(g_pplayer->m_hwnd, WM_CLOSE, 0, 0);
         }
      }
   }
   else
   {
      RestoreBackup();
      // restore layers
      RestoreLayers();
      g_fKeepUndoRecords = fTrue;
      m_pcv->EndSession();
      //delete g_pplayer;
      //g_pplayer = NULL;
   }

   DestroyWindow(hwndProgressDialog);

   //EnableWindow(g_pvp->m_hwndWork, fFalse); // Go modal in our main app window
}


void PinTable::StopPlaying()
{
   // Unhook script connections
   //m_pcv->m_pScript->SetScriptState(SCRIPTSTATE_INITIALIZED);

   m_pcv->SetEnabled(fTrue);

   // Stop all sounds
   // In case we were playing any of the main buffers
   for (int i=0;i<m_vsound.Size();i++)
   {
      m_vsound.ElementAt(i)->m_pDSBuffer->Stop();
   }
   // The usual case - copied sounds
   for (int i=0;i<m_voldsound.Size();i++)
   {
      PinSoundCopy *ppsc = m_voldsound.ElementAt(i);
      ppsc->m_pDSBuffer->Stop();
   }
   ClearOldSounds();

   m_pcv->EndSession();

   m_textureMap.clear();

   ShowWindow(g_pvp->m_hwndWork, SW_SHOW);
   //	EnableWindow(g_pvp->m_hwndWork, fTrue); // Disable modal state after game ends


   // This was causing the application to crash 
   // if the simulation was run without a save first.
   // But I'm not sure how to fix it... - JEP
   RestoreBackup();

   g_fKeepUndoRecords = fTrue;

   BeginAutoSaveCounter();
}


void PinTable::CreateTableWindow()
{
   WNDCLASSEX wcex;
   ZeroMemory(&wcex, sizeof(WNDCLASSEX));
   wcex.cbSize = sizeof(WNDCLASSEX);
   wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;//CS_NOCLOSE | CS_OWNDC;
   wcex.lpfnWndProc = TableWndProc;
   wcex.hInstance = g_hinst;
   wcex.lpszClassName = "PinTable";
   wcex.hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_TABLE));
   wcex.hCursor = NULL;//LoadCursor(NULL, IDC_ARROW);
   wcex.hbrBackground = NULL;
   //wcex.lpszMenuName = MAKEINTRESOURCE(IDR_APPMENU);

   RegisterClassEx(&wcex);

   m_hwnd = ::CreateWindowEx(WS_EX_MDICHILD /*| WS_EX_OVERLAPPEDWINDOW*/,"PinTable",m_szFileName,WS_HSCROLL | WS_VSCROLL | WS_MAXIMIZE | WS_VISIBLE | WS_CHILD | WS_OVERLAPPEDWINDOW/* | WS_MAXIMIZE*/,
      20,20,400,400,m_pvp->m_hwndWork,NULL,g_hinst,0);

   BeginAutoSaveCounter();

   SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (long)this);
}


HRESULT PinTable::InitVBA()
{
   HRESULT hr = S_OK;
#ifdef VBA
   if (SUCCEEDED(hr = StgCreateDocfile(NULL, STGM_TRANSACTED | STGM_READWRITE
      | STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_DELETEONRELEASE, 0, &m_pStg)))
   {
      if (SUCCEEDED(hr = ApcProject.Create(m_pvp->ApcHost, axProjectNormal,
         L"Table")) && SUCCEEDED(hr = ApcProject.InitNew(m_pStg)))
      {
      }
   }

   if (hr != S_OK)
      ShowError("Could not create VBA Project.");

   hr = ApcProjectItem.Define(ApcProject, GetDispatch(), axTypeHostProjectItem, L"Table", NULL);

   if (hr != S_OK)
      ShowError("Could not create VBA ProjectItem Table.");
#endif
   return hr;
}

void PinTable::CloseVBA()
{
   //CHECK_SIGNATURE(SIG_Game);
#ifdef VBA
   ApcProject.Close();
#endif
}


HRESULT PinTable::TableSave()
{
   BOOL fSaveAs = (!m_szFileName[0]);
   return Save(fSaveAs);
}


HRESULT PinTable::SaveAs()
{
   return Save(fTrue);
}


HRESULT PinTable::ApcProject_Save()
{
   BOOL fSaveAs = (!m_szFileName[0]);
   return Save(fSaveAs);
}


void PinTable::BeginAutoSaveCounter()
{
   if (g_pvp->m_autosaveTime > 0)
      SetTimer(m_hwnd, TIMER_ID_AUTOSAVE, g_pvp->m_autosaveTime, NULL);
}


void PinTable::EndAutoSaveCounter()
{
   KillTimer(m_hwnd, TIMER_ID_AUTOSAVE);
}


void PinTable::AutoSave()
{
   if ((m_sdsCurrentDirtyState <= eSaveAutosaved) || CheckPermissions(DISABLE_TABLE_SAVE))
      return;

   KillTimer(m_hwnd, TIMER_ID_AUTOSAVE);

   {
      LocalString ls(IDS_AUTOSAVING);
      g_pvp->SetActionCur(ls.m_szbuffer);
      g_pvp->SetCursorCur(NULL, IDC_WAIT);
   }

   FastIStorage * const pstgroot = new FastIStorage();
   pstgroot->AddRef();

   const HRESULT hr = SaveToStorage(pstgroot);

   m_undo.SetCleanPoint((SaveDirtyState)min((int)m_sdsDirtyProp, (int)eSaveAutosaved));
   m_pcv->SetClean((SaveDirtyState)min((int)m_sdsDirtyScript, (int)eSaveAutosaved));
   SetNonUndoableDirty((SaveDirtyState)min((int)m_sdsNonUndoableDirty, (int)eSaveAutosaved));

   AutoSavePackage * const pasp = new AutoSavePackage();
   pasp->pstg = pstgroot;
   pasp->tableindex = g_pvp->m_vtable.IndexOf(this);
   pasp->HwndTable = m_hwnd;

   if (hr == S_OK)
   {
      HANDLE hEvent = g_pvp->PostWorkToWorkerThread(COMPLETE_AUTOSAVE, (LPARAM)pasp);
      m_vAsyncHandles.AddElement(hEvent);

      g_pvp->SetActionCur("Completing AutoSave");
   }
   else
   {
      g_pvp->SetActionCur("");
   }

   g_pvp->SetCursorCur(NULL, IDC_ARROW);
}

HRESULT PinTable::Save(BOOL fSaveAs)
{
   IStorage* pstgRoot;

#ifdef VBA
   pstgRoot = m_pStg;
#endif


   // Get file name if needed
   if(fSaveAs)
   {
      //need to get a file name
      OPENFILENAME ofn;
      ZeroMemory(&ofn, sizeof(OPENFILENAME));
      ofn.lStructSize = sizeof(OPENFILENAME);
      ofn.hInstance = g_hinst;
      ofn.hwndOwner = g_pvp->m_hwnd;
      // TEXT
      ofn.lpstrFilter = "Visual Pinball Tables (*.vpt)\0*.vpt\0";
      ofn.lpstrFile = m_szFileName;
      ofn.nMaxFile = _MAX_PATH;
      ofn.lpstrDefExt = "vpt";
      ofn.Flags = OFN_OVERWRITEPROMPT;

      char szInitialDir[1024];
      HRESULT hr = GetRegString("RecentDir","LoadDir", szInitialDir, 1024);
      char szFoo[MAX_PATH];
      if (hr == S_OK)
      {
         ofn.lpstrInitialDir = szInitialDir;
      }
      else
      {
         lstrcpy(szFoo, g_pvp->m_szMyPath);
         lstrcat(szFoo, "Tables");
         ofn.lpstrInitialDir = szFoo;
      }

      int ret = GetSaveFileName(&ofn);

      // user cancelled
      if(ret == 0)
         return S_FALSE;

      strcpy_s(szInitialDir, sizeof(szInitialDir), m_szFileName);
      szInitialDir[ofn.nFileOffset] = 0;
      hr = SetRegValue("RecentDir","LoadDir", REG_SZ, szInitialDir, strlen(szInitialDir));

      {
         MAKE_WIDEPTR_FROMANSI(wszCodeFile, m_szFileName);

         STGOPTIONS stg;
         stg.usVersion = 1;
         stg.reserved = 0;
         stg.ulSectorSize = 4096;

         if(FAILED(hr = StgCreateStorageEx(wszCodeFile, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
            STGFMT_DOCFILE, 0, &stg, 0, IID_IStorage, (void**)&pstgRoot)))
         {
            LocalString ls(IDS_SAVEERROR);
            MessageBox(m_hwnd, ls.m_szbuffer, "Visual Pinball", MB_ICONERROR);
            return hr;
         }
      }

      TitleFromFilename(m_szFileName, m_szTitle);
      SetCaption(m_szTitle);
   }
   else
   {
      MAKE_WIDEPTR_FROMANSI(wszCodeFile, m_szFileName);

      STGOPTIONS stg;
      stg.usVersion = 1;
      stg.reserved = 0;
      stg.ulSectorSize = 4096;

      HRESULT hr;
      if(FAILED(hr = StgCreateStorageEx(wszCodeFile, STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
         STGFMT_DOCFILE, 0, &stg, 0, IID_IStorage, (void**)&pstgRoot)))
      {
         LocalString ls(IDS_SAVEERROR);
         MessageBox(m_hwnd, ls.m_szbuffer, "Visual Pinball", MB_ICONERROR);
         return hr;
      }
   }

   {
      LocalString ls(IDS_SAVING);
      g_pvp->SetActionCur(ls.m_szbuffer);
      g_pvp->SetCursorCur(NULL, IDC_WAIT);
   }

   // merge all elements for saving
   BackupLayers();

   HRESULT hr = SaveToStorage(pstgRoot);

   if(!FAILED(hr))
   {
      pstgRoot->Commit(STGC_DEFAULT);
      pstgRoot->Release();

      g_pvp->SetActionCur("");
      g_pvp->SetCursorCur(NULL, IDC_ARROW);

      m_undo.SetCleanPoint(eSaveClean);
      m_pcv->SetClean(eSaveClean);
      SetNonUndoableDirty(eSaveClean);
   }

   // restore layers
   RestoreLayers();
   return S_OK;
}

HRESULT PinTable::SaveToStorage(IStorage *pstgRoot)
{
   IStorage *pstgData, *pstgInfo;
   IStream *pstmGame, *pstmItem;

   m_savingActive=true;
   RECT rc;
   SendMessage(g_pvp->m_hwndStatusBar, SB_GETRECT, 2, (long)&rc);

   HWND hwndProgressBar = CreateWindowEx(0, PROGRESS_CLASS, (LPSTR) NULL,
      WS_CHILD | WS_VISIBLE, rc.left,
      rc.top,
      rc.right-rc.left, rc.bottom-rc.top,
      g_pvp->m_hwndStatusBar, (HMENU) 0, g_hinst, NULL);

   SendMessage(hwndProgressBar, PBM_SETPOS, 1, 0);

   //////////////// Begin Encryption
   HCRYPTPROV hcp;
   HCRYPTHASH hch;
   HCRYPTKEY  hkey;
   HCRYPTHASH hchkey;

   int foo;

   hcp = NULL;
   hch = NULL;
   hkey = NULL;
   hchkey = NULL;

   foo = CryptAcquireContext(&hcp, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET/* | CRYPT_SILENT*/);

   //foo = CryptGenKey(hcp, CALG_RC2, CRYPT_EXPORTABLE, &hkey);

   foo = GetLastError();

   foo = CryptCreateHash(hcp, CALG_MD2/*CALG_MAC*//*CALG_HMAC*/, NULL/*hkey*/, 0, &hch);

   foo = GetLastError();

   BYTE hashval[256];
   DWORD hashlen = 256;

   foo = CryptHashData(hch, (BYTE *)TABLE_KEY, 14, 0);

   foo = GetLastError();

   // create a key hash (we have to use a second hash as deriving a key from the
   // integrity hash actually modifies it and thus it calculates the wrong hash)
   foo = CryptCreateHash(hcp, CALG_MD5, NULL, 0, &hchkey);
   foo = GetLastError();
   // hash the password
   foo = CryptHashData(hchkey, (BYTE *)TABLE_KEY, 14, 0);
   foo = GetLastError();
   // Create a block cipher session key based on the hash of the password.
   foo = CryptDeriveKey(hcp, CALG_RC2, hchkey, CRYPT_EXPORTABLE | 0x00280000, &hkey);
   foo = GetLastError();

   ////////////// End Encryption

   int ctotalitems = m_vedit.Size() + m_vsound.Size() + m_vimage.Size() + m_vfont.Size() + m_vcollection.Size();
   int csaveditems = 0;

   SendMessage(hwndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, ctotalitems));

   //first save our own data
   HRESULT hr;
   if(SUCCEEDED(hr = pstgRoot->CreateStorage(L"GameStg", STGM_DIRECT/*STGM_TRANSACTED*/ | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstgData)))
   {
      if(SUCCEEDED(hr = pstgData->CreateStream(L"GameData", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmGame)))
      {
         if(SUCCEEDED(hr = pstgData->CreateStream(L"Version", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
         {
            ULONG writ;
            int version = CURRENT_FILE_FORMAT_VERSION;
            CryptHashData(hch, (BYTE *)&version, sizeof(version), 0);
            pstmItem->Write(&version, sizeof(version), &writ);
            pstmItem->Release();
            pstmItem = NULL;
         }

         if(SUCCEEDED(hr = pstgRoot->CreateStorage(L"TableInfo",STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstgInfo)))
         {
            SaveInfo(pstgInfo, hch);

            if(SUCCEEDED(hr = pstgData->CreateStream(L"CustomInfoTags", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
            {
               SaveCustomInfo(pstgInfo, pstmItem, hch);
               pstmItem->Release();
               pstmItem = NULL;
            }

            pstgInfo->Release();
         }

         if(SUCCEEDED(hr = SaveData(pstmGame, hch, hkey)))
         {
            char szSuffix[32], szStmName[64];

            for(int i = 0; i < m_vedit.Size(); i++)
            {
               strcpy_s(szStmName, sizeof(szStmName), "GameItem");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if(SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  ULONG writ;
                  IEditable *piedit = m_vedit.ElementAt(i);
                  ItemTypeEnum type = piedit->GetItemType();
                  pstmItem->Write(&type, sizeof(int), &writ);
                  hr = piedit->SaveData(pstmItem, hch, hkey);
                  pstmItem->Release();
                  pstmItem = NULL;
                  //if(FAILED(hr)) goto Error;
               }

               csaveditems++;
               SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
            }

            for (int i=0;i<m_vsound.Size();i++)
            {
               strcpy_s(szStmName, sizeof(szStmName), "Sound");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if(SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  SaveSoundToStream(m_vsound.ElementAt(i), pstmItem);
                  pstmItem->Release();
                  pstmItem = NULL;
               }

               csaveditems++;
               SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
            }

            for (int i=0;i<m_vimage.Size();i++)
            {
               strcpy_s(szStmName, sizeof(szStmName), "Image");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if(SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  m_vimage.ElementAt(i)->SaveToStream(pstmItem, this);
                  pstmItem->Release();
                  pstmItem = NULL;
               }

               csaveditems++;
               SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
            }

            for (int i=0;i<m_vfont.Size();i++)
            {
               strcpy_s(szStmName, sizeof(szStmName), "Font");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if(SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  m_vfont.ElementAt(i)->SaveToStream(pstmItem);
                  pstmItem->Release();
                  pstmItem = NULL;
               }

               csaveditems++;
               SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
            }

            for (int i=0;i<m_vcollection.Size();i++)
            {
               strcpy_s(szStmName, sizeof(szStmName), "Collection");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if(SUCCEEDED(hr = pstgData->CreateStream(wszStmName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
               {
                  m_vcollection.ElementAt(i)->SaveData(pstmItem, hch, hkey);
                  pstmItem->Release();
                  pstmItem = NULL;
               }

               csaveditems++;
               SendMessage(hwndProgressBar, PBM_SETPOS, csaveditems, 0);
            }
         }
         pstmGame->Release();
      }

      // Authentication block
      foo = CryptGetHashParam(hch, HP_HASHSIZE, hashval, &hashlen, 0);

      hashlen = 256;
      foo = CryptGetHashParam(hch, HP_HASHVAL, hashval, &hashlen, 0);

      if(SUCCEEDED(hr = pstgData->CreateStream(L"MAC", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstmItem)))
      {
         ULONG writ;
         //int version = CURRENT_FILE_FORMAT_VERSION;
         pstmItem->Write(hashval, hashlen, &writ);
         pstmItem->Release();
         pstmItem = NULL;
         //if(FAILED(hr)) goto Error;

         //CryptExportKey(hkey, NULL, PUBLICKEYBLOB, 0, BYTE *pbData, DWORD *pdwDataLen);
      }

      foo = CryptDestroyHash(hch);

      foo = CryptDestroyHash(hchkey);

      foo = CryptDestroyKey(hkey);

      foo = CryptReleaseContext(hcp, 0);
      //////// End Authentication block

      if(SUCCEEDED(hr))
         pstgData->Commit(STGC_DEFAULT);
      else
      {
         pstgData->Revert();
         pstgRoot->Revert();
         LocalString ls(IDS_SAVEERROR);
         MessageBox(m_hwnd, ls.m_szbuffer, "Visual Pinball", MB_ICONERROR);
      }
      pstgData->Release();
   }

   //Error:

   DestroyWindow(hwndProgressBar);
   m_savingActive=false;
   return hr;
}

HRESULT PinTable::SaveSoundToStream(PinSound *pps, IStream *pstm)
{
   ULONG writ = 0;
   int len = lstrlen(pps->m_szName);

   HRESULT hr;
   if(FAILED(hr = pstm->Write(&len, sizeof(int), &writ)))
      return hr;

   if(FAILED(hr = pstm->Write(pps->m_szName, len, &writ)))
      return hr;

   len = lstrlen(pps->m_szPath);

   if(FAILED(hr = pstm->Write(&len, sizeof(int), &writ)))
      return hr;

   if(FAILED(hr = pstm->Write(pps->m_szPath, len, &writ)))
      return hr;

   len = lstrlen(pps->m_szInternalName);

   if(FAILED(hr = pstm->Write(&len, sizeof(int), &writ)))
      return hr;

   if(FAILED(hr = pstm->Write(pps->m_szInternalName, len, &writ)))
      return hr;

   WAVEFORMATEX wfx;
   pps->m_pDSBuffer->GetFormat(&wfx, sizeof(wfx), NULL);

   if(FAILED(hr = pstm->Write(&wfx, sizeof(wfx), &writ)))
      return hr;

   if(FAILED(hr = pstm->Write(&pps->m_cdata, sizeof(int), &writ)))
      return hr;

   if(FAILED(hr = pstm->Write(pps->m_pdata, pps->m_cdata, &writ)))
      return hr;

   return S_OK;
}


HRESULT PinTable::LoadSoundFromStream(IStream *pstm)
{
   int len;
   ULONG read = 0;
   HRESULT hr = S_OK;
   WAVEFORMATEX wfx;

   if(FAILED(hr = pstm->Read(&len, sizeof(len), &read)))
      return hr;

   PinSound * const pps = new PinSound();
   if(FAILED(hr = pstm->Read(pps->m_szName, len, &read)))
      return hr;

   pps->m_szName[len] = 0;

   if(FAILED(hr = pstm->Read(&len, sizeof(len), &read)))
      return hr;

   if(FAILED(hr = pstm->Read(pps->m_szPath, len, &read)))
      return hr;

   pps->m_szPath[len] = 0;

   if(FAILED(hr = pstm->Read(&len, sizeof(len), &read)))
      return hr;

   if(FAILED(hr = pstm->Read(pps->m_szInternalName, len, &read)))
      return hr;

   pps->m_szInternalName[len] = 0;

   if(FAILED(hr = pstm->Read(&wfx, sizeof(wfx), &read)))
      return hr;

   if(FAILED(hr = pstm->Read(&pps->m_cdata, sizeof(int), &read)))
      return hr;

   pps->m_pdata = new char [pps->m_cdata];

   //LZWReader lzwreader(pstm, (int *)pps->m_pdata, pps->m_cdata, 1, pps->m_cdata);

   //lzwreader.Decoder();

   if(FAILED(hr = pstm->Read(pps->m_pdata, pps->m_cdata, &read)))
   {
      delete pps;
      return hr;
   }

   if (pps->GetPinDirectSound()->CreateDirectFromNative(pps, &wfx) == S_OK)
   {
      m_vsound.AddElement(pps);
   }

   return S_OK;
}


HRESULT PinTable::WriteInfoValue(IStorage* pstg, WCHAR *wzName, char *szValue, HCRYPTHASH hcrypthash)
{
   HRESULT hr;
   IStream *pstm;

   if(szValue && SUCCEEDED(hr = pstg->CreateStream(wzName, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstm)))
   {
      ULONG writ;
      BiffWriter bw(pstm, hcrypthash, NULL);

      const int len = lstrlen(szValue);
      WCHAR *wzT = new WCHAR[len+1];
      MultiByteToWideChar(CP_ACP, 0, szValue, -1, wzT, len+1);

      bw.WriteBytes(wzT, len*sizeof(WCHAR), &writ);
      delete [] wzT;
      pstm->Release();
      pstm = NULL;
   }

   return S_OK;
}


HRESULT PinTable::SaveInfo(IStorage* pstg, HCRYPTHASH hcrypthash)
{
   WriteInfoValue(pstg, L"TableName", m_szTableName, hcrypthash);
   WriteInfoValue(pstg, L"AuthorName", m_szAuthor, hcrypthash);
   WriteInfoValue(pstg, L"TableVersion", m_szVersion, hcrypthash);
   WriteInfoValue(pstg, L"ReleaseDate", m_szReleaseDate, hcrypthash);
   WriteInfoValue(pstg, L"AuthorEmail", m_szAuthorEMail, hcrypthash);
   WriteInfoValue(pstg, L"AuthorWebSite", m_szWebSite, hcrypthash);
   WriteInfoValue(pstg, L"TableBlurb", m_szBlurb, hcrypthash);
   WriteInfoValue(pstg, L"TableDescription", m_szDescription, hcrypthash);
   WriteInfoValue(pstg, L"TableRules", m_szRules, hcrypthash);

   Texture *pin = GetImage(m_szScreenShot);
   if (pin != NULL && pin->m_ppb != NULL)
   {
      IStream *pstm;
      HRESULT hr;
      ULONG writ;

      if(SUCCEEDED(hr = pstg->CreateStream(L"Screenshot", STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, &pstm)))
      {
         BiffWriter bw(pstm, hcrypthash, NULL);
         bw.WriteBytes(pin->m_ppb->m_pdata, pin->m_ppb->m_cdata, &writ);
         pstm->Release();
         pstm = NULL;
      }
   }

   pstg->Commit(STGC_DEFAULT);

   return S_OK;
}


HRESULT PinTable::SaveCustomInfo(IStorage* pstg, IStream *pstmTags, HCRYPTHASH hcrypthash)
{
   BiffWriter bw(pstmTags, hcrypthash, NULL);

   for (int i=0;i<m_vCustomInfoTag.Size();i++)
   {
      bw.WriteString(FID(CUST), m_vCustomInfoTag.ElementAt(i));
   }


   bw.WriteTag(FID(ENDB));

   for (int i=0;i<m_vCustomInfoTag.Size();i++)
   {
      char *szName = m_vCustomInfoTag.ElementAt(i);
      int len = lstrlen(szName);
      WCHAR *wzName = new WCHAR[len+1];
      MultiByteToWideChar(CP_ACP, 0, szName, -1, wzName, len+1);

      WriteInfoValue(pstg, wzName, m_vCustomInfoContent.ElementAt(i), hcrypthash);

      delete [] wzName;
   }

   pstg->Commit(STGC_DEFAULT);

   return S_OK;
}


HRESULT PinTable::ReadInfoValue(IStorage* pstg, WCHAR *wzName, char **pszValue, HCRYPTHASH hcrypthash)
{
   HRESULT hr;
   IStream *pstm;

   if(SUCCEEDED(hr = pstg->OpenStream(wzName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstm)))
   {
      STATSTG ss;
      pstm->Stat(&ss, STATFLAG_NONAME);

      const int len = ss.cbSize.LowPart / sizeof(WCHAR);
      WCHAR *wzT = new WCHAR[len+1];
      *pszValue = new char[len+1];

      ULONG read;
      BiffReader br(pstm, NULL, NULL, 0, hcrypthash, NULL);
      br.ReadBytes(wzT, ss.cbSize.LowPart, &read);
      wzT[len] = L'\0';

      WideCharToMultiByte(CP_ACP, 0, wzT, -1, *pszValue, len+1, NULL, NULL);

      //delete br;
      //pstm->Read(*pszValue, ss.cbSize.LowPart, &read);

      delete [] wzT;
      pstm->Release();
   }

   return S_OK;
}


HRESULT PinTable::LoadInfo(IStorage* pstg, HCRYPTHASH hcrypthash, int version)
{
   ReadInfoValue(pstg, L"TableName", &m_szTableName, hcrypthash);
   ReadInfoValue(pstg, L"AuthorName", &m_szAuthor, hcrypthash);
   ReadInfoValue(pstg, L"TableVersion", &m_szVersion, hcrypthash);
   ReadInfoValue(pstg, L"ReleaseDate", &m_szReleaseDate, hcrypthash);
   ReadInfoValue(pstg, L"AuthorEmail", &m_szAuthorEMail, hcrypthash);
   ReadInfoValue(pstg, L"AuthorWebSite", &m_szWebSite, hcrypthash);
   ReadInfoValue(pstg, L"TableBlurb", &m_szBlurb, hcrypthash);
   ReadInfoValue(pstg, L"TableDescription", &m_szDescription, hcrypthash);
   ReadInfoValue(pstg, L"TableRules", &m_szRules, hcrypthash);

   // Check pointer.
   if ( m_szVersion != NULL )
   {
      // Write the version to the registry.  This will be read later by the front end.
      SetRegValue ( "Version", m_szTableName, REG_SZ, m_szVersion, strlen(m_szVersion) );
   }

   HRESULT hr;
   IStream *pstm;

   if(SUCCEEDED(hr = pstg->OpenStream(L"Screenshot", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstm)))
   {
      STATSTG ss;
      pstm->Stat(&ss, STATFLAG_NONAME);
      //char *pdata = new char[ss.cbSize.LowPart];
      m_pbTempScreenshot = new PinBinary();

      m_pbTempScreenshot->m_cdata = ss.cbSize.LowPart;
      m_pbTempScreenshot->m_pdata = new char[m_pbTempScreenshot->m_cdata];

      //m_pbTempScreenshot->LoadFromStream(pstm, version);

      ULONG read;
      BiffReader br(pstm, NULL, NULL, 0, hcrypthash, NULL);
      br.ReadBytes(m_pbTempScreenshot->m_pdata, m_pbTempScreenshot->m_cdata, &read);

      //delete pdata;

      pstm->Release();
   }

   return S_OK;
}

HRESULT PinTable::LoadCustomInfo(IStorage* pstg, IStream *pstmTags, HCRYPTHASH hcrypthash, int version)
{
   BiffReader br(pstmTags, this, NULL, version, hcrypthash, NULL);
   br.Load();

   for (int i=0;i<m_vCustomInfoTag.Size();i++)
   {
      char *szName = m_vCustomInfoTag.ElementAt(i);
      char *szValue;
      const int len = lstrlen(szName);
      WCHAR *wzName = new WCHAR[len+1];
      MultiByteToWideChar(CP_ACP, 0, szName, -1, wzName, len+1);

      ReadInfoValue(pstg, wzName, &szValue, hcrypthash);
      m_vCustomInfoContent.AddElement(szValue);

      delete [] wzName;
   }

   return S_OK;
}

HRESULT PinTable::SaveData(IStream* pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
   bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
   bw.WriteFloat(FID(GRSZ), m_gridsize);
   bw.WriteFloat(FID(LEFT), m_left);
   bw.WriteFloat(FID(TOPX), m_top);
   bw.WriteFloat(FID(RGHT), m_right);
   bw.WriteFloat(FID(BOTM), m_bottom);
   bw.WriteFloat(FID(XLTX), m_xlatex);
   bw.WriteFloat(FID(XLTY), m_xlatey);
   bw.WriteFloat(FID(XLTZ), m_xlatez);
   bw.WriteFloat(FID(SCLX), m_scalex);
   bw.WriteFloat(FID(SCLY), m_scaley);
   bw.WriteFloat(FID(SCLZ), m_zScale);

   bw.WriteInt(FID(ORRP), m_fOverridePhysics);
   bw.WriteFloat(FID(GAVT), m_Gravity);
   bw.WriteFloat(FID(FRCT), m_hardFriction);
   bw.WriteFloat(FID(SCAT), m_hardScatter);
   bw.WriteFloat(FID(NDGT), m_nudgeTime);
   bw.WriteInt(FID(MPGC), m_plungerNormalize);
   bw.WriteBool(FID(MPDF), m_plungerFilter);
   bw.WriteInt(FID(PHML), m_PhysicsMaxLoops);

   //bw.WriteFloat(FID(IMTCOL), m_transcolor);

   bw.WriteBool(FID(REEL), m_fRenderEMReels);
   bw.WriteBool(FID(DECL), m_fRenderDecals);

   bw.WriteFloat(FID(OFFX), m_offsetx);
   bw.WriteFloat(FID(OFFY), m_offsety);

   bw.WriteFloat(FID(ZOOM), m_zoom);

   bw.WriteFloat(FID(ROTA), m_rotation);
   bw.WriteFloat(FID(INCL), m_inclination);
   bw.WriteFloat(FID(LAYB), m_layback);
   bw.WriteFloat(FID(FOVX), m_FOV);
   bw.WriteFloat(FID(SLPX), m_angletiltMax);
   bw.WriteFloat(FID(SLOP), m_angletiltMin);

   bw.WriteFloat(FID(MAXSEP), m_maxSeparation);
   bw.WriteFloat(FID(ZPD), m_ZPD);
   bw.WriteBool(FID(OGST), m_overwriteGlobalStereo3D );

   bw.WriteString(FID(IMAG), m_szImage);
   bw.WriteString(FID(BIMG), m_szImageBackdrop);
   bw.WriteString(FID(BLIM), m_szBallImage);
   bw.WriteString(FID(BLIF), m_szBallImageFront);
   bw.WriteString(FID(BLIB), m_szBallImageBack);

   bw.WriteString(FID(SSHT), m_szScreenShot);

   bw.WriteBool(FID(FGRD), m_fGrid);
   bw.WriteBool(FID(FBCK), m_fBackdrop);

   bw.WriteFloat(FID(GLAS), m_glassheight);
   bw.WriteFloat(FID(TBLH), m_tableheight);

   bw.WriteInt(FID(COLR), m_colorplayfield);
   bw.WriteInt(FID(BCLR), m_colorbackdrop);

   bw.WriteBool(FID(DSHD), m_fRenderShadows);

   //////////////////
   bw.WriteFloat(FID(TDFT), m_globalDifficulty);
   bw.WriteBool(FID(ACEL), m_tblAccelerometer);
   bw.WriteBool(FID(AORD), m_tblAccelNormalMount);
   bw.WriteFloat(FID(AANG), m_tblAccelAngle);
   bw.WriteFloat(FID(AAMP), m_tblAccelAmp);
   bw.WriteFloat(FID(AAMX), m_tblAccelAmpX);
   bw.WriteFloat(FID(AAMY), m_tblAccelAmpY);
   bw.WriteFloat(FID(AMAM), m_tblAccelManualAmp);
   //////////////////
   bw.WriteInt(FID(JLTA), m_jolt_amount);
   bw.WriteInt(FID(TLTA), m_tilt_amount);	
   bw.WriteInt(FID(JLTT), m_jolt_trigger_time);
   bw.WriteInt(FID(TLTT), m_tilt_trigger_time);

   bw.WriteInt(FID(LZAM), m_Light[0].ambient);
   bw.WriteInt(FID(LZDI), m_Light[0].diffuse);
   bw.WriteInt(FID(LZSP), m_Light[0].specular);
   bw.WriteFloat(FID(LZPX), m_Light[0].pos.x);
   bw.WriteFloat(FID(LZPY), m_Light[0].pos.y);
   bw.WriteFloat(FID(LZPZ), m_Light[0].pos.z);
   bw.WriteFloat(FID(LZDX), m_Light[0].dir.x);
   bw.WriteFloat(FID(LZDY), m_Light[0].dir.y);
   bw.WriteFloat(FID(LZDZ), m_Light[0].dir.z);
   bw.WriteInt(FID(LZTY), m_Light[0].type);
   bw.WriteInt(FID(LOAM), m_Light[1].ambient);
   bw.WriteInt(FID(LODI), m_Light[1].diffuse);
   bw.WriteInt(FID(LOSP), m_Light[1].specular);
   bw.WriteFloat(FID(LOPX), m_Light[1].pos.x);
   bw.WriteFloat(FID(LOPY), m_Light[1].pos.y);
   bw.WriteFloat(FID(LOPZ), m_Light[1].pos.z);
   bw.WriteFloat(FID(LODX), m_Light[1].dir.x);
   bw.WriteFloat(FID(LODY), m_Light[1].dir.y);
   bw.WriteFloat(FID(LODZ), m_Light[1].dir.z);
   bw.WriteInt(FID(LOTY), m_Light[1].type);

   bw.WriteInt(FID(NONO), m_NormalizeNormals);
   bw.WriteFloat(FID(SDIX), m_shadowDirX);
   bw.WriteFloat(FID(SDIY), m_shadowDirY);

   bw.WriteFloat(FID(SVOL), m_TableSoundVolume);
   bw.WriteFloat(FID(MVOL), m_TableMusicVolume);

   bw.WriteInt(FID(AVSY), m_TableAdaptiveVSync);

   bw.WriteInt(FID(BREF), m_useReflectionForBalls );
   bw.WriteInt(FID(BRST), m_ballReflectionStrength );
   bw.WriteInt(FID(BTRA), m_useTrailForBalls );
   bw.WriteInt(FID(BTST), m_ballTrailStrength );
   bw.WriteInt(FID(ARAC), m_userAlphaRampsAccuracy );
   bw.WriteBool(FID(OGAC), m_overwriteGlobalAlphaRampsAccuracy );

   bw.WriteInt(FID(UAAL), m_useAA );
   bw.WriteInt(FID(UFXA), m_useFXAA );

   // HACK!!!! - Don't save special values when copying for undo.  For instance, don't reset the code.
   // Someday save these values into there own stream, used only when saving to file.

   if (hcrypthash != 0)
   {
      bw.WriteInt(FID(SEDT), m_vedit.Size());
      bw.WriteInt(FID(SSND), m_vsound.Size());
      bw.WriteInt(FID(SIMG), m_vimage.Size());
      bw.WriteInt(FID(SFNT), m_vfont.Size());
      bw.WriteInt(FID(SCOL), m_vcollection.Size());

      bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);

      bw.WriteStruct(FID(CCUS), m_rgcolorcustom, sizeof(COLORREF)*16);

      bw.WriteStruct(FID(SECB), &m_protectionData, sizeof(_protectionData));

      // save the script source code
      bw.WriteTag(FID(CODE));
      // if the script is protected then we pass in the proper cyptokey into the code savestream
      m_pcv->SaveToStream(pstm, hcrypthash, CheckPermissions(DISABLE_SCRIPT_EDITING) ? hcryptkey : NULL);
   }

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT PinTable::LoadGameFromFilename(char *szFileName)
{
   IStorage* pstgRoot;
   HRESULT hr = S_OK;

   //ASSERT(*szFileName, "Empty File Name String!");

   strcpy_s(m_szFileName, sizeof(m_szFileName), szFileName);
   {
      MAKE_WIDEPTR_FROMANSI(wszCodeFile, szFileName);
      if(FAILED(hr = StgOpenStorage(wszCodeFile, NULL, STGM_TRANSACTED | STGM_READ | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgRoot)))
      {
         // TEXT
         char msg[256];
         sprintf_s( msg, "Error loading %s", m_szFileName );
         MessageBox(g_pvp->m_hwnd, msg , "Load Error", 0);
         return hr;
      }
   }

   return LoadGameFromStorage(pstgRoot);
}

HRESULT PinTable::LoadGameFromStorage(IStorage *pstgRoot)
{
   IStorage *pstgData, *pstgInfo;
   IStream *pstmGame, *pstmItem, *pstmVersion;
   HRESULT hr = S_OK;

   int ctotalitems;
   int cloadeditems;

   RECT rc;
   SendMessage(g_pvp->m_hwndStatusBar, SB_GETRECT, 2, (long)&rc);

   HWND hwndProgressBar = CreateWindowEx(0, PROGRESS_CLASS, (LPSTR) NULL,
      WS_CHILD | WS_VISIBLE, rc.left,
      rc.top,
      rc.right-rc.left, rc.bottom-rc.top,
      g_pvp->m_hwndStatusBar, (HMENU) 0, g_hinst, NULL);

   SendMessage(hwndProgressBar, PBM_SETPOS, 1, 0);

   LocalString ls(IDS_LOADING);
   g_pvp->SetActionCur(ls.m_szbuffer);
   g_pvp->SetCursorCur(NULL, IDC_WAIT);

   HCRYPTPROV hcp = NULL;
   HCRYPTHASH hch = NULL;
   HCRYPTHASH hchkey = NULL;
   HCRYPTKEY  hkey = NULL;

   ///////// Begin MAC
   int foo;

   foo = CryptAcquireContext(&hcp, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_NEWKEYSET/* | CRYPT_SILENT*/);

   foo = GetLastError();

   foo = CryptCreateHash(hcp, CALG_MD2/*CALG_MAC*//*CALG_HMAC*/, NULL/*hkey*/, 0, &hch);

   foo = GetLastError();

   BYTE hashval[256];
   DWORD hashlen = 256;

   foo = CryptHashData(hch, (BYTE *)TABLE_KEY, 14, 0);

   foo = GetLastError();

   // create a key hash (we have to use a second hash as deriving a key from the
   // integrity hash actually modifies it and thus it calculates the wrong hash)
   foo = CryptCreateHash(hcp, CALG_MD5, NULL, 0, &hchkey);
   foo = GetLastError();
   // hash the password
   foo = CryptHashData(hchkey, (BYTE *)TABLE_KEY, 14, 0);
   foo = GetLastError();
   // Create a block cipher session key based on the hash of the password.
   // We need to figure out the file verison before we can create the key

   ////////////// End MAC

   //load our stuff first
   if(SUCCEEDED(hr = pstgRoot->OpenStorage(L"GameStg", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgData)))
   {
      if(SUCCEEDED(hr = pstgData->OpenStream(L"GameData", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmGame)))
      {
         int csubobj = 0;
         int csounds = 0;
         int ctextures = 0;
         int cfonts = 0;
         int ccollection = 0;
         int version = CURRENT_FILE_FORMAT_VERSION;

         if(SUCCEEDED(hr = pstgData->OpenStream(L"Version", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmVersion)))
         {
            ULONG read;
            hr = pstmVersion->Read(&version, sizeof(int), &read);
            CryptHashData(hch, (BYTE *)&version, sizeof(int), 0);
            pstmVersion->Release();
            if (version >= BEYOND_FILE_FORMAT_VERSION)
            {
               LocalString ls2(IDS_WRONGFILEVERSION);
               ShowError(ls2.m_szbuffer);
               pstgRoot->Release();
               pstmGame->Release();
               pstgData->Release();
               DestroyWindow(hwndProgressBar);
               g_pvp->SetCursorCur(NULL, IDC_ARROW);
            }

            // Create a block cipher session key based on the hash of the password.
            CryptDeriveKey(hcp, CALG_RC2, hchkey, (version == 600) ? CRYPT_EXPORTABLE : (CRYPT_EXPORTABLE | 0x00280000), &hkey);
         }

         if(SUCCEEDED(hr = pstgRoot->OpenStorage(L"TableInfo", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgInfo)))
         {
            LoadInfo(pstgInfo, hch, version);
            if(SUCCEEDED(hr = pstgData->OpenStream(L"CustomInfoTags", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
            {
               LoadCustomInfo(pstgInfo, pstmItem, hch, version);
               pstmItem->Release();
               pstmItem = NULL;
            }
            pstgInfo->Release();
         }

         if(SUCCEEDED(hr = LoadData(pstmGame, csubobj, csounds, ctextures, cfonts, ccollection, version, hch, hkey)))
         {

            ctotalitems = csubobj + csounds + ctextures + cfonts;
            cloadeditems = 0;
            SendMessage(hwndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, ctotalitems));

            for(int i = 0; i < csubobj; i++)
            {
               char szSuffix[32], szStmName[64];
               strcpy_s(szStmName, sizeof(szStmName), "GameItem");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if(SUCCEEDED(hr = pstgData->OpenStream(wszStmName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  ULONG read;
                  ItemTypeEnum type;
                  IEditable *piedit;
                  hr = pstmItem->Read(&type, sizeof(int), &read);
                  CreateIEditableFromType(type, &piedit);

                  piedit->AddRef();

                  //AddSpriteProjItem();
                  int id = 0; // VBA id for this item
                  hr = piedit->InitLoad(pstmItem, this, &id, version, hch, hkey);
                  piedit->InitVBA(fFalse, id, NULL);
                  pstmItem->Release();
                  pstmItem = NULL;
                  if(FAILED(hr)) break;

                  m_vedit.AddElement(piedit);

                  //hr = piedit->InitPostLoad();
               }
               cloadeditems++;
               SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
            }

            for(int i = 0; i < csounds; i++)
            {
               char szSuffix[32], szStmName[64];
               strcpy_s(szStmName, sizeof(szStmName), "Sound");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if(SUCCEEDED(hr = pstgData->OpenStream(wszStmName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  LoadSoundFromStream(pstmItem);
                  pstmItem->Release();
                  pstmItem = NULL;
               }
               cloadeditems++;
               SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
            }

            for(int i = 0; i < ctextures; i++)
            {
               char szSuffix[32], szStmName[64];
               strcpy_s(szStmName, sizeof(szStmName), "Image");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if(SUCCEEDED(hr = pstgData->OpenStream(wszStmName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  LoadImageFromStream(pstmItem, version);
                  pstmItem->Release();
                  pstmItem = NULL;
               }
               cloadeditems++;
               SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
            }

            for(int i = 0; i < cfonts; i++)
            {
               char szSuffix[32], szStmName[64];
               strcpy_s(szStmName, sizeof(szStmName), "Font");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if(SUCCEEDED(hr = pstgData->OpenStream(wszStmName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  PinFont *ppf;
                  ppf = new PinFont();
                  ppf->LoadFromStream(pstmItem, version);
                  m_vfont.AddElement(ppf);
                  ppf->Register();
                  pstmItem->Release();
                  pstmItem = NULL;
               }
               cloadeditems++;
               SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
            }

            for(int i = 0; i < ccollection; i++)
            {
               char szSuffix[32], szStmName[64];
               strcpy_s(szStmName, sizeof(szStmName), "Collection");
               _itoa_s(i, szSuffix, sizeof(szSuffix), 10);
               strcat_s(szStmName, sizeof(szStmName), szSuffix);

               MAKE_WIDEPTR_FROMANSI(wszStmName, szStmName);

               if(SUCCEEDED(hr = pstgData->OpenStream(wszStmName, NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmItem)))
               {
                  CComObject<Collection> *pcol;
                  CComObject<Collection>::CreateInstance(&pcol);
                  pcol->AddRef();
                  pcol->LoadData(pstmItem, this, version, hch, hkey);
                  m_vcollection.AddElement(pcol);
                  m_pcv->AddItem((IScriptable *)pcol, fFalse);
                  pstmItem->Release();
                  pstmItem = NULL;
               }
               cloadeditems++;
               SendMessage(hwndProgressBar, PBM_SETPOS, cloadeditems, 0);
            }

            for (int i=0;i<m_vedit.Size();i++)
            {
               IEditable * const piedit = m_vedit.ElementAt(i);
               piedit->InitPostLoad();
            }
         }
         pstmGame->Release();

         // Authentication block

         if (version > 40)
         {
            if(SUCCEEDED(hr = pstgData->OpenStream(L"MAC", NULL, STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstmVersion)))
            {
               BYTE hashvalOld[256];
               //DWORD hashlenOld = 256;
               ULONG read;
               hr = pstmVersion->Read(&hashvalOld, HASHLENGTH, &read);

               foo = CryptGetHashParam(hch, HP_HASHSIZE, hashval, &hashlen, 0);

               hashlen = 256;
               foo = CryptGetHashParam(hch, HP_HASHVAL, hashval, &hashlen, 0);

               foo = CryptDestroyHash(hch);

               foo = CryptDestroyHash(hchkey);

               foo = CryptDestroyKey(hkey);

               foo = CryptReleaseContext(hcp, 0);

               for (int i=0;i<HASHLENGTH;i++)
               {
                  if (hashval[i] != hashvalOld[i])
                  {
                     hr = E_ACCESSDENIED;
                  }
               }
            }
            else
            {
               // Error
               hr = E_ACCESSDENIED;
            }				
         }
         //////// End Authentication block
      }
      pstgData->Release();
   }

   if (m_pbTempScreenshot) // For some reason, no image picked up the screenshot.  Not good; but we'll dump it to make sure it gets cleaned up
   {
      delete m_pbTempScreenshot;
      m_pbTempScreenshot = NULL;
   }

   DestroyWindow(hwndProgressBar);
   //DestroyWindow(hwndProgressDialog);

#ifdef VBA
   if(FAILED(hr)) return hr;

   //Now do the vba stuff
   if(SUCCEEDED(hr = ApcProject.Open(g_pvp->ApcHost, axProjectNormal)))
   {
      if(SUCCEEDED(hr = ApcProject.Load(pstgRoot)))
      {
         if(!SUCCEEDED(hr = ApcProject.FinishLoading()))
         {
            ShowError("Some parts of the table could not be loaded.");
            hr = S_OK; // Load the parts that were okay
         }
         m_pStg = pstgRoot;

         MAKE_WIDEPTR_FROMANSI(wszFileName, m_szFileName);
         BSTR bstrFileName = SysAllocString(wszFileName);
         ApcProject->APC_PUT(DisplayName)(bstrFileName);
         SysFreeString(bstrFileName);
      }
   }
#else
   pstgRoot->Release();
#endif

   g_pvp->SetActionCur("");

   for( int t=0;t<8;t++ ) m_layer[t].Empty();

   // copy all elements into their layers
   for( int i=0;i<8;i++ )
   {
      for( int t=0;t<m_vedit.Size();t++ )
      {
         IEditable *piedit = m_vedit.ElementAt(t);
         if ( piedit->GetISelect()->layerIndex==i )
         {
            m_layer[i].AddElement(piedit);
         }
      }
   }

   return hr;
}

HRESULT PinTable::CreateIEditableFromType(int type, IEditable **piedit)
{
   switch (type)
   {
   case eItemSurface: {
      CComObject<Surface> *psurface;
      CComObject<Surface>::CreateInstance(&psurface);
      *piedit = psurface;
      break;}

   case eItemFlipper: {
      CComObject<Flipper> *pflipper;
      CComObject<Flipper>::CreateInstance(&pflipper);
      *piedit = pflipper;
      break;}

   case eItemTimer: {
      CComObject<Timer> *ptimer;
      CComObject<Timer>::CreateInstance(&ptimer);
      *piedit = ptimer;
      break;}

   case eItemPlunger: {
      CComObject<Plunger> *pplunger;
      CComObject<Plunger>::CreateInstance(&pplunger);
      *piedit = pplunger;
      break;}

   case eItemTextbox: {
      CComObject<Textbox> *ptextbox;
      CComObject<Textbox>::CreateInstance(&ptextbox);
      *piedit = ptextbox;
      break;}

   case eItemComControl: {
      CComObject<PinComControl> *pcomcontrol;
      CComObject<PinComControl>::CreateInstance(&pcomcontrol);
      *piedit = pcomcontrol;
      break;}

   case eItemDispReel: {
      CComObject<DispReel> *pdispreel;
      CComObject<DispReel>::CreateInstance(&pdispreel);
      *piedit = pdispreel;
      break;}

   case eItemLightSeq: {
      CComObject<LightSeq> *plightseq;
      CComObject<LightSeq>::CreateInstance(&plightseq);
      *piedit = plightseq;
      break;}

   case eItemBumper: {
      CComObject<Bumper> *pbumper;
      CComObject<Bumper>::CreateInstance(&pbumper);
      *piedit = pbumper;
      break;}

   case eItemTrigger: {
      CComObject<Trigger> *ptrigger;
      CComObject<Trigger>::CreateInstance(&ptrigger);
      *piedit = ptrigger;
      break;}

   case eItemLight: {
      CComObject<Light> *plight;
      CComObject<Light>::CreateInstance(&plight);
      *piedit = plight;
      break;}

   case eItemKicker: {
      CComObject<Kicker> *pkicker;
      CComObject<Kicker>::CreateInstance(&pkicker);
      *piedit = pkicker;
      break;}

   case eItemDecal: {
      CComObject<Decal> *pdecal;
      CComObject<Decal>::CreateInstance(&pdecal);
      *piedit = pdecal;
      break;}

   case eItemGate: {
      CComObject<Gate> *pgate;
      CComObject<Gate>::CreateInstance(&pgate);
      *piedit = pgate;
      break;}

   case eItemSpinner: {
      CComObject<Spinner> *pspinner;
      CComObject<Spinner>::CreateInstance(&pspinner);
      *piedit = pspinner;
      break;}

   case eItemRamp: {
      CComObject<Ramp> *pramp;
      CComObject<Ramp>::CreateInstance(&pramp);
      *piedit = pramp;
      break;}

   case eItemPrimitive: {
      CComObject<Primitive> *pprimitive;
      CComObject<Primitive>::CreateInstance(&pprimitive);
      *piedit = pprimitive;
      break;}

   case eItemFlasher: {
      CComObject<Flasher> *pflasher;
      CComObject<Flasher>::CreateInstance(&pflasher);
      *piedit = pflasher;
      break;}

   default:
      _ASSERTE(fFalse);
      break;
   }

   return S_OK;
}

void PinTable::SetLoadDefaults()
{
   m_szImageBackdrop[0] = 0;
   m_szBallImage[0] = 0;
   m_szBallImageFront[0] = 0;
   m_szBallImageBack[0] = 0;

   m_szScreenShot[0] = 0;

   m_colorplayfield = RGB(128,128,128);
   m_colorbackdrop = RGB(0x62,0x6E,0x8E);

   m_Light[0].ambient = RGB((int)(0.1*255),(int)(0.1*255),(int)(0.1*255));
   m_Light[0].diffuse = RGB((int)(0.4*255),(int)(0.4*255),(int)(0.4*255));
   m_Light[0].specular = RGB((int)(0.0*255),(int)(0.0*255),(int)(0.0*255));
   m_Light[0].pos = Vertex3Ds(0,0,0);
   m_Light[0].dir = Vertex3Ds(0,0,0); // 0,0,0 = backwards compatible mode
   m_Light[0].type = LIGHT_DIRECTIONAL;
   m_Light[0].enabled = true;
   m_Light[1].ambient = RGB((int)(0.1*255),(int)(0.1*255),(int)(0.1*255));
   m_Light[1].diffuse = RGB((int)(0.6*255),(int)(0.6*255),(int)(0.6*255));
   m_Light[1].specular = RGB((int)(1.0*255),(int)(1.0*255),(int)(1.0*255));
   m_Light[1].pos = Vertex3Ds(0,0,0);
   m_Light[1].dir = Vertex3Ds(0,0,0); // 0,0,0 = backwards compatible mode
   m_Light[1].type = LIGHT_DIRECTIONAL;
   m_Light[1].enabled = true;

   for(int i = 2; i < MAX_LIGHT_SOURCES; ++i)
      m_Light[i].enabled = false; //!! expose, too, in UI?! //!!  use remaining 6 lightsources for objects? (detect nearest lightsources for each!) //!! enable these lightsources via/for VP 'lights' via optional flag -> problem that animated stuff won't react to these changes (yet)

   m_angletiltMax = 726.0f;
   m_angletiltMin = 4.5f;

   m_NormalizeNormals = fFalse;

   m_useReflectionForBalls = -1;
   m_ballReflectionStrength = 50;

   m_useTrailForBalls = -1;
   m_ballTrailStrength = 100;

   m_useAA = -1;
   m_useFXAA = -1;

   m_TableSoundVolume = 1.0f;
   m_TableMusicVolume = 1.0f;

   m_TableAdaptiveVSync = -1;
}

HRESULT PinTable::LoadData(IStream* pstm, int& csubobj, int& csounds, int& ctextures, int& cfonts, int& ccollection, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetLoadDefaults();

   int rgi[6] = {0,0,0,0,0,0};

   BiffReader br(pstm, this, rgi, version, hcrypthash, hcryptkey);

   br.Load();

#ifdef VBA
   ApcProjectItem.Register(ApcProject, GetDispatch(), rgi[0]);
#endif

   csubobj = rgi[1];
   csounds = rgi[2];
   ctextures = rgi[3];
   cfonts = rgi[4];
   ccollection = rgi[5];

   return S_OK;
}

BOOL PinTable::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt(&((int *)pbr->m_pdata)[0]);
   }
   else if (id == FID(GRSZ))
   {
      pbr->GetFloat(&m_gridsize);
   }
   else if (id == FID(LEFT))
   {
      pbr->GetFloat(&m_left);
   }
   else if (id == FID(TOPX))
   {
      pbr->GetFloat(&m_top);
   }
   else if (id == FID(RGHT))
   {
      pbr->GetFloat(&m_right); 
   }
   else if (id == FID(BOTM))
   {
      pbr->GetFloat(&m_bottom);
   }
   else if (id == FID(XLTX))
   {
      pbr->GetFloat(&m_xlatex);
   }
   else if (id == FID(XLTY))
   {
      pbr->GetFloat(&m_xlatey);
   }
   else if (id == FID(XLTZ))
   {
      pbr->GetFloat(&m_xlatez);
   }
#if 0
   else if (id == FID(VERS))
   {
      pbr->GetString(szVersion);
   }
#endif
   else if (id == FID(SCLX))
   {
      pbr->GetFloat(&m_scalex);
   }
   else if (id == FID(SCLY))
   {
      pbr->GetFloat(&m_scaley);
   }
   else if (id == FID(SCLZ))
   {
      pbr->GetFloat(&m_zScale);
   }
   else if( id == FID(ORRP))
   {
      pbr->GetInt(&m_fOverridePhysics);
   }
   else if( id == FID(GAVT))
   {
      pbr->GetFloat(&m_Gravity);
   }
   else if( id == FID(FRCT))
   {
      pbr->GetFloat(&m_hardFriction);
   }
   else if( id == FID(SCAT))
   {
      pbr->GetFloat(&m_hardScatter);
   }
   else if( id == FID(NDGT))
   {
      pbr->GetFloat(&m_nudgeTime);
   }
   else if( id == FID(MPGC))
   {
      pbr->GetInt(&m_plungerNormalize);	
      /*const HRESULT hr =*/ GetRegInt("Player", "PlungerNormalize", &m_plungerNormalize);	
   }
   else if( id == FID(MPDF))
   {
      int tmp;
      pbr->GetBool(&tmp);	
      /*const HRESULT hr =*/ GetRegInt("Player", "PlungerFilter", &tmp);
      m_plungerFilter = (tmp != 0);		
   }
   else if( id == FID(PHML))
   {
      pbr->GetInt(&m_PhysicsMaxLoops);
	  if(m_PhysicsMaxLoops == 0xFFFFFFFF)
        /*const HRESULT hr =*/ GetRegInt("Player", "PhysicsMaxLoops", (int*)&m_PhysicsMaxLoops);
   }
   else if (id == FID(DECL))
   {
      pbr->GetBool(&m_fRenderDecals);
   }
   else if (id == FID(REEL))
   {
      pbr->GetBool(&m_fRenderEMReels);
   }
   else if (id == FID(OFFX))
   {
      pbr->GetFloat(&m_offsetx);
   }
   else if (id == FID(OFFY))
   {
      pbr->GetFloat(&m_offsety);
   }
   else if (id == FID(ZOOM))
   {
      pbr->GetFloat(&m_zoom);
   }
   else if (id == FID(ROTA))
   {
      pbr->GetFloat(&m_rotation);
   }
   else if (id == FID(LAYB))
   {
      pbr->GetFloat(&m_layback);
   }
   else if (id == FID(INCL))
   {
      pbr->GetFloat(&m_inclination);
   }
   else if (id == FID(FOVX))
   {
      pbr->GetFloat(&m_FOV);
   }
   else if (id == FID(MAXSEP))
   {
      pbr->GetFloat(&m_maxSeparation);
   }
   else if (id == FID(ZPD))
   {
      pbr->GetFloat(&m_ZPD);
   }
   else if ( id == FID(OGST))
   {
       pbr->GetBool(&m_overwriteGlobalStereo3D);
   }
   else if (id == FID(SLPX))
   {
      pbr->GetFloat(&m_angletiltMax);
   }
   else if (id == FID(SLOP))
   {
      pbr->GetFloat(&m_angletiltMin);
      if(m_angletiltMax == 726.0f) m_angletiltMax = m_angletiltMin; //!! ??
   }
   else if (id == FID(GLAS))
   {
      pbr->GetFloat(&m_glassheight);
   }
   else if (id == FID(TBLH))
   {
      pbr->GetFloat(&m_tableheight);
   }
   else if (id == FID(IMAG))
   {
      pbr->GetString(m_szImage);
   }
   else if (id == FID(BLIM))
   {
      pbr->GetString(m_szBallImage);
   }
   else if (id == FID(BLIF))
   {
      pbr->GetString(m_szBallImageFront);
   }
   else if (id == FID(BLIB))
   {
      pbr->GetString(m_szBallImageBack);
   }
   else if (id == FID(SSHT))
   {
      pbr->GetString(m_szScreenShot);
   }
   else if (id == FID(FGRD))
   {
      pbr->GetBool(&m_fGrid);
   }
   else if (id == FID(FBCK))
   {
      pbr->GetBool(&m_fBackdrop);
   }
   else if (id == FID(SEDT))
   {
      pbr->GetInt(&((int *)pbr->m_pdata)[1]);
   }
   else if (id == FID(SSND))
   {
      pbr->GetInt(&((int *)pbr->m_pdata)[2]);
   }
   else if (id == FID(SIMG))
   {
      pbr->GetInt(&((int *)pbr->m_pdata)[3]);
   }
   else if (id == FID(SFNT))
   {
      pbr->GetInt(&((int *)pbr->m_pdata)[4]);
   }
   else if (id == FID(SCOL))
   {
      pbr->GetInt(&((int *)pbr->m_pdata)[5]);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(BIMG))
   {
      pbr->GetString(m_szImageBackdrop);
   }
   else if (id == FID(COLR))
   {
      pbr->GetInt(&m_colorplayfield);
   }
   else if (id == FID(LZAM))
   {
      pbr->GetInt(&m_Light[0].ambient);
   }
   else if (id == FID(LZDI))
   {
      pbr->GetInt(&m_Light[0].diffuse);
   }
   else if (id == FID(LZSP))
   {
      pbr->GetInt(&m_Light[0].specular);
   }
   else if (id == FID(LZPX))
   {
      pbr->GetFloat(&m_Light[0].pos.x);
   }
   else if (id == FID(LZPY))
   {
      pbr->GetFloat(&m_Light[0].pos.y);
   }
   else if (id == FID(LZPZ))
   {
      pbr->GetFloat(&m_Light[0].pos.z);
   }
   else if (id == FID(LZDX))
   {
      pbr->GetFloat(&m_Light[0].dir.x);
   }
   else if (id == FID(LZDY))
   {
      pbr->GetFloat(&m_Light[0].dir.y);
   }
   else if (id == FID(LZDZ))
   {
      pbr->GetFloat(&m_Light[0].dir.z);
   }
   else if (id == FID(LZTY))
   {
      pbr->GetInt(&m_Light[0].type);
   }
   else if (id == FID(LOAM))
   {
      pbr->GetInt(&m_Light[1].ambient);
   }
   else if (id == FID(LODI))
   {
      pbr->GetInt(&m_Light[1].diffuse);
   }
   else if (id == FID(LOSP))
   {
      pbr->GetInt(&m_Light[1].specular);
   }
   else if (id == FID(LOPX))
   {
      pbr->GetFloat(&m_Light[1].pos.x);
   }
   else if (id == FID(LOPY))
   {
      pbr->GetFloat(&m_Light[1].pos.y);
   }
   else if (id == FID(LOPZ))
   {
      pbr->GetFloat(&m_Light[1].pos.z);
   }
   else if (id == FID(LODX))
   {
      pbr->GetFloat(&m_Light[1].dir.x);
   }
   else if (id == FID(LODY))
   {
      pbr->GetFloat(&m_Light[1].dir.y);
   }
   else if (id == FID(LODZ))
   {
      pbr->GetFloat(&m_Light[1].dir.z);
   }
   else if (id == FID(LOTY))
   {
      pbr->GetInt(&m_Light[1].type);
   }
   else if (id == FID(NONO))
   {
      pbr->GetInt(&m_NormalizeNormals);
   }
   else if (id == FID(BREF))
   {
      pbr->GetInt(&m_useReflectionForBalls);
   }
   else if (id == FID(BRST))
   {
      pbr->GetInt(&m_ballReflectionStrength);
   }
   else if (id == FID(BTRA))
   {
      pbr->GetInt(&m_useTrailForBalls);
   }
   else if (id == FID(BTST))
   {
      pbr->GetInt(&m_ballTrailStrength);
   }
   else if (id == FID(UAAL))
   {
      pbr->GetInt(&m_useAA);
   }
   else if (id == FID(UFXA))
   {
      pbr->GetInt(&m_useFXAA);
   }
   else if (id == FID(SDIX))
   {
      pbr->GetFloat(&m_shadowDirX);
   }
   else if (id == FID(SDIY))
   {
      pbr->GetFloat(&m_shadowDirY);
   }
   else if (id == FID(BCLR))
   {
      pbr->GetInt(&m_colorbackdrop);
   }
   else if (id == FID(SECB))
   {
      pbr->GetStruct(&m_protectionData, sizeof(_protectionData));
      if (pbr->m_version < 700 && m_protectionData.flags != 0)
      {
         m_protectionData.flags |= DISABLE_DEBUGGER;
      }
   }
   else if (id == FID(CODE))
   {
      // if the script is protected then we pass in the proper cyptokey into the code loadstream
      m_pcv->LoadFromStream(pbr->m_pistream, pbr->m_hcrypthash, CheckPermissions(DISABLE_SCRIPT_EDITING) ? pbr->m_hcryptkey : NULL);
   }
   else if (id == FID(CCUS))
   {
      pbr->GetStruct(m_rgcolorcustom, sizeof(COLORREF)*16);
   }
   else if (id == FID(DSHD))
   {
      pbr->GetBool(&m_fRenderShadows);
   }
   else if (id == FID(ACEL)) //////////////////
   {		
      pbr->GetBool(&m_tblAccelerometer);
      GetRegInt("Player", "PBWEnabled", &m_tblAccelerometer);
      m_tblAccelerometer = m_tblAccelerometer != fFalse;
   }
   else if (id == FID(TDFT))
   {
      pbr->GetFloat(&m_globalDifficulty);
      int tmp;
      HRESULT hr = GetRegInt("Player", "GlobalDifficulty", &tmp);
      if (hr == S_OK) m_globalDifficulty = (float)tmp*(float)(1.0/100.0);		
   }
   else if (id == FID(AORD))
   {
      pbr->GetBool(&m_tblAccelNormalMount);
      GetRegInt("Player", "PBWNormalMount", &m_tblAccelNormalMount);
      m_tblAccelNormalMount = (m_tblAccelNormalMount != fFalse);
   }
   else if (id == FID(AANG))
   {
      pbr->GetFloat(&m_tblAccelAngle);
      int tmp;
      HRESULT hr = GetRegInt("Player", "PBWRotation", &tmp);
      if (hr == S_OK) m_tblAccelAngle = (float)tmp;		
   }
   else if (id == FID(AAMP))
   {
      pbr->GetFloat(&m_tblAccelAmp);
      int tmp;
      HRESULT hr = GetRegInt("Player", "PBWAccelGain", &tmp);
      if (hr == S_OK) m_tblAccelAmp = (float)tmp*(float)(1.0/100.0);		
   }
   else if (id == FID(AAMX))
   {
      pbr->GetFloat(&m_tblAccelAmpX);
      int tmp;
      HRESULT hr = GetRegInt("Player", "PBWAccelGainX", &tmp);
      if (hr == S_OK) m_tblAccelAmpX = (float)tmp*(float)(1.0/100.0);		
   }
   else if (id == FID(AAMY))
   {
      pbr->GetFloat(&m_tblAccelAmpY);
      int tmp;
      HRESULT hr = GetRegInt("Player", "PBWAccelGainY", &tmp);
      if (hr == S_OK) m_tblAccelAmpY = (float)tmp*(float)(1.0/100.0);		
   }
   else if (id == FID(AMAM))
   {
      pbr->GetFloat(&m_tblAccelManualAmp);
      int tmp;
      HRESULT hr = GetRegInt("Player", "JoystickGain", &tmp);
      if (hr == S_OK) m_tblAccelManualAmp = (float)tmp*(float)(1.0/100.0);		
   }	
   else if (id == FID(JLTA))
   {
      pbr->GetInt(&m_jolt_amount);
      GetRegInt("Player", "JoltAmount", &m_jolt_amount);
   }
   else if (id == FID(TLTA))
   {	
      pbr->GetInt(&m_tilt_amount);
      GetRegInt("Player", "TiltAmount", &m_tilt_amount);
   }	
   else if (id == FID(JLTT))
   {
      pbr->GetInt(&m_jolt_trigger_time);
      GetRegInt("Player", "JoltTriggerTime", &m_jolt_trigger_time);		
   }
   else if (id == FID(TLTT)) 
   {
      pbr->GetInt(&m_tilt_trigger_time);
      GetRegInt("Player", "TiltTriggerTime", &m_tilt_trigger_time);
   }
   else if (id == FID(CUST)) 
   {
      char szT[1024];  //maximum length of tagnames right now
      pbr->GetString(szT);
      char *szName;
      szName = new char[lstrlen(szT)+1];
      lstrcpy(szName,szT);
      m_vCustomInfoTag.AddElement(szName);
   }
   else if (id == FID(SVOL))
   {
      pbr->GetFloat(&m_TableSoundVolume);
   }
   else if (id == FID(MVOL))
   {
      pbr->GetFloat(&m_TableMusicVolume);
   }
   else if (id == FID(AVSY))
   {
      pbr->GetInt(&m_TableAdaptiveVSync);
   }
   else if ( id == FID(OGAC))
   {
       pbr->GetBool(&m_overwriteGlobalAlphaRampsAccuracy);
   }
   else if ( id == FID(ARAC))
   {      
      pbr->GetInt(&m_userAlphaRampsAccuracy);
   }
   return fTrue;
}

bool PinTable::ExportSound(HWND hwndListView, PinSound *pps,char *szfilename)
{
   MMIOINFO mmio;
   MMCKINFO pck;
   ZeroMemory( &mmio, sizeof(mmio));	
   ZeroMemory( &pck, sizeof(pck));
   WAVEFORMATEX wfx;

   HMMIO hmmio = mmioOpen( szfilename, &mmio,  MMIO_ALLOCBUF |MMIO_CREATE |MMIO_EXCLUSIVE |MMIO_READWRITE);

   if (hmmio != NULL)
   {
      // quick and dirty ... in a big hurry

      pck.ckid = mmioStringToFOURCC("RIFF", MMIO_TOUPPER);
      pck.cksize = pps->m_cdata+36;
      pck.fccType = mmioStringToFOURCC("WAVE", MMIO_TOUPPER);

      MMRESULT result = mmioCreateChunk(hmmio, &pck, MMIO_CREATERIFF); //RIFF header
      mmioWrite(hmmio, "fmt ", 4);			//fmt

      pps->m_pDSBuffer->GetFormat(&wfx, sizeof(wfx), NULL); //CORRECTED for support of all savable VP WAV FORMATS - BDS
      // Create the format chunk.
      pck.cksize = sizeof(WAVEFORMATEX);
      result = mmioCreateChunk(hmmio, &pck, 4);//0
      // Write the wave format data.
      int i=16;
      mmioWrite(hmmio, (char *)&i, 4);
      mmioWrite(hmmio, (char*)&wfx, sizeof(wfx)-2); //END OF CORRECTION

      mmioWrite(hmmio, "data", 4);						//data chunk
      i = pps->m_cdata; mmioWrite(hmmio, (char *)&i, 4);	// data size bytes

      const LONG wcch = mmioWrite(hmmio, pps->m_pdata , pps->m_cdata);
      result = mmioClose(hmmio, 0);

      if (wcch != pps->m_cdata) MessageBox(m_hwnd, "Sound file incomplete!", "Visual Pinball", MB_ICONERROR);
      else return true;

   }
   else MessageBox(m_hwnd, "Can not Open/Create Sound file!", "Visual Pinball", MB_ICONERROR);

   return false;
}

void PinTable::ReImportSound(HWND hwndListView, PinSound *pps, char *filename, BOOL fPlay)
{
   PinSound * const ppsNew = g_pvp->m_pds.LoadWaveFile(filename);

   if (ppsNew == NULL)
   {
      return;
   }

   PinSound psT = *pps;
   *pps = *ppsNew;
   *ppsNew = psT;

   lstrcpy(pps->m_szName, ppsNew->m_szName);
   lstrcpy(pps->m_szInternalName, ppsNew->m_szInternalName);

   // make sure sound data doesn't get deleted twice
   psT.m_pdata = NULL;
   psT.m_pDSBuffer = NULL;
   delete ppsNew;

   if (fPlay)
   {
      pps->m_pDSBuffer->Play( 0, 0, 0 );
   }
}


void PinTable::ImportSound(HWND hwndListView, char *szfilename, BOOL fPlay)
{
   PinSound * const pps = g_pvp->m_pds.LoadWaveFile(szfilename);

   if (pps == NULL)
   {
      return;
   }

   if (fPlay)
   {
      pps->m_pDSBuffer->Play( 0, 0, 0 );
   }

   m_vsound.AddElement(pps);

   const int index = AddListSound(hwndListView, pps);

   ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);
}

void PinTable::ListSounds(HWND hwndListView)
{
   for (int i=0;i<m_vsound.Size();i++)
   {
      AddListSound(hwndListView, m_vsound.ElementAt(i));
   }
}


int PinTable::AddListSound(HWND hwndListView, PinSound *pps)
{
   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = pps->m_szName;
   lvitem.lParam = (long)pps;

   const int index = ListView_InsertItem(hwndListView, &lvitem);

   ListView_SetItemText(hwndListView, index, 1, pps->m_szPath);

   return index;
}

void PinTable::RemoveSound(PinSound *pps)
{
   m_vsound.RemoveElement(pps);
   delete pps;
}

void PinTable::ImportFont(HWND hwndListView, char *filename)
{
   PinFont * const ppb = new PinFont();

   ppb->ReadFromFile(filename);

   if (ppb->m_pdata != NULL)
   {
      m_vfont.AddElement(ppb);

      const int index = AddListBinary(hwndListView, ppb);

      ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);

      ppb->Register();
   }
}


void PinTable::RemoveFont(PinFont *ppf)
{
   m_vfont.RemoveElement(ppf);
   ppf->UnRegister();
   delete ppf;
}


void PinTable::ListFonts(HWND hwndListView)
{
   for (int i=0;i<m_vfont.Size();i++)
   {
      AddListBinary(hwndListView, m_vfont.ElementAt(i));
   }
}

int PinTable::AddListBinary(HWND hwndListView, PinBinary *ppb)
{
   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = ppb->m_szName;
   lvitem.lParam = (long)ppb;

   const int index = ListView_InsertItem(hwndListView, &lvitem);

   ListView_SetItemText(hwndListView, index, 1, ppb->m_szPath);

   return index;
}

void PinTable::NewCollection(HWND hwndListView, BOOL fFromSelection)
{
   WCHAR wzT[128];

   CComObject<Collection> *pcol;
   CComObject<Collection>::CreateInstance(&pcol);
   pcol->AddRef();

   GetUniqueName(eItemCollection, wzT);

   WideStrCopy(wzT, pcol->m_wzName);

   if (fFromSelection && (m_vmultisel.ElementAt(0) != this))
   {
      for (int i=0;i<m_vmultisel.Size();i++)
      {
         ISelect *pisel = m_vmultisel.ElementAt(i);
         IEditable *piedit = pisel->GetIEditable();
         if ( piedit )
         {
            if (piedit->GetISelect() == pisel) // Do this check so we don't put walls in a collection when we only have the control point selected
            {
               if (piedit->GetScriptable()) // check for scriptable because can't add decals to a collection - they have no name
               {
                  piedit->m_vCollection.AddElement(pcol);
                  piedit->m_viCollection.AddElement((void *)pcol->m_visel.Size());
                  pcol->m_visel.AddElement(m_vmultisel.ElementAt(i));
               }
            }
         }
      }
   }

   const int index = AddListCollection(hwndListView, pcol);

   ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);

   m_vcollection.AddElement(pcol);
   m_pcv->AddItem((IScriptable *)pcol, fFalse);
}

int PinTable::AddListCollection(HWND hwndListView, CComObject<Collection> *pcol)
{
   char szT[MAX_PATH];

   WideCharToMultiByte(CP_ACP, 0, pcol->m_wzName, -1, szT, MAX_PATH, NULL, NULL);

   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = szT;
   lvitem.lParam = (long)pcol;

   const int index = ListView_InsertItem(hwndListView, &lvitem);

   return index;
}

void PinTable::ListCollections(HWND hwndListView)
{
   //ListView_DeleteAllItems(hwndListView);

   for (int i=0;i<m_vcollection.Size();i++)
   {
      CComObject<Collection> * const pcol = m_vcollection.ElementAt(i);

      AddListCollection(hwndListView, pcol);
   }
}

void PinTable::RemoveCollection(CComObject<Collection> *pcol)
{
   m_pcv->RemoveItem((IScriptable *)pcol);
   m_vcollection.RemoveElement(pcol);
   pcol->Release();
}

void PinTable::MoveCollectionUp(CComObject<Collection> *pcol )
{
    int idx = m_vcollection.IndexOf(pcol);
    m_vcollection.RemoveElementAt(idx);
    if ( idx-1<0 )
        m_vcollection.AddElement(pcol);
    else
        m_vcollection.InsertElementAt( pcol, idx-1 );
}

int PinTable::GetAlphaRampsAccuracy()
{
    if( m_overwriteGlobalAlphaRampsAccuracy )
        return m_userAlphaRampsAccuracy;
    else
        return m_globalAlphaRampsAccuracy;
}

float PinTable::GetZPD()
{
    if( m_overwriteGlobalStereo3D )
        return m_ZPD;
    else
        return m_globalZPD;
}

float PinTable::GetMaxSeparation()
{
    if( m_overwriteGlobalStereo3D )
        return m_maxSeparation;
    else
        return m_globalMaxSeparation;
}

FRect3D PinTable::GetBoundingBox()
{
    FRect3D bbox;
    bbox.left = m_left;
    bbox.right = m_right;
    bbox.top = m_top;
    bbox.bottom = m_bottom;
    bbox.zlow = m_tableheight;
    bbox.zhigh = m_glassheight;
    return bbox;
}

void PinTable::MoveCollectionDown(CComObject<Collection> *pcol )
{
    int idx = m_vcollection.IndexOf(pcol);
    m_vcollection.RemoveElementAt(idx);
    if( idx+1>=m_vcollection.Size() )
        m_vcollection.InsertElementAt( pcol, 0 );    
    else
        m_vcollection.InsertElementAt( pcol, idx+1 );
}

void PinTable::SetCollectionName(Collection *pcol, char *szName, HWND hwndList, int index)
{
   WCHAR wzT[1024];
   MultiByteToWideChar(CP_ACP, 0, szName, -1, wzT, 1024);
   if (m_pcv->ReplaceName((IScriptable *)pcol, wzT) == S_OK)
   {
      if (hwndList)
      {
         ListView_SetItemText(hwndList, index, 0, szName);
      }
      WideStrCopy(wzT, pcol->m_wzName);
   }
}

STDMETHODIMP PinTable::get_GridSize(float *pgs)
{
   *pgs = m_gridsize;
   return S_OK;
}

STDMETHODIMP PinTable::put_GridSize(float gs)
{
   STARTUNDO

   if (gs < 1)
     gs = 1;

   m_gridsize = gs;

   SetDirtyDraw();

   STOPUNDO

   return S_OK;
}

void PinTable::SetZoom(float zoom)
{
   m_zoom = zoom;
   SetMyScrollInfo();
}

void PinTable::GetViewRect(FRect *pfrect)
{
   if (!g_pvp->m_fBackglassView)
   {
      pfrect->left = m_left;
      pfrect->top = m_top;
      pfrect->right = m_right;
      pfrect->bottom = m_bottom;
   }
   else
   {
      pfrect->left = 0;
      pfrect->top = 0;
      pfrect->right = EDITOR_BG_WIDTH;
      pfrect->bottom = EDITOR_BG_HEIGHT;
   }
}

void PinTable::SetMyScrollInfo()
{
   FRect frect;
   GetViewRect(&frect);

   RECT rc;
   GetClientRect(m_hwnd, &rc);

   HitSur * const phs = new HitSur(NULL, m_zoom, m_offsetx, m_offsety, rc.right - rc.left, rc.bottom - rc.top, 0, 0, NULL);

   Vertex2D rgv[2];
   rgv[0] = phs->ScreenToSurface(rc.left, rc.top);
   rgv[1] = phs->ScreenToSurface(rc.right, rc.bottom);

   delete phs;

   SCROLLINFO si;
   ZeroMemory(&si,sizeof(SCROLLINFO));
   si.cbSize = sizeof(SCROLLINFO);
   si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
   si.nMin = (int)min(frect.left, rgv[0].x);
   si.nMax = (int)max(frect.right, rgv[1].x);
   si.nPage = (int)(rgv[1].x - rgv[0].x);
   si.nPos = (int)(rgv[0].x);

   SetScrollInfo(m_hwnd, SB_HORZ, &si, fTrue);

   si.nMin = (int)min(frect.top, rgv[0].y);
   si.nMax = (int)max(frect.bottom, rgv[1].y);
   si.nPage = (int)(rgv[1].y - rgv[0].y);
   si.nPos = (int)(rgv[0].y);

   SetScrollInfo(m_hwnd, SB_VERT, &si, fTrue);
}

void PinTable::FireKeyEvent(int dispid, int keycode)
{
   CComVariant rgvar[1] = {CComVariant(keycode)};

   DISPPARAMS dispparams  = {
      rgvar,
      NULL,
      1,
      0
   };

#ifdef LOG
   if (g_pplayer)
   {
      if (dispid == DISPID_GameEvents_KeyDown)
      {
         fprintf(g_pplayer->m_flog, "Key Down %d\n", keycode);
      }
      else
      {
         fprintf(g_pplayer->m_flog, "Key Up %d\n", keycode);
      }
   }
#endif

   FireDispID(dispid, &dispparams);
}

void PinTable::DoLButtonDown(int x,int y)
{
   const int ksshift = GetKeyState(VK_SHIFT);
   const int ksctrl = GetKeyState(VK_CONTROL);

   // set the focus of the window so all keyboard and mouse inputs are processed.
   // (this fixes the problem of selecting a element on the properties dialog, clicking on a table
   // object and not being able to use the cursor keys/wheely mouse
   SetFocus(g_pvp->m_hwndWork);

   if ((g_pvp->m_ToolCur == ID_TABLE_MAGNIFY) || (ksctrl & 0x80000000))
   {
      if (m_zoom < MAX_ZOOM)
      {
         Vertex2D v;
         TransformPoint(x,y,&v);
         m_offsetx = v.x;
         m_offsety = v.y;
         SetZoom(m_zoom * 2.0f);
         SetDirtyDraw();
      }
   }

   // if disabling table view then don't allow the table to be selected (thus bringing up table properties)
   else if (!CheckPermissions(DISABLE_TABLEVIEW))
      // Normal click
   {
      ISelect * const pisel = HitTest(x,y);

      const BOOL fAdd = ((ksshift & 0x80000000) != 0);

      if (pisel == (ISelect *)this && fAdd)
      {
         // Can not include the table in multi-select
         // and table will not be unselected, because the
         // user might be drawing a box around other objects
         // to add them to the selection group
         OnLButtonDown(x,y); // Start the band select
         return;
      }

      AddMultiSel(pisel, fAdd, fTrue);

      for (int i=0;i<m_vmultisel.Size();i++)
      {
         m_vmultisel.ElementAt(i)->OnLButtonDown(x,y);
      }
   }
}

void PinTable::DoLButtonUp(int x,int y)
{
   //m_pselcur->OnLButtonUp(x,y);

   if (!m_fDragging) // Not doing band select
   {
      for (int i=0;i<m_vmultisel.Size();i++)
      {
         m_vmultisel.ElementAt(i)->OnLButtonUp(x,y);
      }
   }
   else
   {
      OnLButtonUp(x,y);
   }
}

void PinTable::DoRButtonDown(int x,int y)
{
   DoLButtonUp(x,y); //corrects issue with left mouse button being in 'stuck down' position on a control point or object - BDS

   const int ks = GetKeyState(VK_CONTROL);

   if ((g_pvp->m_ToolCur == ID_TABLE_MAGNIFY) || (ks & 0x80000000))
   {
      if (m_zoom > MIN_ZOOM)
      {
         Vertex2D v;
         TransformPoint(x,y,&v);
         m_offsetx = v.x;
         m_offsety = v.y;
         SetZoom(m_zoom * 0.5f);
         SetDirtyDraw();
      }
   }
   else
   {
      //SetSel(HitTest(x,y));
      AddMultiSel(HitTest(x,y), fFalse, fTrue);
   }
}

void PinTable::DoContextMenu(int x, int y, int menuid, ISelect *psel)
{
   POINT pt;
   pt.x = x;
   pt.y = y;
   ClientToScreen(m_hwnd, &pt);

   HMENU hmenumain;
   HMENU hmenu;
   HMENU subMenu;
   HMENU colSubMenu;
   if (menuid != -1)
   {
      hmenumain = LoadMenu(g_hinst, MAKEINTRESOURCE(menuid));

      hmenu = GetSubMenu(hmenumain, 0);
   }
   else
   {
      hmenu = CreatePopupMenu();
      subMenu = CreatePopupMenu();
      colSubMenu = CreatePopupMenu();
   }

   psel->EditMenu(hmenu);

   if (menuid != IDR_POINTMENU && menuid != IDR_TABLEMENU) //psel->GetIEditable() != NULL)
   {
      if (GetMenuItemCount(hmenu) > 0)
      {
         AppendMenu(hmenu, MF_SEPARATOR, ~0u, "");
         subMenu = CreatePopupMenu();
         colSubMenu = CreatePopupMenu();
      }
      // TEXT
      LocalString ls14(IDS_DRAWING_ORDER_HIT);
      AppendMenu(hmenu, MF_STRING, ID_EDIT_DRAWINGORDER_HIT, ls14.m_szbuffer);
      LocalString ls15(IDS_DRAWING_ORDER_SELECT);
      AppendMenu(hmenu, MF_STRING, ID_EDIT_DRAWINGORDER_SELECT, ls15.m_szbuffer);

      LocalString ls1(IDS_DRAWINFRONT);
      LocalString ls2(IDS_DRAWINBACK);
      AppendMenu(hmenu, MF_STRING, ID_DRAWINFRONT, ls1.m_szbuffer);
      AppendMenu(hmenu, MF_STRING, ID_DRAWINBACK, ls2.m_szbuffer);

      LocalString ls3(IDS_SETASDEFAULT);
      AppendMenu(hmenu, MF_STRING, ID_SETASDEFAULT, ls3.m_szbuffer);

      LocalString ls4(IDS_ASSIGNTO);
      AppendMenu(hmenu, MF_POPUP|MF_STRING, (UINT)subMenu, ls4.m_szbuffer);
      LocalString ls6(IDS_LAYER1);
      AppendMenu(subMenu, MF_POPUP, ID_ASSIGNTO_LAYER1, ls6.m_szbuffer);
      LocalString ls7(IDS_LAYER2);
      AppendMenu(subMenu, MF_POPUP, ID_ASSIGNTO_LAYER2, ls7.m_szbuffer);
      LocalString ls8(IDS_LAYER3);
      AppendMenu(subMenu, MF_POPUP, ID_ASSIGNTO_LAYER3, ls8.m_szbuffer);
      LocalString ls9(IDS_LAYER4);
      AppendMenu(subMenu, MF_POPUP, ID_ASSIGNTO_LAYER4, ls9.m_szbuffer);
      LocalString ls10(IDS_LAYER5);
      AppendMenu(subMenu, MF_POPUP, ID_ASSIGNTO_LAYER5, ls10.m_szbuffer);
      LocalString ls11(IDS_LAYER6);
      AppendMenu(subMenu, MF_POPUP, ID_ASSIGNTO_LAYER6, ls11.m_szbuffer);
      LocalString ls12(IDS_LAYER7);
      AppendMenu(subMenu, MF_POPUP, ID_ASSIGNTO_LAYER7, ls12.m_szbuffer);
      LocalString ls13(IDS_LAYER8);
      AppendMenu(subMenu, MF_POPUP, ID_ASSIGNTO_LAYER8, ls13.m_szbuffer);

      if( psel->layerIndex==0 ) 
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER1, MF_CHECKED);
      else
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER1, MF_UNCHECKED);  
      if( psel->layerIndex==1 ) 
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER2, MF_CHECKED);
      else
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER2, MF_UNCHECKED);  
      if( psel->layerIndex==2 ) 
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER3, MF_CHECKED);
      else
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER3, MF_UNCHECKED);  
      if( psel->layerIndex==3 ) 
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER4, MF_CHECKED);
      else
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER4, MF_UNCHECKED);  
      if( psel->layerIndex==4 ) 
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER5, MF_CHECKED);
      else
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER5, MF_UNCHECKED);  
      if( psel->layerIndex==5 ) 
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER6, MF_CHECKED);
      else
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER6, MF_UNCHECKED);  
      if( psel->layerIndex==6 ) 
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER7, MF_CHECKED);
      else
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER7, MF_UNCHECKED);  
      if( psel->layerIndex==7 ) 
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER8, MF_CHECKED);
      else
         CheckMenuItem(subMenu, ID_ASSIGNTO_LAYER8, MF_UNCHECKED);  

      LocalString ls16(IDS_TO_COLLECTION);
      AppendMenu(hmenu, MF_POPUP|MF_STRING, (UINT)colSubMenu, ls16.m_szbuffer);

      int maxItems = m_vcollection.Size()-1;
      if ( maxItems>32 ) maxItems=32;

      for (int i=maxItems; i>=0;i--)
      {
          CComBSTR bstr;
          m_vcollection.ElementAt(i)->get_Name(&bstr);
          char szT[64]; // Names can only be 32 characters (plus terminator)
          WideCharToMultiByte(CP_ACP, 0, bstr, -1, szT, 64, NULL, NULL);

          AppendMenu(colSubMenu, MF_POPUP, 0x40000+i, szT);
          CheckMenuItem(colSubMenu, 0x40000+i, MF_UNCHECKED );
      }
      for (int i=maxItems; i>=0;i--)
      {
          for( int t=0;t<m_vcollection.ElementAt(i)->m_visel.Size();t++ )
          {
              if( psel==m_vcollection.ElementAt(i)->m_visel.ElementAt(t) )
              {
                  CheckMenuItem(colSubMenu, 0x40000+i, MF_CHECKED );
              }
          }
      }
      LocalString ls5(IDS_LOCK);
      AppendMenu(hmenu, MF_STRING, ID_LOCK, ls5.m_szbuffer);

      AppendMenu(hmenu, MF_SEPARATOR, ~0u, "");
      AppendMenu(hmenu, MF_SEPARATOR, ~0u, "");
      for( int i=0; i<m_allHitElements.Size(); i++ ) 
      {
         if( !m_allHitElements.ElementAt(i)->GetIEditable()->isVisible )
         {
            continue;
         }

         ISelect *ptr = m_allHitElements.ElementAt(i);
         if ( ptr )
         {
            IEditable *pedit = m_allHitElements.ElementAt(i)->GetIEditable();
            if ( pedit )
            {
               char *szTemp;
               szTemp = GetElementName(pedit);

               if( szTemp )
               {
                  // what a hack!
                  // the element index of the allHitElements vector is encoded inside the ID of the context menu item
                  // I didn't find an easy way to identify the selected menu item of a context menu
                  // so the ID_SELECT_ELEMENT is the global ID for selecting an element from the list and the rest is
                  // added for finding the element out of the list
                  // the selection is done in ISelect::DoCommand()
                  unsigned long id = 0x80000000 + (i<<16) + ID_SELECT_ELEMENT;
                  AppendMenu(hmenu, MF_STRING, id, szTemp);
               }
            }
         }
      }
      BOOL fLocked = psel->m_fLocked;
      // HACK
      if (psel == this) // multi-select case
      {
         fLocked = FMutilSelLocked();
      }

      CheckMenuItem(hmenu, ID_LOCK, MF_BYCOMMAND | (fLocked ? MF_CHECKED : MF_UNCHECKED));
   }

   const int icmd = TrackPopupMenuEx(hmenu, TPM_RETURNCMD, pt.x, pt.y, m_hwnd, NULL);

   if (icmd != 0)
   {
         psel->DoCommand(icmd, x, y);
   }

   DestroyMenu(hmenu);

   if (menuid != -1)
   {
      DestroyMenu(hmenumain);
   }
}

char elementName[256];
char *PinTable::GetElementName( IEditable *pedit )
{
   WCHAR *elemName=NULL;
   IScriptable *pscript =NULL;
   if ( pedit )
   {
      pscript = pedit->GetScriptable();
      if ( pedit->GetItemType()==eItemDecal )
      {
          return "Decal";
      }
      if ( pscript )
      {
         elemName = pscript->m_wzName;
      }
   }
   if ( elemName )
   {
      WideCharToMultiByte(CP_ACP, 0, elemName, -1, elementName, 256, NULL, NULL);
      return elementName;
   }
   return NULL;
}

BOOL PinTable::FMutilSelLocked()
{
   BOOL fLocked = fFalse;

   for (int i=0;i<m_vmultisel.Size();i++)
   {
      if (m_vmultisel.ElementAt(i)->m_fLocked)
      {
         fLocked = fTrue;
         break;
      }
   }

   return fLocked;
}

void PinTable::DoCommand(int icmd, int x, int y)
{
   if ( ((icmd & 0x000FFFFF) >= 0x40000 ) && ((icmd & 0x000FFFFF)<0x40020) ) 
   {
       AddToCollection( icmd & 0x000000FF );
       return;
   }
   switch (icmd)
   {
   case ID_DRAWINFRONT:
   case ID_DRAWINBACK:
      {
         for (int i=0;i<m_vmultisel.Size();i++)
         {
            ISelect *psel;
            psel = m_vmultisel.ElementAt(i);
            _ASSERTE(psel != this); // Would make an infinite loop
            psel->DoCommand(icmd, x, y);
         }
      }
      break;

   case ID_EDIT_DRAWINGORDER_HIT:
      g_pvp->ShowDrawingOrderDialog(false);
      break;

   case ID_EDIT_DRAWINGORDER_SELECT:
      g_pvp->ShowDrawingOrderDialog(true);
      break;

   case ID_LOCK:
      {
         BeginUndo();
         BOOL fLock = FMutilSelLocked() ? fFalse : fTrue;
         for (int i=0;i<m_vmultisel.Size();i++)
         {
            ISelect *psel;
            psel = m_vmultisel.ElementAt(i);
            psel->GetIEditable()->MarkForUndo();
            psel->m_fLocked = fLock;
         }
         EndUndo();
         SetDirtyDraw();
      }
      break;

   case ID_WALLMENU_FLIP:
      {
         Vertex2D vCenter;

         GetCenter(&vCenter);

         FlipY(&vCenter);
      }
      break;

   case ID_WALLMENU_MIRROR:
      {
         Vertex2D vCenter;

         GetCenter(&vCenter);

         FlipX(&vCenter);
      }
      break;

   case IDC_PASTEAT:
      {
         Paste(fTrue, x, y);
         break;
      }

   case ID_WALLMENU_ROTATE:
      {
#ifdef VBA
         g_pvp->ApcHost->BeginModalDialog();
#endif
         DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_ROTATE),
            g_pvp->m_hwnd, RotateProc, (long)(ISelect *)this);
#ifdef VBA
         g_pvp->ApcHost->EndModalDialog();
#endif
      }
      break;

   case ID_WALLMENU_SCALE:
      {
#ifdef VBA
         g_pvp->ApcHost->BeginModalDialog();
#endif
         DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_SCALE),
            g_pvp->m_hwnd, ScaleProc, (long)(ISelect *)this);
#ifdef VBA
         g_pvp->ApcHost->EndModalDialog();
#endif
      }
      break;

   case ID_WALLMENU_TRANSLATE:
      {
#ifdef VBA
         g_pvp->ApcHost->BeginModalDialog();
#endif
         DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_TRANSLATE),
            g_pvp->m_hwnd, TranslateProc, (long)(ISelect *)this);
#ifdef VBA
         g_pvp->ApcHost->EndModalDialog();
#endif
      }
      break;
   case ID_ASSIGNTO_LAYER1:
      {
         AssignMultiToLayer(0,x,y);
         break;
      }
   case ID_ASSIGNTO_LAYER2:
      {
         AssignMultiToLayer(1,x,y);
         break;
      }
   case ID_ASSIGNTO_LAYER3:
      {
         AssignMultiToLayer(2,x,y);
         break;
      }
   case ID_ASSIGNTO_LAYER4:
      {
         AssignMultiToLayer(3,x,y);
         break;
      }
   case ID_ASSIGNTO_LAYER5:
      {
         AssignMultiToLayer(4,x,y);
         break;
      }
   case ID_ASSIGNTO_LAYER6:
      {
         AssignMultiToLayer(5,x,y);
         break;
      }
   case ID_ASSIGNTO_LAYER7:
      {
         AssignMultiToLayer(6,x,y);
         break;
      }
   case ID_ASSIGNTO_LAYER8:
      {
         AssignMultiToLayer(7,x,y);
         break;
      }
   }
}
void PinTable::AssignMultiToLayer( int layerNumber, int x, int y )
{
   for( int i=0;i<m_vmultisel.Size();i++ )
   {
      ISelect *psel;
      psel =m_vmultisel.ElementAt(i);
      _ASSERTE(psel != this); // Would make an infinite loop
      switch(layerNumber)
      {
      case 0:
         {
            psel->DoCommand(ID_ASSIGNTO_LAYER1, x, y);
            break;
         }
      case 1:
         {
            psel->DoCommand(ID_ASSIGNTO_LAYER2, x, y);
            break;
         }
      case 2:
         {
            psel->DoCommand(ID_ASSIGNTO_LAYER3, x, y);
            break;
         }
      case 3:
         {
            psel->DoCommand(ID_ASSIGNTO_LAYER4, x, y);
            break;
         }
      case 4:
         {
            psel->DoCommand(ID_ASSIGNTO_LAYER5, x, y);
            break;
         }
      case 5:
         {
            psel->DoCommand(ID_ASSIGNTO_LAYER6, x, y);
            break;
         }
      case 6:
         {
            psel->DoCommand(ID_ASSIGNTO_LAYER7, x, y);
            break;
         }
      case 7:
         {
            psel->DoCommand(ID_ASSIGNTO_LAYER8, x, y);
            break;
         }
      }
   }
}

void PinTable::AddToCollection(int index)
{
    if( index<m_vcollection.Size() && index<32 )
    {
        if ( m_vmultisel.Size()>0 )
        {
            for( int t=0;t<m_vmultisel.Size();t++ )
            {
                ISelect *ptr = m_vmultisel.ElementAt(t);
                for( int k=0;k<m_vcollection.ElementAt(index)->m_visel.Size();k++ )
                {
                    if ( ptr==m_vcollection.ElementAt(index)->m_visel.ElementAt(k))
                        //already assigned
                        return;
                }
                m_vcollection.ElementAt(index)->m_visel.AddElement(m_vmultisel.ElementAt(t));
            }
        }
    }
}

void PinTable::FlipY(Vertex2D * const pvCenter)
{
   BeginUndo();

   for (int i=0;i<m_vmultisel.Size();i++)
   {
      m_vmultisel.ElementAt(i)->FlipY(pvCenter);
   }

   EndUndo();
}

void PinTable::FlipX(Vertex2D * const pvCenter)
{
   BeginUndo();

   for (int i=0;i<m_vmultisel.Size();i++)
   {
      m_vmultisel.ElementAt(i)->FlipX(pvCenter);
   }

   EndUndo();
}

void PinTable::Rotate(float ang, Vertex2D *pvCenter)
{
   BeginUndo();

   for (int i=0;i<m_vmultisel.Size();i++)
   {
      m_vmultisel.ElementAt(i)->Rotate(ang, pvCenter);
   }

   EndUndo();
}

void PinTable::Scale(float scalex, float scaley, Vertex2D *pvCenter)
{
   BeginUndo();

   for (int i=0;i<m_vmultisel.Size();i++)
   {
      m_vmultisel.ElementAt(i)->Scale(scalex, scaley, pvCenter);
   }

   EndUndo();
}

void PinTable::Translate(Vertex2D *pvOffset)
{
   BeginUndo();

   for (int i=0;i<m_vmultisel.Size();i++)
   {
      m_vmultisel.ElementAt(i)->Translate(pvOffset);
   }

   EndUndo();
}

void PinTable::GetCenter(Vertex2D * const pv) const
{
   float minx = FLT_MAX;
   float maxx = -FLT_MAX;
   float miny = FLT_MAX;
   float maxy = -FLT_MAX;

   for (int i=0;i<m_vmultisel.Size();i++)
   {
      ISelect * const psel = m_vmultisel.ElementAt(i);
      Vertex2D vCenter;
      psel->GetCenter(&vCenter);

      minx = min(minx, vCenter.x);
      maxx = max(maxx, vCenter.x);
      miny = min(miny, vCenter.y);
      maxy = max(maxy, vCenter.y);
      //tx += m_vdpoint.ElementAt(i)->m_v.x;
      //ty += m_vdpoint.ElementAt(i)->m_v.y;
   }

   pv->x = (maxx+minx)*0.5f;
   pv->y = (maxy+miny)*0.5f;
}

void PinTable::PutCenter(const Vertex2D * const pv)
{
}

void PinTable::DoRButtonUp(int x,int y)
{
   m_vmultisel.ElementAt(0)->OnRButtonUp(x,y);

   const int ks = GetKeyState(VK_CONTROL);

   // Only bring up context menu if we weren't in magnify mode
   if (!((g_pvp->m_ToolCur == ID_TABLE_MAGNIFY) || (ks & 0x80000000)))
   {
      if (m_vmultisel.Size() > 1)
      {
         DoContextMenu(x, y, IDR_MULTIMENU, this);
      }
      else
      {
         if (m_vmultisel.ElementAt(0) != this)
         {
            // No right click menu for main table object
            DoContextMenu(x, y, m_vmultisel.ElementAt(0)->m_menuid, m_vmultisel.ElementAt(0));
            //m_vmultisel.ElementAt(0)->OnRButtonDown(x,y,m_hwnd);
         }
         else
         {
            DoContextMenu(x, y, IDR_TABLEMENU, m_vmultisel.ElementAt(0));
         }
      }
   }
}

void PinTable::DoMouseMove(int x,int y)
{
   Vertex2D v;
   TransformPoint(x,y,&v);

   g_pvp->SetPosCur(v.x, v.y);

   if (!m_fDragging) // Not doing band select
   {
      for (int i=0;i<m_vmultisel.Size();i++)
      {
         m_vmultisel.ElementAt(i)->OnMouseMove(x,y);
      }
   }
   else
   {
      OnMouseMove(x,y);
   }
}

void PinTable::DoLDoubleClick(int x, int y)
{
#ifdef VBA
   IApcProjectItem *papi;
   papi = m_pselcur->GetIApcProjectItem();
   if (papi)
   {
      papi->ViewEventHandler(NULL);
   }
   else // No code, but if its a control then it will have properties
   {
      IApcControl *pac;
      pac = m_pselcur->GetIApcControl();
      if (pac)
      {
         IApcPropertiesWindow *papw;
         g_pvp->ApcHost->get_PropertiesWindow(&papw);
         papw->put_Visible(VARIANT_TRUE);
      }
   }
#else
   //g_pvp->m_sb.SetVisible(fTrue);
   //SendMessage(g_pvp->m_hwnd, WM_SIZE, 0, 0);
#endif
}

void PinTable::ExportBlueprint()
{
   BOOL fSaveAs = fTrue;

   if(fSaveAs)
   {
      //need to get a file name
      OPENFILENAME ofn;
      ZeroMemory(&ofn, sizeof(OPENFILENAME));
      ofn.lStructSize = sizeof(OPENFILENAME);
      ofn.hInstance = g_hinst;
      ofn.hwndOwner = g_pvp->m_hwnd;
      // TEXT
      ofn.lpstrFilter = "Bitmap (*.bmp)\0*.bmp\0";
      ofn.lpstrFile = m_szBlueprintFileName;
      ofn.nMaxFile = _MAX_PATH;
      ofn.lpstrDefExt = "bmp";
      ofn.Flags = OFN_OVERWRITEPROMPT;

#ifdef VBA
      g_pvp->ApcHost->BeginModalDialog();
#endif
      int ret = GetSaveFileName(&ofn);
#ifdef VBA
      g_pvp->ApcHost->EndModalDialog();
#endif

      // user cancelled
      if(ret == 0)
         return;// S_FALSE;
   }

   HANDLE hfile = CreateFile(m_szBlueprintFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

   float tableheight, tablewidth;
   if (g_pvp->m_fBackglassView)
   {
      tablewidth = (float)EDITOR_BG_WIDTH;
      tableheight = (float)EDITOR_BG_HEIGHT;
   }
   else
   {
      tablewidth = m_right - m_left;
      tableheight = m_bottom - m_top;
   }

   int bmwidth, bmheight;
   if (tableheight > tablewidth)
   {
      bmheight = 4096;
      bmwidth = (int)((tablewidth/tableheight) * bmheight + 0.5f);
   }
   else
   {
      bmwidth = 4096; 
      bmheight = (int)((tableheight/tablewidth) * bmwidth + 0.5f);
   }

   int totallinebytes = bmwidth * 3;
   totallinebytes = (((totallinebytes-1)/4)+1)*4; // make multiple of four
   const int bmlinebuffer = totallinebytes - (bmwidth * 3);

   BITMAPFILEHEADER bmfh;
   ZeroMemory(&bmfh, sizeof(bmfh));
   bmfh.bfType = 'M'<<8 | 'B';
   bmfh.bfSize = sizeof(bmfh) + sizeof(BITMAPINFOHEADER) + totallinebytes*bmheight;
   bmfh.bfOffBits = sizeof(bmfh) + sizeof(BITMAPINFOHEADER);	

   DWORD foo;
   WriteFile(hfile, &bmfh, sizeof(bmfh), &foo, NULL);

   BITMAPINFO bmi;
   ZeroMemory(&bmi, sizeof(bmi));
   bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth = bmwidth;
   bmi.bmiHeader.biHeight = bmheight;
   bmi.bmiHeader.biPlanes = 1;
   bmi.bmiHeader.biBitCount = 24;
   bmi.bmiHeader.biCompression = BI_RGB;
   bmi.bmiHeader.biSizeImage = totallinebytes*bmheight;

   WriteFile(hfile, &bmi, sizeof(BITMAPINFOHEADER), &foo, NULL);

   HDC hdcScreen = GetDC(NULL);
   HDC hdc2 = CreateCompatibleDC(hdcScreen);

   char *pbits;
   HBITMAP hdib = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, (void **)&pbits, NULL, 0);

   /*const HBITMAP hbmOld =*/ (HBITMAP)SelectObject(hdc2, hdib);

   PaintSur * const psur = new PaintSur(hdc2, (float)bmwidth/tablewidth, tablewidth*0.5f, tableheight*0.5f, bmwidth, bmheight, NULL);

   SelectObject(hdc2, GetStockObject(WHITE_BRUSH));
   PatBlt(hdc2, 0, 0, bmwidth, bmheight, PATCOPY);

   if (g_pvp->m_fBackglassView)
   {
      Render3DProjection(psur);
   }

   for (int i=0;i<m_vedit.Size();i++)
   {
      IEditable *ptr = m_vedit.ElementAt(i);
      if (ptr->isVisible && ptr->m_fBackglass == g_pvp->m_fBackglassView)
      {
         ptr->RenderBlueprint(psur);
      }
   }

   //Render(psur);

   delete psur;

   for (int i=0;i<bmheight;i++)
   {
      WriteFile(hfile, (pbits + ((i*bmwidth) * 3)), bmwidth*3, &foo, NULL);
   }

   // For some reason to make our bitmap compatible with all programs,
   // We need to write out dummy bytes as if our totalwidthbytes had been
   // a multiple of 4.
   for (int i=0;i<bmheight;i++)
   {
      for (int l=0;l<bmlinebuffer;l++)
      {
         WriteFile(hfile, pbits, 1, &foo, NULL);
      }
   }

   DeleteDC(hdc2);
   ReleaseDC(NULL, hdcScreen);

   DeleteObject(hdib);

   CloseHandle(hfile);
}


void PinTable::SelectItem(IScriptable *piscript)
{
   ISelect * const pisel = piscript->GetISelect();
   if (pisel)
   {
      AddMultiSel(pisel, fFalse, fTrue);
   }
}

void PinTable::DoCodeViewCommand(int command)
{
   switch (command)
   {
   case ID_SAVE:
      // added by chris as part of table protection
      if (!CheckPermissions(DISABLE_TABLE_SAVE))
      {
         TableSave();
      }
      break;

   case ID_TABLE_PLAY:
      Play();
      break;
   }
}

void PinTable::SetDirtyScript(SaveDirtyState sds)
{
   m_sdsDirtyScript = sds;
   CheckDirty();
}

void PinTable::SetDirty(SaveDirtyState sds)
{
   m_sdsDirtyProp = sds;
   CheckDirty();
}

void PinTable::SetNonUndoableDirty(SaveDirtyState sds)
{
   m_sdsNonUndoableDirty = sds;
   CheckDirty();
}

void PinTable::CheckDirty()
{
   SaveDirtyState sdsNewDirtyState = (SaveDirtyState)max(max((int)m_sdsDirtyProp, (int)m_sdsDirtyScript), (int)m_sdsNonUndoableDirty);

   if (sdsNewDirtyState != m_sdsCurrentDirtyState)
   {
      if (sdsNewDirtyState > eSaveClean)
      {
         char szWindowName[1024+1];
         lstrcpy(szWindowName, m_szTitle);
         lstrcat(szWindowName, "*");

         SetCaption(szWindowName);
      }
      else
      {
         SetCaption(m_szTitle);
      }
   }

   m_sdsCurrentDirtyState = sdsNewDirtyState;
}

BOOL PinTable::FDirty()
{
   return (m_sdsCurrentDirtyState > eSaveClean);
}

void PinTable::BeginUndo()
{
   m_undo.BeginUndo();
}

void PinTable::EndUndo()
{
   m_undo.EndUndo();
}

void PinTable::Undo()
{
   m_undo.Undo();

   SetDirtyDraw();
   g_pvp->m_sb.RefreshProperties();
}

void PinTable::Uncreate(IEditable *pie)
{
   if (pie->GetISelect()->m_selectstate != eNotSelected)
   {
      AddMultiSel(pie->GetISelect(), fTrue, fTrue); // Remove the item from the multi-select list
   }

   pie->GetISelect()->Uncreate();
   pie->Release();
}

void PinTable::Undelete(IEditable *pie)
{
   m_vedit.AddElement(pie);
   pie->Undelete();
   SetDirtyDraw();
}

void PinTable::BackupForPlay()
{
   m_undo.BeginUndo();

   m_undo.MarkForUndo((IEditable *)this);
   for (int i=0;i<m_vedit.Size();i++)
   {
      m_undo.MarkForUndo(m_vedit.ElementAt(i));
   }

   m_undo.EndUndo();
}

void PinTable::RestoreBackup()
{
   m_undo.Undo();
}

void PinTable::Copy()
{
   Vector<IStream> vstm;
   ULONG writ = 0;

   if (m_vmultisel.ElementAt(0) == (ISelect *)this) // Can't copy table
   {
      return;
   }

   //m_vstmclipboard

   for (int i=0;i<m_vmultisel.Size();i++)
   {
      HGLOBAL hglobal = GlobalAlloc(GMEM_MOVEABLE, 1);

      IStream *pstm;
      CreateStreamOnHGlobal(hglobal, TRUE, &pstm);

      //////// BUG!  With multi-select, if you have multiple dragpoints on
      //////// a surface selected, the surface will get copied multiple times
      const int type = m_vmultisel.ElementAt(i)->GetIEditable()->GetItemType();
      pstm->Write(&type, sizeof(int), &writ);

      m_vmultisel.ElementAt(i)->GetIEditable()->SaveData(pstm, NULL, NULL);

      vstm.AddElement(pstm);
   }

   g_pvp->SetClipboard(&vstm);
}

// BUG - in sync with list in ISelect.h
// value gets and'ed with 1(table view) or 2(backglass view)
// if you want to allow an element to be copy'n past for only table view use 1
// for only backglass view 2 and for both use 3
int rgItemViewAllowed[] =
{
   1, //eItemSurface
   1, //eItemFlipper
   3, //eItemTimer
   1, //eItemPlunger
   2, //eItemTextbox
   1, //eItemBumper
   1, //eItemTrigger
   3, //eItemLight
   1, //eItemKicker
   3, //eItemDecal
   1, //eItemGate
   1, //eItemSpinner
   1, //eItemRamp
   0, //eItemTable
   0, //eItemLightCenter
   0, //eItemDragPoint
   0, //eItemCollection
   2, //eItemDispReel
   3, //eItemLightSeq
   1, //eItemPrimitve
   1, //eItemFlasher
   0, //eItemLightSeqCenter
   1, //eItemComControl
   2, //eItemTypeCount
};

void PinTable::Paste(BOOL fAtLocation, int x, int y)
{
   IEditable *peditNew;
   BOOL fError = fFalse;
   int viewflag;
   int cpasted = 0;

   if (CheckPermissions(DISABLE_CUTCOPYPASTE))
   {
      g_pvp->ShowPermissionError();
      return;
   }

   if (g_pvp->m_fBackglassView)
   {
      viewflag = 2;
   }
   else
   {
      viewflag = 1;
   }

   IStream* pstm;

   // Do a backwards loop, so that the primary selection we had when
   // copying will again be the primary selection, since it will be
   // selected last.  Purely cosmetic.
   for (int i=(g_pvp->m_vstmclipboard.Size()-1);i>=0;i--)
      //for (i=0;i<g_pvp->m_vstmclipboard.Size();i++)
   {
      pstm = g_pvp->m_vstmclipboard.ElementAt(i);

      // Go back to beginning of stream to load
      LARGE_INTEGER foo;
      foo.QuadPart = 0;
      pstm->Seek(foo, STREAM_SEEK_SET, NULL);

      ULONG writ = 0;
      int type;
      /*const HRESULT hr =*/ pstm->Read(&type, sizeof(int), &writ);

      if (!(rgItemViewAllowed[type] & viewflag))
      {
         fError = fTrue;
      }
      else
      {
         CreateIEditableFromType(type, &peditNew);
         peditNew->AddRef();

         int id;
         peditNew->InitLoad(pstm, this, &id, CURRENT_FILE_FORMAT_VERSION, NULL, NULL);

         if (type != eItemDecal)
         {
            GetUniqueNamePasting(type, peditNew->GetScriptable()->m_wzName);
            peditNew->InitVBA(fTrue, 0, peditNew->GetScriptable()->m_wzName);
         }

         m_vedit.AddElement(peditNew);
         // copy the new element to the same layer as the source element
         m_layer[peditNew->GetISelect()->layerIndex].AddElement(peditNew);
         peditNew->InitPostLoad();
         peditNew->m_fBackglass = g_pvp->m_fBackglassView;

         AddMultiSel(peditNew->GetISelect(), (i == g_pvp->m_vstmclipboard.Size()-1) ? fFalse : fTrue, fTrue);
         cpasted++;
      }
   }

   // Center view on newly created objects, if they are off the screen
   if (cpasted > 0)
   {
      Vertex2D vcenter;
      GetCenter(&vcenter);
   }

   if ((cpasted > 0) && fAtLocation)
   {
      Vertex2D vcenter;
      GetCenter(&vcenter);

      Vertex2D vPos;
      TransformPoint(x,y,&vPos);

      Vertex2D vOffset;
      vOffset.x = vPos.x-vcenter.x;
      vOffset.y = vPos.y-vcenter.y;
      Translate(&vOffset);
   }

   if (fError)
   {
      LocalString ls(IDS_NOPASTEINVIEW);
      MessageBox(m_hwnd, ls.m_szbuffer, "Visual Pinball", 0);
   }
}

void PinTable::PreRender(Sur * const psur)
{
}

ItemTypeEnum PinTable::GetItemType()
{
   return eItemTable;
}

HRESULT PinTable::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   int csubobj, csounds, ctextures, cfonts, ccollection;

   LoadData(pstm, csubobj, csounds, ctextures, cfonts, ccollection, version, hcrypthash, hcryptkey);

   return S_OK;
}

HRESULT PinTable::InitPostLoad()
{
   return S_OK;
}

HRESULT PinTable::InitVBA(BOOL fNew, int id, WCHAR *wzName)
{
   return S_OK;
}

ISelect *PinTable::GetISelect()
{
   return (ISelect *)this;
}

void PinTable::SetDefaults(bool fromMouseClick)
{
}

IScriptable *PinTable::GetScriptable()
{
   return (IScriptable *)this;
}

// fUpdate tells us whether to go ahead and change the UI
// based on the new selection, or whether more stuff is coming
// down the pipe (speeds up drag-selection)
void PinTable::AddMultiSel(ISelect *psel, BOOL fAdd, BOOL fUpdate)
{
   int index = m_vmultisel.IndexOf(psel);

   //_ASSERTE(m_vmultisel.ElementAt(0)->m_selectstate == eSelected);

   if (index == -1) // If we aren't selected yet, do that
   {
      _ASSERTE(psel->m_selectstate == eNotSelected);
      // If we non-shift click on an element outside the multi-select group, delete the old group
      // If the table is currently selected, deselect it - the table can not be part of a multi-select
      if (!fAdd || (m_vmultisel.ElementAt(0) == (ISelect *)this))
      {
         for (int i=0;i<m_vmultisel.Size();i++)
         {
            m_vmultisel.ElementAt(i)->m_selectstate = eNotSelected;
         }

         m_vmultisel.RemoveAllElements();

         m_vmultisel.AddElement(psel);
      }
      else
      {
         // Make this new selection the primary one for the group
         m_vmultisel.ElementAt(0)->m_selectstate = eMultiSelected;

         m_vmultisel.InsertElementAt(psel, 0);
      }

      psel->m_selectstate = eSelected;
      SetDirtyDraw();
   }
   else if (fAdd) // Take the element off the list
   {
      _ASSERTE(psel->m_selectstate != eNotSelected);
      m_vmultisel.RemoveElementAt(index);
      psel->m_selectstate = eNotSelected;
      if (m_vmultisel.Size() == 0)
      {
         // Have to have something selected
         m_vmultisel.AddElement((ISelect *)this);
      }
      // The main element might have changed
      m_vmultisel.ElementAt(0)->m_selectstate = eSelected;
      SetDirtyDraw();
   }
   else if (m_vmultisel.ElementAt(0) != psel) // Object already in list - no change to selection, only to primary
   {
      _ASSERTE(psel->m_selectstate != eNotSelected);

      // Make this new selection the primary one for the group
      m_vmultisel.ElementAt(0)->m_selectstate = eMultiSelected;

      m_vmultisel.RemoveElementAt(index);

      m_vmultisel.InsertElementAt(psel, 0);

      psel->m_selectstate = eSelected;
      SetDirtyDraw();
   }

   if (fUpdate)
   {
      g_pvp->SetPropSel(&m_vmultisel);
   }

   if (m_vmultisel.ElementAt(0)->GetIEditable() && m_vmultisel.ElementAt(0)->GetIEditable()->GetScriptable())
   {
      m_pcv->SelectItem(m_vmultisel.ElementAt(0)->GetIEditable()->GetScriptable());
   }
}

void PinTable::OnDelete()
{
   Vector<ISelect> m_vseldelete;

   for (int i=0;i<m_vmultisel.Size();i++)
   {
      // Can't delete these items yet - AddMultiSel will try to mark them as unselected
      m_vseldelete.AddElement(m_vmultisel.ElementAt(i));
   }

   AddMultiSel((ISelect *)this, fFalse, fTrue); // Will get rid of the multi-selection

   for (int i=0;i<m_vseldelete.Size();i++)
   {
      DeleteFromLayer( m_vseldelete.ElementAt(i)->GetIEditable() );
      m_vseldelete.ElementAt(i)->Delete();
   }
}

void PinTable::OnKeyDown(int key)
{
   const int fShift = GetKeyState(VK_SHIFT) & 0x8000;
   //const int fCtrl = GetKeyState(VK_CONTROL) & 0x8000;
   //const int fAlt = GetKeyState(VK_MENU) & 0x8000;

   switch (key)
   {
   case VK_DELETE:
      {
         OnDelete();
      }
      break;

   case VK_LEFT:
   case VK_RIGHT:
   case VK_UP:
   case VK_DOWN:
      {
         BeginUndo();
         const int distance = fShift ? 10 : 1;
         for (int i=0;i<m_vmultisel.Size();i++)
         {
            ISelect *const pisel = m_vmultisel.ElementAt(i);
            if (!pisel->GetIEditable()->GetISelect()->m_fLocked) // control points get lock info from parent - UNDONE - make this code snippet be in one place
            {
               switch (key)
               {
               case VK_LEFT:
                  pisel->GetIEditable()->MarkForUndo();
                  pisel->MoveOffset(-distance/m_zoom,0);
                  break;

               case VK_RIGHT:
                  pisel->GetIEditable()->MarkForUndo();
                  pisel->MoveOffset(distance/m_zoom,0);
                  break;

               case VK_UP:
                  pisel->GetIEditable()->MarkForUndo();
                  pisel->MoveOffset(0,-distance/m_zoom);
                  break;

               case VK_DOWN:
                  pisel->GetIEditable()->MarkForUndo();
                  pisel->MoveOffset(0,distance/m_zoom);
                  break;
               }
            }
         }
         EndUndo();
      }
      break;
   }
}

void PinTable::UseTool(int x,int y,int tool)
{
   Vertex2D v;
   TransformPoint(x,y,&v);
   IEditable *pie = NULL;

   switch (tool)
   {
   case ID_INSERT_WALL:
      {
         CComObject<Surface> *psur;
         CComObject<Surface>::CreateInstance(&psur);
         if (psur)
         {
            psur->AddRef();
            psur->Init(this, v.x, v.y, true);
            pie = (IEditable *)psur;
         }
         break;
      }
   case ID_INSERT_TARGET:
      {
         CComObject<Surface> *psur;
         CComObject<Surface>::CreateInstance(&psur);
         if (psur)
         {
            psur->AddRef();
            psur->InitTarget(this, v.x, v.y, true);
            pie = (IEditable *)psur;
         }
      }
      break;
   case ID_INSERT_FLIPPER:
      {
         CComObject<Flipper> *pflipper;
         CComObject<Flipper>::CreateInstance(&pflipper);
         if (pflipper)
         {
            pflipper->AddRef();
            pflipper->Init(this, v.x, v.y, true);
            pie = (IEditable *)pflipper;
         }
         break;
      }
   case ID_INSERT_TIMER:
      {
         CComObject<Timer> *ptimer;
         CComObject<Timer>::CreateInstance(&ptimer);
         if (ptimer)
         {
            ptimer->AddRef();
            ptimer->Init(this, v.x, v.y, true);
            pie = (IEditable *)ptimer;
         }
         break;
      }
   case ID_INSERT_PLUNGER:
      {
         CComObject<Plunger> *pplunger;
         CComObject<Plunger>::CreateInstance(&pplunger);
         if (pplunger)
         {
            pplunger->AddRef();
            pplunger->Init(this, v.x, v.y, true);
            pie = (IEditable *)pplunger;
         }
         break;
      }
   case ID_INSERT_TEXTBOX:
      {
         CComObject<Textbox> *ptextbox;
         CComObject<Textbox>::CreateInstance(&ptextbox);
         if (ptextbox)
         {
            ptextbox->AddRef();
            ptextbox->Init(this, v.x, v.y, true);
            pie = (IEditable *)ptextbox;
         }
         break;
      }
   case ID_INSERT_COM_CONTROL:
      {
         CComObject<PinComControl> *pcomcontrol;
         CComObject<PinComControl>::CreateInstance(&pcomcontrol);
         if (pcomcontrol)
         {
            pcomcontrol->AddRef();
            HRESULT hr = pcomcontrol->Init(this, v.x, v.y, true);
            if (hr == E_FAIL)
            {
               pie = NULL;
               pcomcontrol->Release();
            }
            else
            {
               pie = (IEditable *)pcomcontrol;
            }
         }
         break;
      }
   case ID_INSERT_BUMPER:
      {
         CComObject<Bumper> *pbumper;
         CComObject<Bumper>::CreateInstance(&pbumper);
         if (pbumper)
         {
            pbumper->AddRef();
            pbumper->Init(this, v.x, v.y, true);
            pie = (IEditable *)pbumper;
         }
         break;
      }
   case ID_INSERT_TRIGGER:
      {
         CComObject<Trigger> *ptrigger;
         CComObject<Trigger>::CreateInstance(&ptrigger);
         if (ptrigger)
         {
            ptrigger->AddRef();
            ptrigger->Init(this, v.x, v.y, true);
            pie = (IEditable *)ptrigger;
         }
         break;
      }
   case ID_INSERT_LIGHT:
      {
         CComObject<Light> *plight;
         CComObject<Light>::CreateInstance(&plight);
         if (plight)
         {
            plight->AddRef();
            plight->Init(this, v.x, v.y, true);
            pie = (IEditable *)plight;
         }
         break;
      }
   case ID_INSERT_KICKER: {
      CComObject<Kicker> *pkicker;
      CComObject<Kicker>::CreateInstance(&pkicker);
      if (pkicker)
      {
         pkicker->AddRef();
         pkicker->Init(this, v.x, v.y, true);
         pie = (IEditable *)pkicker;
      }
      break;
                    }
   case ID_INSERT_DECAL:
      {
         CComObject<Decal> *pdecal;
         CComObject<Decal>::CreateInstance(&pdecal);
         if (pdecal)
         {
            pdecal->AddRef();
            pdecal->Init(this, v.x, v.y, true);
            pie = (IEditable *)pdecal;
         }
         break;
      }
   case ID_INSERT_PRIMITIVE:
      {
         CComObject<Primitive> *pprimitive;
         CComObject<Primitive>::CreateInstance(&pprimitive);
         if (pprimitive)
         {
            pprimitive->AddRef();
            pprimitive->Init(this, v.x, v.y, true);
            pie = (IEditable *)pprimitive;
         }
         break;
      }
   case ID_INSERT_GATE:
      {
         CComObject<Gate> *pgate;
         CComObject<Gate>::CreateInstance(&pgate);
         if (pgate)
         {
            pgate->AddRef();
            pgate->Init(this, v.x, v.y, true);
            pie = (IEditable *)pgate;
         }
         break;
      }
   case ID_INSERT_SPINNER:
      {
         CComObject<Spinner> *pspinner;
         CComObject<Spinner>::CreateInstance(&pspinner);
         if (pspinner)
         {
            pspinner->AddRef();
            pspinner->Init(this, v.x, v.y, true);
            pie = (IEditable *)pspinner;
         }
         break;
      }
   case ID_INSERT_RAMP:
      {
         CComObject<Ramp> *pramp;
         CComObject<Ramp>::CreateInstance(&pramp);
         if (pramp)
         {
            pramp->AddRef();
            pramp->Init(this, v.x, v.y, true);
            pie = (IEditable *)pramp;
         }
         break;
      }
   case ID_INSERT_FLASHER:
      {
         CComObject<Flasher> *pflasher;
         CComObject<Flasher>::CreateInstance(&pflasher);
         if (pflasher)
         {
            pflasher->AddRef();
            pflasher->Init(this, v.x, v.y, true);
            pie = (IEditable *)pflasher;
         }
         break;
      }
   case ID_INSERT_DISP_REEL:
      {
         CComObject<DispReel> *pdispreel;
         CComObject<DispReel>::CreateInstance(&pdispreel);
         if (pdispreel)
         {
            pdispreel->AddRef();
            pdispreel->Init(this, v.x, v.y, true);
            pie = (IEditable *)pdispreel;
         }
         break;
      }
   case ID_INSERT_LIGHT_SEQ:
      {
         CComObject<LightSeq> *plightseq;
         CComObject<LightSeq>::CreateInstance(&plightseq);
         if (plightseq)
         {
            plightseq->AddRef();
            plightseq->Init(this, v.x, v.y, true);
            pie = (IEditable *)plightseq;
         }
         break;
      }
   }

   if (pie)
   {
      pie->m_fBackglass = g_pvp->m_fBackglassView;
      m_vedit.AddElement(pie);
      AddMultiSel(pie->GetISelect(), fFalse, fTrue);
      BeginUndo();
      m_undo.MarkForCreate(pie);
      EndUndo();
   }

   g_pvp->ParseCommand(IDC_SELECT, g_pvp->m_hwnd, 0);
}

void PinTable::TransformPoint(int x, int y, Vertex2D *pv)
{
   RECT rc;
   GetClientRect(m_hwnd, &rc);

   HitSur * const phs = new HitSur(NULL, m_zoom, m_offsetx, m_offsety, rc.right - rc.left, rc.bottom - rc.top, 0, 0, NULL);

   *pv = phs->ScreenToSurface(x, y);

   delete phs;
}

void PinTable::OnLButtonDown(int x, int y)
{
   Vertex2D v;
   TransformPoint(x,y,&v);

   m_rcDragRect.left = v.x;
   m_rcDragRect.right = v.x;
   m_rcDragRect.top = v.y;
   m_rcDragRect.bottom = v.y;

   m_fDragging = fTrue;

   SetCapture(GetPTable()->m_hwnd);

   SetDirtyDraw();
}

void PinTable::OnLButtonUp(int x, int y)
{
   if (m_fDragging)
   {
      m_fDragging = fFalse;
      ReleaseCapture();
      if ((m_rcDragRect.left != m_rcDragRect.right) || (m_rcDragRect.top != m_rcDragRect.bottom))
      {
         Vector<ISelect> vsel;

         HDC hdc = GetDC(m_hwnd);

         RECT rc;
         GetClientRect(m_hwnd, &rc);

         HitRectSur * const phrs = new HitRectSur(hdc, m_zoom, m_offsetx, m_offsety, rc.right - rc.left, rc.bottom - rc.top, &m_rcDragRect, &vsel);

         // Just want one rendering pass (no PreRender) so we don't select things twice
         Render(phrs);

         const int ksshift = GetKeyState(VK_SHIFT);
         const BOOL fAdd = ((ksshift & 0x80000000) != 0);

         int minlevel = INT_MAX;

         for (int i=0;i<vsel.Size();i++)
         {
            minlevel = min(minlevel, vsel.ElementAt(i)->GetSelectLevel());
         }

         if (vsel.Size() > 0)
         {
            BOOL fFirstAdd = fTrue;

            for (int i=0;i<vsel.Size();i++)
            {
               if (vsel.ElementAt(i)->GetSelectLevel() == minlevel)
               {
                  if (fFirstAdd)
                  {
                     // If the shift key is not down, adding the first
                     // element will clear the previously selected group
                     AddMultiSel(vsel.ElementAt(i), fAdd, fFalse);
                     fFirstAdd = fFalse;
                  }
                  else
                  {
                     AddMultiSel(vsel.ElementAt(i), fTrue, fFalse);
                  }
               }
            }

            // We told the UI to not update in AddMultiSel because
            // we were potentially adding a bunch of things.
            // So we have to update it manually here.
            g_pvp->SetPropSel(&m_vmultisel);
         }

         delete phrs;

         ReleaseDC(m_hwnd, hdc);
      }
   }

   SetDirtyDraw();
}

void PinTable::OnMouseMove(int x, int y)
{
   Vertex2D v;
   TransformPoint(x,y,&v);

   m_rcDragRect.right = v.x;
   m_rcDragRect.bottom = v.y;

   if (m_fDragging)
   {
      SetDirtyDraw();
   }
}

HRESULT PinTable::GetTypeName(BSTR *pVal)
{
   WCHAR wzName[128];

   int stringid = (!g_pvp->m_fBackglassView) ? IDS_TABLE : IDS_TB_BACKGLASS;

   LocalString ls(stringid);

   MultiByteToWideChar(CP_ACP, 0, ls.m_szbuffer, -1, wzName, 128);

   *pVal = SysAllocString(wzName);

   return S_OK;
}

void PinTable::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   if (!g_pvp->m_fBackglassView)
   {
      PropertyPane *pproppane;

      pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
      pvproppane->AddElement(pproppane);

      pproppane = new PropertyPane(IDD_PROPTABLE_EDITOR, IDS_EDITOR);
      pvproppane->AddElement(pproppane);

      pproppane = new PropertyPane(IDD_PROPTABLE_VISUALS, IDS_VISUALS_SOUND);
      pvproppane->AddElement(pproppane);

      pproppane = new PropertyPane(IDD_PROPTABLE_BALL, IDS_DEFAULTBALL);
      pvproppane->AddElement(pproppane);

      pproppane = new PropertyPane(IDD_PROPTABLE_PHYSICS, IDS_DIMENSIONSSLOPE);
      pvproppane->AddElement(pproppane);

      pproppane = new PropertyPane(IDD_PROPTABLE_LIGHTSOURCES, IDS_LIGHTSOURCES);
      pvproppane->AddElement(pproppane);
   }
   else
   {
      PropertyPane *pproppane;

      pproppane = new PropertyPane(IDD_PROPTABLE_EDITOR, IDS_EDITOR);
      pvproppane->AddElement(pproppane);

      pproppane = new PropertyPane(IDD_PROPBACKGLASS_VISUALS, IDS_VISUALS);
      pvproppane->AddElement(pproppane);

      pproppane = new PropertyPane(IDD_PROPBACKGLASS_PHYSICS, IDS_PHYSICSGFX);
      pvproppane->AddElement(pproppane);
   }
}

LRESULT CALLBACK TableWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   HDC hdc;
   PAINTSTRUCT ps;
   CComObject<PinTable> *pt;

   switch (uMsg)
   {
   case WM_CLOSE:
      {
         // Scary!!!!
         pt = (CComObject<PinTable> *)GetWindowLongPtr(hwnd, GWLP_USERDATA); 			
         //DefMDIChildProc(hwnd, uMsg, wParam, lParam);

         KillTimer(hwnd, TIMER_ID_AUTOSAVE);
         SetTimer(hwnd, TIMER_ID_CLOSE_TABLE, 100, NULL);	//wait 250 milliseconds

         return 0;	// destroy the WM_CLOSE message
      }

   case WM_TIMER:
      {
         pt = (CComObject<PinTable> *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
         switch (wParam)
         {
         case TIMER_ID_AUTOSAVE:
            pt->AutoSave();
            break;

         case TIMER_ID_CLOSE_TABLE:
            KillTimer(hwnd, TIMER_ID_CLOSE_TABLE);
            pt->m_pvp->CloseTable(pt);
            //DestroyWindow(hwnd);
            return 0;
            break;
         }
      }
      break;

   case WM_SETCURSOR:
      {
         if (LOWORD(lParam) == HTCLIENT)
         {
            HCURSOR hcursor;
            char *cursorid;
            HINSTANCE hinst = g_hinst;
            switch (g_pvp->m_ToolCur)
            {
            default:
            case IDC_SELECT:
               hinst = NULL;
               cursorid = IDC_ARROW;
               break;

            case ID_TABLE_MAGNIFY:
               cursorid = MAKEINTRESOURCE(IDC_MAGNIFY);
               break;

            case ID_INSERT_WALL:
               cursorid = MAKEINTRESOURCE(IDC_WALL);
               break;

            case ID_INSERT_LIGHT:
               cursorid = MAKEINTRESOURCE(IDC_LIGHT);
               break;

            case ID_INSERT_FLIPPER:
               cursorid = MAKEINTRESOURCE(IDC_FLIPPER);
               break;

            case ID_INSERT_TRIGGER:
               cursorid = MAKEINTRESOURCE(IDC_TRIGGER);
               break;

            case ID_INSERT_GATE:
               cursorid = MAKEINTRESOURCE(IDC_GATE);
               break;

            case ID_INSERT_TIMER:
               cursorid = MAKEINTRESOURCE(IDC_TIMER);
               break;

            case ID_INSERT_PLUNGER:
               cursorid = MAKEINTRESOURCE(IDC_PLUNGER);
               break;

            case ID_INSERT_TEXTBOX:
               cursorid = MAKEINTRESOURCE(IDC_TEXTBOX);
               break;

            case ID_INSERT_COM_CONTROL:
               cursorid = MAKEINTRESOURCE(IDC_TEXTBOX);
               break;

            case ID_INSERT_BUMPER:
               cursorid = MAKEINTRESOURCE(IDC_BUMPER);
               break;

            case ID_INSERT_KICKER:
               cursorid = MAKEINTRESOURCE(IDC_KICKER);
               break;

            case ID_INSERT_TARGET:
               cursorid = MAKEINTRESOURCE(IDC_TARGET);
               break;

            case ID_INSERT_DECAL:
               cursorid = MAKEINTRESOURCE(IDC_DECAL);
               break;

            case ID_INSERT_SPINNER:
               cursorid = MAKEINTRESOURCE(IDC_SPINNER);
               break;

            case ID_INSERT_RAMP:
               cursorid = MAKEINTRESOURCE(IDC_RAMP);
               break;

            case ID_INSERT_FLASHER:
               cursorid = MAKEINTRESOURCE(IDC_FLASHER);
               break;

            case ID_INSERT_DISP_REEL:
               cursorid = MAKEINTRESOURCE(IDC_DISP_REEL);
               break;

            case ID_INSERT_LIGHT_SEQ:
               cursorid = MAKEINTRESOURCE(IDC_LIGHT_SEQ);
               break;

            case ID_INSERT_PRIMITIVE:
               cursorid = MAKEINTRESOURCE(IDC_PRIMITIVE);
               break;
            }
            hcursor = LoadCursor(hinst, cursorid);
            SetCursor(hcursor);
            return TRUE;
         }
      }
      break;

   case WM_PAINT: {
      hdc = BeginPaint(hwnd,&ps);
      pt = (CComObject<PinTable> *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      pt->Paint(hdc);
      EndPaint(hwnd,&ps);
      break;
                  }

   case WM_MOUSEACTIVATE:
   case WM_ACTIVATE:
      if (LOWORD(wParam) != WA_INACTIVE)
      {
         pt = (CComObject<PinTable> *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
         pt->m_pvp->m_ptableActive = pt;

         // re-evaluate the toolbar/menuitems depending on table permissions
         g_pvp->SetEnableToolbar();
         g_pvp->SetEnableMenuItems();
      }
      break;

      
   case WM_LBUTTONDOWN: {
      pt = (CComObject<PinTable> *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      const short x = (short)(lParam & 0xffff);
      const short y = (short)((lParam>>16) & 0xffff);
      if ((g_pvp->m_ToolCur == IDC_SELECT) || (g_pvp->m_ToolCur == ID_TABLE_MAGNIFY))
      {
         pt->DoLButtonDown(x,y);
      }
      else
      {
         pt->UseTool(x,y,g_pvp->m_ToolCur);
      }
      break;
                        }

   case WM_LBUTTONDBLCLK: {
      pt = (CComObject<PinTable> *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      const short x = (short)(lParam & 0xffff);
      const short y = (short)((lParam>>16) & 0xffff);
      pt->DoLDoubleClick(x,y);
      break;
                          }

   case WM_LBUTTONUP:
      {
         const short x = (short)(lParam & 0xffff);
         const short y = (short)((lParam>>16) & 0xffff);
         pt = (CComObject<PinTable> *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
         pt->DoLButtonUp(x,y);
      }
      break;

   case WM_MOUSEMOVE:
      {
         const short x = (short)(lParam & 0xffff);
         const short y = (short)((lParam>>16) & 0xffff);
         pt = (CComObject<PinTable> *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
         const BOOL altPressed = ((GetKeyState(VK_MENU) & 0x80000000) != 0);
         if ( altPressed )
         {         
            // panning feature starts here...if the user holds alt and moves the mouse 
            // everything is moved in the direction of the mouse was moved
            int factorX= 100;
            int factorY= 100;
            SCROLLINFO si;
            ZeroMemory(&si,sizeof(SCROLLINFO));
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_ALL;
            GetScrollInfo(hwnd, SB_HORZ, &si);
            if ( pt->m_oldMousePosX>x )  pt->m_offsetx -= si.nPage/factorX;
            if ( pt->m_oldMousePosX<x )  pt->m_offsetx += si.nPage/factorX;
            GetScrollInfo(hwnd, SB_VERT, &si);
            if ( pt->m_oldMousePosY>y )  pt->m_offsety -= si.nPage/factorY;
            if ( pt->m_oldMousePosY<y )  pt->m_offsety += si.nPage/factorY;
            pt->SetDirtyDraw();
            pt->SetMyScrollInfo();
            pt->m_oldMousePosX=x;
            pt->m_oldMousePosY=y;
            break;
         }
         pt->DoMouseMove(x,y);
         pt->m_oldMousePosX=x;
         pt->m_oldMousePosY=y;
      }
      break;

   case WM_RBUTTONDOWN:
      {
         pt = (CComObject<PinTable> *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
         const short x = (short)(lParam & 0xffff);
         const short y = (short)((lParam>>16) & 0xffff);

         pt->DoRButtonDown(x,y);
         break;
      }
   case WM_CONTEXTMENU:
      {
         pt = (CComObject<PinTable> *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
         long x = (long)(lParam & 0xffff);
         long y = (long)((lParam>>16) & 0xffff);
         POINT p;
         if ( GetCursorPos(&p) && ScreenToClient( hwnd, &p))
         {
            x = p.x;
            y = p.y;
         }
         pt->DoRButtonUp(x,y);
         break;
      }
   case WM_KEYDOWN:
      {
         pt = (CComObject<PinTable> *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
         pt->OnKeyDown(wParam);
         break;
      }

   case WM_HSCROLL:
      {
         pt = (CComObject<PinTable> *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
         SCROLLINFO si;
         ZeroMemory(&si,sizeof(SCROLLINFO));
         si.cbSize = sizeof(SCROLLINFO);
         si.fMask = SIF_ALL;
         GetScrollInfo(hwnd, SB_HORZ, &si);
         switch (LOWORD(wParam))
         {
         case SB_LINELEFT:
            pt->m_offsetx -= si.nPage/10;
            break;
         case SB_LINERIGHT:
            pt->m_offsetx += si.nPage/10;
            break;
         case SB_PAGELEFT:
            pt->m_offsetx -= si.nPage/2;
            break;
         case SB_PAGERIGHT:
            pt->m_offsetx += si.nPage/2;
            break;
         case SB_THUMBTRACK:
            {
               const int delta = (int)(pt->m_offsetx - si.nPos);
               pt->m_offsetx = (float)((short)HIWORD(wParam) + delta);
               break;
            }
         }
         pt->SetDirtyDraw();
         pt->SetMyScrollInfo();
         return 0;
      }
      break;

   case WM_VSCROLL:
      {
         pt = (CComObject<PinTable> *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
         SCROLLINFO si;
         ZeroMemory(&si,sizeof(SCROLLINFO));
         si.cbSize = sizeof(SCROLLINFO);
         si.fMask = SIF_ALL;
         GetScrollInfo(hwnd, SB_VERT, &si);
         switch (LOWORD(wParam))
         {
         case SB_LINEUP:
            pt->m_offsety -= si.nPage/10;
            break;
         case SB_LINEDOWN:
            pt->m_offsety += si.nPage/10;
            break;
         case SB_PAGEUP:
            pt->m_offsety -= si.nPage/2;
            break;
         case SB_PAGEDOWN:
            pt->m_offsety += si.nPage/2;
            break;
         case SB_THUMBTRACK:
            {
               const int delta = (int)(pt->m_offsety - si.nPos);
               pt->m_offsety = (float)((short)HIWORD(wParam) + delta);
               break;
            }
         }
         pt->SetDirtyDraw();
         pt->SetMyScrollInfo();
         return 0;
      }
      break;

   case WM_MOUSEWHEEL:
      {
         pt = (CComObject<PinTable> *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
         const short zDelta = (short) HIWORD(wParam);    // wheel rotation
         pt->m_offsety -= zDelta / pt->m_zoom;	// change to orientation to match windows default
         pt->SetDirtyDraw();
         pt->SetMyScrollInfo();
         return 0;
      }
      break;

   case WM_SIZE:
      {
         pt = (CComObject<PinTable> *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
         if (pt) // Window might have just been created
         {
            pt->SetMyScrollInfo();
            pt->m_fDirtyDraw = fTrue;
            // this window command is called whenever the MDI window changes over
            // re-evaluate the toolbar/menuitems depending on table permissions
            g_pvp->SetEnableToolbar();
            g_pvp->SetEnableMenuItems();
         }
         break;
      }

   case WM_COMMAND:
      break;

   case DONE_AUTOSAVE:
      {
         pt = (CComObject<PinTable> *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
         if (lParam == S_OK)
         {
            g_pvp->SetActionCur("");
         }
         else
         {
            g_pvp->SetActionCur("Autosave Failed");
         }
         pt->BeginAutoSaveCounter();
         HANDLE hEvent = (HANDLE)wParam;
         pt->m_vAsyncHandles.RemoveElement((int)hEvent);
         CloseHandle(hEvent);
      }
      break;
   }
   return DefMDIChildProc(hwnd, uMsg, wParam, lParam);
}

STDMETHODIMP PinTable::get_Name(BSTR *pVal)
{
   *pVal = SysAllocString((WCHAR *)m_wzName);

   return S_OK;
}

STDMETHODIMP PinTable::put_Name(BSTR newVal)
{
   //GetIApcProjectItem()->put_Name(newVal);

   STARTUNDO

      if ((lstrlenW(newVal) > 32) || (lstrlenW(newVal) < 1))
      {
         return E_FAIL;
      }

      if (m_pcv->ReplaceName((IScriptable *)this, newVal) == S_OK)
      {
         WideStrCopy(newVal, (WCHAR *)m_wzName);
         //lstrcpyW((WCHAR *)m_wzName, newVal);
      }

#ifdef VBA
      GetIApcProjectItem()->put_Name(newVal);
#endif

      STOPUNDO

         return S_OK;
}

STDMETHODIMP PinTable::get_Inclination(float *pVal)
{
   *pVal = m_inclination;

   return S_OK;
}

STDMETHODIMP PinTable::put_Inclination(float newVal)
{
   STARTUNDO

      m_inclination = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_Layback(float *pVal)
{
   *pVal = m_layback;

   return S_OK;
}

STDMETHODIMP PinTable::put_Layback(float newVal)
{
   STARTUNDO

      m_layback = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_MaxSeparation(float *pVal)
{
   if( m_overwriteGlobalStereo3D )
	   *pVal = m_maxSeparation;
   else
	   *pVal = m_globalMaxSeparation;

   return S_OK;
}

STDMETHODIMP PinTable::put_MaxSeparation(float newVal)
{
   STARTUNDO

   if( m_overwriteGlobalStereo3D )
	   m_maxSeparation = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_ZPD(float *pVal)
{
   if( m_overwriteGlobalStereo3D )
	   *pVal = m_ZPD;
   else
	   *pVal = m_globalZPD;

   return S_OK;
}

STDMETHODIMP PinTable::put_ZPD(float newVal)
{
   STARTUNDO

   if( m_overwriteGlobalStereo3D )
	   m_ZPD = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_FieldOfView(float *pVal)
{
   *pVal = m_FOV;

   return S_OK;
}

STDMETHODIMP PinTable::put_FieldOfView(float newVal)
{
   STARTUNDO

      m_FOV = newVal;

   STOPUNDO

      return S_OK;
}

void PinTable::ClearOldSounds()
{
   for (int i=0;i<m_voldsound.Size();i++)
   {
      //LPDIRECTSOUNDBUFFER pdsbOld = (LPDIRECTSOUNDBUFFER)(m_voldsound.ElementAt(i));
      PinSoundCopy * const ppsc = m_voldsound.ElementAt(i);
      DWORD status;
      ppsc->m_pDSBuffer->GetStatus(&status);
      if (!(status & DSBSTATUS_PLAYING)) //sound is done, we can throw it away now
      {
         ppsc->m_pDSBuffer->Release();
         m_voldsound.RemoveElementAt(i);
         delete ppsc;
         i--; // elements will shift, check this one again
      }
   }
}

HRESULT PinTable::StopSound(BSTR Sound)
{
   MAKE_ANSIPTR_FROMWIDE(szName, Sound);
   CharLowerBuff(szName, lstrlen(szName));

   // In case we were playing any of the main buffers
   for (int i=0;i<m_vsound.Size();i++)
   {
      if (!lstrcmp(m_vsound.ElementAt(i)->m_szInternalName, szName))
      {
         m_vsound.ElementAt(i)->m_pDSBuffer->Stop();
         break;
      }
   }

   for (int i=0;i<m_voldsound.Size();i++)
   {
      PinSoundCopy * const ppsc = m_voldsound.ElementAt(i);
      if (!lstrcmp(ppsc->m_ppsOriginal->m_szInternalName, szName))
      {
         ppsc->m_pDSBuffer->Stop();
         break;
      }
   }

   return S_OK;
}

STDMETHODIMP PinTable::PlaySound(BSTR bstr, int loopcount, float volume, float pan, float randompitch, int pitch, VARIANT_BOOL usesame, VARIANT_BOOL restart)
{
   MAKE_ANSIPTR_FROMWIDE(szName, bstr);
   CharLowerBuff(szName, lstrlen(szName));

   if( !lstrcmp( "knock", szName ) || !lstrcmp( "knocker", szName ) )
   {
      hid_knock();
   }

   int i;
   for (i=0;i<m_vsound.Size();i++)
   {
      if (!lstrcmp(m_vsound.ElementAt(i)->m_szInternalName, szName))
      {
         break;
      }
   }

   if (i==m_vsound.Size()) // did not find it
   {
      return S_OK;
   }

   ClearOldSounds();

   const int flags = (loopcount == -1) ? DSBPLAY_LOOPING : 0;
   const float totalvolume = max(min(((float)g_pplayer->m_SoundVolume)*volume*m_TableSoundVolume,100.0f),0.0f);
   const int decibelvolume = (totalvolume == 0.0f) ? DSBVOLUME_MIN : (int)(logf(totalvolume)*(float)(1000.0/log(10.0)) - 2000.0f); // 10 volume = -10Db

   LPDIRECTSOUNDBUFFER pdsb = m_vsound.ElementAt(i)->m_pDSBuffer;
   PinDirectSound *pDS = m_vsound.ElementAt(i)->m_pPinDirectSound;

   PinSoundCopy * ppsc = NULL;
   bool foundsame = false;
   if(usesame)
   {
	   for (int i=0;i<m_voldsound.Size();i++)
	   {
		  if(m_voldsound.ElementAt(i)->m_ppsOriginal->m_pDSBuffer == pdsb)
		  {
			  ppsc = m_voldsound.ElementAt(i);
			  foundsame = true;
			  break;
		  }
	   }
   }

   if(ppsc == NULL)
   {
       ppsc = new PinSoundCopy();
	   pDS->m_pDS->DuplicateSoundBuffer(pdsb, &ppsc->m_pDSBuffer/*&pdsbNew*/);
   }

   if (ppsc->m_pDSBuffer)
   {
      ppsc->m_pDSBuffer->SetVolume(decibelvolume);
	  if(randompitch > 0.f)
	  {
		  DWORD freq;
		  pdsb->GetFrequency(&freq);
		  freq += pitch;
		  const float rndh = rand_mt_01();
		  const float rndl = rand_mt_01();
		  ppsc->m_pDSBuffer->SetFrequency(freq + (DWORD)((float)freq * randompitch * rndh * rndh) - (DWORD)((float)freq * randompitch * rndl * rndl * 0.5f));
	  }
	  else if (pitch != 0)
	  {
		  DWORD freq;
		  pdsb->GetFrequency(&freq);
		  ppsc->m_pDSBuffer->SetFrequency(freq + pitch);
	  }
	  if(pan != 0.f)
		  ppsc->m_pDSBuffer->SetPan((LONG)(pan*DSBPAN_RIGHT));

	  DWORD status;
	  ppsc->m_pDSBuffer->GetStatus(&status);
	  if (!(status & DSBSTATUS_PLAYING))
	     ppsc->m_pDSBuffer->Play(0,0,flags);
	  else if(restart)
		 ppsc->m_pDSBuffer->SetCurrentPosition(0);
	  if(!foundsame)
	  {
		ppsc->m_ppsOriginal = m_vsound.ElementAt(i);
		m_voldsound.AddElement(ppsc);
	  }
   }
   else // Couldn't or didn't want to create a copy - just play the original
   {
      delete ppsc;

      pdsb->SetVolume(decibelvolume);
	  if(randompitch > 0.f)
	  {
	     DWORD freq;
	     pdsb->GetFrequency(&freq); //!! meh, if already randompitched before
	     freq += pitch;
	     const float rndh = rand_mt_01();
	     const float rndl = rand_mt_01();
	     pdsb->SetFrequency(freq + (DWORD)((float)freq * randompitch * rndh * rndh) - (DWORD)((float)freq * randompitch * rndl * rndl * 0.5f));
	  }
	  else if (pitch != 0)
	  {
		 DWORD freq;
		 pdsb->GetFrequency(&freq);
		 pdsb->SetFrequency(freq + pitch);
	  }
	  if(pan != 0.f)
	     pdsb->SetPan((LONG)(pan*DSBPAN_RIGHT));
    
	  DWORD status;
	  pdsb->GetStatus(&status);
	  if (!(status & DSBSTATUS_PLAYING))
		 pdsb->Play(0,0,flags);
	  else if(restart)// Okay, it got played again before it finished.  Well, just start it over.
		 pdsb->SetCurrentPosition(0);
   }

   return S_OK;
}


Texture *PinTable::GetImage(char *szName)
{
    if (szName == NULL || szName[0] == 0)
        return NULL;

    CharLowerBuff(szName, lstrlen(szName));

    // during playback, we use the hashtable for lookup
    if (!m_textureMap.empty())
    {
        std::tr1::unordered_map<const char*, Texture*, StringHashFunctor, StringComparator>::const_iterator
            it = m_textureMap.find(szName);
        if (it != m_textureMap.end())
            return it->second;
        else
            return NULL;
    }

   for (int i=0;i<m_vimage.Size();i++)
   {
      if (!lstrcmp(m_vimage.ElementAt(i)->m_szInternalName, szName))
      {
         return m_vimage.ElementAt(i);
      }
   }

   return NULL;
}

void PinTable::CreateGDIBackdrop()
{
}

void PinTable::ReImportImage(HWND hwndListView, Texture *ppi, char *filename)
{
   char szextension[MAX_PATH];
   ExtensionFromFilename(filename, szextension);

   BOOL fBinary;	
   if (!lstrcmpi(szextension, "bmp"))
   {
      fBinary = fFalse;
   }
   else // other format
   {
      fBinary = fTrue;
   }

   PinBinary *ppb=0;
   if (fBinary)
   {
      ppb = new PinBinary();
      ppb->ReadFromFile(filename);
   }

   MemTexture *tex = MemTexture::CreateFromFile(filename);

   if (tex == NULL)
   {
      if( ppb ) delete ppb;
      return;
   }

   ppi->FreeStuff();

   if (fBinary)
   {
      ppi->m_ppb = ppb;
   }

   //SAFE_RELEASE(ppi->m_pdsBuffer);

   ppi->m_width = tex->width();
   ppi->m_height = tex->height();
   ppi->m_pdsBuffer = tex;

   lstrcpy(ppi->m_szPath, filename);
}


bool PinTable::ExportImage(HWND hwndListView, Texture *ppi, char *szfilename)
{
   if (ppi->m_ppb != NULL)return ppi->m_ppb->WriteToFile(szfilename); 
   else if (ppi->m_pdsBuffer != NULL)
   {
      HANDLE hFile = CreateFile(szfilename,GENERIC_WRITE, FILE_SHARE_READ,
         NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

      if (hFile == INVALID_HANDLE_VALUE)
      {
         ShowError("The graphic file could not be written.");
         return false;
      }
      const int surfwidth = ppi->m_width;					// texture width 
      const int surfheight = ppi->m_height;					// and height		

      int bmplnsize = (surfwidth*4+3) & -4;		// line size ... 4 bytes per pixel + pad to 4 byte boundary		

      //<<<< began bmp file header and info <<<<<<<<<<<<<<<

      BITMAPFILEHEADER bmpf;		// file header
      bmpf.bfType = 'MB';
      bmpf.bfSize = sizeof(BITMAPFILEHEADER)+ sizeof(BITMAPINFOHEADER) + surfheight*bmplnsize;
      bmpf.bfReserved1 = 0; 
      bmpf.bfReserved2 = 0;
      bmpf.bfOffBits = sizeof(BITMAPFILEHEADER)+ sizeof(BITMAPINFOHEADER);

      DWORD write;
      // write BMP file header	
      WriteFile(hFile, &bmpf, sizeof(BITMAPFILEHEADER), &write, NULL);

      BITMAPINFOHEADER bmpi;		// info header		
      bmpi.biSize = sizeof(BITMAPINFOHEADER);	//only a few fields are used 
      bmpi.biWidth = surfwidth;
      bmpi.biHeight = surfheight;
      bmpi.biPlanes = 1;
      bmpi.biBitCount = 32;
      bmpi.biCompression = 0;
      bmpi.biSizeImage = surfheight*bmplnsize;
      bmpi.biXPelsPerMeter = 0;
      bmpi.biYPelsPerMeter = 0;
      bmpi.biClrUsed = 0;
      bmpi.biClrImportant = 0;

      //write BMP Info Header
      WriteFile(hFile, &bmpi, sizeof(BITMAPINFOHEADER), &write, NULL);

      unsigned char* sinfo = new unsigned char[bmplnsize+4]; //linebuffer and safty pad
      if (!sinfo) 
      {
         CloseHandle(hFile);
         return false;
      }

      unsigned char* info;
      for (info = sinfo + surfwidth*3; info < sinfo + bmplnsize; *info++ = 0); //fill padding with 0			

      const int pitch = ppi->m_pdsBuffer->pitch();
      const BYTE *spch = ppi->m_pdsBuffer->data() + (surfheight * pitch);	// just past the end of the Texture part of DD surface

      for (int i=0;i<surfheight;i++)
      {
         info = sinfo; //reset to start	
         const BYTE *pch = (spch -= pitch);  // start on previous previous line

         for (int l=0;l<surfwidth;l++)
         {					
            *(unsigned int*)info = *(unsigned int*)pch;
            info += 4;
            pch += 4;
         }

         WriteFile(hFile, sinfo, bmplnsize, &write, NULL);
         GetLastError();			
      }

      delete [] sinfo; sinfo = NULL;
      CloseHandle(hFile);	
      return true;
   }
   return false;
}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++>>

void PinTable::ImportImage(HWND hwndListView, char *filename)
{
   Texture * const ppi = new Texture();

   int colorkey;
   HRESULT hr = GetRegInt("Editor", "TransparentColorKey", &colorkey);

   if (hr == S_OK)	ppi->m_rgbTransparent = (COLORREF)colorkey; //assign from Registry 		
   else 
   {
      ppi->m_rgbTransparent = colorkey = RGB(255,0,255); //assume common transparent color 
      hr = SetRegValue("Editor", "TransparentColorKey", REG_DWORD, &ppi->m_rgbTransparent, 4);
   }

   ReImportImage(hwndListView, ppi, filename);

   if (ppi->m_pdsBuffer == NULL)
   {
      delete ppi;
      return;
   }


   // The first time we import a file, parse the name of the texture from the filename

   int begin, end;
   const int len = lstrlen(filename);

   for (begin=len;begin>=0;begin--)
   {
      if (filename[begin] == '\\')
      {
         begin++;
         break;
      }
   }

   for (end=len;end>=0;end--)
   {
      if (filename[end] == '.')
      {
         break;
      }
   }

   if (end == 0)
   {
      end = len-1;
   }

   lstrcpy(ppi->m_szName, &filename[begin]);

   ppi->m_szName[end-begin] = 0;

   lstrcpy(ppi->m_szInternalName, ppi->m_szName);

   CharLowerBuff(ppi->m_szInternalName, lstrlen(ppi->m_szInternalName));

   m_vimage.AddElement(ppi);

   const int index = AddListImage(hwndListView, ppi);

   ListView_SetItemState(hwndListView, index, LVIS_SELECTED, LVIS_SELECTED);
}

void PinTable::ListImages(HWND hwndListView)
{
   for (int i=0;i<m_vimage.Size();i++)
   {
      AddListImage(hwndListView, m_vimage.ElementAt(i));
   }
}

int PinTable::AddListImage(HWND hwndListView, Texture *ppi)
{
   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = ppi->m_szName;
   lvitem.lParam = (long)ppi;

   const int index = ListView_InsertItem(hwndListView, &lvitem);

   ListView_SetItemText(hwndListView, index, 1, ppi->m_szPath);

   return index;
}

void PinTable::RemoveImage(Texture *ppi)
{
   m_vimage.RemoveElement(ppi);
   delete ppi;
}

int PinTable::GetImageLink(Texture *ppi)
{
   if (!lstrcmp(ppi->m_szInternalName, m_szScreenShot))
   {
      return 1;
   }

   return 0;
}

PinBinary *PinTable::GetImageLinkBinary(int id)
{
   switch (id)
   {
   case 1: //Screenshot
      // Transfer ownership of the screenshot pinbary blob to the image
      PinBinary * const pbT = m_pbTempScreenshot;
      m_pbTempScreenshot = NULL;
      return pbT;
      break;
   }

   return NULL;
}

void PinTable::ListCustomInfo(HWND hwndListView)
{
   for (int i=0;i<m_vCustomInfoTag.Size();i++)
      AddListItem(hwndListView, m_vCustomInfoTag.ElementAt(i), m_vCustomInfoContent.ElementAt(i), NULL);
}

int PinTable::AddListItem(HWND hwndListView, char *szName, char *szValue1, LPARAM lparam)
{
   LVITEM lvitem;
   lvitem.mask = LVIF_DI_SETITEM | LVIF_TEXT | LVIF_PARAM;
   lvitem.iItem = 0;
   lvitem.iSubItem = 0;
   lvitem.pszText = szName;
   lvitem.lParam = lparam;

   const int index = ListView_InsertItem(hwndListView, &lvitem);

   ListView_SetItemText(hwndListView, index, 1, szValue1);

   return index;
}


HRESULT PinTable::LoadImageFromStream(IStream *pstm, int version)
{
   if (version < 100) // Tech Beta 3 and below
   {
       ShowError("Tables from Tech Beta 3 and below are not supported in this version.");
       return E_FAIL;
   }
   else
   {
      Texture * const ppi = new Texture(); 

      if (ppi->LoadFromStream(pstm, version, this) == S_OK)
      {
         m_vimage.AddElement(ppi);
      }
      else
      {
         delete ppi;
      }
   }

   return S_OK;
}

STDMETHODIMP PinTable::get_Image(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_szImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_Image(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szImage, 32, NULL, NULL);

   if (!g_pplayer)
   {
      CreateGDIBackdrop();
      SetDirtyDraw();
   }

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::GetPredefinedStrings(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut)
{
   return GetPredefinedStrings(dispID, pcaStringsOut, pcaCookiesOut, NULL);
}

STDMETHODIMP PinTable::GetPredefinedValue(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut)
{
   return GetPredefinedValue(dispID, dwCookie, pVarOut, NULL);
}

STDMETHODIMP PinTable::GetPredefinedStrings(DISPID dispID, CALPOLESTR *pcaStringsOut, CADWORD *pcaCookiesOut, IEditable *piedit)
{
   if (pcaStringsOut == NULL || pcaCookiesOut == NULL)
      return E_POINTER;

   WCHAR *wzDst;
   int cvar;
   WCHAR **rgstr;
   DWORD *rgdw;

   switch (dispID)
   {
   case DISPID_Image:
   case DISPID_Image2:
   case DISPID_Image3:
   case DISPID_Image4:
   case DISPID_Image5:
      {
         cvar = m_vimage.Size();

         rgstr = (WCHAR **) CoTaskMemAlloc((cvar+1) * sizeof(WCHAR *));
         rgdw = (DWORD *) CoTaskMemAlloc((cvar+1) * sizeof(DWORD));

         wzDst = (WCHAR *) CoTaskMemAlloc(7*sizeof(WCHAR));
         // TEXT
         LocalString ls(IDS_NONE);
         MultiByteToWideChar(CP_ACP, 0, ls.m_szbuffer, -1, wzDst, 7);
         rgstr[0] = wzDst;
         rgdw[0] = ~0u;

         for (int ivar = 0 ; ivar < cvar ; ivar++)
         {
            char *szSrc = m_vimage.ElementAt(ivar)->m_szName;
            DWORD cwch = lstrlen(szSrc)+1;
            wzDst = (WCHAR *) CoTaskMemAlloc(cwch*sizeof(WCHAR));
            if (wzDst == NULL)
               ShowError("DISPID_Image alloc failed");

            MultiByteToWideChar(CP_ACP, 0, szSrc, -1, wzDst, cwch);

            //MsoWzCopy(szSrc,szDst);
            rgstr[ivar+1] = wzDst;
            rgdw[ivar+1] = ivar;
         }
         cvar++;
      }
      break;

   case DISPID_Sound:
      {
         cvar = m_vsound.Size();

         rgstr = (WCHAR **) CoTaskMemAlloc((cvar+1) * sizeof(WCHAR *));
         rgdw = (DWORD *) CoTaskMemAlloc((cvar+1) * sizeof(DWORD));

         wzDst = (WCHAR *) CoTaskMemAlloc(7*sizeof(WCHAR));
         // TEXT
         MultiByteToWideChar(CP_ACP, 0, "<None>", -1, wzDst, 7);
         rgstr[0] = wzDst;
         rgdw[0] = ~0u;

         for (int ivar = 0 ; ivar < cvar ; ivar++)
         {
            char *szSrc = m_vsound.ElementAt(ivar)->m_szName;
            DWORD cwch = lstrlen(szSrc)+1;
            wzDst = (WCHAR *) CoTaskMemAlloc(cwch*sizeof(WCHAR));
            if (wzDst == NULL)
            {
               ShowError("DISPID_Sound alloc failed");
            }

            MultiByteToWideChar(CP_ACP, 0, szSrc, -1, wzDst, cwch);

            //MsoWzCopy(szSrc,szDst);
            rgstr[ivar+1] = wzDst;
            rgdw[ivar+1] = ivar;
         }
         cvar++;
      }
      break;

   case DISPID_Collection:
      {
         cvar = m_vcollection.Size();

         rgstr = (WCHAR **) CoTaskMemAlloc((cvar+1) * sizeof(WCHAR *));
         rgdw = (DWORD *) CoTaskMemAlloc((cvar+1) * sizeof(DWORD));

         wzDst = (WCHAR *) CoTaskMemAlloc(7*sizeof(WCHAR));
         // TEXT
         MultiByteToWideChar(CP_ACP, 0, "<None>", -1, wzDst, 7);
         rgstr[0] = wzDst;
         rgdw[0] = ~0u;

         for (int ivar = 0 ; ivar < cvar ; ivar++)
         {
            DWORD cwch = sizeof(m_vcollection.ElementAt(ivar)->m_wzName)+sizeof(DWORD); //!! +DWORD?
            wzDst = (WCHAR *) CoTaskMemAlloc(cwch);
            if (wzDst == NULL)
               ShowError("DISPID_Collection alloc failed (1)");
            else
               memcpy (wzDst, m_vcollection.ElementAt(ivar)->m_wzName, cwch);
            rgstr[ivar+1] = wzDst;
            rgdw[ivar+1] = ivar;
         }
         cvar++;
      }
      break;

   case DISPID_Surface:
      {
         cvar = 0;

         const BOOL fRamps = fTrue;

         for (int ivar = 0;ivar < m_vedit.Size();ivar++)
            if (m_vedit.ElementAt(ivar)->GetItemType() == eItemSurface || (fRamps && m_vedit.ElementAt(ivar)->GetItemType() == eItemRamp))
               cvar++;

         rgstr = (WCHAR **) CoTaskMemAlloc((cvar+1) * sizeof(WCHAR *));
         rgdw = (DWORD *) CoTaskMemAlloc((cvar+1) * sizeof(DWORD));

         cvar = 0;

         wzDst = (WCHAR *) CoTaskMemAlloc(7*sizeof(WCHAR));
         //MultiByteToWideChar(CP_ACP, 0, "None", -1, wzDst, 5);
         // TEXT
         WideStrCopy(L"<None>", wzDst);
         rgstr[cvar] = wzDst;
         rgdw[cvar] = ~0u;
         cvar++;

         for (int ivar = 0;ivar < m_vedit.Size();ivar++)
         {
            if (m_vedit.ElementAt(ivar)->GetItemType() == eItemSurface|| (fRamps && m_vedit.ElementAt(ivar)->GetItemType() == eItemRamp))
            {
               CComBSTR bstr;
#ifdef VBA
               m_vedit.ElementAt(ivar)->GetIApcProjectItem()->get_Name(&bstr);
#else
               m_vedit.ElementAt(ivar)->GetScriptable()->get_Name(&bstr);
#endif

               DWORD cwch = lstrlenW(bstr)+1;
               //wzDst = ::SysAllocString(bstr);

               wzDst = (WCHAR *) CoTaskMemAlloc(cwch*sizeof(WCHAR));
               if (wzDst == NULL)
                  ShowError("DISPID_Surface alloc failed (1)");

               WideStrCopy(bstr, wzDst);

               //MultiByteToWideChar(CP_ACP, 0, "Hello", -1, wzDst, cwch);

               //MsoWzCopy(szSrc,szDst);
               rgstr[cvar] = wzDst;
               rgdw[cvar] = ivar;
               cvar++;
            }
         }
      }
      break;

   default:
      return E_NOTIMPL;
   }

   pcaStringsOut->cElems = cvar;
   pcaStringsOut->pElems = rgstr;

   pcaCookiesOut->cElems = cvar;
   pcaCookiesOut->pElems = rgdw;

   return S_OK;
}

STDMETHODIMP PinTable::GetPredefinedValue(DISPID dispID, DWORD dwCookie, VARIANT *pVarOut, IEditable *piedit)
{
   WCHAR *wzDst;

   switch (dispID)
   {
   default:
      return E_NOTIMPL;

   case DISPID_Image:
   case DISPID_Image2:
   case DISPID_Image3:
   case DISPID_Image4:
   case DISPID_Image5:
      {
         if (dwCookie == -1)
         {
            wzDst = (WCHAR *) CoTaskMemAlloc(1*sizeof(WCHAR));
            wzDst[0] = L'\0';
         }
         else
         {
            char *szSrc = m_vimage.ElementAt(dwCookie)->m_szName;
            DWORD cwch = lstrlen(szSrc)+1;
            wzDst = (WCHAR *) CoTaskMemAlloc(cwch*sizeof(WCHAR));

            MultiByteToWideChar(CP_ACP, 0, szSrc, -1, wzDst, cwch);
         }
      }
      break;
   case DISPID_Sound:
      {
         if (dwCookie == -1)
         {
            wzDst = (WCHAR *) CoTaskMemAlloc(1*sizeof(WCHAR));
            wzDst[0] = L'\0';
         }
         else
         {
            char *szSrc = m_vsound.ElementAt(dwCookie)->m_szName;
            DWORD cwch = lstrlen(szSrc)+1;
            wzDst = (WCHAR *) CoTaskMemAlloc(cwch*sizeof(WCHAR));

            MultiByteToWideChar(CP_ACP, 0, szSrc, -1, wzDst, cwch);
         }
      }
      break;
   case DISPID_Collection:
      {
         if (dwCookie == -1)
         {
            wzDst = (WCHAR *) CoTaskMemAlloc(1*sizeof(WCHAR));
            wzDst[0] = L'\0';
         }
         else
         {
            DWORD cwch = sizeof(m_vcollection.ElementAt(dwCookie)->m_wzName)+sizeof(DWORD); //!! +DWORD?
            wzDst = (WCHAR *) CoTaskMemAlloc(cwch);
            if (wzDst == NULL)
               ShowError("DISPID_Collection alloc failed (2)");
            else
               memcpy (wzDst, m_vcollection.ElementAt(dwCookie)->m_wzName, cwch);
         }
      }
      break;
   case DISPID_Surface:
      {
         if (dwCookie == -1)
         {
            wzDst = (WCHAR *) CoTaskMemAlloc(1*sizeof(WCHAR));
            wzDst[0] = L'\0';
         }
         else
         {
            CComBSTR bstr;
#ifdef VBA
            m_vedit.ElementAt(dwCookie)->GetIApcProjectItem()->get_Name(&bstr);
#else
            m_vedit.ElementAt(dwCookie)->GetScriptable()->get_Name(&bstr);
#endif

            DWORD cwch = lstrlenW(bstr)+1;
            //wzDst = ::SysAllocString(bstr);

            wzDst = (WCHAR *) CoTaskMemAlloc(cwch*sizeof(WCHAR));
            if (wzDst == NULL)
            {
               ShowError("DISPID_Surface alloc failed (2)");
            }

            WideStrCopy(bstr, wzDst);
         }
      }
      break;
   }

   CComVariant var(wzDst);

   CoTaskMemFree(wzDst);

   return var.Detach(pVarOut);
}

float PinTable::GetSurfaceHeight(char *szName, float x, float y)
{
   if (szName == NULL || szName[0] == 0)
       return 0.0f;

   for (int i=0;i<m_vedit.Size();i++)
   {
      IEditable *item=m_vedit.ElementAt(i);
      if (item->GetItemType() == eItemSurface || item->GetItemType() == eItemRamp)
      {
         CComBSTR bstr;
#ifdef VBA
         item->GetIApcProjectItem()->get_Name(&bstr);
#else
         item->GetScriptable()->get_Name(&bstr);
#endif
         if (!WzSzStrCmp(bstr, szName))
         {
            IEditable * const piedit = item;
            switch (piedit->GetItemType())
            {
            case eItemSurface:
               return ((Surface *)piedit)->m_d.m_heighttop;
               break;

            case eItemRamp: {
               Ramp * const pramp = (Ramp *)piedit;

               Vector<RenderVertex> vvertex;
               pramp->GetRgVertex(&vvertex);

               const int cvertex = vvertex.Size();

               int iSeg;
               Vertex2D vOut;
               ClosestPointOnPolygon(vvertex, Vertex2D(x,y), &vOut, &iSeg, false);

               // Go through vertices (including iSeg itself) counting control points until iSeg
               float totallength = 0.f;
               float startlength = 0.f;
               float zheight = 0.f;

               if (iSeg == -1)
               {
                  //zheight = 0;
                  goto HeightError;
                  //return 0; // Object is not on ramp path
               }

               for (int i2=1;i2<cvertex;i2++)
               {
                  const float dx = vvertex.ElementAt(i2)->x - vvertex.ElementAt(i2-1)->x;
                  const float dy = vvertex.ElementAt(i2)->y - vvertex.ElementAt(i2-1)->y;
                  const float len = sqrtf(dx*dx + dy*dy);
                  if (i2 <= iSeg)
                  {
                     startlength += len;
                  }
                  totallength += len;
               }

               {
                  const float dx = vOut.x - vvertex.ElementAt(iSeg)->x;
                  const float dy = vOut.y - vvertex.ElementAt(iSeg)->y;
                  const float len = sqrtf(dx*dx + dy*dy);
                  startlength += len; // Add the distance the object is between the two closest polyline segments.  Matters mostly for straight edges.

                  zheight = (startlength/totallength) * (pramp->m_d.m_heighttop - pramp->m_d.m_heightbottom) + pramp->m_d.m_heightbottom;
               }
HeightError:
               for (int i2=0;i2<cvertex;i2++)
                  delete vvertex.ElementAt(i2);

               return zheight;
               }
            }
         }
      }
   }
   return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP PinTable::get_DisplayGrid(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_fGrid);

   return S_OK;
}

STDMETHODIMP PinTable::put_DisplayGrid(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_fGrid = VBTOF(newVal);

   SetDirtyDraw();

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_DisplayBackdrop(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_fBackdrop);

   return S_OK;
}

STDMETHODIMP PinTable::put_DisplayBackdrop(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_fBackdrop = VBTOF(newVal);

   SetDirtyDraw();

   STOPUNDO

   return S_OK;
}

INT_PTR CALLBACK ProgressProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_INITDIALOG:
      {
         RECT rcProgress;
         RECT rcMain;
         GetWindowRect(g_pvp->m_hwnd, &rcMain);
         GetWindowRect(hwndDlg, &rcProgress);

         SetWindowPos(hwndDlg, NULL,
            (rcMain.right + rcMain.left)/2 - (rcProgress.right - rcProgress.left)/2,
            (rcMain.bottom + rcMain.top)/2 - (rcProgress.bottom - rcProgress.top)/2,
            0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);
         return TRUE;
      }
      break;
   }

   return FALSE;
}

STDMETHODIMP PinTable::get_GlassHeight(float *pVal)
{
   *pVal = m_glassheight;

   return S_OK;
}

STDMETHODIMP PinTable::put_GlassHeight(float newVal)
{
   STARTUNDO

   m_glassheight = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_TableHeight(float *pVal)
{
   *pVal = m_tableheight;

   return S_OK;
}

STDMETHODIMP PinTable::put_TableHeight(float newVal)
{
   STARTUNDO

   m_tableheight = newVal;

   STOPUNDO

   return S_OK;
}
STDMETHODIMP PinTable::get_Width(float *pVal)
{
   *pVal = m_right;

   return S_OK;
}

STDMETHODIMP PinTable::put_Width(float newVal)
{
   STARTUNDO

   m_right = newVal;

   SetDirtyDraw();

   SetMyScrollInfo();

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Height(float *pVal)
{
   *pVal = m_bottom;

   return S_OK;
}

STDMETHODIMP PinTable::put_Height(float newVal)
{
   STARTUNDO

   m_bottom = newVal;

   SetDirtyDraw();

   SetMyScrollInfo();

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_PlayfieldColor(OLE_COLOR *pVal)
{
   *pVal = m_colorplayfield;

   return S_OK;
}

STDMETHODIMP PinTable::put_PlayfieldColor(OLE_COLOR newVal)
{
   STARTUNDO

   m_colorplayfield = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light0Ambient(OLE_COLOR *pVal)
{
   *pVal = m_Light[0].ambient;

   return S_OK;
}

STDMETHODIMP PinTable::put_Light0Ambient(OLE_COLOR newVal)
{
   STARTUNDO

   m_Light[0].ambient = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light0Diffuse(OLE_COLOR *pVal)
{
   *pVal = m_Light[0].diffuse;

   return S_OK;
}

STDMETHODIMP PinTable::put_Light0Diffuse(OLE_COLOR newVal)
{
   STARTUNDO

   m_Light[0].diffuse = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light0Specular(OLE_COLOR *pVal)
{
   *pVal = m_Light[0].specular;

   return S_OK;
}

STDMETHODIMP PinTable::put_Light0Specular(OLE_COLOR newVal)
{
   STARTUNDO

   m_Light[0].specular = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light0PX(float *pVal)
{
   *pVal = m_Light[0].pos.x;

   return S_OK;
}

STDMETHODIMP PinTable::put_Light0PX(float newVal)
{
   STARTUNDO

   m_Light[0].pos.x = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light0PY(float *pVal)
{
   *pVal = m_Light[0].pos.y;

   return S_OK;
}

STDMETHODIMP PinTable::put_Light0PY(float newVal)
{
   STARTUNDO

   m_Light[0].pos.y = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light0PZ(float *pVal)
{
   *pVal = -m_Light[0].pos.z; // transform from local

   return S_OK;
}

STDMETHODIMP PinTable::put_Light0PZ(float newVal)
{
   STARTUNDO

   m_Light[0].pos.z = -newVal; // transform to local

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light0DX(float *pVal)
{
   *pVal = m_Light[0].dir.x;

   return S_OK;
}

STDMETHODIMP PinTable::put_Light0DX(float newVal)
{
   STARTUNDO

   m_Light[0].dir.x = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light0DY(float *pVal)
{
   *pVal = m_Light[0].dir.y;

   return S_OK;
}

STDMETHODIMP PinTable::put_Light0DY(float newVal)
{
   STARTUNDO

   m_Light[0].dir.y = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light0DZ(float *pVal)
{
   *pVal = -m_Light[0].dir.z; // transform from local

   return S_OK;
}

STDMETHODIMP PinTable::put_Light0DZ(float newVal)
{
   STARTUNDO

   m_Light[0].dir.z = -newVal; // transform to local

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light0Type(int *pVal)
{
   *pVal = m_Light[0].type;

   return S_OK;
}

STDMETHODIMP PinTable::put_Light0Type(int newVal)
{
   STARTUNDO

   m_Light[0].type = (LightType)newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light1Ambient(OLE_COLOR *pVal)
{
   *pVal = m_Light[1].ambient;

   return S_OK;
}

STDMETHODIMP PinTable::put_Light1Ambient(OLE_COLOR newVal)
{
   STARTUNDO

   m_Light[1].ambient = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light1Diffuse(OLE_COLOR *pVal)
{
   *pVal = m_Light[1].diffuse;

   return S_OK;
}

STDMETHODIMP PinTable::put_Light1Diffuse(OLE_COLOR newVal)
{
   STARTUNDO

   m_Light[1].diffuse = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light1Specular(OLE_COLOR *pVal)
{
   *pVal = m_Light[1].specular;

   return S_OK;
}

STDMETHODIMP PinTable::put_Light1Specular(OLE_COLOR newVal)
{
   STARTUNDO

   m_Light[1].specular = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light1PX(float *pVal)
{
   *pVal = m_Light[1].pos.x;

   return S_OK;
}

STDMETHODIMP PinTable::put_Light1PX(float newVal)
{
   STARTUNDO

   m_Light[1].pos.x = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light1PY(float *pVal)
{
   *pVal = m_Light[1].pos.y;

   return S_OK;
}

STDMETHODIMP PinTable::put_Light1PY(float newVal)
{
   STARTUNDO

   m_Light[1].pos.y = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light1PZ(float *pVal)
{
   *pVal = -m_Light[1].pos.z; // transform from local

   return S_OK;
}

STDMETHODIMP PinTable::put_Light1PZ(float newVal)
{
   STARTUNDO

   m_Light[1].pos.z = -newVal; // transform to local

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light1DX(float *pVal)
{
   *pVal = m_Light[1].dir.x;

   return S_OK;
}

STDMETHODIMP PinTable::put_Light1DX(float newVal)
{
   STARTUNDO

   m_Light[1].dir.x = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light1DY(float *pVal)
{
   *pVal = m_Light[1].dir.y;

   return S_OK;
}

STDMETHODIMP PinTable::put_Light1DY(float newVal)
{
   STARTUNDO

   m_Light[1].dir.y = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light1DZ(float *pVal)
{
   *pVal = -m_Light[1].dir.z; // transform from local

   return S_OK;
}

STDMETHODIMP PinTable::put_Light1DZ(float newVal)
{
   STARTUNDO

   m_Light[1].dir.z = -newVal; // transform to local

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Light1Type(int *pVal)
{
   *pVal = m_Light[1].type;

   return S_OK;
}

STDMETHODIMP PinTable::put_Light1Type(int newVal)
{
   STARTUNDO

   m_Light[1].type = (LightType)newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_NormalizeNormals(int *pVal)
{
   *pVal = m_NormalizeNormals;

   return S_OK;
}

STDMETHODIMP PinTable::put_NormalizeNormals(int newVal )
{
   STARTUNDO

   m_NormalizeNormals = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BallReflection(int *pVal)
{
   *pVal = m_useReflectionForBalls;

   return S_OK;
}

STDMETHODIMP PinTable::put_BallReflection(int newVal )
{
   STARTUNDO

   m_useReflectionForBalls = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_ReflectionStrength(int *pVal)
{
   *pVal = m_ballReflectionStrength;

   return S_OK;
}

STDMETHODIMP PinTable::put_ReflectionStrength(int newVal )
{
   STARTUNDO

   m_ballReflectionStrength = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_ShadowX(float *pVal)
{
   *pVal = m_shadowDirX;

   return S_OK;
}

STDMETHODIMP PinTable::put_ShadowX(float newVal )
{
   STARTUNDO

   if ( newVal>1.0f ) newVal=1.0f;
   if ( newVal<-1.0f ) newVal=-1.0f;
   m_shadowDirX = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_ShadowY(float *pVal)
{
   *pVal = m_shadowDirY;

   return S_OK;
}

STDMETHODIMP PinTable::put_ShadowY(float newVal )
{
   STARTUNDO

   if ( newVal>1.0f ) newVal=1.0f;
   if ( newVal<-1.0f ) newVal=-1.0f;
   m_shadowDirY = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BallTrail(int *pVal)
{
   *pVal = m_useTrailForBalls;

   return S_OK;
}

STDMETHODIMP PinTable::put_BallTrail(int newVal )
{
   STARTUNDO

   m_useTrailForBalls = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_TrailStrength(int *pVal)
{
   *pVal = m_ballTrailStrength;

   return S_OK;
}

STDMETHODIMP PinTable::put_TrailStrength(int newVal )
{
   STARTUNDO

   m_ballTrailStrength = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_TableSoundVolume(int *pVal)
{
   *pVal = (int)(m_TableSoundVolume*100.0f);

   return S_OK;
}

STDMETHODIMP PinTable::put_TableSoundVolume(int newVal )
{
   STARTUNDO

   m_TableSoundVolume = (float)newVal/100.0f;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_AlphaRampAccuracy(int *pVal)
{
    if( m_overwriteGlobalAlphaRampsAccuracy )
        *pVal = m_userAlphaRampsAccuracy;
    else
        *pVal = m_globalAlphaRampsAccuracy;

   return S_OK;
}

STDMETHODIMP PinTable::put_AlphaRampAccuracy(int newVal )
{
   STARTUNDO

   if( m_overwriteGlobalAlphaRampsAccuracy )
   {
       m_userAlphaRampsAccuracy = newVal;
   }

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_GlobalAlphaAcc(VARIANT_BOOL *pVal)
{
    *pVal = (VARIANT_BOOL)FTOVB(m_overwriteGlobalAlphaRampsAccuracy);

    return S_OK;
}

STDMETHODIMP PinTable::put_GlobalAlphaAcc(VARIANT_BOOL newVal )
{
    STARTUNDO

    m_overwriteGlobalAlphaRampsAccuracy = VBTOF(newVal);
    if ( !m_overwriteGlobalAlphaRampsAccuracy )
    {
        m_userAlphaRampsAccuracy = m_globalAlphaRampsAccuracy;
    }

	STOPUNDO

    return S_OK;
}

STDMETHODIMP PinTable::get_GlobalStereo3D(VARIANT_BOOL *pVal)
{
    *pVal = (VARIANT_BOOL)FTOVB(m_overwriteGlobalStereo3D);

    return S_OK;
}

STDMETHODIMP PinTable::put_GlobalStereo3D(VARIANT_BOOL newVal )
{
    STARTUNDO

    m_overwriteGlobalStereo3D = VBTOF(newVal);
    if ( !m_overwriteGlobalStereo3D )
    {
		m_maxSeparation = m_globalMaxSeparation;
		m_ZPD = m_globalZPD;
    }

	STOPUNDO

    return S_OK;
}

STDMETHODIMP PinTable::get_TableMusicVolume(int *pVal)
{
   *pVal = (int)(m_TableMusicVolume*100.0f);

   return S_OK;
}

STDMETHODIMP PinTable::put_TableMusicVolume(int newVal )
{
   STARTUNDO

   m_TableMusicVolume = (float)newVal/100.0f;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_TableAdaptiveVSync(int *pVal)
{
   *pVal = m_TableAdaptiveVSync;

   return S_OK;
}

STDMETHODIMP PinTable::put_TableAdaptiveVSync(int newVal )
{
   STARTUNDO

   m_TableAdaptiveVSync = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BackdropColor(OLE_COLOR *pVal)
{
   *pVal = m_colorbackdrop;

   return S_OK;
}

STDMETHODIMP PinTable::put_BackdropColor(OLE_COLOR newVal)
{
   STARTUNDO

   m_colorbackdrop = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BackdropImage(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_szImageBackdrop, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_BackdropImage(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szImageBackdrop, 32, NULL, NULL);

   if (!g_pplayer)
   {
      CreateGDIBackdrop();
      SetDirtyDraw();
   }

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Gravity(float *pVal)
{
   *pVal = m_Gravity*(float)(1.0/GRAVITYCONST);

   return S_OK;
}

STDMETHODIMP PinTable::put_Gravity(float newVal )
{
   if (newVal < 0) newVal = 0;

   if (g_pplayer)
   {
      m_Gravity = newVal*GRAVITYCONST;
      const float slope = m_angletiltMin + (m_angletiltMax - m_angletiltMin)* m_globalDifficulty;
      g_pplayer->SetGravity(slope, m_fOverridePhysics ? m_fOverrideGravityConstant : m_Gravity);
   }
   else
   {
      STARTUNDO
      m_Gravity = newVal*GRAVITYCONST;
      STOPUNDO
   }	
   return S_OK;
}

STDMETHODIMP PinTable::get_HardFriction(float *pVal)
{
   *pVal = m_hardFriction;

   return S_OK;
}

STDMETHODIMP PinTable::put_HardFriction(float newVal )
{	
   STARTUNDO

   m_hardFriction = clamp(newVal, 0.0f, 1.0f);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_HardScatter(float *pVal)
{
   *pVal = RADTOANG(m_hardScatter);

   return S_OK;
}

STDMETHODIMP PinTable::put_HardScatter(float newVal )
{
   STARTUNDO

   m_hardScatter = ANGTORAD(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_NudgeTime(float *pVal)
{
   *pVal = m_nudgeTime;

   return S_OK;
}

STDMETHODIMP PinTable::put_NudgeTime(float newVal )
{
   STARTUNDO
   m_nudgeTime = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_PlungerNormalize(int *pVal)
{
   *pVal = m_plungerNormalize;

   return S_OK;
}

STDMETHODIMP PinTable::put_PlungerNormalize(int newVal )
{
   STARTUNDO

   m_plungerNormalize = newVal;
   GetRegInt("Player", "PlungerNormalize", &m_plungerNormalize);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_PlungerFilter(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_plungerFilter);
   return S_OK;
}

STDMETHODIMP PinTable::put_PlungerFilter(VARIANT_BOOL newVal )
{
   STARTUNDO

   BOOL tmp = VBTOF(newVal);
   GetRegInt("Player", "PlungerFilter", &tmp);
   m_plungerFilter = (tmp != 0);

   STOPUNDO
   return S_OK;
}

STDMETHODIMP PinTable::get_PhysicsLoopTime(int *pVal)
{
   *pVal = m_PhysicsMaxLoops;

   return S_OK;
}

STDMETHODIMP PinTable::put_PhysicsLoopTime(int newVal )
{
   STARTUNDO

   m_PhysicsMaxLoops = newVal;

   STOPUNDO

   return S_OK;
}


STDMETHODIMP PinTable::get_Scalex(float *pVal)
{
   *pVal = m_scalex;

   return S_OK;
}

STDMETHODIMP PinTable::put_Scalex(float newVal)
{
   STARTUNDO

   m_scalex = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Scaley(float *pVal)
{
   *pVal = m_scaley;

   return S_OK;
}

STDMETHODIMP PinTable::put_Scaley(float newVal)
{
   STARTUNDO

   m_scaley = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Scalez(float *pVal)
{
   *pVal = m_zScale;

   return S_OK;
}

STDMETHODIMP PinTable::put_Scalez(float newVal)
{
   STARTUNDO

      m_zScale = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_Xlatex(float *pVal)
{
   *pVal = m_xlatex;

   return S_OK;
}

STDMETHODIMP PinTable::put_Xlatex(float newVal)
{
   STARTUNDO

   m_xlatex = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Xlatey(float *pVal)
{
   *pVal = m_xlatey;

   return S_OK;
}

STDMETHODIMP PinTable::put_Xlatey(float newVal)
{
   STARTUNDO

   m_xlatey = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_Xlatez(float *pVal)
{
   *pVal = m_xlatez;

   return S_OK;
}

STDMETHODIMP PinTable::put_Xlatez(float newVal)
{
   STARTUNDO

      m_xlatez = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP PinTable::get_Rotation(float *pVal)
{
   *pVal = m_rotation;

   return S_OK;
}

STDMETHODIMP PinTable::put_Rotation(float newVal)
{
   STARTUNDO

   m_rotation = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_SlopeMax(float *pVal)
{
   if (m_angletiltMax == 726.0f) m_angletiltMax = m_angletiltMin;
   *pVal = m_angletiltMax;

   return S_OK;
}

STDMETHODIMP PinTable::put_SlopeMax(float newVal)
{
   if (g_pplayer)
   {
      m_angletiltMax = newVal;
      const float slope = m_angletiltMin + (m_angletiltMax - m_angletiltMin)* m_globalDifficulty;
      g_pplayer->SetGravity(slope, m_fOverridePhysics ? m_fOverrideGravityConstant : m_Gravity);
   }
   else
   {
      STARTUNDO
      m_angletiltMax = newVal;
      STOPUNDO
   }
   return S_OK;
}


STDMETHODIMP PinTable::get_SlopeMin(float *pVal)
{
   *pVal = m_angletiltMin;

   return S_OK;
}

STDMETHODIMP PinTable::put_SlopeMin(float newVal)
{
   if (g_pplayer)
   {
      m_angletiltMin = newVal;
      const float slope = m_angletiltMin + (m_angletiltMax - m_angletiltMin)* m_globalDifficulty;
      g_pplayer->SetGravity(slope, m_fOverridePhysics ? m_fOverrideGravityConstant : m_Gravity);
   }
   else
   {
      STARTUNDO
      m_angletiltMin = newVal;
      STOPUNDO
   }
   return S_OK;
}

STDMETHODIMP PinTable::get_BallImage(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_szBallImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_BallImage(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szBallImage, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_YieldTime(long *pVal)
{
   if (!g_pplayer)
   {
      *pVal = NULL;
      return E_FAIL;
   }
   else
      *pVal = g_pplayer->m_sleeptime;

   return S_OK;
}

STDMETHODIMP PinTable::put_YieldTime(long newVal)
{
   if (!g_pplayer)
      return E_FAIL;
   else
   {
      g_pplayer->m_sleeptime = newVal;
      if (g_pplayer->m_sleeptime > 1000)
         g_pplayer->m_sleeptime = 1000;
   }

   return S_OK;
}

STDMETHODIMP PinTable::get_RenderShadows(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_fRenderShadows);

   return S_OK;
}

STDMETHODIMP PinTable::put_RenderShadows(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_fRenderShadows = VBTOF(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_EnableAntialiasing(int *pVal)
{
   *pVal = m_useAA;

   return S_OK;
}

STDMETHODIMP PinTable::put_EnableAntialiasing(int newVal)
{
   STARTUNDO
   m_useAA = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_EnableFXAA(int *pVal)
{
   *pVal = m_useFXAA;

   return S_OK;
}

STDMETHODIMP PinTable::put_EnableFXAA(int newVal)
{
   STARTUNDO
   m_useFXAA = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_OverridePhysics(long *pVal)
{
   *pVal = m_fOverridePhysics;

   return S_OK;
}

STDMETHODIMP PinTable::put_OverridePhysics(long newVal)
{
   STARTUNDO
   m_fOverridePhysics = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::ImportPhysics()
{
	char szFileName[1024];
	char szInitialDir[1024];
	szFileName[0] = '\0';

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = g_hinst;
	ofn.hwndOwner = g_pvp->m_hwnd;
	// TEXT
	ofn.lpstrFilter = "Visual Pinball Physics (*.vpp)\0*.vpp\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrDefExt = "vpp";
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

	const HRESULT hr = GetRegString("RecentDir","LoadDir", szInitialDir, 1024);
	char szFoo[MAX_PATH];
	if (hr == S_OK)
	{
		ofn.lpstrInitialDir = szInitialDir;
	}
	else
	{
		lstrcpy(szFoo, "c:\\");
		ofn.lpstrInitialDir = szFoo;
	}

#ifdef VBA
	ApcHost->BeginModalDialog();
#endif
	const int ret = GetOpenFileName(&ofn);
#ifdef VBA
	ApcHost->EndModalDialog();
#endif
	if(ret == 0)
		return S_OK;

	FILE *f;
	fopen_s(&f,ofn.lpstrFile,"r");
	if( !f )
		return S_OK;

	float FlipperPhysicsSpeed,FlipperPhysicsStrength,FlipperPhysicsElasticity,FlipperPhysicsScatter,FlipperPhysicsReturnStrength,FlipperPhysicsRecoil,FlipperPhysicsPowerLaw,FlipperPhysicsOblique;
	fscanf_s(f,"%f %f %f %f %f %f %f %f\n", &FlipperPhysicsSpeed,&FlipperPhysicsStrength,&FlipperPhysicsElasticity,&FlipperPhysicsScatter,&FlipperPhysicsReturnStrength,&FlipperPhysicsRecoil,&FlipperPhysicsPowerLaw,&FlipperPhysicsOblique);
	float TablePhysicsGravityConstant,TablePhysicsContactFriction,TablePhysicsContactScatterAngle;
	fscanf_s(f,"%f %f %f\n", &TablePhysicsGravityConstant,&TablePhysicsContactFriction,&TablePhysicsContactScatterAngle);
	//char tmp2[256]; // not used here
	//fscanf_s(f,"%s",tmp2);
	fclose(f);

	for (int i=0;i<m_vedit.Size();i++)
		if (m_vedit.ElementAt(i)->GetItemType() == eItemFlipper)
		{
			Flipper * const flipper = (Flipper *)m_vedit.ElementAt(i);
			flipper->put_Speed(FlipperPhysicsSpeed);
			flipper->put_Strength(FlipperPhysicsStrength);
			flipper->put_Elasticity(FlipperPhysicsElasticity);
			flipper->put_ScatterAngle(FlipperPhysicsScatter);
			flipper->put_Return(FlipperPhysicsReturnStrength);
			flipper->put_Recoil(FlipperPhysicsRecoil);
			flipper->put_PowerLaw(FlipperPhysicsPowerLaw);
			flipper->put_ObliqueCorrection(FlipperPhysicsOblique);
		}

	put_Gravity(TablePhysicsGravityConstant);
	put_HardFriction(TablePhysicsContactFriction);
	put_HardScatter(TablePhysicsContactScatterAngle);

	return S_OK;
}

STDMETHODIMP PinTable::ExportPhysics()
{
	bool foundflipper = false;
	int i;
	for (i=0;i<m_vedit.Size();i++)
	{
		if (m_vedit.ElementAt(i)->GetItemType() == eItemFlipper)
		{
			foundflipper = true;
			break;
		}
	}

	if(!foundflipper)
	{
		ShowError("No Flipper found to copy settings from");
		return S_OK;
	}

	Flipper * const flipper = (Flipper *)m_vedit.ElementAt(i);

	char szFileName[1024];
	char szInitialDir[1024];
	szFileName[0] = '\0';

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = g_hinst;
	ofn.hwndOwner = g_pvp->m_hwnd;
	// TEXT
	ofn.lpstrFilter = "Visual Pinball Physics (*.vpp)\0*.vpp\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrDefExt = "vpp";
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

	const HRESULT hr = GetRegString("RecentDir","LoadDir", szInitialDir, 1024);
	char szFoo[MAX_PATH];
	if (hr == S_OK)
	{
		ofn.lpstrInitialDir = szInitialDir;
	}
	else
	{
		lstrcpy(szFoo, "c:\\");
		ofn.lpstrInitialDir = szFoo;
	}

#ifdef VBA
	ApcHost->BeginModalDialog();
#endif
	const int ret = GetSaveFileName(&ofn);
#ifdef VBA
	ApcHost->EndModalDialog();
#endif
	if(ret == 0)
		return S_OK;

	FILE *f;
	fopen_s(&f,ofn.lpstrFile,"w");
	if( !f )
		return S_OK;

	float val;

	flipper->get_Speed(&val);
	fprintf_s(f,"%f ",val);

	flipper->get_Strength(&val);
	fprintf_s(f,"%f ",val);

	flipper->get_Elasticity(&val);
	fprintf_s(f,"%f ",val);

	flipper->get_ScatterAngle(&val);
	fprintf_s(f,"%f ",val);

	flipper->get_Return(&val);
	fprintf_s(f,"%f ",val);

	flipper->get_Recoil(&val);
	fprintf_s(f,"%f ",val);

	flipper->get_PowerLaw(&val);
	fprintf_s(f,"%f ",val);

	flipper->get_ObliqueCorrection(&val);
	fprintf_s(f,"%f\n",val);


	get_Gravity(&val);
	fprintf_s(f,"%f ",val);

	get_HardFriction(&val);
	fprintf_s(f,"%f ",val);

	get_HardScatter(&val);
	fprintf_s(f,"%f ",val);


	fprintf_s(f,"%s",m_szTitle);

	fclose(f);

	return S_OK;
}

STDMETHODIMP PinTable::get_EnableDecals(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_fRenderDecals);

   return S_OK;
}

STDMETHODIMP PinTable::put_EnableDecals(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_fRenderDecals = VBTOF(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_EnableEMReels(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_fRenderEMReels);

   return S_OK;
}

STDMETHODIMP PinTable::put_EnableEMReels(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_fRenderEMReels = VBTOF(newVal);
   STOPUNDO

   return S_OK;
}
//////////////////////// uShock Accelerometer controls ///////////////////

STDMETHODIMP PinTable::get_GlobalDifficulty(float *pVal)
{
   *pVal = m_globalDifficulty;						//VP Editor

   return S_OK;
}

STDMETHODIMP PinTable::put_GlobalDifficulty(float newVal)
{
   if (!g_pplayer)
   {						//VP Editor
      int tmp;
      const HRESULT hr = GetRegInt("Player", "GlobalDifficulty", &tmp);
      if (hr == S_OK) m_globalDifficulty = (float)tmp*(float)(1.0/100.0);
      else 
      {
         STARTUNDO
         if (newVal < 0) newVal = 0;
            else if (newVal > 1.0f) newVal = 1.0f;

         m_globalDifficulty = newVal;
         STOPUNDO
      }
   }
   return S_OK;
}

STDMETHODIMP PinTable::get_AlternateRender(VARIANT_BOOL *pVal)      // TODO: remove
{
   *pVal = (VARIANT_BOOL)FTOVB(false);
   return S_OK;
}

STDMETHODIMP PinTable::put_AlternateRender(VARIANT_BOOL newVal)     // TODO: remove
{
   return S_OK;
}

STDMETHODIMP PinTable::get_HardwareRender(VARIANT_BOOL *pVal)
{
	//!! deprecated
    *pVal = (VARIANT_BOOL)FTOVB((g_pvp) ? true : false); //VP Editor

    return S_OK;
}

STDMETHODIMP PinTable::put_HardwareRender(VARIANT_BOOL newVal)
{
	//!! deprecated
	return S_OK;
}

STDMETHODIMP PinTable::get_Accelerometer(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB((g_pplayer) ? g_pplayer->m_fAccelerometer : m_tblAccelerometer); //VB Script or VP Editor

   return S_OK;
}

STDMETHODIMP PinTable::put_Accelerometer(VARIANT_BOOL newVal)
{
   if (g_pplayer) g_pplayer->m_fAccelerometer = VBTOF(newVal); //VB Script
   else
   {														//VP Editor
      const HRESULT hr = GetRegInt("Player", "PBWEnabled", &m_tblAccelerometer);
      if (hr == S_OK) m_tblAccelerometer = ((BOOL)m_tblAccelerometer != fFalse);
      else
      {
         STARTUNDO
         m_tblAccelerometer = VBTOF(newVal);
         STOPUNDO
      }
   }
   return S_OK;
}

STDMETHODIMP PinTable::get_AccelNormalMount(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB((g_pplayer) ? g_pplayer->m_AccelNormalMount : m_tblAccelNormalMount); //VB Script or VP Editor

   return S_OK;
}

STDMETHODIMP PinTable::put_AccelNormalMount(VARIANT_BOOL newVal)
{
   if (g_pplayer) g_pplayer->m_AccelNormalMount = VBTOF(newVal); //VB Script
   else 
   {			
      const HRESULT hr = GetRegInt("Player", "PBWNormalMount", &m_tblAccelNormalMount);
      if (hr == S_OK) m_tblAccelNormalMount = ((BOOL)m_tblAccelNormalMount != fFalse);
      else
      {
         STARTUNDO
         m_tblAccelNormalMount = VBTOF(newVal);
         STOPUNDO
      }
   }
   return S_OK;
}

STDMETHODIMP PinTable::get_AccelerometerAngle(float *pVal)
{
   *pVal = (g_pplayer) ? RADTOANG(g_pplayer->m_AccelAngle) : m_tblAccelAngle; //VB Script convert to radians or VP Editor in degrees

   return S_OK;
}

STDMETHODIMP PinTable::put_AccelerometerAngle(float newVal)
{
   if (g_pplayer) g_pplayer->m_AccelAngle = ANGTORAD(newVal); //VB Script conert to radians
   else
   {	//VP Editor in degrees
      int tmp;
      const HRESULT hr = GetRegInt("Player", "PBWRotation", &tmp);
      if (hr == S_OK) m_tblAccelAngle = (float)tmp;
      else 
      {		
         STARTUNDO
         m_tblAccelAngle = newVal;
         STOPUNDO
      }
   }
   return S_OK;
}

STDMETHODIMP PinTable::get_AccelerometerAmp(float *pVal)
{
   *pVal = (g_pplayer) ? g_pplayer->m_AccelAmp : m_tblAccelAmp; //VB Script or VP Editor

   return S_OK;
}

STDMETHODIMP PinTable::put_AccelerometerAmp(float newVal)
{
   if (g_pplayer) g_pplayer->m_AccelAmp = newVal; //VB Script
   else
   {						//VP Editor
      int tmp;
      const HRESULT hr = GetRegInt("Player", "PBWAccelGain", &tmp);
      if (hr == S_OK) m_tblAccelAmp = (float)tmp*(float)(1.0/100.0);
      else 
      {
         STARTUNDO
         m_tblAccelAmp = newVal;
         STOPUNDO
      }
   }
   return S_OK;
}

STDMETHODIMP PinTable::get_AccelerometerAmpX(float *pVal)
{
   *pVal = (g_pplayer) ? g_pplayer->m_AccelAmpX : m_tblAccelAmpX; //VB Script or VP Editor

   return S_OK;
}

STDMETHODIMP PinTable::put_AccelerometerAmpX(float newVal)
{
   if (g_pplayer) g_pplayer->m_AccelAmpX = newVal; //VB Script
   else
   {						//VP Editor
      int tmp;
      const HRESULT hr = GetRegInt("Player", "PBWAccelGainX", &tmp);
      if (hr == S_OK) m_tblAccelAmpX = (float)tmp*(float)(1.0/100.0);
      else 
      {
         STARTUNDO
         m_tblAccelAmpX = newVal;
         STOPUNDO
      }
   }
   return S_OK;
}

STDMETHODIMP PinTable::get_AccelerometerAmpY(float *pVal)
{
   *pVal = (g_pplayer) ? g_pplayer->m_AccelAmpY : m_tblAccelAmpY; //VB Script or VP Editor

   return S_OK;
}

STDMETHODIMP PinTable::put_AccelerometerAmpY(float newVal)
{
   if (g_pplayer) g_pplayer->m_AccelAmpY = newVal; //VB Script
   else
   {						//VP Editor
      int tmp;
      const HRESULT hr = GetRegInt("Player", "PBWAccelGainY", &tmp);
      if (hr == S_OK) m_tblAccelAmpY = (float)tmp*(float)(1.0/100.0);
      else 
      {
         STARTUNDO
         m_tblAccelAmpY = newVal;
         STOPUNDO
      }
   }
   return S_OK;
}


STDMETHODIMP PinTable::get_AccelerManualAmp(float *pVal)
{
   *pVal = (g_pplayer) ? g_pplayer->m_AccelMAmp : m_tblAccelManualAmp; //VB Script or VP Editor

   return S_OK;
}

STDMETHODIMP PinTable::put_AccelerManualAmp(float newVal)
{
   if (g_pplayer) g_pplayer->m_AccelMAmp = newVal; //VB Script
   else
   {						//VP Editor		
      int tmp;
      const HRESULT hr = GetRegInt("Player", "JoystickGain", &tmp);
      if (hr == S_OK) m_tblAccelManualAmp = (float)tmp*(float)(1.0/100.0);
      else
      {
         STARTUNDO
         m_tblAccelManualAmp = newVal;
         STOPUNDO
      }
   }
   return S_OK;
}

///////////////////////////////////////////////////////////
STDMETHODIMP PinTable::get_DeadSlider(int *pVal)
{
   int deadz;
   const HRESULT hr = GetRegInt("Player", "DeadZone", &deadz);
   if (hr != S_OK)
	   deadz = 0; // The default
   *pVal = deadz;

   return S_OK;
}

STDMETHODIMP PinTable::put_DeadSlider(int newVal)
{
   if (newVal>100) newVal=100;
   if (newVal<0) newVal=0;

   SetRegValue("Player", "DeadZone", REG_DWORD, &newVal, 4);

   return S_OK;
}

STDMETHODIMP PinTable::get_DeadZone(int *pVal)
{
   int deadz;
   const HRESULT hr = GetRegInt("Player", "DeadZone", &deadz);
   if (hr != S_OK)
	   deadz = 0;

   *pVal = deadz;

   return S_OK;
}

STDMETHODIMP PinTable::put_DeadZone(int newVal)
{
   if (newVal>100) newVal=100;
   if (newVal<0) newVal=0;

   SetRegValue("Player", "DeadZone", REG_DWORD, &newVal, 4);

   return S_OK;
}

STDMETHODIMP PinTable::get_JoltAmount(int *pVal)
{
   *pVal = (g_pplayer) ? g_pplayer->m_jolt_amount : m_jolt_amount;	//VB Script or VP Editor

   return S_OK;
}

STDMETHODIMP PinTable::put_JoltAmount(int newVal)
{
   if (g_pplayer) g_pplayer->m_jolt_amount = (U32)newVal; //VB Script
   else
   {						//VP Editor				
      const HRESULT hr = GetRegInt("Player", "JoltAmount", &m_jolt_amount);
      if (hr != S_OK) 
      {
         STARTUNDO
         m_jolt_amount = newVal;
         STOPUNDO
      }
   }
   return S_OK;
}

STDMETHODIMP PinTable::get_TiltAmount(int *pVal)
{
   *pVal = (g_pplayer) ? g_pplayer->m_tilt_amount : m_tilt_amount; //VB Script or VP Editor

   return S_OK;
}

STDMETHODIMP PinTable::put_TiltAmount(int newVal)
{
   if (g_pplayer) g_pplayer->m_tilt_amount = (U32)newVal; //VB Script
   else
   {						//VP Editor		
      const HRESULT hr = GetRegInt("Player", "TiltAmount", &m_tilt_amount);
      if (hr != S_OK) 
      {
         STARTUNDO
         m_tilt_amount = newVal;
         STOPUNDO
      }
   }
   return S_OK;
}

STDMETHODIMP PinTable::get_JoltTriggerTime(int *pVal)
{
   *pVal = (g_pplayer) ? g_pplayer->m_jolt_trigger_time : m_jolt_trigger_time; //VB Script or VP Editor

   return S_OK;
}

STDMETHODIMP PinTable::put_JoltTriggerTime(int newVal)
{
   if (g_pplayer) g_pplayer->m_jolt_trigger_time = (U32)newVal; //VB Script
   else
   {						//VP Editor		
      const HRESULT hr = GetRegInt("Player", "JoltTriggerTime", &m_jolt_trigger_time);
      if (hr != S_OK)
      {
         STARTUNDO
         m_jolt_trigger_time = newVal;
         STOPUNDO
      }
   }
   return S_OK;
}

STDMETHODIMP PinTable::get_TiltTriggerTime(int *pVal)
{
   *pVal = (g_pplayer) ? g_pplayer->m_tilt_trigger_time : m_tilt_trigger_time; //VB Script or VP Editor

   return S_OK;
}

STDMETHODIMP PinTable::put_TiltTriggerTime(int newVal)
{
   if (g_pplayer) g_pplayer->m_tilt_trigger_time = (U32)newVal; //VB Script
   else
   {						//VP Editor		
      const HRESULT hr = GetRegInt("Player", "TiltTriggerTime", &m_tilt_trigger_time);
      if (hr != S_OK)
      {
         STARTUNDO
         m_tilt_trigger_time = newVal;
         STOPUNDO
      }
   }
   return S_OK;
}

STDMETHODIMP PinTable::get_BallFrontDecal(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_szBallImageFront, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_BallFrontDecal(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szBallImageFront, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::get_BallBackDecal(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_szBallImageBack, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP PinTable::put_BallBackDecal(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szBallImageBack, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP PinTable::FireKnocker(int Count)
{
   if (g_pplayer)
   {
      hid_knock( Count );
   }	
   return S_OK;
}

STDMETHODIMP PinTable::QuitPlayer(int CloseType)
{
   if( g_pplayer )
   {		
      g_pplayer->m_fCloseType = CloseType;
      ExitApp();		
   }

   return S_OK;
}

STDMETHODIMP PinTable::StartShake()
{
   m_Shake = true;	

   return S_OK;
}

STDMETHODIMP PinTable::StopShake()
{
   m_Shake = false;	

   return S_OK;
}

STDMETHODIMP PinTable::Version(int *pVal)
{
	*pVal = VP_VERSION_MAJOR*1000 + VP_VERSION_MINOR*100 + VP_VERSION_REV;
	return S_OK;
}
