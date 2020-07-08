
#include <GeometryOperationsManager.h>

GeometryOperationsManager::GeometryOperationsManager(MemoryDatabase & m_database): database(m_database) {}

void GeometryOperationsManager::createSphere(){
    Ellipsoid ellipsoid;
    ellipsoid.SetSphere(Vector3D(0,0,0),300);
    ellipsoid.SetName("Newly created sphere 42");
    database.Add(ellipsoid);


    Ellipsoid ellipsoid2;
    ellipsoid2.SetSphere(Vector3D(0,0,0),300);
    ellipsoid2.SetName("Newly created sphere 42");
    database.Add(ellipsoid2);

}

