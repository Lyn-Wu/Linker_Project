#ifndef _LD_32_
#define _LD_32_
#endif

#include "../../include/x86/ld32.h"
#include "../../include/include/constvar.h"
#include "../../include/include/elf32_common.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * These valiuables defined in kernel/main.c
 */
extern _Bool DisplayELFHeader;
extern long  nFiles;
extern _Bool DisplayELFSections;
extern _Bool DisplayELFSymbolTable;


static char *
Elf32_Type(Elf32_Word Type)
{
	Elf32_Word Index;

	if(Type == 0xfe00)
		Index = 5;
	else if(Type == 0xfeff)
		Index = 6;
	else if(Type == 0xff00)
		Index = 7;
	else if(Type == 0xffff)
		Index = 8;
	else 
		Index = Type;
	return (FileTypes[Index]);
}


static char *
Elf32_Machine(Elf32_Word Machine){
	Elf32_Word Index;

	if(Machine == 183)
		Index = 112;
	else if(Machine == 243)
		Index = 113;
	else
		Index = Machine;

	return (MachineTypes[Index]);
}

static void
Elf32_DisplayELFHeader(Elf32_Ehdr *Ehdr)
{
	char *Magic = Ehdr->e_ident;
	printf("Magic:	");
	for(int Index = 0;Index < 16;Index++,Magic += 1) {
		printf("%02x ",*Magic);
	}
	printf("\n");

	printf("ELF type: 			0x%08x %s\n",Ehdr->e_type,Elf32_Type(Ehdr->e_type));
	printf("Machine:  			0x%08x (%s)\n",Ehdr->e_machine,Elf32_Machine(Ehdr->e_machine));
	printf("Version:  			0x%08x\n",Ehdr->e_version);
	printf("Entry Point: 			0x%08x\n",Ehdr->e_entry);
	printf("Program offset: 		0x%08x (bytes into file)\n",Ehdr->e_phoff);
	printf("Sections offset: 		0x%08x (bytes into file)\n",Ehdr->e_shoff);
	printf("Flags:	  			0x%08x\n",Ehdr->e_flags);
	printf("Header Size: 			0x%08x (bytes)\n",Ehdr->e_ehsize);
	printf("Program Header's Size: 		0x%08x (bytes)\n",Ehdr->e_phentsize);
	printf("Number of Program Header: 	0x%08x\n",Ehdr->e_phnum);
	printf("Size of Section: 		0x%08x (bytes)\n",Ehdr->e_shentsize);
	printf("Number of section headers: 	0x%08x\n",Ehdr->e_shnum);
	printf("String table Index: 		0x%08x\n",Ehdr->e_shstrndx);
}

error_t
Elf32_getELFHeader(Elf32_Ehdr *Ehdr,FILE *ObjectFile)
{
	fread(Ehdr,sizeof(Elf32_Ehdr),1,ObjectFile);
	return (0);
}

error_t
Elf32_ReadELFHeader(char *FileName[])
{
	int ErrorCode = 0;
	FILE *ObjectFile = NULL;

	int nFile = 0;
	
	Elf32_Ehdr *EhdrIter = Elf32_Ehdrs.Elf32_Ehdrs;

	for(;nFile < nFiles;nFile++,EhdrIter += 1) {
		ObjectFile = fopen(FileName[nFile + 1],"r");

		if(ObjectFile == NULL) {
			printf("No Such File: %s\n",FileName[nFile + 1]);
			ErrorCode = -1;
			exit(ErrorCode);
		}

		Elf32_getELFHeader(EhdrIter,ObjectFile);

		if(DisplayELFHeader) {
			printf("File Name: %s\n",FileName[nFile + 1]);
			Elf32_DisplayELFHeader(EhdrIter);
			printf("\n");
		}
		fclose(ObjectFile);
	}
}


#define DisplayFormatTitle(flags)		do{\
						printf("[Nr] Name%16cType%16cAddr%10cOff%10cSize	ES Flg Lk Inf Al\n",' ',' ',' ',' ');\
						}while(0);

static void
PrintNSpace(Elf32_Word Spaces)
{
	if(Spaces <0)
		Spaces = 1;

	while(Spaces--)
		printf("%c",' ');
}

