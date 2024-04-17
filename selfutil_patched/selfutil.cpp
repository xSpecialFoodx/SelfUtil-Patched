// selfutil.cpp : 
//

#include "pch.h"

#include "selfutil.h"
#include <filesystem>

//
// //
// // //
// // // // SelfUtil
// // //
// //
//

// notes :
// debug needs to be configured as Active x86


void print_usage()
{
	printf("selfutil [--input] [--output] [--dry-run] [--verbose] [--overwrite] [--align-size] [--not-patch-first-segment-duplicate] [--not-patch-version-segment]\n");
}

string input_file_path = "";
string output_file_path = "";
bool dry_run = false;
bool verbose = false;
bool overwrite = false;
bool align_size = false;
bool patch_first_segment_duplicate = true;
Elf64_Off patch_first_segment_safety_percentage = 2;// min amount of cells (in percentage) that should fit in other words
bool patch_version_segment = true;

int first_min_offset = -1;

int main(int argc, char* argv[])
{
	vector<string> args;

	auto inputted_args = argv;
	int inputted_args_amount = argc;

	//string inputted_args[] = { "doesnt_matter", "C:/somefolder/somefile.prx" };
	//int inputted_args_amount = sizeof(inputted_args) / sizeof(inputted_args[0]);

	if (inputted_args_amount < 2) {
		print_usage();
		exit(0);
	}

	bool error_found = false;

	for (int inputted_args_index = 1; inputted_args_index < inputted_args_amount; inputted_args_index++)
		args.push_back(inputted_args[inputted_args_index]);

	if (inputted_args_amount == 2)
		input_file_path = args[0];
	else
		for (int inputted_args_index = 0; inputted_args_index < inputted_args_amount - 1; inputted_args_index++)
			if (args[inputted_args_index] == "--input")
			{
				input_file_path = args[inputted_args_index + 1];

				inputted_args_index++;
			}
			else if (args[inputted_args_index] == "--output")
			{
				output_file_path = args[inputted_args_index + 1];

				inputted_args_index++;
			}
			else if (args[inputted_args_index] == "--dry-run")
				dry_run = true;
			else if (args[inputted_args_index] == "--verbose")
				verbose = true;
			else if (args[inputted_args_index] == "--overwrite")
				overwrite = true;
			else if (args[inputted_args_index] == "--align-size")
				align_size = true;
			else if (args[inputted_args_index] == "--not-patch-first-segment-duplicate")
				patch_first_segment_duplicate = false;
			else if (args[inputted_args_index] == "--not-patch-version-segment")
				patch_version_segment = false;
			else
			{
				error_found = true;

				break;
			}

	if (error_found == true || input_file_path == "")
	{
		print_usage();
		exit(0);
	}
	else
	{
		string fixed_output_file_path = "";

		if (output_file_path != "")
			fixed_output_file_path = output_file_path;
		else
		{
			fixed_output_file_path = input_file_path;

			if (overwrite == false)
			{
				size_t newSize = input_file_path.rfind('.');	// *FIXME* if this is already named .elf it will save to same file!

				if (newSize > 0)
				{
					fixed_output_file_path.resize(newSize);

					fixed_output_file_path += ".elf";
				}
			}
		}

		printf(
			"%s: %s\n%s: %s\n%s: %s\n%s: %s\n%s: %s\n%s: %s\n"
			, "Input File Name", input_file_path.c_str()
			, "Output File Name", fixed_output_file_path.c_str()
			, "Dry Run", ((dry_run == true) ? "True" : "False")
			, "Verbose", ((verbose == true) ? "True" : "False")
			, "Overwrite", ((overwrite == true) ? "True" : "False")
			, "Align Size", ((align_size == true) ? "True" : "False")
			, "Patch First Segment Duplicate", ((patch_first_segment_duplicate == true) ? "True" : "False")
			, "Patch Version Segment", ((patch_version_segment == true) ? "True" : "False")
			);

		SelfUtil util(input_file_path);

		if (!util.SaveToELF(fixed_output_file_path))
			printf("Error, Save to ELF failed!\n");

		return 0;
	}
}








bool compare_u8_array(u8* first_array, u8* second_array, int size)
{
	bool result = true;

	for (int size_index = 0; size_index < size; size_index++)
		if (first_array[size_index] != second_array[size_index])
		{
			result = false;

			break;
		}

	return result;
}

void set_u8_array(u8* first_array, int value, int size)
{
	for (int size_index = 0; size_index < size; size_index++)
		first_array[size_index] = value;
}

bool SelfUtil::Load(string filePath)
{
	if (!filesystem::exists(filePath)) {
		printf("Failed to find file: \"%s\" \n", filePath.c_str());
		return false;
	}

	size_t fileSize = (size_t)filesystem::file_size(filePath);
	data.resize(fileSize);

	FILE* f = nullptr;
	fopen_s(&f, filePath.c_str(), "rb");
	if (f) {
		fread(&data[0], 1, fileSize, f);
		fclose(f);
	}
	else {
		printf("Failed to open file: \"%s\" \n", filePath.c_str());
		return false;
	}

	return Parse();
}

