#include "mc_kmxfile.h"
#include "u16.h"
#include <typeinfo>

KMX_BOOL KMX_VerifyKeyboard(LPKMX_BYTE filebase, KMX_DWORD sz);

LPKMX_KEYBOARD KMX_FixupKeyboard(PKMX_BYTE bufp, PKMX_BYTE base, KMX_DWORD dwFileSize);

KMX_BOOL KMX_SaveKeyboard(LPKMX_KEYBOARD kbd, PKMX_WCHAR filename) {

  FILE *fp;
  fp = Open_File(filename, u"wb");

  if(fp == NULL)
  {
    KMX_LogError(L"Failed to create output file (%d)", errno);
    return FALSE;
  }

  KMX_DWORD err = KMX_WriteCompiledKeyboard(kbd, fp, FALSE);
  fclose(fp);

  if(err != CERR_None) {
    KMX_LogError(L"Failed to write compiled keyboard with error %d", err);

    std::u16string u16_filname(filename);
    std::string s = string_from_u16string(u16_filname);

    remove(s.c_str());
    return FALSE;
  }

  return TRUE;
}

KMX_DWORD KMX_WriteCompiledKeyboard(LPKMX_KEYBOARD fk, FILE* hOutfile, KMX_BOOL FSaveDebug) {

	LPKMX_GROUP fgp;
	LPKMX_STORE fsp;
	LPKMX_KEY fkp;

	PKMX_COMP_KEYBOARD ck;
	PKMX_COMP_GROUP gp;
	PKMX_COMP_STORE sp;
	PKMX_COMP_KEY kp;
	PKMX_BYTE buf;
	KMX_DWORD size, offset;
	DWORD i, j;

	// Calculate how much memory to allocate
	size = sizeof(KMX_COMP_KEYBOARD) +
			fk->cxGroupArray * sizeof(KMX_COMP_GROUP) +
			fk->cxStoreArray * sizeof(KMX_COMP_STORE) +
      //wcslen(fk->szName)*2 + 2 +
			//wcslen(fk->szCopyright)*2 + 2 +
			//wcslen(fk->szLanguageName)*2 + 2 +
			//wcslen(fk->szMessage)*2 + 2 +
      fk->dwBitmapSize;

	for(i = 0, fgp = fk->dpGroupArray; i < fk->cxGroupArray; i++, fgp++) {
    if(fgp->dpName)
	    size += u16len(fgp->dpName)*2 + 2;
		size += fgp->cxKeyArray * sizeof(KMX_COMP_KEY);
		for(j = 0, fkp = fgp->dpKeyArray; j < fgp->cxKeyArray; j++, fkp++) {
			size += u16len(fkp->dpOutput)*2 + 2;
			size += u16len(fkp->dpContext)*2 + 2;
		}

		if( fgp->dpMatch ) size += u16len(fgp->dpMatch)*2 + 2;
		if( fgp->dpNoMatch ) size += u16len(fgp->dpNoMatch)*2 + 2;
	}

	for(i = 0; i < fk->cxStoreArray; i++)
	{
		size += u16len(fk->dpStoreArray[i].dpString)*2 + 2;
    if(fk->dpStoreArray[i].dpName)
      size += u16len(fk->dpStoreArray[i].dpName)*2 + 2;
	}

	buf = new KMX_BYTE[size];
	if(!buf) return CERR_CannotAllocateMemory;
	memset(buf, 0, size);

	ck = (PKMX_COMP_KEYBOARD) buf;

	ck->dwIdentifier = FILEID_COMPILED;

  ck->dwFileVersion = fk->dwFileVersion;
	ck->dwCheckSum = 0; // No checksum in 16.0, see #7276
	ck->KeyboardID = fk->xxkbdlayout;
  ck->IsRegistered = fk->IsRegistered;
	ck->cxStoreArray = fk->cxStoreArray;
	ck->cxGroupArray = fk->cxGroupArray;
	ck->StartGroup[0] = fk->StartGroup[0];
	ck->StartGroup[1] = fk->StartGroup[1];
	ck->dwHotKey = fk->dwHotKey;

	ck->dwFlags = fk->dwFlags;

	offset = sizeof(KMX_COMP_KEYBOARD);

	// ck->dpLanguageName = offset;
	//wcscpy((PWSTR)(buf + offset), fk->szLanguageName);
	//offset += wcslen(fk->szLanguageName)*2 + 2;

	//ck->dpName = offset;
	//wcscpy((PWSTR)(buf + offset), fk->szName);
	//offset += wcslen(fk->szName)*2 + 2;

	//ck->dpCopyright = offset;
	//wcscpy((PWSTR)(buf + offset), fk->szCopyright);
	//offset += wcslen(fk->szCopyright)*2 + 2;

	//ck->dpMessage = offset;
	//wcscpy((PWSTR)(buf + offset), fk->szMessage);
	//offset += wcslen(fk->szMessage)*2 + 2;

	ck->dpStoreArray = offset;
	sp = (PKMX_COMP_STORE)(buf+offset);
	fsp = fk->dpStoreArray;
	offset += sizeof(KMX_COMP_STORE) * ck->cxStoreArray;
	for(i = 0; i < ck->cxStoreArray; i++, sp++, fsp++) {
		sp->dwSystemID = fsp->dwSystemID;
		sp->dpString = offset;
		u16ncpy((PKMX_WCHAR)(buf+offset), fsp->dpString, (size-offset) / sizeof(KMX_WCHAR));  // I3481   // I3641
		offset += u16len(fsp->dpString)*2 + 2;

    if(!fsp->dpName) {
      sp->dpName = 0;
    } else {
      sp->dpName = offset;
      u16ncpy((PKMX_WCHAR)(buf+offset), fsp->dpName, (size-offset) / sizeof(KMX_WCHAR));  // I3481   // I3641
		  offset += u16len(fsp->dpName)*2 + 2;
    }
	}

	ck->dpGroupArray = offset;
	gp = (PKMX_COMP_GROUP)(buf+offset);
	fgp = fk->dpGroupArray;

	offset += sizeof(KMX_COMP_GROUP) * ck->cxGroupArray;

	for(i = 0; i < ck->cxGroupArray; i++, gp++, fgp++) {
		gp->cxKeyArray = fgp->cxKeyArray;
		gp->fUsingKeys = fgp->fUsingKeys;

		gp->dpMatch = gp->dpNoMatch = 0;

		if(fgp->dpMatch) {
			gp->dpMatch = offset;
			u16ncpy((PKMX_WCHAR)(buf+offset), fgp->dpMatch, (size-offset) / sizeof(KMX_WCHAR));  // I3481   // I3641
			offset += u16len(fgp->dpMatch)*2 + 2;
		}
		if(fgp->dpNoMatch) {
			gp->dpNoMatch = offset;
			u16ncpy((PKMX_WCHAR)(buf+offset), fgp->dpNoMatch, (size-offset) / sizeof(KMX_WCHAR));  // I3481   // I3641
			offset += u16len(fgp->dpNoMatch)*2 + 2;
		}

    if(fgp->dpName) {
			gp->dpName = offset;
			u16ncpy((PKMX_WCHAR)(buf+offset), fgp->dpName, (size-offset) / sizeof(KMX_WCHAR));  // I3481   // I3641
			offset += u16len(fgp->dpName)*2 + 2;
		}	else {
      gp->dpName = 0;
    }

    gp->dpKeyArray = offset;
		kp = (PKMX_COMP_KEY) (buf + offset);
		fkp = fgp->dpKeyArray;
		offset += gp->cxKeyArray * sizeof(KMX_COMP_KEY);
		for(j = 0; j < gp->cxKeyArray; j++, kp++, fkp++) {
			kp->Key = fkp->Key;
			kp->Line = fkp->Line;
			kp->ShiftFlags = fkp->ShiftFlags;
			kp->dpOutput = offset;

			u16ncpy((PKMX_WCHAR)(buf+offset), fkp->dpOutput, (size-offset) / sizeof(KMX_WCHAR));  // I3481   // I3641
			offset += u16len(fkp->dpOutput)*2 + 2;

      kp->dpContext = offset;
      u16ncpy((PKMX_WCHAR)(buf+offset), fkp->dpContext, (size-offset) / sizeof(KMX_WCHAR));  // I3481   // I3641
		  offset += u16len(fkp->dpContext)*2 + 2;
		}
	}

  if(fk->dwBitmapSize > 0) {
    ck->dwBitmapSize = fk->dwBitmapSize;
	  ck->dpBitmapOffset = offset;
    memcpy(buf + offset, ((PKMX_BYTE)fk) + fk->dpBitmapOffset, fk->dwBitmapSize);
	  offset += fk->dwBitmapSize;
  } else {
    ck->dwBitmapSize = 0;
	  ck->dpBitmapOffset = 0;
  }

	if(offset != size)
  {
    delete[] buf;
    return CERR_SomewhereIGotItWrong;
  }

  fwrite(buf, size,1,hOutfile);
	if(offset != size)
  {
    delete[] buf;
    return CERR_UnableToWriteFully;
  }

	delete[] buf;

	return CERR_None;
}

