#pragma once
#include <vector>
#include <stdint.h>
class bofpack
{
public:
	bofpack();
	void * getBuffer(unsigned int offset);
	void setCmdId(uint32_t cmdid);
	uint32_t getSize();
	template <typename t>
	void addscalar(t value)
	{
		if (finalized) return;
		uint32_t original_size = buffer.size();
		buffer.resize(original_size + sizeof(t));
		memcpy(&(buffer[original_size]), &value, sizeof(t));
	}
	template <typename t>
	void addstr(t string, unsigned int length) // should only be used with char* / wchar_t *, length does not have the terminating null
	{
		if (finalized) return;
		uint32_t original_size = buffer.size();
		if (length == 0)
		{
			length = GetStringLength(string);
		}
		if(length == 0 || string[length - 1] != 0) // if we're not null terminated we're adding it, if we already are, we're leaving it
			length += 1;
		length *= sizeof(string[0]); // turn our length into bytes
		buffer.resize(original_size + length + 4); //extra 4 for size we're about to input
		memcpy(&(buffer[original_size]), &length, 4);
		memcpy(&(buffer[original_size + 4]), string, length);
	}
	void addblob(void* data, unsigned int length)
	{
		if (finalized) return;
		uint32_t original_size = buffer.size();
		buffer.resize(original_size + length + 4);
		memcpy(&(buffer[original_size]), &length, 4);
		memcpy(&(buffer[original_size + 4]), data, length);
	}
	void clear();
private:
	template <typename t>
	int GetStringLength(t string) { return 0; }
	template<>
	int GetStringLength(const char* string) { return strlen(string); }
	template<>
	int GetStringLength(const wchar_t* string) { return wcslen(string); }
	std::vector<unsigned char> buffer;
	bool finalized = false;
};