bool SelfUtil::SaveToELF(string savePath)
{
	if (verbose == true)
	{
		printf("\n");
		printf("SaveToELF(\"%s\")\n", savePath.c_str());
	}

	Elf64_Off first, last;
	size_t saveSize;
	Elf64_Phdr
		*ph_first = NULL
		, *ph_last = NULL
		, *ph_PT_SCE_VERSION = NULL;

	bool patched_PT_SCE_VERSION = false;

	for (auto ph : phdrs)
	{
		if (ph->p_type == PT_SCE_VERSION)
			ph_PT_SCE_VERSION = ph;

		if (
			ph_first == NULL
			||
			ph_first->p_offset == 0// try to get away from offset 0
			||
			(
				// if the current first ph is not null and its offset is bigger than 0
				// , then replace it only with a smaller ph that its offset is also bigger than 0
				ph->p_offset > 0
				&& ph->p_offset < ph_first->p_offset
				)
			)
			ph_first = ph;

		if (
			ph_last == NULL
			||
			ph->p_offset > ph_last->p_offset
			)
			ph_last = ph;
	}

	if (ph_first == NULL)
		first = 0;
	else
		first = ph_first->p_offset;

	if (ph_last == NULL)
	{
		last = 0;

		saveSize = 0;
	}
	else
	{
		last = ph_last->p_offset;

		saveSize = (size_t)(ph_last->p_offset + ph_last->p_filesz);

		if (align_size == true)
			saveSize = AlignUp<size_t>(saveSize, 0x10);// in the original selfutil it was an alignment of PS4_PAGE_SIZE
	}

	if (verbose == true)
	{
		printf("\n");
		printf("Save Size: %d bytes (0x%X)\n", saveSize, saveSize);
		printf("first: %llX, last: %llX\n", first, last);
	}

	save.clear();
	save.resize(saveSize);

	u8* pd = &save[0];
	memset(pd, 0, saveSize);


#if 1
	memcpy(pd, eHead, first);	// just copy everything from head to what should be first seg offs //
#else
	memcpy(pd, eHead, sizeof(elf64_hdr));

#if 0	// and hope it took care of all of this 
	Elf64_Half e_ehsize;

	Elf64_Half e_phentsize;
	Elf64_Half e_phnum;

	Elf64_Half e_shentsize;
	Elf64_Half e_shnum;
#endif
#endif

	for (auto ee : entries)
	{
		bool method_found = false;
		unat phIdx = (ee->props >> 20) & 0xFFF;

		Elf64_Phdr* ph = phdrs.at(phIdx);

		if ((ee->props & 0x800) == 0)
		{
			if (ph_PT_SCE_VERSION != NULL && ph == ph_PT_SCE_VERSION)
				if (patch_version_segment == true)
					method_found = true;
		}
		else
			method_found = true;

		if (method_found == true)
		{
			method_found = false;

			if (ph->p_filesz != NULL && ph->p_filesz != ee->memSz)
				if (verbose == true)
					printf("idx: %d SEGMENT size: %d != phdr size: %d \n", phIdx, ee->memSz, ph->p_filesz);

			void* srcp = NULL;
			void* dstp = (void*)((unat)pd + ph->p_offset);
			size_t datasize = 0;

			if (ph_PT_SCE_VERSION != NULL && ph == ph_PT_SCE_VERSION)
			{
				patched_PT_SCE_VERSION = true;

				printf("\n");
				printf("patching version segment\n");

				if (verbose == true)
					printf(
						"%s: 0x%08llX\t%s: 0x%08llX\n"
						, "segment address", data.capacity() - ph->p_filesz
						, "segment size", ph->p_filesz
						);

				srcp = (void*)((unat)&data[0] + data.capacity() - ph->p_filesz);
				datasize = ph->p_filesz;

				memcpy(dstp, srcp, datasize);

				printf("patched version segment\n");
			}
			else
			{
				srcp = (void*)((unat)&data[0] + ee->offs);
				datasize = ee->fileSz;

				memcpy(dstp, srcp, datasize);
			}
		}
	}

	if (patch_version_segment == true)
		if (patched_PT_SCE_VERSION == false)
			if (ph_PT_SCE_VERSION != NULL)
			{
				Elf64_Phdr* ph = ph_PT_SCE_VERSION;

				printf("\n");
				printf("patching version segment\n");

				if (verbose == true)
					printf(
						"%s: 0x%08llX\t%s: 0x%08llX\n"
						, "segment address", data.capacity() - ph->p_filesz
						, "segment size", ph->p_filesz
						);

				void* srcp = (void*)((unat)&data[0] + data.capacity() - ph->p_filesz);
				void* dstp = (void*)((unat)pd + ph->p_offset);
				size_t datasize = ph->p_filesz;

				memcpy(dstp, srcp, datasize);

				printf("patched version segment\n");
			}

	if (patch_first_segment_duplicate == true)
	{
		int entries_amount = entries.size();

		for (int entries_index = 0; entries_index < entries_amount; entries_index++)
			if (
				entries[entries_index]->offs - elfHOffs >= 0
				&& entries[entries_index]->offs - elfHOffs < first
				)
			{
				if (
					first_min_offset == -1
					|| entries[entries_index]->offs - elfHOffs > first_min_offset
					)
					first_min_offset = entries[entries_index]->offs - elfHOffs;
			}

		if (first_min_offset != -1)
			if (pd[first_min_offset] == 0)
			{
				// go forward looking for data

				for (int pd_index = 1; (Elf64_Off)first_min_offset + pd_index < first; pd_index++)
					if (pd[first_min_offset + pd_index] != 0)
					{
						first_min_offset += pd_index - 1;// go 1 place before the zero

						break;
					}
			}

		for (
			int first_index = 0;
			(
				first_min_offset == -1
				|| first_index < first_min_offset
				)
			&& (
				first_index < (first * (100 - patch_first_segment_safety_percentage) / 100)
				&& first - first_index >= 0x000000C0
				);
			first_index++
			)
			if (
				compare_u8_array(
					pd + first_index
					, pd + first
					, (first - first_index >= 0x000000C0 ? 0x000000C0 : first - first_index)
					) == true
				)
			{
				// was first - first_index instead of 0x000000C0
				// , but usually the first section's important data is at the size of 0xC0 and that goes for all the modules

				first_min_offset = first_index;

				break;
			}

		if (first_min_offset != -1)
		{
			printf("\n");
			printf("patching first segment duplicate\n");

			if (verbose == true)
				printf(
					"%s: 0x%08X\t%s: 0x%08X\n"
					, "address", first_min_offset
					, "size", first - first_min_offset
					);

			set_u8_array(pd + first_min_offset, 0, first - first_min_offset);

			printf("patched first segment duplicate\n");
		}
	}


	if (dry_run == false)
	{
		FILE* f = nullptr;

		fopen_s(&f, savePath.c_str(), "wb");

		if (f) {
			fwrite(pd, 1, saveSize, f);
			fclose(f);
		}
		else
			return false;
	}

	return true;
}