PKMX_WCHAR KMX_StringOffset(PKMX_BYTE base, KMX_DWORD offset) {
  if(offset == 0) return NULL;
  return (PKMX_WCHAR)(base + offset);
}

#ifdef KMX_64BIT

/**
  CopyKeyboard will copy the data read into bufp from x86-sized structures into
  x64-sized structures starting at `base`
  * After this function finishes, we still need to keep the original data because
    we don't copy the strings
  This method is used on 64-bit architectures.
*/
LPKMX_KEYBOARD KMX_CopyKeyboard(PKMX_BYTE bufp, PKMX_BYTE base)
{
  PKMX_COMP_KEYBOARD ckbp = (PKMX_COMP_KEYBOARD) base;

  /* Copy keyboard structure */

  LPKMX_KEYBOARD kbp = (LPKMX_KEYBOARD) bufp;
  bufp += sizeof(KMX_KEYBOARD);

  kbp->dwIdentifier = ckbp->dwIdentifier;
  kbp->dwFileVersion = ckbp->dwFileVersion;
  kbp->dwCheckSum = ckbp->dwCheckSum;
  kbp->xxkbdlayout = ckbp->KeyboardID;
  kbp->IsRegistered = ckbp->IsRegistered;
  kbp->version = ckbp->version;
  kbp->cxStoreArray = ckbp->cxStoreArray;
  kbp->cxGroupArray = ckbp->cxGroupArray;
  kbp->StartGroup[0] = ckbp->StartGroup[0];
  kbp->StartGroup[1] = ckbp->StartGroup[1];
  kbp->dwFlags = ckbp->dwFlags;
  kbp->dwHotKey = ckbp->dwHotKey;

  kbp->dpStoreArray = (LPKMX_STORE) bufp;
  bufp += sizeof(KMX_STORE) * kbp->cxStoreArray;

  kbp->dpGroupArray = (LPKMX_GROUP) bufp;
  bufp += sizeof(KMX_GROUP) * kbp->cxGroupArray;

  PKMX_COMP_STORE csp;
  LPKMX_STORE sp;
  KMX_DWORD i;

  for(
    csp = (PKMX_COMP_STORE)(base + ckbp->dpStoreArray), sp = kbp->dpStoreArray, i = 0;
    i < kbp->cxStoreArray;
    i++, sp++, csp++)
  {
    sp->dwSystemID = csp->dwSystemID;
    sp->dpName = KMX_StringOffset(base, csp->dpName);
    sp->dpString = KMX_StringOffset(base, csp->dpString);
  }

  PKMX_COMP_GROUP cgp;
  LPKMX_GROUP gp;

  for(
    cgp = (PKMX_COMP_GROUP)(base + ckbp->dpGroupArray), gp = kbp->dpGroupArray, i = 0;
    i < kbp->cxGroupArray;
    i++, gp++, cgp++)
  {
    gp->dpName = KMX_StringOffset(base, cgp->dpName);
    gp->dpKeyArray = cgp->cxKeyArray > 0 ? (LPKMX_KEY) bufp : NULL;
    gp->cxKeyArray = cgp->cxKeyArray;
    bufp += sizeof(KMX_KEY) * gp->cxKeyArray;
    gp->dpMatch = KMX_StringOffset(base, cgp->dpMatch);
    gp->dpNoMatch = KMX_StringOffset(base, cgp->dpNoMatch);
    gp->fUsingKeys = cgp->fUsingKeys;

    PKMX_COMP_KEY ckp;
    LPKMX_KEY kp;
    KMX_DWORD j;

    for(
      ckp = (PKMX_COMP_KEY)(base + cgp->dpKeyArray), kp = gp->dpKeyArray, j = 0;
      j < gp->cxKeyArray;
      j++, kp++, ckp++)
    {
      kp->Key = ckp->Key;
      kp->Line = ckp->Line;
      kp->ShiftFlags = ckp->ShiftFlags;
      kp->dpOutput = KMX_StringOffset(base, ckp->dpOutput);
      kp->dpContext = KMX_StringOffset(base, ckp->dpContext);
    }
  }
  return kbp;
}

