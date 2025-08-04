/*                            G R A P H I C V I E W . C P P
 * BRL-CAD
 *
 * Copyright (c) 2018 United States Government as represented by
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
/** @file GraphicView.cpp
 *
 *  BRL-CAD Qt GUI:
 *      implementation of the graphical visualization
 */

#include <cmath>

#include <QPainter>
#include <QMessageBox>

#include "RaytraceView.h"
#include <QBitmap>
#include <QtWidgets/QFileDialog>
#include <QtOpenGL/QtOpenGL>

RaytraceView::RaytraceView
(
    Document* document,
    QWidget*               parent
) : QWidget(parent),
    document(document),
    m_database(*document->getDatabase()),
    m_transformation(),
    m_image(),
    m_imageUpTodate(false),
    m_updatingImage(false) {
    setMinimumSize(100, 100);
    setWindowIcon(*new QIcon(*new QBitmap(":/icons/arbalest_icon.png")));
    setWindowFlags(Qt::Window| Qt::WindowCloseButtonHint);
}


void RaytraceView::Update() {
    m_imageUpTodate = false;

    update();
}


void RaytraceView::UpdateTrafo(const QMatrix4x4& transformation) {
    if (transformation != m_transformation) {
        m_transformation = transformation;

        Update();
    }
}


void RaytraceView::paintEvent
(
    QPaintEvent*
) {

    QPainter painter(this);
    painter.drawImage(0, 0, m_image);
}


static void RayTraceCallback
(
    const QVector3D&                  direction,
    QColor&                           color,
    const BRLCAD::ConstDatabase::Hit& hit
) {
    double    brightness         = 0;
    double    ambient            = 0.1;
    double    diffuseWeight      = 0.5;
    double    specularWeight     = 0.5;
    int       n                  = 4;
    QVector3D normal             = QVector3D(hit.SurfaceNormalIn().coordinates[0], hit.SurfaceNormalIn().coordinates[1], hit.SurfaceNormalIn().coordinates[2]);
    double    dotProduct         = QVector3D::dotProduct(direction, normal); // negative because of opposite directions

    // value from 0 to 1
    double    diffuse            = -dotProduct;

    // refleced = incidence - 2 normal
    QVector3D reflectedDir       = direction - 2. * dotProduct * normal;
    reflectedDir.normalize();

    // value from 0 to 1
    double    reflectedDotCamDir = std::max(0.f, QVector3D::dotProduct(reflectedDir, direction));
    double    specular           = pow(reflectedDotCamDir, n);

    brightness += ambient + diffuse * diffuseWeight;

    double    red                = std::min(hit.Red() * brightness + specular * specularWeight, 1.0);
    double    green              = std::min(hit.Green() * brightness + specular * specularWeight, 1.0);
    double    blue               = std::min(hit.Blue() * brightness + specular * specularWeight, 1.0);

    color.setRgbF(red, green, blue);
}


void RaytraceView::UpdateImage() {
    int w  = width();
    int h = height();

    m_image = QImage(w, h, QImage::Format_RGB32);

    QVector3D directionStart = m_transformation.map(QVector3D(0., 0., 1.));
    QVector3D directionEnd   = m_transformation.map(QVector3D(0., 0., 0.));
    QVector3D direction      = directionEnd - directionStart;
    direction.normalize();

    for (int column = 0; column < w; column++) {
        for (int row = 0; row < h; row++) {
            QVector3D     imagePoint(column, h - row - 1., 0.);
            QVector3D     modelPoint = m_transformation.map(imagePoint);
            QColor        pixelColor(color);
            BRLCAD::Ray3D ray;

            ray.origin.coordinates[0]    = modelPoint.x();
            ray.origin.coordinates[1]    = modelPoint.y();
            ray.origin.coordinates[2]    = modelPoint.z();
            ray.direction.coordinates[0] = direction.x();
            ray.direction.coordinates[1] = direction.y();
            ray.direction.coordinates[2] = direction.z();

            m_database.ShootRay(ray, [&direction, &pixelColor](const BRLCAD::ConstDatabase::Hit& hit){RayTraceCallback(direction, pixelColor, hit); return false;}, BRLCAD::ConstDatabase::StopAfterFirstHit);

            m_image.setPixelColor(column, row, pixelColor);
        }
    }
}


void RaytraceView::raytrace() {
    QSettings settings("BRLCAD", "arbalest");
    color=settings.value("raytraceBackground").value<QColor>();
    bool valid = color.isValid();
    if (!valid) color = Qt::black;

    hide();
    document->getDatabase()->UnSelectAll();
    document->getObjectTree()->traverseSubTree(0, false, [this]
                                                       (unsigned int objectId){
                                                   ObjectTreeItem *item = document->getObjectTree()->getItems()[objectId];
                                                   switch(item->getVisibilityState()){
                                                       case ObjectTreeItem::Invisible:
                                                           return false;
                                                       case ObjectTreeItem::SomeChildrenVisible:
                                                           return true;
                                                       case ObjectTreeItem::FullyVisible:
                                                           QString fullPath = item->getPath();
                                                           document->getDatabase()->Select(fullPath.toUtf8());
                                                           return false;
                                                   }
                                                   return true;
                                               }
    );

    QMatrix4x4 transformation;
    transformation.translate(document->getViewport()->getCamera()->getEyePosition().x(),
                             document->getViewport()->getCamera()->getEyePosition().y(),
                             document->getViewport()->getCamera()->getEyePosition().z());
    transformation.rotate(-document->getViewport()->getCamera()->getAnglesAroundAxes().y(), 0., 1., 0.);
    transformation.rotate(-document->getViewport()->getCamera()->getAnglesAroundAxes().z(), 0., 0., 1.);
    transformation.rotate(-document->getViewport()->getCamera()->getAnglesAroundAxes().x(), 1., 0., 0.);
    transformation.translate(0, 0, 10000);
    transformation.scale(document->getViewport()->getCamera()->getVerticalSpan()/document->getViewport()->getH());
    transformation.translate(-document->getViewport()->getW()/2.,-document->getViewport()->getH()/2.);

    resize(document->getViewport()->getW(),document->getViewport()->getH());
    UpdateTrafo(transformation);
    UpdateImage();

    Update();
    setWindowTitle("Raytrace");
    show();



    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Save?", "Do you want to save the raytraced image?",  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        const QString filePath = QFileDialog::getSaveFileName(this, tr("Save raytraced image"), QString(), "PNG file (*.png)");
        if (!filePath.isEmpty()) {
            m_image.save(filePath);
        }
    } else {

    }

}