static Elf32_Word
PrintFlags(Elf32_Word flags)
{
	char *Flags = (char *)malloc(sizeof(char) * MAX_SECTION_FLAGS + 1);
	short Index = 0;

	if(flags & SHF_WRITE) 
		Flags[Index++] = 'W';
	if(flags & SHF_ALLOC)
		Flags[Index++] = 'A';
	if(flags & SHF_EXECINSTR)
		Flags[Index++] = 'E';
	if(flags & SHF_MERGE)
		Flags[Index++] = 'M';
	if(flags & SHF_STRINGS)
		Flags[Index++] = 'S';
	if(flags & SHF_INFO_LINK)
		Flags[Index++] = 'I';
	if(flags & SHF_LINK_ORDER)
		Flags[Index++] = 'L';
	if(flags & SHF_OS_NONCONFORMING)
		Flags[Index++] = 'O';
	if(flags & SHF_GROUP)
		Flags[Index++] = 'G';
	if(flags & SHF_TLS)
		Flags[Index++] = 'T';
	if(flags & SHF_COMPRESSED)
		Flags[Index++] = 'C';
	if(flags & SHF_MASKOS)
		Flags[Index++] = 'K';
	if(flags & SHF_MASKPROC)
		Flags[Index++] = 'P';
	Flags[Index] = '\0';
	printf("%s",Flags);
	return (Index);	
}

static void
DisplayFormatSectionInfo(Elf32_Shdr *Shdr,char *SourceString)
{
	Elf32_Word Row,Col;
	getWindowSize();

	Row = getWindowSizeRow();
	Col = getWindowSizeCol();

	Elf32_Word StringLength = strlen(SourceString);

	PrintNSpace(20 - StringLength);
	printf("%s",SectionType[Shdr->sh_type]);

	StringLength = strlen(SectionType[Shdr->sh_type]);
	PrintNSpace(20 - StringLength);
	printf("%08x",Shdr->sh_addr);

	StringLength = 8;
	PrintNSpace(14 - StringLength);
	printf("%06x",Shdr->sh_offset);

	StringLength = 6;
	PrintNSpace(13 - StringLength);
	printf("%06x",Shdr->sh_size);

	StringLength = 6;
	PrintNSpace(8 - StringLength);
	printf("%02x",Shdr->sh_entsize);

	printf(" ");
	StringLength = PrintFlags(Shdr->sh_flags);

	PrintNSpace(4 - StringLength);
	printf("%2d",Shdr->sh_link);

	PrintNSpace(3);
	printf("%d",Shdr->sh_info);

	StringLength = Shdr->sh_info > 10 ? 2 : 1;
	PrintNSpace(3 - StringLength);
	printf("%-d\n",Shdr->sh_addralign);

}

error_t
Elf32_DisplaySections(Elf32_Shdr *Shdr,Elf32_Ehdr *Ehdr,FILE *ObjectFile) 
{
	Elf32_Off Entry = 0;
	Elf32_Off SectionsStringSectionIndex = Ehdr->e_shstrndx;
	Elf32_Word SectionsNumber = Ehdr->e_shnum;

	Elf32_Shdr *SectionsStringSection = Shdr + SectionsStringSectionIndex;
	Elf32_Shdr *ShdrIter = Shdr;

	char *SectionsName = (char *)malloc(sizeof(char) * SectionsStringSection->sh_size);

	fseek(ObjectFile,SectionsStringSection->sh_offset,SEEK_SET);
	fread(SectionsName,sizeof(char),SectionsStringSection->sh_size,ObjectFile);

	DisplayFormatTitle(NULL);

	for(;Entry < SectionsNumber;Entry++,ShdrIter += 1) {
		if(Entry >= 10) 
			printf("[%d] ",Entry);
		else
			printf("[ %d] ",Entry);
		printf("%s",SectionsName + ShdrIter->sh_name);
		DisplayFormatSectionInfo(ShdrIter,SectionsName + ShdrIter->sh_name);
	}
	printf("\n");
	free(SectionsName);
}

