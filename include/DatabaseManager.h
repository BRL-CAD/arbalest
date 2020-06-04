#ifndef RT3_DATABASEMANAGER_H
#define RT3_DATABASEMANAGER_H


#include <brlcad/MemoryDatabase.h>
#include <map>
#include <vector>
#include <string>

class DatabaseManager {
private:
    int lastDatabaseIndex = -1;
    std::map<int,BRLCAD::MemoryDatabase*> databases;
public:
    int addNewDatabase(const char *filePath);
    BRLCAD::MemoryDatabase *getDatabase(int index);
    std::vector<std::string> getObjectsList(int index);
};


#endif //RT3_DATABASEMANAGER_H
