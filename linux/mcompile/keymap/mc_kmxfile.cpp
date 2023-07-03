
#include "mc_kmxfile.h"
#include "helpers.h"
#include "u16.h"
#include <typeinfo>

int dummytest_mc_kmx_file(){
  std::cout<< " dummytest_mc_kmx_file    is available\t";
  return 0;
  }

KMX_BOOL VerifyKeyboard(LPKMX_BYTE filebase, KMX_DWORD sz);

LPKEYBOARD FixupKeyboard(PKMX_BYTE bufp, PKMX_BYTE base, KMX_DWORD dwFileSize);

/*void Err(wchar_t *s) {
	LogError(L"LoadKeyboard: %s, last error = %d\n", s, GetLastError());
}*/

PKMX_WCHAR StringOffset(PKMX_BYTE base, KMX_DWORD offset) {
  if(offset == 0) return NULL;
  return (PKMX_WCHAR)(base + offset);
}


LPKEYBOARD FixupKeyboard(PKMX_BYTE bufp, PKMX_BYTE base, KMX_DWORD dwFileSize) {

  UNREFERENCED_PARAMETER(dwFileSize);

  KMX_DWORD i, j;
  PCOMP_KEYBOARD ckbp = (PCOMP_KEYBOARD) base;
  PCOMP_GROUP cgp;
  PCOMP_STORE csp;
  PCOMP_KEY ckp;
  LPKEYBOARD kbp = (LPKEYBOARD) bufp;
  LPSTORE sp;
  LPGROUP gp;
  LPKEY kp;
	kbp->dpStoreArray = (LPSTORE) (base + ckbp->dpStoreArray);
	kbp->dpGroupArray = (LPGROUP) (base + ckbp->dpGroupArray);

	for(sp = kbp->dpStoreArray, csp = (PCOMP_STORE) sp, i = 0; i < kbp->cxStoreArray; i++, sp++, csp++)	{
    sp->dpName = StringOffset(base, csp->dpName);
		sp->dpString = StringOffset(base, csp->dpString);
	}

	for(gp = kbp->dpGroupArray, cgp = (PCOMP_GROUP) gp, i = 0; i < kbp->cxGroupArray; i++, gp++, cgp++)	{
    gp->dpName = StringOffset(base, cgp->dpName);
		gp->dpKeyArray = (LPKEY) (base + cgp->dpKeyArray);
		if(cgp->dpMatch != NULL) gp->dpMatch = (PKMX_WCHAR) (base + cgp->dpMatch);
		if(cgp->dpNoMatch != NULL) gp->dpNoMatch = (PKMX_WCHAR) (base + cgp->dpNoMatch);

		for(kp = gp->dpKeyArray, ckp = (PCOMP_KEY) kp, j = 0; j < gp->cxKeyArray; j++, kp++, ckp++) {
			kp->dpOutput = (PKMX_WCHAR) (base + ckp->dpOutput);
			kp->dpContext = (PKMX_WCHAR) (base + ckp->dpContext);
		}
	}
  return kbp;
}


KMX_BOOL LoadKeyboard(char* fileName, LPKEYBOARD *lpKeyboard) {
  std::cout << "##### LoadKeyboard of mcompile started #####\n";
  std::cout << "fileName: " <<fileName<< "\n";

  LPKMX_BYTE buf;
  FILE * fp;
  LPKEYBOARD kbp;
  PKMX_BYTE filebase;

  //DebugLog("Loading file '%s'",fileName); 				  //_S2 ToDo find replacement:   DebugLog

  if(!fileName || !lpKeyboard) {
	std::cout << "TODO: Replace Err(LBad Filename)\n" ;	//_S2 ToDo find replacement:   Err(L"Bad Filename");
	return FALSE;
  }

  fp = Open_File(fileName, "rb");

  if(fp == NULL)
  {
    std::cout << "Could not open file\n"; 						//_S2 ToDo find replacement: DebugLog("Could not open file");
    return FALSE;
  }
  else															                  // _S2 remove
  std::cout << "Could OPEN file"<<fp<<"\n";           // _S2 remove

	MyCout("##### Line 93",1);									        // _S2 remove

  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    MyCout("Could not fseek file",1);							    //_S2 ToDo find replacement:	DebugLog("Could not fseek file");
    return FALSE;
  }
  else
  MyCout("CCould  fseek file",1);								      // _S2 remove