error_t
Elf32_getELFSections(char *FileName[])
{
	Elf32_Ehdr *EhdrIter;

	Elf32_Shdr *ShdrIter;
	Elf32_Off	Entry = 0;
	
	FILE *ObjectFile = NULL;

	Elf32_Off SectionOffset;
	Elf32_Word SectionTableSize;

	for(;Entry < nFiles;Entry++) {
		ShdrIter = PFILE_SHDR_HEADERS(Entry); 
		EhdrIter = PFILE_EHDR(Entry);
		
		PFILE_SHDR_SHDRS_ELEMS(Entry) = EhdrIter->e_shnum;

		ObjectFile = fopen(FileName[Entry + 1],"r");

		if(ObjectFile == NULL) {
			printf("No Such File: %s\n",FileName[Entry + 1]);
			exit(-1);
		}

		SectionOffset = EhdrIter->e_shoff;
		SectionTableSize = EhdrIter->e_shnum * EhdrIter->e_shentsize;

		fseek(ObjectFile,SectionOffset,SEEK_SET);
		fread(ShdrIter,SectionTableSize,1,ObjectFile);

		if(DisplayELFSections) {
			printf("File Name: %s\n",FileName[Entry + 1]);
			Elf32_DisplaySections(ShdrIter,EhdrIter,ObjectFile);
		}
		fclose(ObjectFile);
	}
	return (0);
}


#define CANNOT_ALLOC	-1
error_t
Elf32_InitPointers(Elf32_Word nFiles) 
{
	Elf32_Ehdrs.Elf32_Ehdrs = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr) * nFiles);
	Elf32_Ehdrs.Elems = nFiles;
	Elf32_Ehdr *EhdrIter;

	Elf32_Shdrs = (struct SectionsHeader *)malloc(sizeof(struct SectionsHeader *) * nFiles);
	Elf32_Shdrs->Elems = nFiles;
	struct SectionsHeader *SectionsHeaderIter;

	Elf32_Syms = (struct SymbolTables *)malloc(sizeof(struct SymbolTables *) * nFiles);
	Elf32_Syms->Elems = nFiles;
	struct SymbolTables *SymbolTableIter;
	Elf32_Word SymbolTableNumber;

	Elf32_Word Entry = 0;
	for(;Entry < nFiles;Entry++) {
		SectionsHeaderIter = Elf32_Shdrs + Entry;
		EhdrIter = Elf32_Ehdrs.Elf32_Ehdrs + Entry;

		SectionsHeaderIter->Elf32_Shdrs = (struct SectionsHeaderEachFile *)malloc(
								sizeof(struct SectionsHeaderEachFile));
		SectionsHeaderIter->Elf32_Shdrs->SectionsHeader = 
						  (Elf32_Shdr *)malloc(EhdrIter->e_shentsize * EhdrIter->e_shnum);
		SectionsHeaderIter->Elf32_Shdrs->Elems = EhdrIter->e_shnum;
		
		if(SectionsHeaderIter->Elf32_Shdrs == NULL
				|| SectionsHeaderIter->Elf32_Shdrs->SectionsHeader == NULL) {
			return (CANNOT_ALLOC);
		}
		SymbolTableIter = Elf32_Syms + Entry;
		SymbolTableIter->Elf32_Syms = (struct SymbolTableEachFile *)malloc(
							sizeof(struct SymbolTableEachFile));
	}
}

static Elf32_Word
Elf32_foundSymbolTable(Elf32_Ehdr *Ehdr,Elf32_Shdr *Shdr,Elf32_Sym **SymTab,FILE *ObjectFile)
{
	Elf32_Word Entries = Ehdr->e_shnum;
	Elf32_Word Entry = 0;

	Elf32_Shdr *ShdrIter;

	for(;Entry < Entries;Entry++) {
		ShdrIter = Shdr + Entry;
	
		if(ShdrIter->sh_type == SHT_SYMTAB) {
			*SymTab = (Elf32_Sym *)malloc(ShdrIter->sh_size);
			fseek(ObjectFile,ShdrIter->sh_offset,SEEK_SET);
			fread(*SymTab,ShdrIter->sh_size,1,ObjectFile);
			return ShdrIter->sh_size / ShdrIter->sh_entsize;
		}
	}
	return (-1);
}

