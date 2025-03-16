#include <QtWidgets/QLabel>
#include <QBitmap>
#include <QIcon>
#include <QScreen>
#include <QApplication>
#include <QtWidgets/QScrollArea>
#include <algorithm>
#include "AboutWindow.h"

AboutWindow::AboutWindow() : QVBoxWidget()
{

    setWindowFlags(Qt::Window| Qt::WindowCloseButtonHint);
    setObjectName("aboutWindow");
    setWindowTitle("About");
    setWindowIcon(*new QIcon(*new QBitmap(":/icons/arbalest_icon.png")));

    QVBoxWidget *container = new QVBoxWidget();
    QLabel *icon = new QLabel();
    QPixmap *pixmap = new QPixmap(":/icons/arbalest_icon.png");
    icon->setPixmap(*pixmap);
    icon->setWordWrap(true);
    icon->setMargin(20);

    QLabel *intro = new QLabel(
            "Arbalest is an open source geometry editor application for the open source project "
            "BRL-CAD (<a href=\"https://brlcad.org\">brlcad.org</a>). It is a part of BRL-CAD and relies heavily on "
            "the BRL-CAD libraries. "
            "<br><br>Arbalest source : <a href=\"https://github.com/sadeepdarshana/arbalest\">github.com/sadeepdarshana/arbalest</a> "
            "<br>BRL-CAD source : <a href=\"https://sourceforge.net/projects/brlcad/\">sourceforge.net/projects/brlcad/</a> <br> <br>"

            "Arbalest is dedicated to the memory of Michael Muuss, who served as a Senior Computer Scientist at the U.S. "
            "Army Research Laboratory's (ARL) Survivability/Lethality Analysis Directorate (SLAD) at Aberdeen Proving Ground, "
            "MD, from 1979 until his death in November 2000.<br><br>"

            "Arbalest was initially developed for a 2020 Google Summer of Code by Sadeep Weerasinghe (sadeep.16@cse.mrt.ac.lk)."
            "<br><br>"

            "License follows from here.<br>"
            "BRL-CAD Copying and Distribution<br>"
            "================================<br>"
            "<br>"
            "BRL-CAD is Open Source software with copyright primarily held by the<br>"
            "U.S. Government.  The source code is controlled and maintained by a<br>"
            "core team of Open Source developers working around the world.  Those<br>"
            "core developers operate under a meritocracy organizational structure<br>"
            "adhering to developer guidelines outlined in the HACKING developer's<br>"
            "guide and to the legal conventions and requirements outlined in this<br>"
            "document.<br>"
            "<br>"
            "As a unified work, BRL-CAD is made available under the terms of the<br>"
            "GNU Lesser General Public License (LGPL) as published by the Free<br>"
            "Software Foundation.<br>"
            "<br>"
            "As distinctly organized and separate components, parts of BRL-CAD are<br>"
            "made available under different licenses for different portions<br>"
            "including the BSD license and files in the public domain. The Overview<br>"
            "section describes how the various licenses apply to the different<br>"
            "portions of BRL-CAD.<br>"
            "<br>"
            "BRL-CAD is distributed in the hope that it will be useful, but WITHOUT<br>"
            "ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or<br>"
            "FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public<br>"
            "License for more details.<br>"
            "<br>"
            "You should have received a copy of the GNU Lesser General Public<br>"
            "License along with this file.  If you did not, please contact one of<br>"
            "the BRL-CAD project administrators listed at the SourceForge project<br>"
            "site http://sf.net/projects/brlcad/ or listed in the AUTHORS file<br>"
            "included as part of the distribution.<br>"
            "<br>"
            "Overview<br>"
            "--------<br>"
            "<br>"
            "BRL-CAD consists primarily of libraries, applications, scripts,<br>"
            "documentation, geometric models, images, data resources, and build<br>"
            "infrastructure.  The BRL-CAD libraries provide their headers in the<br>"
            "include/ directory as well as all of the directories that begin with<br>"
            "the prefix \"lib\" in the src/ directory (e.g. src/librt/).  All of<br>"
            "BRL-CAD's libraries, applications, and any content lacking a license<br>"
            "specification are covered by the terms of the LGPL.<br>"
            "<br>"
            "Most of BRL-CAD's build infrastructure including files required by<br>"
            "the GNU Build System (such as the CMakeLists.txt files), a variety<br>"
            "of scripts located in the misc/ and sh/ directories, and the testing<br>"
            "infrastructure (scripts and resources) are provided either under<br>"
            "the BSD license or are in the public domain.  Similarly, most of<br>"
            "the geometric models, images, and other data resources are also<br>"
            "provided under the BSD license.  Refer to each individual file for<br>"
            "specific terms.<br>"
            "<br>"
            "BRL-CAD's documentation consists of manual pages located throughout<br>"
            "the package, most of the files in the doc/ directory, and the<br>"
            "top-level administrative project text files (e.g. README & AUTHORS).<br>"
            "Unless otherwise denoted, all of BRL-CAD's documentation is made<br>"
            "available under the terms of the BSD Documentation License (BDL) or<br>"
            "are in the public domain.  Refer to each document for specific terms.<br>"
            "<br>"
            "The full text of the LGPL, BDL, and BSD license should be included in<br>"
            "the source distribution of BRL-CAD in the doc/legal/ directory.  Refer<br>"
            "to the full license text for more details, information, requirements,<br>"
            "and implications of each license.<br>"
            "<br>"
            "3rd-Party Components<br>"
            "--------------------<br>"
            "<br>"
            "The BRL-CAD package utilizes and redistributes several 3rd party<br>"
            "source codes, libraries, and applications.  Said 3rd party source<br>"
            "code, libraries, and applications all retain their respective<br>"
            "agreements, licenses, and copyrights and should be referred to<br>"
            "individually for their specific terms.  For the purposes of<br>"
            "redistribution and legal compatibility for all of BRL-CAD's existing<br>"
            "user base, NO 3RD PARTY LIBRARIES SHALL BE INCLUDED THAT ARE UNDER THE<br>"
            "TERMS OF THE GNU GENERAL PUBLIC LICENSE or any other license with<br>"
            "similar legal implications that would impose legal obligations on<br>"
            "external codes that utilize BRL-CAD as a library.<br>"
            "<br>"
            "When referring to BRL-CAD, its source code, documentation, and<br>"
            "libraries, the reference is not meant to include or imply 3rd party<br>"
            "source code, libraries, and/or applications unless specifically stated<br>"
            "otherwise.  No endorsement or claim of authorship is implied to any<br>"
            "3rd party asset that is included with BRL-CAD.  The majority of the<br>"
            "3rd party source code, libraries, and applications included with<br>"
            "BRL-CAD are located in the src/other/ directory of the source code<br>"
            "distribution.<br>"
            "<br>"
            "Apache License 2.0<br>"
            "------------------<br>"
            "<br>"
            "Copyright 2022 BRL - CAD<br>"
            "<br>"
            "Licensed under the Apache License, Version 2.0 (the \"License\");<br>"
            "you may not use this file except in compliance with the License.<br>"
            "You may obtain a copy of the License at<br>"
            "<br>"
            "http ://www.apache.org/licenses/LICENSE-2.0<br>"
            "<br>"
            "Unless required by applicable law or agreed to in writing, software<br>"
            "distributed under the License is distributed on an \"AS IS\" BASIS,<br>"
            "WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.<br>"
            "See the License for the specific language governing permissionsand<br>"
            "limitations under the License.<br>"
            "<br>"
            "Files under Apache License 2.0:<br>"
            "1. resources\\icons\\quitIcon.png<br>"
            "2. resources\\icons\\saveAsIcon.png<br>"
            "<br>"
            "Copyright<br>"
            "---------<br>"
            "<br>"
            "All contributions to BRL-CAD have been provided under agreement.  By<br>"
            "requiring all contributions be given back to the BRL-CAD developers,<br>"
            "this agreement allows the BRL-CAD project to continue to grow<br>"
            "unhindered.  As such, a majority of the source code is copyright by<br>"
            "the United States Government as represented by the United States Army<br>"
            "Research Laboratory.<br>"
            "<br>"
            "Authors and other BRL-CAD contributors must comply with the copyright<br>"
            "terms for their respective contributions unless otherwise noted or<br>"
            "arranged.  This includes an implicit assignment of copyright for any<br>"
            "and all contributions being made.  The following notice should be<br>"
            "prominent in the BRL-CAD sources:<br>"
            "<br>"
            "  Copyright (c) 1984-2016 United States Government as represented by<br>"
            "  the U.S. Army Research Laboratory.<br>"
            "<br>"
            "Contact<br>"
            "-------<br>"
            "<br>"
            "Questions or comments regarding BRL-CAD legal issues pertaining to<br>"
            "copying, linking, licensing, trademark, or otherwise should be<br>"
            "directed to the BRL-CAD Development Team at the following address:<br>"
            "<br>"
            "BRL-CAD Development Team<br>"
            "devs@brlcad.org<br>"
            "http://brlcad.org<br>"
            "<br>"
            "---<br>"
            "$Revision: 68616 $"
            );
    intro->setWordWrap(true);
    intro->setMargin(15);
    intro->setObjectName("aboutText");

    intro->setTextFormat(Qt::RichText);
    intro->setTextInteractionFlags(Qt::TextBrowserInteraction);
    intro->setOpenExternalLinks(true);

    container->addWidget(icon);
    container->addWidget(intro);
    QScrollArea * scrollArea = new QScrollArea();
    scrollArea->setObjectName("aboutWindow");
    icon->setObjectName("aboutWindow");
    addWidget(scrollArea);
    scrollArea->setWidget(container);

    setMinimumWidth(std::max(icon->width()*1.02,intro->width()*1.02));
    setFixedHeight(QApplication::primaryScreen()->geometry().height() * .75);
}
