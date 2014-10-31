#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QThread>
#include <Solid/Device>
#include <QMutex>

class DeviceManager : public QThread
{
    Q_OBJECT
    
public:
    enum RelatedType {
        AudioType,
        VideoType,
        AllTypes
    };
    typedef QPair<Solid::Device, DeviceManager::RelatedType> DeviceDescription;

    static DeviceManager* instance();
    static void drop();
    QList<Solid::Device> deviceList(RelatedType type = AllTypes);

signals:
    void deviceListChanged(DeviceManager::RelatedType type);

protected:
    virtual void run();

private:

    DeviceManager(QObject* parent = 0);
    ~DeviceManager();
    DeviceManager(const DeviceManager &); // hide copy constructor
    DeviceManager& operator=(const DeviceManager &); // hide assign op
                                 // we leave just the declarations, so the compiler will warn us
                                 // if we try to use those two functions by accident

    static DeviceManager* m_instance;
    QMutex *m_deviceMutex;
    QMap<QString, DeviceDescription> m_devices;

private slots:
    void deviceAdded(const QString &udi);
    void deviceRemoved(const QString &udi);
};

#endif // DEVICEMANAGER_H
