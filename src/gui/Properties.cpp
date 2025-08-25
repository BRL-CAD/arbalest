/*                         P R O P E R T I E S . C P P
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
/** @file Properties.cpp */


#include "Document.h"
#include "Globals.h"
#include "TypeSpecificProperties.h"

#include "Properties.h"


Properties::Properties(Document* document) : document(document), object(nullptr), current(nullptr) {
    nameWidget = new QLabel(this);
    nameWidget->setWordWrap(true);
    nameWidget->setObjectName("properties-nameWidget");

    fullPathWidget = new QLabel(this);
    fullPathWidget->setWordWrap(true);
    fullPathWidget->setObjectName("properties-fullPathWidget");

    typeSpecificPropertiesArea = new QVBoxWidget(this);

    layout()->addWidget(nameWidget);
    layout()->addWidget(fullPathWidget);
    layout()->addWidget(typeSpecificPropertiesArea);
    getBoxLayout()->addStretch();
}


void Properties::bindObject(const size_t objectId) {
    ObjectTreeItem* item = document->getObjectTree()->getItems()[objectId];

    name = item->getName();
    fullPath = item->getPath();
    fullPathWidget->setText(QString(fullPath).replace("/"," / "));

    delete object;
    object = document->getDatabase()->Get(fullPath.toUtf8().data());
    QString objectType = object->Type();

    delete current;
    current = new TypeSpecificProperties(document, object, objectId);
    typeSpecificPropertiesArea->addWidget(current);

    QString nameType = "<font color='$Color-PropertiesObjectNameText'>" + name + "</font> ("
                       "<font color='$Color-PropertiesObjectTypeText'>" + breakStringAtCaps(objectType) + "</font>)";
    nameWidget->setText(Globals::theme->process(nameType));
}


void Properties::rewriteObjectNameAndType() {
    if (name == "") return;  // To avoid rewriting if no object is selected (meaning that properties is empty)
    QString nameType = "<font color='$Color-PropertiesObjectNameText'>" + name + "</font> ("
                       "<font color='$Color-PropertiesObjectTypeText'>" + breakStringAtCaps(objectType) + "</font>)";
    nameWidget->setText(Globals::theme->process(nameType));
}
