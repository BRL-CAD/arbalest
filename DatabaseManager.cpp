#include "../include/DatabaseManager.h"
#include <string>
#include <vector>

int DatabaseManager::addNewDatabase(const char *filePath) {
    auto  database =  new BRLCAD::MemoryDatabase();
    database->Load(filePath);
    databases[++lastDatabaseIndex] = database;
    geometryOperationsManagers[lastDatabaseIndex] = new GeometryOperationsManager(database);
    return lastDatabaseIndex;
}

BRLCAD::MemoryDatabase *DatabaseManager::getDatabase(int index) {
    return databases[index];
}
GeometryOperationsManager *DatabaseManager::getGeometryOperationsManager(int index) {
    return geometryOperationsManagers[index];
}

std::vector<std::string> DatabaseManager::getObjectsList(int index) {
    std::vector<std::string> list;
    auto database = getDatabase(index);

    BRLCAD::ConstDatabase::TopObjectIterator it = database->FirstTopObject();
    while (it.Good()){
        list.push_back(it.Name());
        ++it;
    }
    return list;
}
