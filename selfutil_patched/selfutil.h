#pragma once

#include "elf.h"
#include "self.h"


using namespace std;

class SelfUtil
{
	vector<u8> data, save;

	Self_Hdr * seHead;
	vector<Self_Entry*> entries;

	elf64_hdr* eHead;
	unat elfHOffs;

	vector<Elf64_Phdr*> phdrs;


public:
	SelfUtil()
	{	}

	SelfUtil(string filePath)
	{
		if(!Load(filePath))
			printf("Error, Load() failed!\n");
	}

	
	bool Parse();
	bool TestIdent();

	bool Load(string filePath);
	bool SaveToELF(string savePath);
};