MyCout("##### Line 103",1);
  auto sz = ftell(fp);
  if (sz < 0) {
    fclose(fp);
    return FALSE;
  }

MyCout("##### Line 110",1);								            // _S2 remove
  if (fseek(fp, 0, SEEK_SET) != 0) {
    fclose(fp);
    MyCout("Could not fseek(set) file",1);					  //_S2 ToDo find replacement:	DebugLog("Could not fseek(set) file");
    return FALSE;
  }

#ifdef KMX_64BIT
  // allocate enough memory for expanded data structure + original data.
  // Expanded data structure is double the size of data on disk (8-byte
  // pointers) - on disk the "pointers" are relative to the beginning of
  // the file.
  // We save the original data at the end of buf; we don't copy strings, so
  // those will remain in the location at the end of the buffer.
  buf = new KMX_BYTE[sz * 3];
#else
  buf = new KMX_BYTE[sz];
#endif

MyCout("#### Line 129 ",1);
  if(!buf)
  {
    fclose(fp);
    MyCout("Not allocmem",1);								//_S2 ToDo find replacement: DebugLog()"Not allocmem");		
															              // _S2 delete [] buf; ????
    return FALSE;
  }

#ifdef KMX_64BIT
  filebase = buf + sz*2;
#else
  filebase = buf;
#endif

  if (fread(filebase, 1, sz, fp) < (size_t) sz) {
    MyCout("Could not read file",1);        //_S2 ToDo find replacement: DebugLog("Could not read file");
    fclose(fp);
															              // _S2 delete [] buf; ????
    return FALSE;
  }

  fclose(fp);

MyCout("##### Line 153",1);

if(!VerifyKeyboard(filebase, sz)) {
	// Err(L"errVerifyKeyboard");             //_S2 ToDo find replacement: Err
	                                          // _S2 delete [] buf; ????
  return FALSE;
  }


  MyCout("##### Line 157",1);
  kbp = FixupKeyboard(buf, filebase,sz);
  MyCout("##### Line 159",1);


std::cout << "kbp: "<<kbp<< "\n";
  if(!kbp) {
	//Err(L"errFixupKeyboard");								//_S2 ToDo find replacement: Err
															              // _S2 delete [] buf; ????

  MyCout("##### errFixupKeyboard ",1);
	return FALSE;}


  MyCout("##### Line 171 ",1);
                                            //_S2 can go:
                                              /*if(kbp->dwIdentifier != FILEID_COMPILED) {
                                              Err(L"errNotFileID");
                                              delete[] buf;
                                              return FALSE;
                                            }*/

std::cout << "kbp->dwIdentifier: "<<kbp->dwIdentifier<< "\n";
/*
 if(kbp->dwIdentifier != FILEID_COMPILED) {
    delete [] buf;
    //MyCout("errNotFileID",1);								//_S2 ToDo find replacement: DebugLog("errNotFileID");
															                // _S2 delete [] buf; ????
    return FALSE;
  }*/
MyCout("##### Line 187",1);
	*lpKeyboard = kbp;
															                // _S2 delete [] buf; ????
	MyCout("##### LoadKeyboard of mcompile ended #####",1);
	return TRUE;
}

