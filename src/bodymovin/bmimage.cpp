/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the lottie-qt module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "bmimage_p.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonObject>

#include "bmtrimpath_p.h"

QT_BEGIN_NAMESPACE

BMImage::BMImage(const BMImage &other)
    : BMBase(other)
{
    m_position = other.m_position;
    m_radius = other.m_radius;
    m_image = other.m_image;
}

BMImage::BMImage(const QJsonObject &definition, BMBase *parent)
{
    setParent(parent);
    construct(definition);
}

BMBase *BMImage::clone() const
{
    return new BMImage(*this);
}

void BMImage::construct(const QJsonObject &definition)
{
    BMBase::parse(definition);
    if (m_hidden)
        return;

    qCDebug(lcLottieQtBodymovinParser) << "BMImage::construct():" << m_name;

    QJsonObject asset = definition.value(QLatin1String("asset")).toObject();
    QString assetString = asset.value(QLatin1String("p")).toString();

    if (assetString.startsWith(QLatin1String("data:image"))) {
        QStringList assetsDataStringList = assetString.split(QLatin1String(","));
        if (assetsDataStringList.length() > 1) {
            QByteArray assetData = QByteArray::fromBase64(assetsDataStringList[1].toLatin1());
            m_image.loadFromData(assetData);
        }
    }
    else {
        QFileInfo info(asset.value(QLatin1String("fileSource")).toString());
        QString url = info.path() + QDir::separator() + asset.value(QLatin1String("u")).toString() + assetString;
        QString path = QUrl(url).toLocalFile();
        m_image.load(path);
        if (m_image.isNull()) {
            qWarning() << "Unable to load file " << path;
        }
    }

    QJsonObject position = definition.value(QLatin1String("p")).toObject();
    position = resolveExpression(position);
    m_position.construct(position);

    QJsonObject radius = definition.value(QLatin1String("r")).toObject();
    radius = resolveExpression(radius);
    m_radius.construct(radius);
}

void BMImage::updateProperties(int frame)
{
    m_position.update(frame);
    m_radius.update(frame);

    m_center = QPointF(m_position.value().x() - m_radius.value() / 2,
                             m_position.value().y() - m_radius.value() / 2);
}

void BMImage::render(LottieRenderer &renderer) const
{
    renderer.render(*this);
}

QPointF BMImage::position() const
{
    return m_position.value();
}

qreal BMImage::radius() const
{
    return m_radius.value();
}

QT_END_NAMESPACE
