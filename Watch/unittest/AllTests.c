#include <stdio.h>
#include <string.h>
#include "CuTest.h"

CuSuite* cfsGetSuite(void);
CuSuite* WindowGetSuite(void);
CuSuite* GestureGetSuite(void);
CuSuite* StlvProtocalGetSuite(void);
CuSuite* BleHandlerTestGetSuite(void);

char listmode = 0;

void AddSuite(CuSuite* suite, CuSuite* child, const char* name)
{
	if (name == NULL || strcmp(child->name, name) == 0)
		CuSuiteAddSuite(suite, child);

	if (listmode)
		printf("%s\n", child->name);
}

void AddAllSuite(CuSuite* suite, const char* name)
{
	AddSuite(suite, cfsGetSuite(), name);
	AddSuite(suite, WindowGetSuite(), name);
	AddSuite(suite, GestureGetSuite(), name);
        AddSuite(suite, StlvProtocalGetSuite(), name);
        AddSuite(suite, BleHandlerTestGetSuite(), name);
}

void RunAllTests(const char* name)
{
	CuSuite* suite;
	if (name != NULL)
		suite = CuSuiteNew(name);
	else
		suite = CuSuiteNew("all");
	AddAllSuite(suite, name);

	CuString *output = CuStringNew();
	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
}

void ListAllTests()
{
	listmode = 1;

	CuSuite* suite = CuSuiteNew("iwatch");
	AddAllSuite(suite, NULL);
}

int main(int argc, char** argv)
{
	if (argc == 2)
	{
		if (!strcmp(argv[1], "help"))
		{
			printf("iwatch [help|all|list|<casename>]\n");
			return 0;
		}
		else if (!strcmp(argv[1], "all"))
		{
			RunAllTests(NULL);
			return 0;	
		}
		else if (!strcmp(argv[1], "list"))
		{
			ListAllTests();
			return 0;				
		}
		else
		{
			RunAllTests(argv[1]);
		}
	}
	else
		RunAllTests(NULL);
}
