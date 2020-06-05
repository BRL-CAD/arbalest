#ifndef RT3_DATABASEMANAGER_H
#define RT3_DATABASEMANAGER_H


#include <brlcad/MemoryDatabase.h>
#include <map>
#include <vector>
#include <string>
#include <string>
#include <GeometryOperationsManager.h>

class DatabaseManager {
private:
    int lastDatabaseIndex = -1;
    std::map<int,BRLCAD::MemoryDatabase*> databases;
    std::map<int,GeometryOperationsManager*> geometryOperationsManagers;
public:
    int addNewDatabase(const char *filePath);
    BRLCAD::MemoryDatabase *getDatabase(int index);
    GeometryOperationsManager *getGeometryOperationsManager(int index);
    std::vector<std::string> getObjectsList(int index);

};


#endif //RT3_DATABASEMANAGER_H
