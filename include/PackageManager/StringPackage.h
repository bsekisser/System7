/*
 * StringPackage.h
 * System 7.1 Portable String Package (International Utilities)
 *
 * Implements Mac OS International Utilities Package (Pack 6) for string processing,
 * comparison, formatting, and international text handling.
 * Critical for proper text display and user interface functionality.
 */

#ifndef __STRING_PACKAGE_H__
#define __STRING_PACKAGE_H__

#include "SystemTypes.h"

#include "SystemTypes.h"

#include "PackageTypes.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/* International Utilities selector constants */
#define iuSelDateString     0
#define iuSelTimeString     2
#define iuSelMetric         4
#define iuSelGetIntl        6
#define iuSelSetIntl        8
#define iuSelMagString      10
#define iuSelMagIDString    12
#define iuSelDatePString    14
#define iuSelTimePString    16
#define iuSelLDateString    20
#define iuSelLTimeString    22
#define iuSelClearCache     24
#define iuSelMagPString     26
#define iuSelMagIDPString   28
#define iuSelScriptOrder    30
#define iuSelLangOrder      32
#define iuSelTextOrder      34
#define iuSelGetItlTable    36

/* Date formatting constants */
typedef enum {
    shortDate = 0,      /* Short format: "1/15/25" */
    longDate = 1,       /* Long format: "Friday, January 15, 2025" */
    abbrevDate = 2      /* Abbreviated: "Fri, Jan 15, 2025" */
} DateForm;

/* International resource types */

/* String comparison result */

/* Text encoding types */

/* Language codes */

/* International resource structure */
          /* Decimal point character */
    SInt16     thousSep;           /* Thousands separator */
    SInt16     listSep;            /* List separator */
    SInt16     currSym1;           /* Currency symbol 1 */
    SInt16     currSym2;           /* Currency symbol 2 */
    SInt16     currSym3;           /* Currency symbol 3 */
    UInt8     currFmt;            /* Currency format */
    UInt8     dateOrder;          /* Date order */
    UInt8     shrtDateFmt;        /* Short date format */
    SInt8      dateSep;            /* Date separator */
    UInt8     timeCycle;          /* Time cycle (12/24 hour) */
    UInt8     timeFmt;            /* Time format */
    SInt8      mornStr[4];         /* Morning string */
    SInt8      eveStr[4];          /* Evening string */
    SInt8      timeSep;            /* Time separator */
    SInt8      time1Suff;          /* Time suffix 1 */
    SInt8      time2Suff;          /* Time suffix 2 */
    SInt8      time3Suff;          /* Time suffix 3 */
    SInt8      time4Suff;          /* Time suffix 4 */
    SInt8      time5Suff;          /* Time suffix 5 */
    SInt8      time6Suff;          /* Time suffix 6 */
    SInt8      time7Suff;          /* Time suffix 7 */
    SInt8      time8Suff;          /* Time suffix 8 */
    UInt8     metricSys;          /* Metric system flag */
    SInt16     intl0Vers;          /* International resource version */
} Intl0Rec, *Intl0Ptr, **Intl0Hndl;

/* String Package API Functions */

/* Package initialization and management */
SInt32 InitStringPackage(void);
void CleanupStringPackage(void);
SInt32 StringDispatch(SInt16 selector, void *params);

/* String comparison functions */
SInt16 IUMagString(const void *aPtr, const void *bPtr, SInt16 aLen, SInt16 bLen);
SInt16 IUMagIDString(const void *aPtr, const void *bPtr, SInt16 aLen, SInt16 bLen);
SInt16 IUCompString(const char *aStr, const char *bStr);
SInt16 IUEqualString(const char *aStr, const char *bStr);

/* International string comparison */
SInt16 IUMagPString(const void *aPtr, const void *bPtr, SInt16 aLen, SInt16 bLen, Handle intlParam);
SInt16 IUMagIDPString(const void *aPtr, const void *bPtr, SInt16 aLen, SInt16 bLen, Handle intlParam);
SInt16 IUCompPString(const char *aStr, const char *bStr, Handle intlParam);
SInt16 IUEqualPString(const char *aStr, const char *bStr, Handle intlParam);

/* Script and language ordering */
SInt16 IUScriptOrder(ScriptCode aScript, ScriptCode bScript);
SInt16 IULangOrder(LangCode aLang, LangCode bLang);
SInt16 IUTextOrder(const void *aPtr, const void *bPtr, SInt16 aLen, SInt16 bLen,
                    ScriptCode aScript, ScriptCode bScript, LangCode aLang, LangCode bLang);