// _S2 Version for char16_t filename
KMX_BOOL LoadKeyboard(char16_t* fileName, LPKEYBOARD* lpKeyboard) {
  std::cout << "##### LoadKeyboard of mcompile started #####\n";
  std::cout << "fileName: " <<fileName << "\n";

  LPKMX_BYTE buf;
  FILE* fp;
  LPKEYBOARD kbp;
  PKMX_BYTE filebase;

  //DebugLog("Loading file '%s'",fileName); 				  //_S2 ToDo find replacement:   DebugLog

  if(!fileName || !lpKeyboard) {
       std::cout << "TODO: Replace Err(LBad Filename)\n";//_S2 ToDo find replacement:   Err(L"Bad Filename");
       return FALSE;
  }

  fp = Open_File((const KMX_WCHAR*)fileName, u"rb");

  if(fp == NULL) {
    std::cout << "Could not open file\n";          //_S2 ToDo find replacement: DebugLog("Could not open file");
    return FALSE;
  } else                                           // _S2 remove
    std::cout << "Could OPEN file" << fp << "\n";  // _S2 remove

   MyCout("##### Line 224", 1);                     // _S2 remove

  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    MyCout("Could not fseek file", 1);  //_S2 ToDo find replacement:	DebugLog("Could not fseek file");
    return FALSE;
  } else
    MyCout("CCould  fseek file", 1);  // _S2 remove

    MyCout("##### Line 234", 1);
  auto sz = ftell(fp);
  if (sz < 0) {
    fclose(fp);
    return FALSE;
  }

    MyCout("##### Line 241", 1);  // _S2 remove
  if (fseek(fp, 0, SEEK_SET) != 0) {
    fclose(fp);
    MyCout("Could not fseek(set) file", 1);  //_S2 ToDo find replacement:	DebugLog("Could not fseek(set) file");
    return FALSE;
  }

  // #ifdef KMX_64BIT
  //  allocate enough memory for expanded data structure + original data.
  //  Expanded data structure is double the size of data on disk (8-byte
  //  pointers) - on disk the "pointers" are relative to the beginning of
  //  the file.
  //  We save the original data at the end of buf; we don't copy strings, so
  //  those will remain in the location at the end of the buffer.
  //  buf = new KMX_BYTE[sz * 3];
  // #else
  buf = new KMX_BYTE[sz];
  // #endif

    MyCout("#### Line 260 ", 1);
  if (!buf) {
    fclose(fp);
    MyCout("Not allocmem", 1);  //_S2 ToDo find replacement: DebugLog()"Not allocmem");
                                // _S2 delete [] buf; ????
    return FALSE;
  }

  // #ifdef KMX_64BIT
  // ilebase = buf + sz*2;
  // #else
  filebase = buf;
  // #endif

  if (fread(filebase, 1, sz, fp) < (size_t)sz) {
    MyCout("Could not read file", 1);  //_S2 ToDo find replacement: DebugLog("Could not read file");
    fclose(fp);
    // _S2 delete [] buf; ????
    return FALSE;
  }

  fclose(fp);

  MyCout("##### Line 285", 1);
  ;
  KMX_DWORD sz_dw = (KMX_DWORD)sz;  //_S2
  size_t sz_t = (size_t)sz;  //_S2
  // shold call VerifyKeyboard_M of class
  // if(!VerifyKeyboard(filebase, sz_t)) {
  if (!VerifyKeyboard_S2(filebase, sz_t)) {
    MyCout("##### errVerifyKeyboard", 1);
    // Err(L"errVerifyKeyboard");             //_S2 ToDo find replacement: Err
    // _S2 delete [] buf; ????
    return FALSE;
  }

  MyCout("##### Line 297", 1);
  kbp = FixupKeyboard(buf, filebase, sz_dw);    // _S" changed from sz->sz_dw
  MyCout("##### Line 299", 1);

  std::cout << "kbp: " << kbp << "\n";
  if (!kbp) {
    // Err(L"errFixupKeyboard");								//_S2 ToDo find replacement: Err
    //  _S2 delete [] buf; ????

    MyCout("##### errFixupKeyboard ", 1);
    return FALSE;
  }

  MyCout("##### Line 311 ", 1);
  //_S2 can go:
  /*if(kbp->dwIdentifier != FILEID_COMPILED) {
  Err(L"errNotFileID");
  delete[] buf;
  return FALSE;
}*/

  std::cout << "kbp->dwIdentifier: " << kbp->dwIdentifier << " FILEID_COMPILED: " << FILEID_COMPILED << "\n";
  std::cout << "..xxxxx.\n";
  if (kbp->dwIdentifier != FILEID_COMPILED) {
    delete[] buf;
    MyCout("errNotFileID", 1);  //_S2 ToDo find replacement: DebugLog("errNotFileID");
    return FALSE;
  }
  MyCout("##### Line 327", 1);
  *lpKeyboard = kbp;
  // _S2 delete [] buf; ????
  MyCout("##### LoadKeyboard of mcompile ended #####", 1);
  return TRUE;
}