bool SelfUtil::Parse()
{
	seHead = (Self_Hdr*)&data[0];

	if (seHead->magic == PS4_SELF_MAGIC)
	{
		if (verbose == true)
			printf("Valid PS4 Magic");

		if (data.size() < PS4_PAGE_SIZE)
			if (verbose == true)
				printf("Small file size! (%d)\nContinuing regardless.\n", data.size());
	}
	else if (seHead->magic == PS5_SELF_MAGIC)
	{
		if (verbose == true)
			printf("Valid PS5 Magic");

		if (data.size() < PS5_PAGE_SIZE)
			if (verbose == true)
				printf("Small file size! (%d)\nContinuing regardless.\n", data.size());
	}
	else
	{
		printf("Invalid Magic! (0x%08X)\n", seHead->magic);
		return false;
	}

	entries.clear();
	for (unat seIdx = 0; seIdx < seHead->num_entries; seIdx++)
	{
		entries.push_back(&((Self_Entry*)&data[0])[1 + seIdx]);

		const auto se = entries.back();

		if (verbose == true)
		{
			printf("Segment[%02d] P:%08X ", seIdx, se->props);
			printf(" (id: %X) ", (se->props >> 20));
			printf("@ 0x%016llX +%llX   (mem: %llX)\n", se->offs, se->fileSz, se->memSz);
		}
	}

	elfHOffs = (1 + seHead->num_entries) * 0x20;

	eHead = (elf64_hdr*)(&data[0] + elfHOffs);

	if (!TestIdent()) {
		printf("Elf e_ident invalid!\n");
		return false;
	}


	for (unat phIdx = 0; phIdx < eHead->e_phnum; phIdx++)
		phdrs.push_back(&((Elf64_Phdr*)(&data[0] + elfHOffs + eHead->e_phoff))[phIdx]);


	return true;
}






bool SelfUtil::TestIdent()
{
	if (ELF_MAGIC != ((u32*)eHead->e_ident)[0]) {
		printf("File is invalid! e_ident magic: %08X\n", ((u32*)eHead->e_ident)[0]);
		return false;
	}
	if (!(
		(eHead->e_ident[EI_CLASS] == ELFCLASS64) &&
		(eHead->e_ident[EI_DATA] == ELFDATA2LSB) &&
		(eHead->e_ident[EI_VERSION] == EV_CURRENT) &&
		(eHead->e_ident[EI_OSABI] == ELFOSABI_FREEBSD)))
		return false;

	if ((eHead->e_type >> 8) != 0xFE) // != ET_SCE_EXEC)
		printf(" Elf64::e_type: 0x%04X \n", eHead->e_type);

	if (!((eHead->e_machine == EM_X86_64) && (eHead->e_version == EV_CURRENT)))
		return false;

	return true;
}