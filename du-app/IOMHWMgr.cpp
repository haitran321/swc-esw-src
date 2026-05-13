#include "IOMHWMgr.h"
#include "ConfigDataManager.h"

#include <iostream>
#include <iomanip>
#include <bitset>
#include <modbus.h>

IOMHWMgr::IOMHWMgr() :
    _logger(Logger::getInstance()),
    _modbusCtx(nullptr)
{
}

IOMHWMgr::~IOMHWMgr()
{
    close();
}

IOMHWMgr &IOMHWMgr::getInstance()
{
    static IOMHWMgr object;
    return (object);
}

void IOMHWMgr::close()
{
    if (_modbusCtx != nullptr)
    {
        modbus_close(_modbusCtx);
        modbus_free(_modbusCtx);
        _modbusCtx = nullptr;
    }
}

STATUS IOMHWMgr::initialize()
{
    STATUS rc = OK;

    string IO_MODULE_IP_ADDRESS;
    int IO_MODULE_PORT;

    _logger.logInfo("IOHWMgr Initializing");

    printf("\nLoading Config file\n");
    if (ConfigDataManager::getInstance().load() != OK)
    {
        printf("Error loading Config file\n");
    }

    ConfigDataManager& configs = ConfigDataManager::getInstance();
    rc = rc || configs.get("IO_MODULE_IP_ADDRESS", IO_MODULE_IP_ADDRESS);
    rc = rc || configs.get("IO_MODULE_PORT", IO_MODULE_PORT);

    const char *ipAddress = IO_MODULE_IP_ADDRESS.c_str();

    // Create Modbus TCP context
    _modbusCtx = modbus_new_tcp(ipAddress, IO_MODULE_PORT);
    if (_modbusCtx == nullptr)
    {
        _logger.logError("Unable to create Modbus context");
        printf("Unable to create Modbus context\n");
        return ERROR;
    }

    // Connect
    if (modbus_connect(_modbusCtx) == -1)
    {
        _logger.logError("Connection failed");
        printf("Connection failed\n");
        modbus_free(_modbusCtx);
        _modbusCtx = nullptr;
        return ERROR;
    }

    return rc;
}

IOMStatusDataType IOMHWMgr::readStatus()
{
    printf("******In IOM readstatus\n");

    IOMStatusDataType status = {0};

    if (_modbusCtx == nullptr)
    {
        _logger.logError("Modbus context not initialized");
        printf("Modbus context not initialized\n");
        return status;
    }

    // Buffer for discrete inputs
    uint8_t bits[16];

    // Read 16 discrete inputs starting at address 0
    int rc = modbus_read_input_bits(_modbusCtx, 0, 16, bits);
    if (rc == -1)
    {
        _logger.logError("Read failed");
        modbus_close(_modbusCtx);
        modbus_free(_modbusCtx);
        _modbusCtx = nullptr;
        return status;
    }

    status.ch1 = bits[0];
    status.ch2 = bits[1];
    status.atbIOMStatus = bits[2];
    status.ch4 = bits[3];
    status.tempIOMStatus = bits[4];
    status.ch6 = bits[5];
    status.ps12IOMStatus = bits[6];
    status.ch8 = bits[7];
    status.ps24IOMStatus = bits[8];
    status.ch10 = bits[9];
    status.ch11 = bits[10];
    status.ch12 = bits[11];
    status.ch13 = bits[12];
    status.ch14 = bits[13];
    status.ch15 = bits[14];
    status.ch16 = bits[15];

    // Print raw response bytes
    std::cout << "Raw response bytes: ";
    for (int i = 0; i < 16; i++)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(bits[i]) << " ";
    }
    std::cout << std::dec << "\n";

    // Build 16-bit word
    uint16_t word = 0;
    for (int ch = 0; ch < 16; ch++)
    {
        if (bits[ch])
        {
            word |= (1 << ch);
        }
    }

    std::cout << "\n16-bit word (hex): 0x"
              << std::hex << std::setw(4) << std::setfill('0') << word << "\n";

    std::cout << "16-bit word (bin): "
              << std::bitset<16>(word) << "\n";

    // Print channel states
    std::cout << "\nChannel state:\n";
    for (int ch = 0; ch < 16; ch++)
    {
        std::cout << "CH" << std::setw(2) << std::setfill('0') << (ch + 1)
                  << ": " << (bits[ch] ? "ON" : "OFF") << "\n";
    }

    return status;
}