// else KMX_FixupKeyboard
#else
/**
 Fixup the keyboard by expanding pointers. On disk the pointers are stored relative to the
 beginning of the file, but we need real pointers. This method is used on 32-bit architectures.
*/

LPKMX_KEYBOARD KMX_FixupKeyboard(PKMX_BYTE bufp, PKMX_BYTE base, KMX_DWORD dwFileSize) {

  UNREFERENCED_PARAMETER(dwFileSize);

  KMX_DWORD i, j;
  PKMX_COMP_KEYBOARD ckbp = (PKMX_COMP_KEYBOARD) base;
  PKMX_COMP_GROUP cgp;
  PKMX_COMP_STORE csp;
  PKMX_COMP_KEY ckp;
  LPKMX_KEYBOARD kbp = (LPKMX_KEYBOARD) bufp;
  LPKMX_STORE sp;
  LPKMX_GROUP gp;
  LPKMX_KEY kp;

	kbp->dpStoreArray = (LPKMX_STORE) (base + ckbp->dpStoreArray);
	kbp->dpGroupArray = (LPKMX_GROUP) (base + ckbp->dpGroupArray);

	for(sp = kbp->dpStoreArray, csp = (PKMX_COMP_STORE) sp, i = 0; i < kbp->cxStoreArray; i++, sp++, csp++)	{
    sp->dpName = KMX_StringOffset(base, csp->dpName);
		sp->dpString = KMX_StringOffset(base, csp->dpString);
	}

	for(gp = kbp->dpGroupArray, cgp = (PKMX_COMP_GROUP) gp, i = 0; i < kbp->cxGroupArray; i++, gp++, cgp++)	{
    gp->dpName = KMX_StringOffset(base, cgp->dpName);
		gp->dpKeyArray = (LPKMX_KEY) (base + cgp->dpKeyArray);
		if(cgp->dpMatch != NULL) gp->dpMatch = (PKMX_WCHAR) (base + cgp->dpMatch);
		if(cgp->dpNoMatch != NULL) gp->dpNoMatch = (PKMX_WCHAR) (base + cgp->dpNoMatch);

    // _S2  Version of kmx_file v
    /*gp->dpName = StringOffset(base, cgp->dpName);
    gp->dpKeyArray = cgp->cxKeyArray > 0 ? (LPKEY) (base + cgp->dpKeyArray) : NULL;
    gp->dpMatch = StringOffset(base, cgp->dpMatch);
    gp->dpNoMatch = StringOffset(base, cgp->dpNoMatch);*/
    // _S2  Version of kmx_file ^

		for(kp = gp->dpKeyArray, ckp = (PKMX_COMP_KEY) kp, j = 0; j < gp->cxKeyArray; j++, kp++, ckp++) {
			kp->dpOutput = (PKMX_WCHAR) (base + ckp->dpOutput);
			kp->dpContext = (PKMX_WCHAR) (base + ckp->dpContext);
		}
	}

  return kbp;
}
#endif

