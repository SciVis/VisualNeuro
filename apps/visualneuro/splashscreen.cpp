/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include "splashscreen.h"
#include <inviwo/core/common/inviwocommondefines.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/stringconversion.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <visualneuro/visualneurocommondefines.h>

#include <QApplication>
#include <QPainter>
#include <QSplashScreen>
#include <QTextStream>

namespace inviwo {

InviwoSplashScreen::InviwoSplashScreen(bool enable)
    : QSplashScreen(QPixmap(":/resources/splashscreen.png")), showSplashScreen_(enable) {}

InviwoSplashScreen::~InviwoSplashScreen() = default;

void InviwoSplashScreen::show() {
    if (showSplashScreen_) QSplashScreen::show();
}

void InviwoSplashScreen::drawContents(QPainter* painter) {
    QString versionLabel;
    QTextStream labelStream(&versionLabel);
    labelStream << "Visual Neuro Version " << QString::fromStdString(toString(visualneuro::build::version));
    painter->setPen(Qt::white);
    painter->drawText(12, 306, versionLabel);
    QString inviwoVersionLabel;
    QTextStream inviwoLabelStream(&inviwoVersionLabel);
    inviwoLabelStream << "Powered by Inviwo Version " << QString::fromStdString(toString(build::version));
    painter->drawText(12, 326, inviwoVersionLabel);
    auto font = painter->font();
    font.setPointSizeF(font.pointSizeF() * 0.8f);
    painter->setFont(font);
    painter->drawText(12, 346, message());
}

void InviwoSplashScreen::showMessage(std::string_view message) {
    if (showSplashScreen_) QSplashScreen::showMessage(utilqt::toQString(message));
}

void InviwoSplashScreen::finish(QWidget* waitFor) {
    if (showSplashScreen_) QSplashScreen::finish(waitFor);
}

}  // namespace inviwo
