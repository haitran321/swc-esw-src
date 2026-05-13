/**
 * $Id: DeviceFactory.h
 *
 * DeviceFactory creates devices based on names, where the
 * details of the devices are specified in a configuration file.
 * <p/>
 * The devices configuration file is formatted like a traditional
 * <code>key=value</code> pair configuration file.
 * Different types of devices will require different details in
 * their configuration, but they all follow the same pattern:<p/>
 * <code>deviceName.option=value</code><p/>
 * 
 * Take a UDP-based device as an an example:
 * <code><pre>
 * ctrlNodeSbc1.nodeMgr.internal
 *   type=UDP
 *   host=192.168.1.5
 *   port=2000
 * </pre></code>
 *
 * The code acting as the server uses
 * <code>DeviceFactory.createTCPServerDevice("ctrlNodeSbc1.nodeMgr.internal")</code>
 * to create the server end of the socket.  
 * This specialized call is required due to the requirement to call <code>accept()</code>
 * on the returned device (<code>accept()</code> is not part of the standard <code>Device</code>).</p>
 * The code on the client end of the connection uses
 * <code>DeviceFactory.createDevice("ctrlNodeSbc1.nodeMgr.internal")</code>
 * to create the client end of the socket using the same configuration options
 * that the server uses.
 * Note that the client end doesn't care what type of device it is so it
 * uses the generalized <code>createDevice()</code> call.
 * 
 * Supported types and their options:
 * <ul>
 *   <li>TCP</li>
 *   <li>UPD</li>
 *   <li>Timer</li>
 *   <li>Axis</li>
 *   <li>PCI</li>
 *   <li>Pipe</li>
 *   <li>Null</li>
 * </ul>
 * 
 * The Null device type is intended to be used to "stub out" interfaces to other
 * pieces of the system to simplify unit testing.
 */
#ifndef DeviceFactory_H
#define DeviceFactory_H

#include <map>
#include "Uncopyable.h"
#include "UDPNetworkDevice.h"
#include "Properties.h"

using std::string;
using std::map;

// forward reference to the creator class
class DeviceCreator;

class DeviceFactory : Uncopyable
{
public:

    /**
     * Destructor
     */
    virtual ~DeviceFactory();
    
    enum Mode
    {
        Inbound,
        Outbound,
        Bidirectional
    };

    /**
     * Register an creator that's used to create {@link Device}s of the specified
     * type.  This allows custom devices to be able to use the same factory pattern
     * as the devices explicitly handled by this class.<p/>
     * Requests for devices of type <code>type</code> (case insensitive) will be
     * delegated to the specified creator.
     * The creator must have been created with <code>new</code> and will deleted
     * by this class when no longer needed.
     */
    void registerCreator(const string& type, DeviceCreator* creator);
    
    /**
     * Deregisters a previously registered creator.
     * Deregistration includes invoking <code>delete</code> on the registered creator.
     * 
     * @see #registerCreator
     */
    void deregisterCreator(const string& type);

    STATUS load(const char* configFilename);

    /**
     * Creates a new instance of a Device associated with the specified name.
     *
     * @param name name of the configuration entry used to configure the device
     * @param mode mode of the device (TBD)
     * @return newly instantiated Device
     */
    Device* createDevice(const string& name, Mode mode);

    /**
     * Singleton accessor
     */
    static DeviceFactory& getInstance()
    {
        static DeviceFactory instance;
        return instance;
    }
    
protected:

    /**
     * Constructor
     */
    explicit DeviceFactory();
    
    UDPNetworkDevice* createUDPDevice(const string& name, Mode mode);
    Device* createMulticastDevice(const string& name);
    
    /** Create a device based on the definition of another device */
    Device* createAlias(const string& name, Mode mode);
    
    /** collection of properties that determine our configuration */ 
    Properties props;
    
    /** where the properties were loaded from.  for diagnostic purposes only */
    string propsFile;
    
    /** map of device types to the creator used to create instances of those devices */
    typedef map<string, DeviceCreator*> CreatorsType;
    CreatorsType creators;
};

/**
 * Class used to create {@link Device}s to allow custom devices to be able to 
 * use the same factory pattern as the devices explicitly handled by 
 * {@link DeviceFactory}.<p/>
 */
class DeviceCreator
{
public:

    /**
     * Create a new {@link Device} (with <code>new</code>) with name <code>name</code>
     * matching mode <code>mode</code>.
     * On failure the implementation is expected to use <code>printf()</code> to 
     * report failures and return <code>NULL</code>.
     */
    virtual Device* createDevice(const string& name, DeviceFactory::Mode mode, const Properties& props) = 0;
};

#endif  // DeviceFactory_H
