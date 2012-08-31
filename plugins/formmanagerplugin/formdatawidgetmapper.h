/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2012 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program (COPYING.FREEMEDFORMS file).                   *
 *  If not, see <http://www.gnu.org/licenses/>.                            *
 ***************************************************************************/
/***************************************************************************
 *   Main developers : Eric MAEKER, <eric.maeker@gmail.com>                *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 *       NAME <MAIL@ADDRESS.COM>                                           *
 ***************************************************************************/
#ifndef FORM_INTERNAL_FORMDATAWIDGETMAPPER_H
#define FORM_INTERNAL_FORMDATAWIDGETMAPPER_H

#include <QWidget>
QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/**
 * \file formdatawidgetmapper.h
 * \author Eric MAEKER
 * \version 0.8.0
 * \date 30 Aug 2012
*/

namespace Form {
class FormMain;
namespace Internal {
class FormDataWidgetMapperPrivate;

class FormDataWidgetMapper : public QWidget
{
    Q_OBJECT
public:
    explicit FormDataWidgetMapper(QWidget *parent = 0);
    ~FormDataWidgetMapper();

    bool initialize();

    void setCurrentForm(const QString &formUid);
    void setCurrentForm(FormMain *form);

    void setCurrentEpisode(const QVariant &uid);
    void setCurrentEpisode(const QModelIndex &index);

Q_SIGNALS:
    
public Q_SLOTS:
    
private Q_SLOTS:

private:
    FormDataWidgetMapperPrivate *d;
};

} // namespace Internal
} // namespace Form

#endif // FORM_INTERNAL_FORMDATAWIDGETMAPPER_H