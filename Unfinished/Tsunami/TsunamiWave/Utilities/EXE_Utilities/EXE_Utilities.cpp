#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
//#define DEBUG_LEVEL DEBUG_VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"

#include "EXE_Utilities.h"
#include "LIB_Utilities.h"




#define SMALL_TEXT_FILENAME		L"small-text-test.txt"
#define LARGE_TEXT_FILENAME		L"large-text-test.txt"
#define SMALL_BINARY_FILENAME	L"small-binary-test.bin"
#define LARGE_BINARY_FILENAME	L"large-binary-test.bin"

#define MAX_PASSWORD_LENGTH		256
#define DEFAULT_PASSWORD		"{E3AEA3F6-D548-4989-9A42-80BAC9321AE0}"






DWORD ReadFileIntoBufferW(LPCWSTR szFileName, LPBYTE* lppBuffer, LPDWORD lpdwBufferSize)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	HANDLE	hFile = INVALID_HANDLE_VALUE;
	LPBYTE	pBuffer = NULL;
	DWORD	dwSize = 0;
	DWORD	dwBytesRead = 0;

	__try
	{
		__try
		{
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szFileName:         %s"), szFileName);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("lppBuffer:          %p"), lppBuffer);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("lpdwBufferSize:     %p"), lpdwBufferSize);

			*lppBuffer = NULL;
			if (NULL != lpdwBufferSize)
			{
				*lpdwBufferSize = 0;
			}

			// Open the file to load into a buffer
			//DBGPRINT( DEBUG_VERBOSE, TEXT("CreateFileA( %s, %08x, %d, %d, %08x, %d, %d )..."), 
			//	szFileName,
			//	GENERIC_READ,
			//	FILE_SHARE_READ,
			//	0,
			//	OPEN_EXISTING,
			//	0,
			//	0
			//);
			hFile = CreateFileW(
				szFileName,
				GENERIC_READ,
				FILE_SHARE_READ,
				0,
				OPEN_EXISTING,
				0,
				0
			);
			if (INVALID_HANDLE_VALUE == hFile)
			{
				dwErrorCode = GetLastError();
				DBGPRINT( DEBUG_ERROR, TEXT("CreateFileA( %s, %08x, %d, %d, %08x, %d, %d ) failed. (%d)"),
					szFileName,		// lpFileName
					GENERIC_READ,	// dwDesiredAccess
					FILE_SHARE_READ,// dwSharedMode
					0,				// lpSecurityAttributes
					OPEN_EXISTING,	// dwCreationDisposition
					0,				// dwFlagsAndAttributes
					0,				// hTemplateFile
					dwErrorCode
				);
				__leave;
			}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("hFile: %p"), hFile);


			//DBGPRINT( DEBUG_VERBOSE, TEXT("GetFileSize( %p, %p )..."), hFile, NULL );
			dwSize = GetFileSize(hFile, NULL);
			if ((INVALID_FILE_SIZE == dwSize) || (0 == dwSize))
			{
				dwErrorCode = GetLastError();
				DBGPRINT( DEBUG_ERROR, TEXT("GetFileSize( %p, %p )..."), hFile, NULL, dwErrorCode );
				__leave;
			}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwSize: %d"), dwSize);

			//DBGPRINT( DEBUG_VERBOSE, TEXT("ALLOC( %08x, %p )..."), dwSize, (LPVOID *)&pBuffer );
			dwErrorCode = ALLOC(dwSize, (LPVOID *)&pBuffer);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				dwErrorCode = GetLastError();
				DBGPRINT( DEBUG_ERROR, TEXT("ALLOC( %08x, %p ) failed. (%08x"), dwSize,	dwErrorCode	);
				__leave;
			}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("pBuffer: %p"), pBuffer);


			//DBGPRINT( DEBUG_VERBOSE, TEXT("ReadFile( %p, %p, %d, %p, 0 )..."),
			//	hFile,
			//	pBuffer,
			//	dwSize,
			//	&dwBytesRead
			//);
			if (FALSE ==
				ReadFile(
					hFile,
					pBuffer,
					dwSize,
					&dwBytesRead,
					0
				)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT( DEBUG_ERROR, TEXT("ReadFile( %p, %p, %d, %p, 0 ) failed. (%08x)"),
					hFile,
					pBuffer,
					dwSize,
					&dwBytesRead,
					dwErrorCode
				);
				__leave;
			}

			if (dwBytesRead != dwSize)
			{
				dwErrorCode = ERROR_HANDLE_EOF;
				DBGPRINT( DEBUG_ERROR, TEXT("ReadFile( %p, %p, %d, %p, 0 ) failed to read all the bytes. ( %d != %d )"),
					hFile,
					pBuffer,
					dwSize,
					&dwBytesRead,
					dwBytesRead,
					dwSize
				);
				__leave;
			}



			*lppBuffer = pBuffer;

			if (NULL != lpdwBufferSize)
			{
				*lpdwBufferSize = dwSize;
			}


		} // end try-finally
		__finally
		{
			DWORD dwTempErrorCode = ERROR_SUCCESS;

			if ((ERROR_SUCCESS != dwErrorCode) && (NULL != pBuffer))
			{
				dwTempErrorCode = FREE(pBuffer);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT( DEBUG_ERROR, TEXT("FREE( %p ) failed. (%08x)"), pBuffer, dwTempErrorCode );
				}
				pBuffer = NULL;
			}

			if ((INVALID_HANDLE_VALUE != hFile) && (NULL != hFile))
			{
				if (FALSE == CloseHandle( hFile	) )
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT( DEBUG_ERROR, TEXT("CloseHandle( %p ) failed. (%08x)"), hFile,	dwTempErrorCode	);
				}
				hFile = INVALID_HANDLE_VALUE;
			}

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)\n"), dwErrorCode);
	} // end except

	return dwErrorCode;
}