KMX_BOOL KMX_LoadKeyboard(char16_t* fileName, LPKMX_KEYBOARD* lpKeyboard) {

  PKMX_BYTE buf;
  FILE* fp;
  LPKMX_KEYBOARD kbp;
  PKMX_BYTE filebase;

  if(!fileName || !lpKeyboard) {
    KMX_LogError(L"LogError1: Bad Filename\n" );
    return FALSE;
  }

  fp = Open_File((const KMX_WCHAR*)fileName, u"rb");

  if(fp == NULL) {
    KMX_LogError(L"LogError1: Could not open file\n" );
    return FALSE;
  }

  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    KMX_LogError(L"LogError1: Could not fseek file\n" );
    return FALSE;
  }

  auto sz = ftell(fp);
  if (sz < 0) {
    fclose(fp);
    return FALSE;
  }

  if (fseek(fp, 0, SEEK_SET) != 0) {
    fclose(fp);
    KMX_LogError(L"LogErr1: Could not fseek(set) file\n" );
    return FALSE;
  }

  #ifdef KMX_64BIT
  //  allocate enough memory for expanded data structure + original data.
  //  Expanded data structure is double the size of data on disk (8-byte
  //  pointers) - on disk the "pointers" are relative to the beginning of
  //  the file.
  //  We save the original data at the end of buf; we don't copy strings, so
  //  those will remain in the location at the end of the buffer.
    buf = new KMX_BYTE[sz * 3];
  #else
    buf = new KMX_BYTE[sz];
  #endif

  if (!buf) {
    fclose(fp);
    KMX_LogError(L"LogErr1: Not allocmem\n" );
    return FALSE;
  }

  #ifdef KMX_64BIT
    filebase = buf + sz*2;
  #else
    filebase = buf;
  #endif

  if (fread(filebase, 1, sz, fp) < (size_t)sz) {
    KMX_LogError(L"LogError1: Could not read file\n" );
    fclose(fp);
    return FALSE;
  }

  fclose(fp);

  if(*PKMX_DWORD(filebase) != KMX_DWORD(FILEID_COMPILED))
  {
    delete [] buf;
    KMX_LogError(L"Invalid file - signature is invalid\n");
    return FALSE;
  }

  if (!KMX_VerifyKeyboard(filebase, sz)) {
    KMX_LogError(L"LogError1: errVerifyKeyboard\n" );
    return FALSE;
  }

