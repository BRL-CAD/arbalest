/*                           P R O P E R T I E S . H
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
/** @file Properties.h */

#ifndef RT3_PROPERTIES_H
#define RT3_PROPERTIES_H


#include <QLabel>
#include <brlcad/Database/Object.h>
#include "QVBoxWidget.h"


class Document;
class TypeSpecificProperties;


class Properties: public QVBoxWidget {
public:
    explicit Properties(Document* document);

    void bindObject(const size_t objectId);
    void rewriteObjectNameAndType();

private:
    QString name, fullPath, objectType;

	// UI components
    Document* document;
    TypeSpecificProperties* current;
    BRLCAD::Object* object;
    QLabel* nameWidget;
    QLabel* fullPathWidget;
    QVBoxWidget* typeSpecificPropertiesArea;
};


#endif //RT3_PROPERTIES_H