DWORD TestEncode(LPCBYTE lpOriginalBuffer, DWORD dwOriginalBufferSize)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	LPBYTE	lpEncodedBuffer = NULL;
	DWORD	dwEncodedBufferSize = 0;

	LPBYTE	lpDecodedBuffer = NULL;
	DWORD	dwDecodedBufferSize = NULL;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_Utilities %S start"), __func__);

			// Apply the encoding function
			DBGPRINT(DEBUG_INFO, TEXT("--------------------------------------------------------------------------------"));
			DBGPRINT(DEBUG_INFO, TEXT("Testing EncodeBuffer..."));
			dwErrorCode = EncodeBuffer((PBYTE)lpOriginalBuffer, dwOriginalBufferSize, &lpEncodedBuffer, &dwEncodedBufferSize );
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("EncodeBuffer failed.(%08x)"), dwErrorCode);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwEncodedBufferSize:   %d (%08x)"), dwEncodedBufferSize, dwEncodedBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpEncodedBuffer:       %p"), lpEncodedBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpEncodedBuffer:       %02x %02x %02x %02x %02x %02x %02x %02x"), lpEncodedBuffer[0], lpEncodedBuffer[1], lpEncodedBuffer[2], lpEncodedBuffer[3], lpEncodedBuffer[4], lpEncodedBuffer[5], lpEncodedBuffer[6], lpEncodedBuffer[7]);


			// Apply the decoding function
			DBGPRINT(DEBUG_INFO, TEXT("--------------------------------------------------------------------------------"));
			DBGPRINT(DEBUG_INFO, TEXT("Testing DecodeBuffer..."));
			dwErrorCode = DecodeBuffer((PBYTE)lpEncodedBuffer, dwEncodedBufferSize, &lpDecodedBuffer, &dwDecodedBufferSize );
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("DecodeBuffer failed.(%08x)"), dwErrorCode);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwDecodedBufferSize:   %d (%08x)"), dwDecodedBufferSize, dwDecodedBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpDecodedBuffer:       %p"), lpDecodedBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpDecodedBuffer:       %02x %02x %02x %02x %02x %02x %02x %02x"), lpDecodedBuffer[0], lpDecodedBuffer[1], lpDecodedBuffer[2], lpDecodedBuffer[3], lpDecodedBuffer[4], lpDecodedBuffer[5], lpDecodedBuffer[6], lpDecodedBuffer[7]);


			// Compare the decoded results to the original to see if transformation was successful
			DBGPRINT(DEBUG_INFO, TEXT("--------------------------------------------------------------------------------"));
			if (memcmp(lpOriginalBuffer, lpDecodedBuffer, dwOriginalBufferSize) == 0)
			{
				DBGPRINT(DEBUG_INFO, TEXT("EncodeBuffer/DecodeBuffer was SUCCESSFUL"));
				dwErrorCode = ERROR_SUCCESS;
			}
			else
			{
				DBGPRINT(DEBUG_WARNING, TEXT("EncodeBuffer/DecodeBuffer FAILED!"));
				dwErrorCode = ERROR_NO_MATCH;
			}

		} // end try-finally
		__finally
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_Utilities %S end"), __func__);

			if (NULL != lpDecodedBuffer)
			{
				dwTempErrorCode = FREE(lpDecodedBuffer);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(DEBUG_ERROR, TEXT("FREE( %p ) failed. (%08x)"), lpDecodedBuffer, dwTempErrorCode);
				}
				lpDecodedBuffer = NULL;
			}
			dwDecodedBufferSize = 0;

			if (NULL != lpEncodedBuffer)
			{
				dwTempErrorCode = FREE(lpEncodedBuffer);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(DEBUG_ERROR, TEXT("FREE( %p ) failed. (%08x)"), lpEncodedBuffer, dwTempErrorCode);
				}
				lpEncodedBuffer = NULL;
			}
			dwEncodedBufferSize = 0;
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}


