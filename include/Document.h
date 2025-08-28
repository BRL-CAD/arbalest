/*                          D O C U M E N T . H
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

#ifndef RT3_DOCUMENT_H
#define RT3_DOCUMENT_H

#include "Viewport.h"
#include "ObjectTree.h"
#include "ObjectTreeWidget.h"
#include "Properties.h"
#include "GeometryRenderer.h"
#include "ViewportGrid.h"
#include <include/RaytraceView.h>


class Properties;
class Viewport;
class GeometryRenderer;
class ViewportGrid;
class RaytraceView;
class ObjectTreeWidget;
class Console;


class Document {
private:
    QString *filePath = nullptr;
    BRLCAD::MemoryDatabase *database;
    ViewportGrid *displayGrid;
    ObjectTreeWidget *objectTreeWidget;
    Properties *properties;
    Console *console;
    const int documentId;
    ObjectTree* objectTree;
    GeometryRenderer * geometryRenderer;
    bool modified;


public:
    explicit Document(int documentId, const QString *filePath = nullptr);
    virtual ~Document();

    void modifyObject(BRLCAD::Object* newObject);

    RaytraceView * raytraceWidget;
    // getters setters
    QString* getFilePath() const
    {
	    return filePath;
    }

    RaytraceView * getRaytraceWidget() const
    {
        return raytraceWidget;
    }
    BRLCAD::ConstDatabase* getDatabase() const
    {
        return database;
    }

    Viewport* getViewport();

    ObjectTreeWidget* getObjectTreeWidget() const
    {
	    return objectTreeWidget;
    }

    Properties* getProperties() const
    {
	    return properties;
    }

    Console* getConsole() const
    {
	    return console;
    }

    ViewportGrid *getViewportGrid()  {
        return displayGrid;
    }

    int getDocumentId() const
    {
	    return documentId;
    }

    ObjectTree* getObjectTree() const
    {
	    return objectTree;
    }
    GeometryRenderer *getGeometryRenderer(){
        return geometryRenderer;
    }

    void setFilePath(const QString& filePath)
    {
        this->filePath = new QString(filePath);
    }

    void setModified()
    {
        this->modified = true;
    }

    bool isModified();
    bool AddObject(const BRLCAD::Object& object, const bool isVisible);
    bool Save(const char* fileName);
    void getBRLCADConstObject(const QString& objectName, const std::function<void(const BRLCAD::Object&)>& func) const;
    void getBRLCADObject(const QString& objectName, const std::function<void(BRLCAD::Object&)>& func);
};


#endif //RT3_DOCUMENT_H
