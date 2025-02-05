#pragma once
#include "main.h"

extern "C" void ReadAll_asm(LARGE_INTEGER * global_array);

class RWClass
{
public:
	enum class Register
	{
		CR0 = 1,
		CR2 = 2,
		CR3 = 3,
		CR4 = 4,
		IA32_LSTAR = 5,
	};

	RWClass(Register param)
	{
		target = param;
	}

	~RWClass() {}

public:
	LARGE_INTEGER ReadRegister();
	NTSTATUS WriteRegister(LARGE_INTEGER input_data);
	BOOLEAN ReplaceReg(Register param);

private:
	#define ARRAY_SIZE 5
	VOID DisableCR0WP();
	VOID EnableCR0WP();
	LARGE_INTEGER global_array[ARRAY_SIZE] = { 0 };
	Register target;
};