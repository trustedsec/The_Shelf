#pragma once
#include <Windows.h>
#include <bcrypt.h>
#include <stdint.h>
#include <vector>

typedef NTSTATUS(WINAPI* fBCryptOpenAlgorithmProvider)(BCRYPT_ALG_HANDLE* phAlgorithm, LPCWSTR pszAlgId, LPCWSTR pszImplementation, ULONG dwFlags);
typedef NTSTATUS(WINAPI* fBCryptGetProperty) (BCRYPT_HANDLE hObject, LPCWSTR pszProperty, PUCHAR pbOutput, ULONG cbOutput, ULONG* pcbResult, ULONG dwFlags);
typedef NTSTATUS(WINAPI* fBCryptSetProperty) (BCRYPT_HANDLE hObject, LPCWSTR pszProperty, PUCHAR pbInput, ULONG cbInput, ULONG dwFlags);
typedef NTSTATUS(WINAPI* fBCryptGenerateSymmetricKey) (BCRYPT_ALG_HANDLE hAlgorithm, BCRYPT_KEY_HANDLE* phKey, PUCHAR pbKeyObject, ULONG cbKeyObject, PUCHAR pbSecret, ULONG cbSecret, ULONG dwFlags);
typedef NTSTATUS(WINAPI* fBCryptDestroyKey) (BCRYPT_KEY_HANDLE hKey);
typedef NTSTATUS(WINAPI* fBCryptCloseAlgorithmProvider) (BCRYPT_ALG_HANDLE hAlgorithm, ULONG dwFlags);
typedef NTSTATUS(WINAPI* fBCryptDecrypt) (BCRYPT_KEY_HANDLE hKey, PUCHAR pbInput, ULONG cbInput, VOID* pPaddingInfo, PUCHAR pbIV, ULONG cbIV, PUCHAR pbOutput, ULONG cbOutput, ULONG* pcbResult, ULONG dwFlags);
typedef NTSTATUS(WINAPI* fBCryptEncrypt) (BCRYPT_KEY_HANDLE hKey, PUCHAR pbInput, ULONG cbInput, VOID* pPaddingInfo, PUCHAR pbIV, ULONG cbIV, PUCHAR pbOutput, ULONG cbOutput, ULONG* pcbResult, ULONG dwFlags);


class rc4
{
public:
	rc4(LPVOID key, DWORD keylen);
	~rc4();
	bool encrypt(void* data, uint32_t length);
	bool encrypt(std::vector<byte>& data, int offset, int length);
	bool decrypt(void* data, uint32_t length);

private:
	HMODULE hBcrypt;
	BCRYPT_ALG_HANDLE hAlg;
	BCRYPT_KEY_HANDLE hKey;
	LPVOID key;
	DWORD keylen;
	fBCryptOpenAlgorithmProvider fpBCryptOpenAlgorithmProvider ;
	fBCryptGetProperty fpBCryptGetProperty ;
	fBCryptSetProperty fpBCryptSetProperty ;
	fBCryptGenerateSymmetricKey fpBCryptGenerateSymmetricKey ;
	fBCryptDestroyKey fpBCryptDestroyKey ;
	fBCryptCloseAlgorithmProvider fpBCryptCloseAlgorithmProvider ;
	fBCryptDecrypt fpBCryptDecrypt ;
	fBCryptEncrypt fpBCryptEncrypt;
};

