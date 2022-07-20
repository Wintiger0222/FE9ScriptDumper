#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<string.h>
#define CONVERT_TO_BIG(X) ((X)=(temp[3]+(temp[2]*0x100)+(temp[1]*0x10000)+(temp[0]*0x1000000)))
#define ORG(X) (X+0x20)
int main(int argc, char* argv[])
{
	FILE* pf = NULL;
	FILE* fout = NULL;
	unsigned char sex_parameta = 0;
	unsigned char line_parameta = 0;
	if (argc <= 1)
	{
		printf("화이아 엠블람 창염의 궤적 스크립트 .m -> .asm 변환기 v1.1 by WindowsTiger 2021\n");
		printf("Usage: fe9converter <INPUT> <OUTPUT> [optional parameters]\n");
		printf("\n");
		printf("Optional parameters:\n");
		printf(" -sex\t\t비문자 코드를 \\xXX형태로 출력합니다. ('새벽의 여신'용)\n");
		printf(" -line\t\t줄바꿈을 스크립트에 표현합니다.\n");
		printf("\n");
		printf("File arguments:\n");
		printf(" <INPUT>\t.m 파일 원본\n");
		printf(" <OUTPUT>\tarmips용 .asm 스크립트, \n");
		printf("         \tstdout을 입력할경우 콘솔창에 출력합니다. \n");
		printf("         \t\n");

		return -1;
	}
	else if (argc == 2)
	{
		printf("error: OUTPUT이 지정되지 않았습니다.\n");
		return -1;
	}
	else if (argc >= 3)
	{
		pf = fopen(argv[1], "rb");
		if (pf == NULL)
		{
			printf("error: INPUT 파일 읽기 에러\n");
			return -1;
		}
		if (strcmp(argv[2], "stdout") == 0)
		{
			printf("warnning: OUTPUT을 콘솔창에 출력합니다.\n");
			fout = stdout;
		}
		else
		{
			fout = fopen(argv[2], "w");
			if (fout == NULL)
			{
				printf("warnning: OUTPUT을 열 수 없습니다. 콘솔창에 출력합니다.\n");
				fout = stdout;
			}
		}
		if (argc >= 4)
		{
			for (int i = 3; i < argc; i++)
			{
				if (strcmp(argv[i], "-sex") == 0)
				{
					printf("notice: sex 파라미터를 입력받았습니다.\n");
					sex_parameta = 1;
				}
				if (strcmp(argv[i], "-line") == 0)
				{
					printf("notice: line 파라미터를 입력받았습니다.\n");
					line_parameta = 1;
				}
			}
		}

	}
	unsigned char temp[4];
	unsigned int file_size;
	unsigned int list_pointer;
	unsigned int dummy1;
	unsigned int list_count;
	unsigned int function_name_pointer;

	int max_count;

	unsigned int pointer_list[100000];

	fprintf(fout, ".arm.big\n");
	//	fprintf(fout, ".open \"%s.ori\", \"root/Mess/%s\", 0x00000000\n", argv[1], argv[1]);
	//fprintf(fout, ".open \"root/Mess/%s\", 0x00000000\n", argv[1]);
	fprintf(fout, ".open \"%s\", 0x00000000\n", argv[1]);
	fprintf(fout, ".loadtable \"fe9k.tbl\"\n");

	fread(&temp[0], sizeof(int), 1, pf);
	CONVERT_TO_BIG(file_size);
	//printf(fout, "//file size: %x\n", file_size);
	fprintf(fout, "//original file size: %x\n", file_size);

	fread(&temp[0], sizeof(int), 1, pf);
	CONVERT_TO_BIG(list_pointer);
	fprintf(fout, "//original list pointer: %x\n", list_pointer + 0x20);

	fread(&temp[0], sizeof(int), 1, pf);
	CONVERT_TO_BIG(dummy1);
	//fprintf(fout, "//dummy: %x\n", dummy1);

	fread(&temp[0], sizeof(int), 1, pf);
	CONVERT_TO_BIG(list_count);
	fprintf(fout, "//original list count:%x\n", list_count);

	//fprintf(fout,"%x\n", list_count);

	fseek(pf, ORG(list_pointer), SEEK_SET);
	for (int count = 0; count < list_count * 2; count++)
	{
		fread(&temp[0], sizeof(int), 1, pf);
		CONVERT_TO_BIG(pointer_list[count]);
	}

	function_name_pointer = ftell(pf);
	//fprintf(fout,"//function name pointer: 0x%X\n", function_name_pointer);

	fprintf(fout, ".org 0x0\n");
	fprintf(fout, "dw File_End\n");
	fprintf(fout, "dw String_pointers-0x20\n");
	fprintf(fout, "dw 0x0\n");
	fprintf(fout, "dw 0x%X//total count\n", list_count);
	fprintf(fout, "dw 0x0\n");
	fprintf(fout, "dw 0x0\n");
	fprintf(fout, "dw 0x0\n");
	fprintf(fout, "dw 0x0\n");
	//fprintf(fout, ".org 0x20\n");
	fprintf(fout, "Normal_String:\n");
	for (int count = 0; count < list_count * 2; count++)
	{
		unsigned char tempchar[2] = { 0xff, 0xff };
		unsigned char formerchar = 0x0;
		if (count % 2 == 1)//함수명
		{
			fseek(pf, pointer_list[count] + function_name_pointer, SEEK_SET);
			fprintf(fout, "//");
			fread(tempchar, sizeof(char), 1, pf);
			while (tempchar[0]) {
				fprintf(fout, "%c", tempchar[0]);
				fread(tempchar, sizeof(char), 1, pf);
			}
			fprintf(fout, "\n\n");
		}
		else//문자열
		{
			fseek(pf, ORG(pointer_list[count]), SEEK_SET);
			fprintf(fout, "//0x%X\n", pointer_list[count] + 0x20);
			fprintf(fout, "Text_%X:\n", pointer_list[count]);
			if (line_parameta)
				fprintf(fout, ".stringn \"");
			else
				fprintf(fout, ".string \"");
			fread(tempchar, sizeof(char), 1, pf);
			while (tempchar[0]) {
				if (tempchar[0] == 0x5C)//*역슬래시
				{
					if (formerchar >= 0x81 && formerchar <= 0x9F || formerchar >= 0xE0 && formerchar <= 0xEF)
						fprintf(fout, "%c", tempchar[0]);
					else
						fprintf(fout, "\\x5C");
				}

				else if (tempchar[0] == '\n')
				{
					if (line_parameta)
					{
						fprintf(fout, "\\n\"\n");
						fprintf(fout, ".stringn \"");
					}
					else
					{
						fprintf(fout, "\\n");
					}
				}
				else if (tempchar[0] == '\r')
				{
					if (line_parameta)
					{
						fprintf(fout, "\\r\"\n");
						fprintf(fout, ".stringn \"");
					}
					else
					{
						fprintf(fout, "\\n");
					}
				}
				else if (tempchar[0] < 0x10 && sex_parameta)
					fprintf(fout, "\\x0%X", tempchar[0]);
				else if (tempchar[0] < 0x20 && sex_parameta)
					fprintf(fout, "\\x%X", tempchar[0]);
				else if (tempchar[0] == 0x7F && sex_parameta)
					fprintf(fout, "\\x%X", tempchar[0]);
				else if (tempchar[0] > 0xFC && sex_parameta)
					fprintf(fout, "\\x%X", tempchar[0]);
				else
					fprintf(fout, "%c", tempchar[0]);
				formerchar = tempchar[0];
				fread(tempchar, sizeof(char), 1, pf);
			}
			fprintf(fout, "\"\n");
			if (line_parameta)
				fprintf(fout, "db 0\n");
			fprintf(fout, ".align 4\n");
		}

	}
	fprintf(fout, "String_pointers:\n");
	for (int count = 0; count < list_count * 2; count++)
	{
		if (count % 2)
		{
			//홀수: 함수명
			fprintf(fout, "\tdw 0x%X\n", pointer_list[count]);
		}
		else
		{
			//짝수: 문자열쪽
			fprintf(fout, "\tdw Text_%X-0x20\n", pointer_list[count]);
		}
	}

	fprintf(fout, "Function_Strings:\n");
	fseek(pf, function_name_pointer, SEEK_SET);
	unsigned char nullcheck = 1;
	for (int count = 0; ftell(pf) < file_size; count++)
	{
		unsigned char tempchar[1] = { 0xff };
		if (nullcheck)
		{
			fprintf(fout, "db ");
			nullcheck = 0;
		}
		fread(tempchar, sizeof(char), 1, pf);
		fprintf(fout, "0x%X", tempchar[0]);
		if (!tempchar[0])
		{
			fprintf(fout, "\n");
			nullcheck = 1;
		}
		else
			fprintf(fout, ", ");

	}

	fprintf(fout, "File_End:\n");
	fprintf(fout, ".close\n");
	printf("notice: 추출이 완료되었습니다.");
	printf("\n");

	return 0;
}