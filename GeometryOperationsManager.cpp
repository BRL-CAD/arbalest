
#include <GeometryOperationsManager.h>

GeometryOperationsManager::GeometryOperationsManager(MemoryDatabase * m_database) {
    this->m_database = m_database;
}

void GeometryOperationsManager::createSphere(){
    Ellipsoid ellipsoid;
    ellipsoid.SetSphere(Vector3D(0,0,0),300);
    ellipsoid.SetName("Newly created sphere 42");
    m_database->Add(ellipsoid);

}

