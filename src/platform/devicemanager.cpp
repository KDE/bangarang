#include "devicemanager.h"

#include <QtGlobal>

#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/OpticalDisc>
#include <Solid/DeviceNotifier>

DeviceManager *DeviceManager::m_instance = NULL;

DeviceManager::DeviceManager(QObject* parent): QThread(parent)
{
    m_deviceMutex = new QMutex();
}

DeviceManager::~DeviceManager()
{
    delete m_deviceMutex;
}

DeviceManager* DeviceManager::instance()
{
    static QMutex mutex;
    if (!m_instance)
    {
        mutex.lock();

        if (!m_instance) {
            m_instance = new DeviceManager();
            m_instance->start();
        }

        mutex.unlock();
    }

    return m_instance;
}

void DeviceManager::drop()
{
    static QMutex mutex;
    mutex.lock();
    m_instance->quit();
    delete m_instance;
    m_instance = 0;
    mutex.unlock();
}

QList< Solid::Device > DeviceManager::deviceList(DeviceManager::RelatedType reqType)
{
    QList< Solid::Device > list;
    m_deviceMutex->lock();
    QMapIterator<QString, DeviceDescription> i(m_devices);
    while (i.hasNext()) {
        i.next();
        const DeviceDescription & dev = i.value();
        //filter the list
        if (!dev.first.isValid()) {
            continue;
        }
        if (dev.second != AllTypes && reqType != dev.second ) {
            continue;
        }
        list.append(dev.first);
    }
    m_deviceMutex->unlock();
    return list;
}

void DeviceManager::run()
{
    //Currently only discs are supported
    QList<Solid::Device> devices = Solid::Device::listFromType(Solid::DeviceInterface::OpticalDisc, QString());
    //Create the device list
    m_deviceMutex->lock();
    foreach (Solid::Device device, devices) {
        const Solid::OpticalDisc *disc = device.as<const Solid::OpticalDisc> ();
        RelatedType relT = AllTypes;
        if (disc == NULL)
            continue;
        int type = disc->availableContent();
        if (type & Solid::OpticalDisc::Audio) {
            relT = AudioType;
        } else if (type & Solid::OpticalDisc::VideoDvd) {
            relT = VideoType;
        }
        m_devices.insert(device.udi(), DeviceDescription(device, relT));
    }
    m_deviceMutex->unlock();
    //connect to signals to stay up-to-date with our list
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(const QString & )),
            this, SLOT(deviceAdded(const QString & )));
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(const QString & )),
            this, SLOT(deviceRemoved(const QString & )));
    //don't forget to run the event loop, or signals/slots won't work
    exec();
}

void DeviceManager::deviceAdded(const QString &udi)
{
    //Check type of device that was added and add it to cached list
    Solid::Device devAdded(udi);
    RelatedType relT = AllTypes;
    if (!devAdded.isDeviceInterface(Solid::DeviceInterface::OpticalDisc))
        return;
    const Solid::OpticalDisc *disc = devAdded.as<const Solid::OpticalDisc> ();
    if (disc == NULL)
        return;
    int type = disc->availableContent();
    if (type & Solid::OpticalDisc::Audio) {
        relT = AudioType;
    } else if (type & Solid::OpticalDisc::VideoDvd) {
        relT = VideoType;
    }
    m_deviceMutex->lock();
    m_devices.insert(udi, DeviceDescription(devAdded, relT));
    m_deviceMutex->unlock();
    emit deviceListChanged( relT );
}

void DeviceManager::deviceRemoved(const QString &udi)
{
    //remove the device from the list
    m_deviceMutex->lock();
    if (m_devices.find(udi) == m_devices.end()) {
        m_deviceMutex->unlock();
        return;
    }
    RelatedType relT = m_devices.value(udi).second;
    m_devices.remove(udi);
    m_deviceMutex->unlock();
    emit deviceListChanged( relT );
}