KMX_BOOL VerifyKeyboard(LPBYTE filebase, KMX_DWORD sz) {
  KMX_DWORD i;
  PCOMP_KEYBOARD ckbp = (PCOMP_KEYBOARD) filebase;
  PCOMP_STORE csp;

	// Check file version //

	if(ckbp->dwFileVersion < VERSION_MIN ||
	   ckbp->dwFileVersion > VERSION_MAX) {
		// Old or new version -- identify the desired program version //
			for(csp = (PCOMP_STORE)(filebase + ckbp->dpStoreArray), i = 0; i < ckbp->cxStoreArray; i++, csp++) {
				if(csp->dwSystemID == TSS_COMPILEDVERSION) {
					wchar_t buf2[256];
          if(csp->dpString == 0) {
					// _S2 wsprintf(buf2, L"errWrongFileVersion:NULL");
          MyCout("errWrongFileVersion",1);
          } else {
					  // _S2 wsprintf(buf2, L"errWrongFileVersion:%10.10ls", StringOffset(filebase, csp->dpString));

          MyCout("errWrongFileVersion-offset",1);
          }

          MyCout("err buf",1);
					// _S2 Err(buf2);
					return FALSE;
				}
		}

          MyCout("errWrongFileVersion",1);
		// _S2 Err(L"errWrongFileVersion");
		return FALSE;
	}
/**/
MyCout("will return true",1);
  return TRUE;
}


KMX_BOOL VerifyKeyboard_S2(LPBYTE filebase, KMX_DWORD sz) {
  KMX_DWORD i;
  PCOMP_KEYBOARD ckbp = (PCOMP_KEYBOARD)filebase;
  PCOMP_STORE csp;

  // Check file version //

  if (ckbp->dwFileVersion < VERSION_MIN || ckbp->dwFileVersion > VERSION_MAX) {
    // Old or new version -- identify the desired program version //
    for (csp = (PCOMP_STORE)(filebase + ckbp->dpStoreArray), i = 0; i < ckbp->cxStoreArray; i++, csp++) {
      if (csp->dwSystemID == TSS_COMPILEDVERSION) {
        wchar_t buf2[256];
        if (csp->dpString == 0) {
          // _S2 wsprintf(buf2, L"errWrongFileVersion:NULL");
          MyCout("errWrongFileVersion", 1);
        } else {
          // _S2 wsprintf(buf2, L"errWrongFileVersion:%10.10ls", StringOffset(filebase, csp->dpString));

          MyCout("errWrongFileVersion-offset", 1);
        }

        MyCout("err buf", 1);
        // _S2 Err(buf2);
        return FALSE;
      }
    }

    MyCout("errWrongFileVersion", 1);
    // _S2 Err(L"errWrongFileVersion");
    return FALSE;
  }
  /**/
  MyCout("will return true", 1);
  return TRUE;
}