SInt16 IUStringOrder(const char *aStr, const char *bStr, ScriptCode aScript, ScriptCode bScript,
                      LangCode aLang, LangCode bLang);

/* Date and time formatting */
void IUDateString(UInt32 dateTime, DateForm longFlag, char *result);
void IUTimeString(UInt32 dateTime, Boolean wantSeconds, char *result);
void IUDatePString(UInt32 dateTime, DateForm longFlag, char *result, Handle intlParam);
void IUTimePString(UInt32 dateTime, Boolean wantSeconds, char *result, Handle intlParam);
void IULDateString(const LongDateTime *dateTime, DateForm longFlag, char *result, Handle intlParam);
void IULTimeString(const LongDateTime *dateTime, Boolean wantSeconds, char *result, Handle intlParam);

/* International resource management */
Handle IUGetIntl(SInt16 theID);
void IUSetIntl(SInt16 refNum, SInt16 theID, const void *intlParam);
Boolean IUMetric(void);
void IUClearCache(void);
Handle IUGetItlTable(ScriptCode script, SInt16 tableCode, Handle *itlHandle, SInt32 *offset, SInt32 *length);

/* String utility functions */
void StringToNum(const char *theString, SInt32 *theNum);
void NumToString(SInt32 theNum, char *theString);
SInt32 StringWidth(const char *theString);
void TruncString(SInt16 width, char *theString, SInt16 truncWhere);

/* Character classification and conversion */
Boolean IsLower(char ch);
Boolean IsUpper(char ch);
char ToLower(char ch);
char ToUpper(char ch);
Boolean IsAlpha(char ch);
Boolean IsDigit(char ch);
Boolean IsAlphaNum(char ch);
Boolean IsSpace(char ch);
Boolean IsPunct(char ch);

/* String manipulation */
void CopyString(const char *source, char *dest, SInt16 maxLen);
void ConcatString(const char *source, char *dest, SInt16 maxLen);
SInt16 FindString(const char *searchIn, const char *searchFor, SInt16 startPos);
void ReplaceString(char *theString, const char *oldStr, const char *newStr);
void TrimString(char *theString);

/* Pascal/C string conversion */
void C2PStr(char *cString);
void P2CStr(char *pString);
void CopyC2PStr(const char *cString, char *pString);
void CopyP2CStr(const char *pString, char *cString);

/* Text encoding and conversion */
SInt32 TextEncodingToScript(SInt32 encoding);
SInt32 ScriptToTextEncoding(ScriptCode script, LangCode language);
OSErr ConvertFromTextEncoding(SInt32 srcEncoding, SInt32 dstEncoding,
                             const void *srcText, SInt32 srcLen,
                             void *dstText, SInt32 dstMaxLen, SInt32 *dstLen);

/* International utilities configuration */
void SetStringPackageScript(ScriptCode script);
ScriptCode GetStringPackageScript(void);
void SetStringPackageLanguage(LangCode language);
LangCode GetStringPackageLanguage(void);

/* String sorting and collation */

void SortStringArray(StringSortEntry *entries, SInt16 count,
                     ScriptCode script, LangCode language);
SInt16 CompareStringEntries(const StringSortEntry *a, const StringSortEntry *b,
                            ScriptCode script, LangCode language);

/* Token parsing */

SInt16 TokenizeString(const char *input, char **tokens, SInt16 maxTokens,
                      const TokenizeOptions *options);
void FreeTokenArray(char **tokens, SInt16 count);

/* String formatting */
SInt32 FormatString(char *output, SInt32 maxLen, const char *format, ...);
SInt32 FormatStringArgs(char *output, SInt32 maxLen, const char *format, va_list args);

/* Localized string resources */
Handle GetLocalizedString(SInt16 stringID, ScriptCode script, LangCode language);
void GetLocalizedCString(SInt16 stringID, char *buffer, SInt16 maxLen,
                        ScriptCode script, LangCode language);

/* String validation and sanitization */
Boolean ValidateStringEncoding(const char *str, SInt16 len, ScriptCode script);
void SanitizeString(char *str, SInt16 maxLen, ScriptCode script);
Boolean IsValidFilename(const char *filename, ScriptCode script);

/* Platform integration */
void SetPlatformStringCallbacks(void *callbacks);
void EnableUnicodeSupport(Boolean enabled);
void SetDefaultSystemScript(ScriptCode script);
void SetDefaultSystemLanguage(LangCode language);

#ifdef __cplusplus
}
#endif

#endif /* __STRING_PACKAGE_H__ */
