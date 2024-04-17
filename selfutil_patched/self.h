#pragma once


struct Self_Hdr // '<4s4B'
{
	u32 magic;
	u8 version;
	u8 mode;
	u8 endian;
	u8 attribs;
//};
//struct Self_ExtHdr // '<I2HQ2H4x'
//{
	u32 key_type;
	u16 header_size;
	u16 meta_size;
	u64 file_size;
	u16 num_entries;
	u16 flags;
	u8  pad[4];
};

struct Self_Entry {
	u64 props;
	u64 offs;
	u64 fileSz;
	u64 memSz;
};



#define PS4_PAGE_SIZE	0x4000
#define PS5_PAGE_SIZE	0x4000

#define PS4_PAGE_MASK	0x3FFF
#define PS5_PAGE_MASK	0x3FFF




#define ELF_MAGIC  0x464C457F // \x7F E L F

#define PS4_SELF_MAGIC 0x1D3D154F
#define PS5_SELF_MAGIC 0xEEF51454


// ExInfo::ptype
#define SELF_PT_FAKE          0x1
#define SELF_PT_NPDRM_EXEC    0x4
#define SELF_PT_NPDRM_DYNLIB  0x5
#define SELF_PT_SYSTEM_EXEC   0x8
#define SELF_PT_SYSTEM_DYNLIB 0x9 // including Mono binaries
#define SELF_PT_HOST_KERNEL   0xC
#define SELF_PT_SEC_MODULE    0xE
#define SELF_PT_SEC_KERNEL    0xF



/* SCE Custom Elf64 Entries */



/* SCE-specific definitions for e_type: */
#define ET_SCE_EXEC			0xFE00		/* SCE Executable file */
//#idk   ET_SCE_REPLAY_EXEC        = 0xfe01
#define ET_SCE_RELEXEC		0xFE04		/* SCE Relocatable Executable file */
#define ET_SCE_STUBLIB		0xFE0C		/* SCE SDK Stubs */
#define ET_SCE_DYNEXEC		0xFE10		/* SCE EXEC_ASLR */
#define ET_SCE_DYNAMIC		0xFE18		/* Unused */
#define ET_SCE_PSPRELEXEC	0xFFA0		/* Unused (PSP ELF only) */
#define ET_SCE_PPURELEXEC	0xFFA4		/* Unused (SPU ELF only) */
#define ET_SCE_UNK			0xFFA5		/* Unknown */



/* ?? */
#define PT_SCE_RELA			PT_LOOS	// .rela No +0x1000000 ?

#define PT_SCE_DYNLIBDATA	PT_LOOS + 0x1000000	// .sce_special
#define PT_SCE_PROCPARAM	PT_LOOS + 0x1000001	// .sce_process_param
#define PT_SCE_RELRO		PT_LOOS + 0x1000010	// .data.rel.ro
#define PT_SCE_COMMENT		PT_LOOS + 0xfffff00	// .sce_comment
#define PT_SCE_VERSION		PT_LOOS + 0xfffff01	// .sce_version



#define PT_GNU_EH_FRAME   0x6474E550  // .eh_frame_hdr
#define PT_GNU_STACK      0x6474E551



/* SCE_PRIVATE: bug 63164, add for objdump */
#define DT_SCE_IDTABENTSZ           0x61000005
#define DT_SCE_FINGERPRINT          0x61000007
#define DT_SCE_ORIGINAL_FILENAME    0x61000009
#define DT_SCE_MODULE_INFO          0x6100000d
#define DT_SCE_NEEDED_MODULE        0x6100000f
#define DT_SCE_MODULE_ATTR          0x61000011
#define DT_SCE_EXPORT_LIB           0x61000013
#define DT_SCE_IMPORT_LIB           0x61000015
#define DT_SCE_EXPORT_LIB_ATTR      0x61000017
#define DT_SCE_IMPORT_LIB_ATTR      0x61000019
#define DT_SCE_STUB_MODULE_NAME     0x6100001d
#define DT_SCE_STUB_MODULE_VERSION  0x6100001f
#define DT_SCE_STUB_LIBRARY_NAME    0x61000021
#define DT_SCE_STUB_LIBRARY_VERSION 0x61000023
#define DT_SCE_HASH                 0x61000025
#define DT_SCE_PLTGOT               0x61000027
#define DT_SCE_JMPREL               0x61000029
#define DT_SCE_PLTREL               0x6100002b
#define DT_SCE_PLTRELSZ             0x6100002d
#define DT_SCE_RELA                 0x6100002f
#define DT_SCE_RELASZ               0x61000031
#define DT_SCE_RELAENT              0x61000033
#define DT_SCE_STRTAB               0x61000035
#define DT_SCE_STRSZ                0x61000037
#define DT_SCE_SYMTAB               0x61000039
#define DT_SCE_SYMENT               0x6100003b
#define DT_SCE_HASHSZ               0x6100003d
#define DT_SCE_SYMTABSZ             0x6100003f

#define SHT_SCE_NID          0x61000001
#define SHT_SCE_IDK          0x09010102


enum { EF_ORBIS_FUNCTION_DATA_SECTIONS = 0x04000000 };

/* SCE_PRIVATE: bug 13593, add a special relocation that allows the
   linker to more efficiently optimize some loads to not use a got
   entry.  The compiler generates it, SN linker will patch the
   instruction, the gold linker will treat it like a
   R_X86_64_GOTPCREL relocation. */
#define R_X86_64_ORBIS_GOTPCREL_LOAD 40 // RELOC_NUMBER(R_X86_64_ORBIS_GOTPCREL_LOAD, 40)
//HOWTO(R_X86_64_ORBIS_GOTPCREL_LOAD, 0, 2, 32, TRUE, 0, complain_overflow_signed, bfd_elf_generic_reloc, "R_X86_64_ORBIS_GOTPCREL_LOAD", FALSE, 0xffffffff, 0xffffffff, TRUE),












