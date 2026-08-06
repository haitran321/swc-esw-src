#include "IOMHWMgr.h"
#include "ConfigDataManager.h"

#include <errno.h>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <unistd.h>
#include <modbus.h>

namespace
{
    const int IOM_STATUS_BIT_COUNT = 16;
    const int IOM_CONNECT_RETRY_COUNT = 5;
    const int IOM_READ_RETRY_COUNT = 2;
    const unsigned int IOM_CONNECT_RETRY_DELAY_US = 1000 * 1000;
    const uint32_t IOM_RESPONSE_TIMEOUT_SEC = 1;
    const uint32_t IOM_RESPONSE_TIMEOUT_USEC = 500 * 1000;
    const uint32_t IOM_BYTE_TIMEOUT_SEC = 0;
    const uint32_t IOM_BYTE_TIMEOUT_USEC = 250 * 1000;
}

IOMHWMgr::IOMHWMgr() :
    _verbose(0),
    _logger(Logger::getInstance()),
    _ioModuleIpAddress(""),
    _ioModulePort(0),
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

STATUS IOMHWMgr::createContext()
{
    close();

    _modbusCtx = modbus_new_tcp(_ioModuleIpAddress.c_str(), _ioModulePort);
    if (_modbusCtx == nullptr)
    {
        _logger.logError("Unable to create Modbus context for %s:%d",
                         _ioModuleIpAddress.c_str(), _ioModulePort);
        printf("Unable to create Modbus context for %s:%d\n",
               _ioModuleIpAddress.c_str(), _ioModulePort);
        return ERROR;
    }

    modbus_set_error_recovery(
        _modbusCtx,
        (modbus_error_recovery_mode)(MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL));
    modbus_set_response_timeout(_modbusCtx, IOM_RESPONSE_TIMEOUT_SEC, IOM_RESPONSE_TIMEOUT_USEC);
    modbus_set_byte_timeout(_modbusCtx, IOM_BYTE_TIMEOUT_SEC, IOM_BYTE_TIMEOUT_USEC);
    modbus_set_debug(_modbusCtx, _verbose);

    return OK;
}

STATUS IOMHWMgr::ensureConnected()
{
    if (_ioModuleIpAddress.empty() || _ioModulePort <= 0)
    {
        _logger.logError("IOM Modbus configuration is invalid");
        printf("IOM Modbus configuration is invalid\n");
        return ERROR;
    }

    if (_modbusCtx != nullptr)
    {
        return OK;
    }

    for (int attempt = 1; attempt <= IOM_CONNECT_RETRY_COUNT; ++attempt)
    {
        if (createContext() != OK)
        {
            return ERROR;
        }

        if (modbus_connect(_modbusCtx) != -1)
        {
            if (attempt > 1)
            {
                _logger.logInfo("Reconnected to IO module after %d attempts", attempt);
            }
            return OK;
        }

        _logger.logError("IOM Modbus connect attempt %d/%d failed: %s",
                         attempt, IOM_CONNECT_RETRY_COUNT, modbus_strerror(errno));
        printf("IOM Modbus connect attempt %d/%d failed: %s\n",
               attempt, IOM_CONNECT_RETRY_COUNT, modbus_strerror(errno));

        close();

        if (attempt < IOM_CONNECT_RETRY_COUNT)
        {
            usleep(IOM_CONNECT_RETRY_DELAY_US);
        }
    }

    return ERROR;
}

STATUS IOMHWMgr::initialize()
{
    STATUS rc = OK;

    _logger.logInfo("IOHWMgr Initializing");

    ConfigDataManager& configs = ConfigDataManager::getInstance();

    // Verbose parameters
    rc = rc || configs.get("VERBOSE", _verbose);

    // Acromag parameters
    rc = rc || configs.get("IO_MODULE_IP_ADDRESS", _ioModuleIpAddress);
    rc = rc || configs.get("IO_MODULE_PORT", _ioModulePort);

    if (rc != OK)
    {
        return ERROR;
    }

    close();

    return ensureConnected();
}

IOMStatusDataType IOMHWMgr::readStatus()
{
    IOMStatusDataType status = {0};

    for (int attempt = 1; attempt <= IOM_READ_RETRY_COUNT; ++attempt)
    {
        if (ensureConnected() != OK)
        {
            _logger.logError("Modbus context not initialized");
            printf("Modbus context not initialized\n");
            return status;
        }

        // Buffer for discrete inputs
        uint8_t bits[IOM_STATUS_BIT_COUNT] = {0};

        // Read 16 discrete inputs starting at address 0
        int rc = modbus_read_input_bits(_modbusCtx, 0, IOM_STATUS_BIT_COUNT, bits);
        if (rc == IOM_STATUS_BIT_COUNT)
        {
            status.configBit0 = bits[0];
            status.configBit1 = bits[1];
            status.mode = bits[2];
            status.ps12 = bits[3];
            status.ps24 = bits[4];
            status.temp = bits[5];
            status.ch7 = bits[6];
            status.ch8 = bits[7];
            status.ch9 = bits[8];
            status.ch10 = bits[9];
            status.ch11 = bits[10];
            status.ch12 = bits[11];
            status.ch13 = bits[12];
            status.ch14 = bits[13];
            status.ch15 = bits[14];
            status.ch16 = bits[15];

            // Print raw response bytes
            if (_verbose)
            {
                std::cout << "Raw response bytes: ";
                for (int i = 0; i < IOM_STATUS_BIT_COUNT; i++)
                {
                    std::cout << std::hex << std::setw(2) << std::setfill('0')
                              << static_cast<int>(bits[i]) << " ";
                }
                std::cout << std::dec << "\n";

                // Build 16-bit word
                uint16_t word = 0;
                for (int ch = 0; ch < IOM_STATUS_BIT_COUNT; ch++)
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
                for (int ch = 0; ch < IOM_STATUS_BIT_COUNT; ch++)
                {
                    std::cout << "CH" << std::setw(2) << std::setfill('0') << (ch + 1)
                              << ": " << (bits[ch] ? "ON" : "OFF") << "\n";
                }
            }

            return status;
        }

        if (rc == -1)
        {
            _logger.logError("IOM Modbus read attempt %d/%d failed: %s",
                             attempt, IOM_READ_RETRY_COUNT, modbus_strerror(errno));
            printf("IOM Modbus read attempt %d/%d failed: %s\n",
                   attempt, IOM_READ_RETRY_COUNT, modbus_strerror(errno));
        }
        else
        {
            _logger.logError("IOM Modbus read attempt %d/%d returned %d bits, expected %d",
                             attempt, IOM_READ_RETRY_COUNT, rc, IOM_STATUS_BIT_COUNT);
            printf("IOM Modbus read attempt %d/%d returned %d bits, expected %d\n",
                   attempt, IOM_READ_RETRY_COUNT, rc, IOM_STATUS_BIT_COUNT);
        }

        close();
    }

    return status;
}
