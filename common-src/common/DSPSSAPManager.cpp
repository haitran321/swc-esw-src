#include "DSPSSAPManager.h"

DSPSSAPManager::DSPSSAPManager()
{
}

DSPSSAPManager::~DSPSSAPManager()
{
}

DSPSSAPManager &DSPSSAPManager::getInstance()
{
   static DSPSSAPManager instance;
   return instance;
}

STATUS DSPSSAPManager::ReadDataFiles()
{
    STATUS status = OK;

    FILE *fp;
    char         fname[50];

    dspsSap.sap011                = 0;        // Replica range bin offset
    dspsSap.sap015                = 0.1;      // Time interval for sending DSPS summary status report to RIMS
    dspsSap.chassisFanUpperLimit         = 3600;     // Fan upper limit
    dspsSap.chassisFanLowerLimit         = 3200;     // Fan lower limit

    sprintf(fname, "/mnt/sd-mmcblk0p1/conf/dsps_sap.txt");
    fp = fopen(fname, "rb");
    if (fp == NULL)
    {
        printf("DSPSDataManager: Could not open file %s\n", fname);
    }

    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END);
        int fileSize = (int)ftell(fp);
        rewind(fp);

        char *fileBuf = (char *)malloc(fileSize);
        if (fileBuf)
        {
            int numRead = fread(fileBuf, fileSize, 1, fp);
            if (numRead != 1)
            {
                printf("DSPSDataManager: Error reading\n");
                fclose(fp);
            }
            else
            {
                fclose(fp);
                if (GetSingleSapValue(fileBuf, fileSize, "SAP_011", INT_TYPE, &dspsSap.sap011) < 0)
                {
                    printf("DSPSDataManager: Can't find field SAP_011\n");
                }

                if (GetSingleSapValue(fileBuf, fileSize, "SAP_015", FLOAT_TYPE, &dspsSap.sap015) < 0)
                {
                    printf("DSPSDataManager: Can't find field SAP_015\n");
                }
            }
            free(fileBuf);
        }
        else
        {
            fclose(fp);
        }
    }

    sprintf(fname, "/mnt/sd-mmcblk0p1/conf/dsps_config.txt");
    fp = fopen(fname, "rb");
    if (fp == NULL)
    {
        printf("DSPSDataManager: Could not open file %s\n", fname);
    }

    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END);
        int fileSize = (int)ftell(fp);
        rewind(fp);

        char *fileBuf = (char *)malloc(fileSize);
        if (fileBuf)
        {
            int numRead = fread(fileBuf, fileSize, 1, fp);
            if (numRead != 1)
            {
                printf("DSPSDataManager: Error reading\n");
                fclose(fp);
            }
            else
            {
                fclose(fp);
                if (GetSingleSapValue(fileBuf, fileSize, "FAN_UP", INT_TYPE, &dspsSap.chassisFanUpperLimit) < 0)
                {
                    printf("DSPSDataManager: Can't find field FAN_UP\n");
                }
                if (GetSingleSapValue(fileBuf, fileSize, "FAN_LO", INT_TYPE, &dspsSap.chassisFanLowerLimit) < 0)
                {
                    printf("DSPSDataManager: Can't find field FAN_LO\n");
                }
            }
            free(fileBuf);
        }
        else
        {
            fclose(fp);
        }
    }

    // Read noise input
//  sprintf(fname, "/mnt/sd-mmcblk0p1/conf/dsps_dcoffset.txt");
//  fp = fopen(fname, "r");
//  if (fp == NULL)
//  {
//      printf("DSPSDataManager: Could not open file %s\n", fname);
//  }
//
//  if (fp != NULL)
//  {
//      for (int i = 0; i < MAX_PULSE_TYPES; i++)
//      {
//          fscanf(fp, "%f", &dspsSap.dcOffsets[i]);
//      }
//      fclose(fp);
//  }

    return status;
}

/*!
 *
 * \brief Gets a single SAP data parameter from SAP memory buffer.
 *
 * Gets a single SAP data parameter from SAP memory buffer. The SAP parameter to
 * extract is specified as character string input argument. The method checks to see if
 * parameter exists in the SAP data buffer. If so, it sets the method return value
 * to 0 and assigns the value to the last method output pointer argument.
 *
 * @param fileBuf Input pointer to memory buffer
 *
 * @param fileSize Input that contains the size of the memory buffer.
 *
 * @param sapName Input pointer to character string that contains the name of the
 * SAP parameter.
 *
 * @param sapDataType Input that specifies the SAP data type as integer or float.
 *
 * @param sapValue Output pointer used to return SAP numeric vale.
 *
 * @return Zero if the SAP value is ectracted successfully, -1 otherwise.
 *
 */
int DSPSSAPManager::GetSingleSapValue(char *fileBuf, int fileSize, char *sapName, int sapDataType, void *sapValue)
{
    char *pf;
    char buf[100];
    pf = fileBuf;
    int status = -1;

    int length = strlen(sapName);

    for (int i = 0; i < fileSize - length; i++)
    {
        memcpy(buf, pf, length);
        buf[length] = '\0';

        if (strcmp(sapName, buf) == 0)
        {
            pf += length;
            if (sapDataType == INT_TYPE)
            {
                sscanf(pf, "%d", (int *)sapValue);
            }
            else if (sapDataType == FLOAT_TYPE)
            {
                sscanf(pf, "%f", (float *)sapValue);
            }
            status = 0;
            break;
        }
        pf++;
    }

    return status;
}