//---------------------old----------------------------------------
/*
#include "pch.h"


static BOOL LoadKeyboardFile(LPSTR fileName, LPKEYBOARD *lpKeyboard);
BOOL VerifyKeyboard(LPBYTE filebase, DWORD sz);

LPKEYBOARD FixupKeyboard(PBYTE bufp, PBYTE base, DWORD dwFileSize);

void Err(wchar_t *s) {
	LogError(L"LoadKeyboard: %s, last error = %d\n", s, GetLastError());
}

BOOL LoadKeyboard(LPWSTR fileName, LPKEYBOARD *lpKeyboard) {
	DWORD sz;
	LPBYTE buf;
	HANDLE hFile;
	LPKEYBOARD kbp;
  PBYTE filebase;

	if(!fileName || !lpKeyboard) {
		Err(L"Bad Filename");
		return FALSE;
	}

	hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile == INVALID_HANDLE_VALUE) {
		Err(L"Could not open file");
		return FALSE;
	}

	sz = GetFileSize(hFile, NULL);

	buf = new BYTE[sz];

	if(!buf) {
		Err(L"Not allocmem");
		CloseHandle(hFile);
		return FALSE;
	}

  filebase = buf;

	if(!ReadFile(hFile, filebase, sz, &sz, NULL)) {
    Err(L"errReadFile");
    CloseHandle(hFile);
    delete[] buf;
    return FALSE;
  }
	CloseHandle(hFile);

	if(!VerifyKeyboard(filebase, sz)) {
    Err(L"errVerifyKeyboard");
    delete[] buf;
    return FALSE;
  }

	kbp = FixupKeyboard(buf, filebase, sz);
  if(!kbp) {
    Err(L"errFixupKeyboard");
    delete[] buf;
    return FALSE;
  }

	if(kbp->dwIdentifier != FILEID_COMPILED) {
    Err(L"errNotFileID");
    delete[] buf;
    return FALSE;
  }

	*lpKeyboard = kbp;
	return TRUE;
}

PWCHAR StringOffset(PBYTE base, DWORD offset) {
  if(offset == 0) return NULL;
  return (PWCHAR)(base + offset);
}

LPKEYBOARD FixupKeyboard(PBYTE bufp, PBYTE base, DWORD dwFileSize) {
  UNREFERENCED_PARAMETER(dwFileSize);

  DWORD i, j;
  PCOMP_KEYBOARD ckbp = (PCOMP_KEYBOARD) base;
  PCOMP_GROUP cgp;
  PCOMP_STORE csp;
  PCOMP_KEY ckp;
  LPKEYBOARD kbp = (LPKEYBOARD) bufp;
  LPSTORE sp;
  LPGROUP gp;
  LPKEY kp;

	kbp->dpStoreArray = (LPSTORE) (base + ckbp->dpStoreArray);
	kbp->dpGroupArray = (LPGROUP) (base + ckbp->dpGroupArray);

	for(sp = kbp->dpStoreArray, csp = (PCOMP_STORE) sp, i = 0; i < kbp->cxStoreArray; i++, sp++, csp++)	{
    sp->dpName = StringOffset(base, csp->dpName);
		sp->dpString = StringOffset(base, csp->dpString);
	}

	for(gp = kbp->dpGroupArray, cgp = (PCOMP_GROUP) gp, i = 0; i < kbp->cxGroupArray; i++, gp++, cgp++)	{
    gp->dpName = StringOffset(base, cgp->dpName);
		gp->dpKeyArray = (LPKEY) (base + cgp->dpKeyArray);
		if(cgp->dpMatch != NULL) gp->dpMatch = (PWSTR) (base + cgp->dpMatch);
		if(cgp->dpNoMatch != NULL) gp->dpNoMatch = (PWSTR) (base + cgp->dpNoMatch);

		for(kp = gp->dpKeyArray, ckp = (PCOMP_KEY) kp, j = 0; j < gp->cxKeyArray; j++, kp++, ckp++) {
			kp->dpOutput = (PWSTR) (base + ckp->dpOutput);
			kp->dpContext = (PWSTR) (base + ckp->dpContext);
		}
	}

  return kbp;
}

BOOL VerifyKeyboard(LPBYTE filebase, DWORD sz) {
  DWORD i;
  PCOMP_KEYBOARD ckbp = (PCOMP_KEYBOARD) filebase;
  PCOMP_STORE csp;

	// Check file version //

	if(ckbp->dwFileVersion < VERSION_MIN ||
	   ckbp->dwFileVersion > VERSION_MAX) {
		// Old or new version -- identify the desired program version //
			for(csp = (PCOMP_STORE)(filebase + ckbp->dpStoreArray), i = 0; i < ckbp->cxStoreArray; i++, csp++) {
				if(csp->dwSystemID == TSS_COMPILEDVERSION) {
					wchar_t buf2[256];
          if(csp->dpString == 0) {
  					wsprintf(buf2, L"errWrongFileVersion:NULL");
          } else {
					  wsprintf(buf2, L"errWrongFileVersion:%10.10ls", StringOffset(filebase, csp->dpString));
          }
					Err(buf2);
					return FALSE;
				}
		}
		Err(L"errWrongFileVersion");
		return FALSE;
	}


  return TRUE;
}
*/