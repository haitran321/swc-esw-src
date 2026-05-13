#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sstream>
#include "ConfigDataManager.h"
#include "DUACmdMgr.h"
#include "DUBCmdMgr.h"
#include "TUCmdMgr.h"

int main(int argc, char *argv[])
{
    printf("****In SwcAppInit: num arg = %d, arg[0] = %s, arg[1] = %s\n", argc, argv[0], argv[1]);

    if (argc == 2)
    {
        char MODULE_TYPE[2];
        sprintf(MODULE_TYPE, "%s", argv[1]);
        printf("\n*******************************************************\n");
        printf("MODULE_TYPE = %s\n", MODULE_TYPE);

        if ((strcmp(MODULE_TYPE, "DUA") == 0) || (strcmp(MODULE_TYPE, "dua") == 0))
        {
            printf("******This is DU Alpha Application******\n");
        
            DUACmdMgr * cmdMgr = new DUACmdMgr();
            if(cmdMgr == NULL)
            {
                printf("DUACmdMgr task is NULL\n");
                return ERROR;
            }
            else
            {
                cmdMgr->start();
            }
        }
        else if ((strcmp(MODULE_TYPE, "DUB") == 0) || (strcmp(MODULE_TYPE, "dub") == 0))
        {
            printf("******This is DU Beta Application******\n");
        
            DUBCmdMgr * cmdMgr = new DUBCmdMgr();
            if(cmdMgr == NULL)
            {
                printf("DUBCmdMgr task is NULL\n");
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