static Elf32_Off
Elf32_sortSectionsIndexByName(char *SectionName,Elf32_Off nFile,FILE *ObjectFile)
{
	Elf32_Shdr *ShdrIter = PFILE_SHDR_HEADERS(nFile);
	Elf32_Word Elems = PFILE_SHDR_SHDRS_ELEMS(nFile);
	Elf32_Off Offset = PFILE_EHDR_ATTRIBUTE(nFile,e_shstrndx);

	ShdrIter = ShdrIter + Offset;
	char *StringBuffer = (char *)malloc(ShdrIter->sh_size);
	Elf32_Off OffsetSectionsStringSection = ShdrIter->sh_offset;
	
	fseek(ObjectFile,OffsetSectionsStringSection,SEEK_SET);
	fread(StringBuffer,ShdrIter->sh_size,1,ObjectFile);

	ShdrIter = PFILE_SHDR_HEADERS(nFile);

	Elf32_Word Entry = 0;
	for(;Entry < Elems;Entry++,ShdrIter += 1) {
		if(strcmp(SectionName,StringBuffer + ShdrIter->sh_name) == 0) {
			free(StringBuffer);
			return Entry;
		}
	}
	free(StringBuffer);
	return (Entry);
}

#define FormatDisplaySymbolTitle(flags)			do{\
							printf(" Num:	Value		Size  Type	Vis	 Ndx 	Name\n");\
							}while(0);
static const char *
Elf32_SymbolTableBind(unsigned char SymbolTableInfo)
{
	int Mask = 0xf0;
	return (SymbolTableBind[(Mask & SymbolTableInfo)]);
}

static const char *
Elf32_SymbolTableType(unsigned char SymbolTableInfo)
{
	int Mask = 0x0f;
	return (SymbolTableType[(Mask & SymbolTableInfo)]);
}

static void
Elf32_DisplayELFSymbolTable(struct SymbolTableEachFile *Syms,Elf32_Off nFile,FILE *ObjectFile)
{
	Elf32_Sym	*SymbolTableIter = Syms->SymbolTable;
	Elf32_Off	Index = Elf32_sortSectionsIndexByName(".strtab",nFile,ObjectFile);
	
	Elf32_Shdr	*ShdrIter = PFILE_SHDR_HEADERS(nFile) + Index;

	Elf32_Off	Elems = Syms->Elems;
	Elf32_Off	Entry = 0;

	char *StringTab = (char *)malloc(ShdrIter->sh_size);

	fseek(ObjectFile,ShdrIter->sh_offset,SEEK_SET);
	fread(StringTab,ShdrIter->sh_size,1,ObjectFile);
	
	Elf32_Word	Length;

	FormatDisplaySymbolTitle(NULL)
	for(;Entry < Elems;Entry++,SymbolTableIter += 1) {
		printf("%4d: ",Entry);
		printf("%016x",SymbolTableIter->st_value);
		printf("%6d",SymbolTableIter->st_size);
		printf("  %s",Elf32_SymbolTableType(SymbolTableIter->st_info));
//		printf("  %s",Elf32_SymbolTableBind(SymbolTableIter->st_info));
		Length = strlen(Elf32_SymbolTableType(SymbolTableIter->st_info));
		Length = 10 - Length;
		while(Length--)	printf("%c",' ');
		printf("%s","DEFAULT");
		printf("\t%4x",SymbolTableIter->st_shndx);
		printf("\t%s\n",StringTab + SymbolTableIter->st_name);
	}
	free(StringTab);
}

error_t
Elf32_getSymbolTable(char *FileName[])
{
	Elf32_Word Entry = 0;

	Elf32_Ehdr *EhdrIter;
	Elf32_Shdr *ShdrIter;
	Elf32_Sym  *SymTabIter;
	Elf32_Word Elems;

	FILE *ObjectFile;
	for(;Entry < nFiles;Entry++) {
		EhdrIter = PFILE_EHDR(Entry);
		ShdrIter = PFILE_SHDR_HEADERS(Entry);

		ObjectFile = fopen(FileName[Entry + 1],"r");
		Elems = Elf32_foundSymbolTable(EhdrIter,ShdrIter,PFILE_SYM_TABLE_P(Entry),ObjectFile);

		PFILE_SYM_SYMS_ELEMS(Entry) = Elems;
		
		if(DisplayELFSymbolTable) {
			printf("\n%s: \nTotal %d  \n",FileName[Entry + 1],PFILE_SYM_SYMS(Entry)->Elems);
			Elf32_DisplayELFSymbolTable(PFILE_SYM_SYMS(Entry),Entry,ObjectFile);
		}
		fclose(ObjectFile);
	}
	return (0);
}
