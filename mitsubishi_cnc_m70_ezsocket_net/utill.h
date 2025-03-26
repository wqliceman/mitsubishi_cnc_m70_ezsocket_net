#ifndef __UTILL_H__
#define __UTILL_H__

#include "typedef.h"

typedef struct _tag_byte_array_info
{
	byte* data; // Content
	int length; // Length
} byte_array_info;

typedef struct _tag_bool_array_info
{
	bool* data; // Content
	int length; // Length
} bool_array_info;

void short_to_bytes(short i, byte* bytes);
short bytes_to_short(byte* bytes);

void ushort_to_bytes(ushort i, byte* bytes);
ushort bytes_to_ushort(byte* bytes);

void int32_to_bytes(int32 i, byte* bytes);
int32 bytes_to_int32(byte* bytes);

void uint32_to_bytes(uint32 i, byte* bytes);
uint32 bytes_to_uint32(byte* bytes);

void big_nt_to_bytes(int64 i, byte* bytes);
int64 bytes_to_big_int(byte* bytes);

void ubig_int_to_bytes(uint64 i, byte* bytes);
uint64 bytes_to_ubig_int(byte* bytes);

void float_to_bytes(float i, byte* bytes);
float bytes_to_float(byte* bytes);

void double_to_bytes(double i, byte* bytes);
double bytes_to_double(byte* bytes);

int str_to_int(const char* address);
void str_toupper(char* input);
void str_tolower(char* input);
int str_start_with(const char* origin, char* prefix);

uint32 htonf_(float value);
float ntohf_(uint32 value);
uint64 htond_(double value);
double ntohd_(uint64 value);
uint64 htonll_(uint64 Value);
uint64 ntohll_(uint64 Value);

#ifndef _WIN32
char* itoa(unsigned long long value, char str[], int radix);
#endif // !_WIN32

bool is_little_endian();

#endif