#ifdef KMX_64BIT
  kbp = KMX_CopyKeyboard(buf, filebase);
#else
  kbp = KMX_FixupKeyboard(buf, filebase, sz);
#endif


  if (!kbp) {
    KMX_LogError(L"LogError1: errFixupKeyboard\n" );
    return FALSE;
  }

  if (kbp->dwIdentifier != FILEID_COMPILED) {
    delete[] buf;
    KMX_LogError(L"LogError1: errNotFileID\n" );
    return FALSE;
  }
  *lpKeyboard = kbp;
  return TRUE;
}

KMX_BOOL KMX_VerifyKeyboard(LPKMX_BYTE filebase, KMX_DWORD sz){
  KMX_DWORD i;
  PKMX_COMP_KEYBOARD ckbp = (PKMX_COMP_KEYBOARD)filebase;
  PKMX_COMP_STORE csp;

  // Check file version //

  if (ckbp->dwFileVersion < VERSION_MIN || ckbp->dwFileVersion > VERSION_MAX) {
    // Old or new version -- identify the desired program version //
    for (csp = (PKMX_COMP_STORE)(filebase + ckbp->dpStoreArray), i = 0; i < ckbp->cxStoreArray; i++, csp++) {
      if (csp->dwSystemID == TSS_COMPILEDVERSION) {
        if (csp->dpString == 0) {
          KMX_LogError(L"LogErr1: errWrongFileVersion:NULL");
        } else {
          wprintf(L"LogErr1: errWrongFileVersion:%10.10ls",(const PKMX_WCHAR) KMX_StringOffset((PKMX_BYTE)filebase, csp->dpString));
        }
        return FALSE;
      }
    }
    KMX_LogError(L"LogErr1: errWrongFileVersion");
    return FALSE;
  }
  return TRUE;
}

PKMX_WCHAR KMX_incxstr(PKMX_WCHAR p) {

  if (*p == 0)
    return p;
  if (*p != UC_SENTINEL) {
    if (*p >= 0xD800 && *p <= 0xDBFF && *(p + 1) >= 0xDC00 && *(p + 1) <= 0xDFFF)
      return p + 2;
    return p + 1;
  }
  // UC_SENTINEL(FFFF) with UC_SENTINEL_EXTENDEDEND(0x10) == variable length
  if (*(p + 1) == CODE_EXTENDED) {
    p += 2;
    while (*p && *p != UC_SENTINEL_EXTENDEDEND)
      p++;

    if (*p == 0)        return p;
    return p + 1;
  }

  if (*(p + 1) > CODE_LASTCODE || CODE__SIZE[*(p + 1)] == -1) {
    return p + 1;
  }

  int deltaptr = 2 + CODE__SIZE[*(p + 1)];

  // check for \0 between UC_SENTINEL(FFFF) and next printable character
  for (int i = 0; i < deltaptr; i++) {
    if (*p == 0)
      return p;
    p++;
  }
  return p;
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