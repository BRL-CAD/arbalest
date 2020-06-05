#ifndef GeometryOperationsManagerL
#define GeometryOperationsManagerL

#include <brlcad/MemoryDatabase.h>
#include <brlcad/Ellipsoid.h>

using namespace BRLCAD;

class GeometryOperationsManager{
private:
    MemoryDatabase * m_database;
public:
    GeometryOperationsManager(MemoryDatabase * m_database);
    void createSphere();
};

#endif