DWORD TestEncrypt(LPCBYTE lpOriginalBuffer, DWORD dwOriginalBufferSize)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	LPBYTE	lpEncodedBuffer = NULL;
	DWORD	dwEncodedBufferSize = 0;

	LPBYTE	lpDecodedBuffer = NULL;
	DWORD	dwDecodedBufferSize = NULL;

	LPBYTE	lpPasswordBuffer[MAX_PASSWORD_LENGTH];
	DWORD	dwPasswordBufferSize = 0;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_Utilities %S start"), __func__);

			ZeroMemory(lpPasswordBuffer, MAX_PASSWORD_LENGTH);
			strcpy_s((LPSTR)lpPasswordBuffer, MAX_PASSWORD_LENGTH, DEFAULT_PASSWORD);
			dwPasswordBufferSize = (DWORD)strlen((LPSTR)lpPasswordBuffer);

			// Apply the encoding function
			DBGPRINT(DEBUG_INFO, TEXT("--------------------------------------------------------------------------------"));
			DBGPRINT(DEBUG_INFO, TEXT("Testing EncryptBuffer..."));
			dwErrorCode = EncryptBuffer((PBYTE)lpOriginalBuffer, dwOriginalBufferSize, &lpEncodedBuffer, &dwEncodedBufferSize, (PBYTE)lpPasswordBuffer, dwPasswordBufferSize);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("EncryptBuffer failed.(%08x)"), dwErrorCode);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwEncodedBufferSize:   %d (%08x)"), dwEncodedBufferSize, dwEncodedBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpEncodedBuffer:       %p"), lpEncodedBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpEncodedBuffer:       %02x %02x %02x %02x %02x %02x %02x %02x"), lpEncodedBuffer[0], lpEncodedBuffer[1], lpEncodedBuffer[2], lpEncodedBuffer[3], lpEncodedBuffer[4], lpEncodedBuffer[5], lpEncodedBuffer[6], lpEncodedBuffer[7]);


			// Apply the decoding function
			DBGPRINT(DEBUG_INFO, TEXT("--------------------------------------------------------------------------------"));
			DBGPRINT(DEBUG_INFO, TEXT("Testing DecryptBuffer..."));
			dwErrorCode = DecryptBuffer((PBYTE)lpEncodedBuffer, dwEncodedBufferSize, &lpDecodedBuffer, &dwDecodedBufferSize, (PBYTE)lpPasswordBuffer, dwPasswordBufferSize);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("DecryptBuffer failed.(%08x)"), dwErrorCode);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwDecodedBufferSize:   %d (%08x)"), dwDecodedBufferSize, dwDecodedBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpDecodedBuffer:       %p"), lpDecodedBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpDecodedBuffer:       %02x %02x %02x %02x %02x %02x %02x %02x"), lpDecodedBuffer[0], lpDecodedBuffer[1], lpDecodedBuffer[2], lpDecodedBuffer[3], lpDecodedBuffer[4], lpDecodedBuffer[5], lpDecodedBuffer[6], lpDecodedBuffer[7]);


			// Compare the decoded results to the original to see if transformation was successful
			DBGPRINT(DEBUG_INFO, TEXT("--------------------------------------------------------------------------------"));
			if (memcmp(lpOriginalBuffer, lpDecodedBuffer, dwOriginalBufferSize) == 0)
			{
				DBGPRINT(DEBUG_INFO, TEXT("EncryptBuffer/DecryptBuffer was SUCCESSFUL"));
				dwErrorCode = ERROR_SUCCESS;
			}
			else
			{
				DBGPRINT(DEBUG_WARNING, TEXT("EncryptBuffer/DecryptBuffer FAILED!"));
				dwErrorCode = ERROR_NO_MATCH;
			}

		} // end try-finally
		__finally
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_Utilities %S end"), __func__);

			if (NULL != lpDecodedBuffer)
			{
				dwTempErrorCode = FREE(lpDecodedBuffer);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(DEBUG_ERROR, TEXT("FREE( %p ) failed. (%08x)"), lpDecodedBuffer, dwTempErrorCode);
				}
				lpDecodedBuffer = NULL;
			}
			dwDecodedBufferSize = 0;

			if (NULL != lpEncodedBuffer)
			{
				dwTempErrorCode = FREE(lpEncodedBuffer);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(DEBUG_ERROR, TEXT("FREE( %p ) failed. (%08x)"), lpEncodedBuffer, dwTempErrorCode);
				}
				lpEncodedBuffer = NULL;
			}
			dwEncodedBufferSize = 0;
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}


