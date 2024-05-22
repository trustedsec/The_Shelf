#include "rc4.h"
#include "stringobf.h"
#include <exception>
#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS
#include <winternl.h>
#include <ntstatus.h>

unsigned char keydata[] = { 0xef, 0xdc, 0xa5, 0x05, 0x39, 0x57, 0xa7, 0xfb, 0x46, 0x50, 0x2d, 0x07, 0xee, 0x76, 0xa9, 0xd0, 0xf0, 0x7f, 0xe9, 0xb5, 0xb9, 0xbd, 0x62, 0x57, 0xf3, 0x63, 0xd7, 0xda, 0x89, 0x17, 0x74, 0x15, 0x15, 0x35, 0xa0, 0xd9, 0x92, 0xe4, 0x12, 0x80 };

rc4::rc4(LPVOID key, DWORD keylen) : key(key), keylen(keylen), hBcrypt(nullptr), hAlg(nullptr), hKey(nullptr)
{
	char* err = OBF("Failed to initialize bcrypt");
	hBcrypt = LoadLibraryA(OBF("bcrypt.dll"));
	if (hBcrypt == NULL)
	{
		throw std::exception(err);
	}
	//rc4 supports a key between 8 and 512 bits long on windows (1-64 chars)
	fpBCryptOpenAlgorithmProvider = (fBCryptOpenAlgorithmProvider)GetProcAddress(hBcrypt, OBF("BCryptOpenAlgorithmProvider"));
	fpBCryptGetProperty = (fBCryptGetProperty)GetProcAddress(hBcrypt, OBF("BCryptGetProperty"));
	fpBCryptSetProperty = (fBCryptSetProperty)GetProcAddress(hBcrypt, OBF("BCryptSetProperty"));
	fpBCryptGenerateSymmetricKey = (fBCryptGenerateSymmetricKey)GetProcAddress(hBcrypt, OBF("BCryptGenerateSymmetricKey"));
	fpBCryptDestroyKey = (fBCryptDestroyKey)GetProcAddress(hBcrypt, OBF("BCryptDestroyKey"));
	fpBCryptCloseAlgorithmProvider = (fBCryptCloseAlgorithmProvider)GetProcAddress(hBcrypt, OBF("BCryptCloseAlgorithmProvider"));
	fpBCryptDecrypt = (fBCryptDecrypt)GetProcAddress(hBcrypt, OBF("BCryptDecrypt"));
	fpBCryptEncrypt = (fBCryptEncrypt)GetProcAddress(hBcrypt, OBF("BCryptEncrypt"));
	if (!NT_SUCCESS(fpBCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RC4_ALGORITHM, NULL, 0)))
	{
		throw std::exception(err);
	}
	if (!NT_SUCCESS(fpBCryptGenerateSymmetricKey(hAlg, &hKey, NULL, 0, (key) ? (PUCHAR)key : (PUCHAR)keydata, (key) ? keylen : sizeof(keydata), 0)))
	{
		throw std::exception(err);
	}

}

rc4::~rc4()
{
	if(hKey)
		fpBCryptDestroyKey(&hKey);
	if (hAlg)
		fpBCryptCloseAlgorithmProvider(&hAlg, 0);
	if(hBcrypt != nullptr)
		FreeLibrary(hBcrypt);

}




bool rc4::encrypt(void* data, uint32_t length)
{
	unsigned long _;
	if (!NT_SUCCESS(fpBCryptEncrypt(hKey, (PUCHAR)data, length, NULL, NULL, 0, (PUCHAR)data, length, &_, 0)))
	{
		return false;
	}
	return true;
}

bool rc4::encrypt(std::vector<byte>& data, int offset, int length)
{
	if (offset + length > data.size())
	{
		return false;
	}
	return encrypt((void*)&(data[offset]), length);
}

bool rc4::decrypt(void* data, uint32_t length)
{
	unsigned long _;
	NTSTATUS status = fpBCryptDecrypt(hKey, (PUCHAR)data, length, NULL, NULL, 0, (PUCHAR)data, length, &_, 0);
	if (!NT_SUCCESS(status))
	{
		return false;
	}
	return true;
}
