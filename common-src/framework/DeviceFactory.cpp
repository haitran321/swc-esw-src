/**
 * $Id: DeviceFactory.cpp 6060 2010-06-30 21:25:32Z ste38548 $
 */
#include <algorithm>
#include <vector>
#include <sstream>
#include <string>
#include "DeviceFactory.h"
#include "MulticastDevice.h"
#include "StringUtils.h"

DeviceFactory::DeviceFactory()
{
}

STATUS DeviceFactory::load(const char* configFilename)
{
    propsFile = configFilename;
    return props.load(configFilename);
}

void DeviceFactory::registerCreator(const string& type, DeviceCreator* creator)
{
    deregisterCreator(type);

    string upperType = type;

    // uppercase to resolve ambiguity/quicker access
    std::transform(upperType.begin(), upperType.end(), upperType.begin(), ::toupper);

    creators[upperType] = creator;
}

void DeviceFactory::deregisterCreator(const string& type)
{
    string upperType = type;

    // uppercase to resolve ambiguity/quicker access
    std::transform(upperType.begin(), upperType.end(), upperType.begin(), ::toupper);

    CreatorsType::iterator iter = creators.find(upperType);
    if (iter != creators.end())
    {
        delete iter->second;
        creators.erase(iter);
    }
}

Device* DeviceFactory::createDevice(const string& name, Mode mode)
{
    Device* device = NULL;
    string type;

    if (props.get(name + ".type", type) != OK)
    {
        fprintf(stderr, "DeviceFactory: Unable to determine type for device %s. Expected %s in %s\n",
                name.c_str(), (name + ".type").c_str(), propsFile.c_str());
        return NULL;
    }

    // uppercase for simplicity
    std::transform(type.begin(), type.end(), type.begin(), ::toupper);

    CreatorsType::iterator iter = creators.find(type);
    if (iter != creators.end())
    {
        DeviceCreator* creator = iter->second;

        device = creator->createDevice(name, mode, props);
    }
    else if (type == "UDP")
    {
        device = createUDPDevice(name, mode);
    }
    else if (type == "MULTICAST")
    {
        device = createMulticastDevice(name);
    }
    else if (type == "ALIAS")
    {
        device = createAlias(name, mode);
    }
    else if (type == "NULL" || type == "NOOP")
    {
        device = new Device("/null");
    }
    else
    {
        fprintf(stderr, "DeviceFactory: Invalid device type [%s] specified for device %s in %s\n",
                type.c_str(), name.c_str(), propsFile.c_str());
        fprintf(stderr, "Valid device types for use with createDevice: UDP|MULTICAST|ALIAS|NULL\n");
    }

    return device;
}

UDPNetworkDevice* DeviceFactory::createUDPDevice(const string& name, Mode mode)
{
    string host = "";
    int port;
    bool requiresHost;

    if (props.get(name + ".host", host) != OK)
    {
        fprintf(stderr, "DeviceFactory: Unable to determine host for UDP device %s. Expected %s in %s\n",
                name.c_str(), (name + ".host").c_str(), propsFile.c_str());
        return NULL;
    }

    if (props.get(name + ".port", port) != OK)
    {
        fprintf(stderr, "DeviceFactory: Unable to determine port for UDP device %s. Expected %s in %s\n",
                name.c_str(), (name + ".port").c_str(), propsFile.c_str());
        return NULL;
    }

    if (props.get(name + ".requiresHost", requiresHost) != OK)
    {
        // default to require the destination host to be there
        // if it's not there then the first write will fail as it tries to resolve
        // the host's mac address while attempting to persist the ARP entry
        requiresHost = true;
    }

    UDPNetworkDevice* device = new UDPNetworkDevice(mode == Outbound ? NetworkClient : NetworkServer,
                                        host, port, requiresHost);
    stringstream devName;
    devName << "UDP " << (mode == Outbound ? "client " : "server ");
    devName << host << ":" << port;
    device->setName(devName.str());

    return device;
}
Device* DeviceFactory::createMulticastDevice(const string& name)
{
    string deviceNamesCsv;
    vector<string> deviceNames;

    if (props.get(name + ".devices", deviceNamesCsv) != OK)
    {
        fprintf(stderr, "DeviceFactory: Unable to determine devices to use for multicast device %s. Expected %s in %s\n",
                name.c_str(), (name + ".devices").c_str(), propsFile.c_str());
        return NULL;
    }

    MulticastDevice* multicaster = new MulticastDevice(name);

    // populate collection of deviceNames from the comma-separated list
    tokenize(deviceNamesCsv, deviceNames, ",");

    vector<string>::iterator iter;
    for (iter = deviceNames.begin(); iter != deviceNames.end(); ++iter)
    {
        // create each outbound device & add it to the multicaster
        string& deviceName = *iter;
        deviceName = trim(deviceName);
        Device* device = createDevice(deviceName, Outbound);

        if (device == NULL)
        {
            // failures already reported
            delete multicaster;
            return NULL;
        }

        multicaster->add(device);
    }

    return multicaster;
}


Device* DeviceFactory::createAlias(const string& name, Mode mode)
{
    string aliasFor;

    if (props.get(name + ".aliasFor", aliasFor) != OK)
    {
        fprintf(stderr, "DeviceFactory: Unable to determine the aliased device name for device %s. Expected %s in %s\n",
                name.c_str(), (name + ".aliasFor").c_str(), propsFile.c_str());
        return NULL;
    }

    // delegate to the aliased device
    return createDevice(aliasFor, mode);
}

DeviceFactory::~DeviceFactory()
{
    CreatorsType::iterator iter;
    for (iter = creators.begin(); iter != creators.end(); ++iter)
    {
        delete iter->second;
    }
}