DWORD TestCompress(LPCBYTE lpOriginalBuffer, DWORD dwOriginalBufferSize)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	LPBYTE	lpEncodedBuffer = NULL;
	DWORD	dwEncodedBufferSize = 0;

	LPBYTE	lpDecodedBuffer = NULL;
	DWORD	dwDecodedBufferSize = NULL;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_Utilities %S start"), __func__);

			// Apply the encoding function
			DBGPRINT(DEBUG_INFO, TEXT("--------------------------------------------------------------------------------"));
			DBGPRINT(DEBUG_INFO, TEXT("Testing CompressBuffer..."));
			dwErrorCode = CompressBuffer((PBYTE)lpOriginalBuffer, dwOriginalBufferSize, &lpEncodedBuffer, &dwEncodedBufferSize);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("CompressBuffer failed.(%08x)"), dwErrorCode);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwEncodedBufferSize:   %d (%08x)"), dwEncodedBufferSize, dwEncodedBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpEncodedBuffer:       %p"), lpEncodedBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpEncodedBuffer:       %02x %02x %02x %02x %02x %02x %02x %02x"), lpEncodedBuffer[0], lpEncodedBuffer[1], lpEncodedBuffer[2], lpEncodedBuffer[3], lpEncodedBuffer[4], lpEncodedBuffer[5], lpEncodedBuffer[6], lpEncodedBuffer[7]);


			// Apply the decoding function
			DBGPRINT(DEBUG_INFO, TEXT("--------------------------------------------------------------------------------"));
			DBGPRINT(DEBUG_INFO, TEXT("Testing DecompressBuffer..."));
			dwErrorCode = DecompressBuffer((PBYTE)lpEncodedBuffer, dwEncodedBufferSize, &lpDecodedBuffer, &dwDecodedBufferSize);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("DecompressBuffer failed.(%08x)"), dwErrorCode);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwDecodedBufferSize:   %d (%08x)"), dwDecodedBufferSize, dwDecodedBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpDecodedBuffer:       %p"), lpDecodedBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpDecodedBuffer:       %02x %02x %02x %02x %02x %02x %02x %02x"), lpDecodedBuffer[0], lpDecodedBuffer[1], lpDecodedBuffer[2], lpDecodedBuffer[3], lpDecodedBuffer[4], lpDecodedBuffer[5], lpDecodedBuffer[6], lpDecodedBuffer[7]);


			// Compare the decoded results to the original to see if transformation was successful
			DBGPRINT(DEBUG_INFO, TEXT("--------------------------------------------------------------------------------"));
			if (memcmp(lpOriginalBuffer, lpDecodedBuffer, dwOriginalBufferSize) == 0)
			{
				DBGPRINT(DEBUG_INFO, TEXT("CompressBuffer/DecompressBuffer was SUCCESSFUL"));
				dwErrorCode = ERROR_SUCCESS;
			}
			else
			{
				DBGPRINT(DEBUG_WARNING, TEXT("CompressBuffer/DecompressBuffer FAILED!"));
				dwErrorCode = ERROR_NO_MATCH;
			}

		} // end try-finally
		__finally
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_Utilities %S end"), __func__);

			if (NULL != lpDecodedBuffer)
			{
				dwTempErrorCode = FREE(lpDecodedBuffer);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(DEBUG_ERROR, TEXT("FREE( %p ) failed. (%08x)"), lpDecodedBuffer, dwTempErrorCode);
				}
				lpDecodedBuffer = NULL;
			}
			dwDecodedBufferSize = 0;

			if (NULL != lpEncodedBuffer)
			{
				dwTempErrorCode = FREE(lpEncodedBuffer);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(DEBUG_ERROR, TEXT("FREE( %p ) failed. (%08x)"), lpEncodedBuffer, dwTempErrorCode);
				}
				lpEncodedBuffer = NULL;
			}
			dwEncodedBufferSize = 0;
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}


