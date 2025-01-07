#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sstream>
#include "ConfigDataManager.h"
#include "DUCmdMgr.h"
#include "TUCmdMgr.h"

int main(int argc, char *argv[])
{
    printf("****In Main: num arg = %d, arg[0] = %s, arg[1] = %s\n", argc, argv[0], argv[1]);

    if (argc == 2)
    {
        char MODULE_TYPE[2];
        sprintf(MODULE_TYPE, "%s", argv[1]);
        printf("\n*******************************************************\n");
        printf("MODULE_TYPE = %s\n", MODULE_TYPE);

        if ((strcmp(MODULE_TYPE, "DU") == 0) || (strcmp(MODULE_TYPE, "du") == 0))
        {
            printf("******This is DU Application******\n");
        
            DUCmdMgr * cmdMgr = new DUCmdMgr();
            if(cmdMgr == NULL)
            {
                printf("DUCmdMgr task is NULL\n");
                return ERROR;
            }
            else
            {
                cmdMgr->start();
            }
        }
        else if ((strcmp(MODULE_TYPE, "TU") == 0) || (strcmp(MODULE_TYPE, "tu") == 0))
        {
            printf("******This is TU Application******\n");
        
            TUCmdMgr * cmdMgr = new TUCmdMgr();
            if(cmdMgr == NULL)
            {
                printf("TUCmdMgr task is NULL\n");
                return ERROR;
            }
            else
            {
                cmdMgr->start();
            }
        }
    }
    else
    {
        printf("ERROR: invalid number of arguments, require module type: du, tu\n");
        return ERROR;
    }

	return OK;
}
