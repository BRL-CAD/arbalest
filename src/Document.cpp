/*                        D O C U M E N T . C P P
 * BRL-CAD
 *
 * Copyright (c) 2020-2025 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/** @file Document.h */


#include <Document.h>
#include <Viewport.h>
#include "Console.h"
#include <brlcad/Database/Torus.h>


Document::Document(const int documentId, const QString *filePath) : documentId(documentId) {
    if (filePath != nullptr) this->filePath = new QString(*filePath);
    database =  new BRLCAD::MemoryDatabase();
    if (filePath != nullptr) {
        if (!database->Load(filePath->toUtf8().data()))
        {
            throw std::runtime_error("Failed to open file");
        }
    }

    modified = false;
    objectTree = new ObjectTree(database, this);
    properties = new Properties(this);
    console = new Console(this);
    geometryRenderer = new GeometryRenderer(this);
    objectTreeWidget = new ObjectTreeWidget(this);
    displayGrid = new ViewportGrid(this);

    displayGrid->forceRerenderAllViewports();

    raytraceWidget = new RaytraceView(this);
}

Document::~Document() {
    delete database;
}

void Document::modifyObject(BRLCAD::Object *newObject) {
    modified = true;
    database->Set(*newObject);
    QString objectName = newObject->Name();
    getObjectTree()->traverseSubTree(getObjectTree()->getRootItem(), false, [this, objectName](ObjectTreeItem* currItem) {
        if (currItem->getName() == objectName)
            geometryRenderer->clearObject(currItem->getObjectId());
        return true;
    });
    geometryRenderer->refreshForVisibilityAndSolidChanges();
    for (Viewport * display : displayGrid->getViewports())display->forceRerenderFrame();
}

bool Document::isModified() {
    return modified;
}

bool Document::AddObject(const BRLCAD::Object& object, const bool isVisible) {
    bool ret = database->Add(object);
    if (ret) {
        modified = true;
        size_t objectId = getObjectTree()->addTopObject(QString(object.Name()));
        getObjectTree()->changeVisibilityState(objectId, isVisible);
        getObjectTreeWidget()->build(objectId);
        getObjectTreeWidget()->refreshItemTextColors();
        getGeometryRenderer()->refreshForVisibilityAndSolidChanges();
        getViewportGrid()->forceRerenderAllViewports();
    }
    return ret;
}

bool Document::Save(const char* fileName) {
    modified = false;
    return database->Save(fileName);
}

void Document::getBRLCADConstObject(const QString& objectName, const std::function<void(const BRLCAD::Object&)>& func) const {
    database->Get(objectName.toUtf8(), [func](const BRLCAD::Object& object){func(object);});
}

void Document::getBRLCADObject(const QString& objectName, const std::function<void(BRLCAD::Object&)>& func) {
    database->Get(objectName.toUtf8(), func);
    modified = true;
}

Viewport* Document::getViewport()
{
    return displayGrid->getActiveViewport();
}