DWORD TestCompressEncryptEncode(LPCBYTE lpOriginalBuffer, DWORD dwOriginalBufferSize)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	LPBYTE	lpEncodedBuffer = NULL;
	DWORD	dwEncodedBufferSize = 0;

	LPBYTE	lpDecodedBuffer = NULL;
	DWORD	dwDecodedBufferSize = NULL;

	LPBYTE	lpPasswordBuffer[MAX_PASSWORD_LENGTH];
	DWORD	dwPasswordBufferSize = 0;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_Utilities %S start"), __func__);

			ZeroMemory(lpPasswordBuffer, MAX_PASSWORD_LENGTH);
			strcpy_s((LPSTR)lpPasswordBuffer, MAX_PASSWORD_LENGTH, DEFAULT_PASSWORD);
			dwPasswordBufferSize = (DWORD)strlen((LPSTR)lpPasswordBuffer);

			// Apply the encoding function
			DBGPRINT(DEBUG_INFO, TEXT("--------------------------------------------------------------------------------"));
			DBGPRINT(DEBUG_INFO, TEXT("Testing CompressEncryptEncodeBuffer..."));
			dwErrorCode = CompressEncryptEncodeBuffer((PBYTE)lpOriginalBuffer, dwOriginalBufferSize, (PBYTE)lpPasswordBuffer, dwPasswordBufferSize, &lpEncodedBuffer, &dwEncodedBufferSize );
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("CompressEncryptEncodeBuffer failed.(%08x)"), dwErrorCode);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwEncodedBufferSize:   %d (%08x)"), dwEncodedBufferSize, dwEncodedBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpEncodedBuffer:       %p"), lpEncodedBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpEncodedBuffer:       %02x %02x %02x %02x %02x %02x %02x %02x"), lpEncodedBuffer[0], lpEncodedBuffer[1], lpEncodedBuffer[2], lpEncodedBuffer[3], lpEncodedBuffer[4], lpEncodedBuffer[5], lpEncodedBuffer[6], lpEncodedBuffer[7]);


			// Apply the decoding function
			DBGPRINT(DEBUG_INFO, TEXT("--------------------------------------------------------------------------------"));
			DBGPRINT(DEBUG_INFO, TEXT("Testing DecodeDecryptDecompressBuffer..."));
			dwErrorCode = DecodeDecryptDecompressBuffer((PBYTE)lpEncodedBuffer, dwEncodedBufferSize, (PBYTE)lpPasswordBuffer, dwPasswordBufferSize, &lpDecodedBuffer, &dwDecodedBufferSize);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("DecodeDecryptDecompressBuffer failed.(%08x)"), dwErrorCode);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwDecodedBufferSize:   %d (%08x)"), dwDecodedBufferSize, dwDecodedBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpDecodedBuffer:       %p"), lpDecodedBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpDecodedBuffer:       %02x %02x %02x %02x %02x %02x %02x %02x"), lpDecodedBuffer[0], lpDecodedBuffer[1], lpDecodedBuffer[2], lpDecodedBuffer[3], lpDecodedBuffer[4], lpDecodedBuffer[5], lpDecodedBuffer[6], lpDecodedBuffer[7]);


			// Compare the decoded results to the original to see if transformation was successful
			DBGPRINT(DEBUG_INFO, TEXT("--------------------------------------------------------------------------------"));
			if (memcmp(lpOriginalBuffer, lpDecodedBuffer, dwOriginalBufferSize) == 0)
			{
				DBGPRINT(DEBUG_INFO, TEXT("CompressEncryptEncodeBuffer/DecodeDecryptDecompressBuffer was SUCCESSFUL"));
				dwErrorCode = ERROR_SUCCESS;
			}
			else
			{
				DBGPRINT(DEBUG_WARNING, TEXT("CompressEncryptEncodeBuffer/DecodeDecryptDecompressBuffer FAILED!"));
				dwErrorCode = ERROR_NO_MATCH;
			}

		} // end try-finally
		__finally
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_Utilities %S end"), __func__);

			if (NULL != lpDecodedBuffer)
			{
				dwTempErrorCode = FREE(lpDecodedBuffer);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(DEBUG_ERROR, TEXT("FREE( %p ) failed. (%08x)"), lpDecodedBuffer, dwTempErrorCode);
				}
				lpDecodedBuffer = NULL;
			}
			dwDecodedBufferSize = 0;

			if (NULL != lpEncodedBuffer)
			{
				dwTempErrorCode = FREE(lpEncodedBuffer);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(DEBUG_ERROR, TEXT("FREE( %p ) failed. (%08x)"), lpEncodedBuffer, dwTempErrorCode);
				}
				lpEncodedBuffer = NULL;
			}
			dwEncodedBufferSize = 0;
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}

DWORD wmain(INT argc, LPCWCHAR argv[], LPCWCHAR envp[])
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;
	
	WCHAR	lpswzFileName[4][MAX_PATH];
	DWORD	dwFileNameCount = 0;

	LPBYTE	lpOriginalBuffer = NULL;
	DWORD	dwOriginalBufferSize = 0;


	LPSTR	szMixTextTest = NULL;


	MIX_TEXT_INIT(2,50);


	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_Utilities wmain() start"));

			if (argc > 2)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(DEBUG_ERROR, TEXT("Invalid usage."));
				DBGPRINT(DEBUG_INFO, TEXT("%s <filename>"), argv[0]);
				DBGPRINT(DEBUG_INFO, TEXT("\t<filename> is optional file to run tests with"));
				__leave;
			}


			szMixTextTest = _("MixedTextString1");
			DBGPRINT(DEBUG_INFO, TEXT("Mixed1: %S"), szMixTextTest);
			DBGPRINT(DEBUG_INFO, TEXT("Mixed2: %S"), _("MixedTextString2"));



			if (1 < argc)
			{
				dwFileNameCount = 1;
				ZeroMemory(lpswzFileName[0], MAX_PATH);
				wcscpy_s(lpswzFileName[0], MAX_PATH, argv[1]);
			}
			else
			{
				dwFileNameCount = 4;
				ZeroMemory(lpswzFileName[0], MAX_PATH);
				wcscpy_s(lpswzFileName[0], MAX_PATH, SMALL_TEXT_FILENAME);
				ZeroMemory(lpswzFileName[1], MAX_PATH);
				wcscpy_s(lpswzFileName[1], MAX_PATH, LARGE_TEXT_FILENAME);
				ZeroMemory(lpswzFileName[2], MAX_PATH);
				wcscpy_s(lpswzFileName[2], MAX_PATH, SMALL_BINARY_FILENAME);
				ZeroMemory(lpswzFileName[3], MAX_PATH);
				wcscpy_s(lpswzFileName[3], MAX_PATH, LARGE_BINARY_FILENAME);
			}
			

			// Loop through all the test files
			for (DWORD i = 0; i < dwFileNameCount; i++)
			{
				DBGPRINT(DEBUG_INFO, TEXT("================================================================================"));
				DBGPRINT(DEBUG_INFO, TEXT("Running tests with \"%s\""), lpswzFileName[i]);


				// Read the current test file into a buffer
				dwErrorCode = ReadFileIntoBufferW(lpswzFileName[i], &lpOriginalBuffer, &dwOriginalBufferSize);
				if (ERROR_SUCCESS != dwErrorCode)
				{
					DBGPRINT(DEBUG_ERROR, TEXT("ReadFileIntoBufferW failed.(%08x)"), dwErrorCode);
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("dwOriginalBufferSize:  %d (%08x)"), dwOriginalBufferSize, dwOriginalBufferSize);
				DBGPRINT(DEBUG_VERBOSE, TEXT("lpOriginalBuffer:      %p"), lpOriginalBuffer);
				DBGPRINT(DEBUG_VERBOSE, TEXT("lpOriginalBuffer:      %02x %02x %02x %02x %02x %02x %02x %02x"), lpOriginalBuffer[0], lpOriginalBuffer[1], lpOriginalBuffer[2], lpOriginalBuffer[3], lpOriginalBuffer[4], lpOriginalBuffer[5], lpOriginalBuffer[6], lpOriginalBuffer[7]);
				
				

				//DBGPRINT(DEBUG_INFO, TEXT("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"));
				//DBGPRINT(DEBUG_INFO, TEXT("TestEncode..."));
				//dwErrorCode = TestEncode(lpOriginalBuffer, dwOriginalBufferSize);
				//if (ERROR_SUCCESS != dwErrorCode)
				//{
				//	DBGPRINT(DEBUG_ERROR, TEXT("TestEncode failed."));
				//}
				//else
				//{
				//	DBGPRINT(DEBUG_INFO, TEXT("TestEncode was successful."));
				//}



				//DBGPRINT(DEBUG_INFO, TEXT("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"));
				//DBGPRINT(DEBUG_INFO, TEXT("TestEncrypt..."));
				//dwErrorCode = TestEncrypt(lpOriginalBuffer, dwOriginalBufferSize);
				//if (ERROR_SUCCESS != dwErrorCode)
				//{
				//	DBGPRINT(DEBUG_ERROR, TEXT("TestEncrypt failed."));
				//}
				//else
				//{
				//	DBGPRINT(DEBUG_INFO, TEXT("TestEncrypt was successful."));
				//}


				//
				//DBGPRINT(DEBUG_INFO, TEXT("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"));
				//DBGPRINT(DEBUG_INFO, TEXT("TestCompress..."));
				//dwErrorCode = TestCompress(lpOriginalBuffer, dwOriginalBufferSize);
				//if (ERROR_SUCCESS != dwErrorCode)
				//{
				//	DBGPRINT(DEBUG_ERROR, TEXT("TestCompress failed."));
				//}
				//else
				//{
				//	DBGPRINT(DEBUG_INFO, TEXT("TestCompress was successful."));
				//}




				DBGPRINT(DEBUG_INFO, TEXT("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"));
				DBGPRINT(DEBUG_INFO, TEXT("TestCompressEncryptEncode..."));
				dwErrorCode = TestCompressEncryptEncode(lpOriginalBuffer, dwOriginalBufferSize);
				if (ERROR_SUCCESS != dwErrorCode)
				{
					DBGPRINT(DEBUG_ERROR, TEXT("TestCompressEncryptEncode failed."));
				}
				else
				{
					DBGPRINT(DEBUG_INFO, TEXT("TestCompressEncryptEncode was successful."));
				}



				// Clean up the buffers for the next file
				if (NULL != lpOriginalBuffer)
				{
					dwTempErrorCode = FREE(lpOriginalBuffer);
					if (ERROR_SUCCESS != dwTempErrorCode)
					{
						dwTempErrorCode = GetLastError();
						DBGPRINT(DEBUG_ERROR, TEXT("FREE( %p ) failed. (%08x)"), lpOriginalBuffer, dwTempErrorCode);
					}
					lpOriginalBuffer = NULL;
				}
				dwOriginalBufferSize = 0;

			} // end loop through test files

		} // end try-finally
		__finally
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_Utilities wmain() end"));
			
			if (NULL != lpOriginalBuffer)
			{
				dwTempErrorCode = FREE(lpOriginalBuffer);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(DEBUG_ERROR, TEXT("FREE( %p ) failed. (%08x)"), lpOriginalBuffer, dwTempErrorCode);
				}
				lpOriginalBuffer = NULL;
			}
			dwOriginalBufferSize = 0;
			

			MIX_TEXT_FINI;

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}
