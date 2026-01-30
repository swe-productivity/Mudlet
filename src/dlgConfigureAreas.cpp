/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "dlgConfigureAreas.h"
#include "dlgMapper.h"
#include "TArea.h"
#include "TMap.h"
#include "TRoomDB.h"

#include "mudlet.h"

#include <QElapsedTimer>
#include <QInputDialog>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QProgressDialog>

enum Columns 
{
    ColAreaName = 0,
    ColAreaId,
    ColRoomCount,
    ColCount
};

dlgConfigureAreas::dlgConfigureAreas(TMap* map, QWidget* parent) : QDialog(parent), mpMap(map)
{
    setupUi(this);
    areaTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    areaTable->setSelectionMode(QAbstractItemView::SingleSelection);

    setWindowTitle(tr("Configure Areas"));
    resize(400, 300);

    areaTable->setColumnCount(ColCount);
    areaTable->setColumnHidden(ColAreaId, true);

    areaTable->setHorizontalHeaderLabels({tr("Area Name"), QString(), tr("Rooms")});

    connect(addAreaBtn, &QPushButton::clicked, this, &dlgConfigureAreas::slot_addArea);
    connect(removeAreaBtn, &QPushButton::clicked, this, &dlgConfigureAreas::slot_removeArea);

    populateAreaList();
}

bool checkDefaultArea(TMap* map, int areaId)
{
    if (!map || !map->mpRoomDB) 
    {
        return false;
    }

    const QString defaultAreaName = map->getDefaultAreaName();
    const auto& areaNames = map->mpRoomDB->getAreaNamesMap();

    return areaNames.value(areaId) == defaultAreaName;
}

int dlgConfigureAreas::currentAreaId() const
{
    const auto selected = areaTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) 
    {
        return -1;
    }

    return areaTable->item(selected.first().row(), ColAreaId)->text().toInt();
}

void dlgConfigureAreas::refreshAreas()
{
    QWidget* p = parentWidget();
    while (p) 
    {
        const auto mappers = p->findChildren<dlgMapper*>();
        for (dlgMapper* mapper : mappers) 
        {
            mapper->updateAreaComboBox();
        }
        p = p->parentWidget();
    }
}

void dlgConfigureAreas::populateAreaList()
{
    if (!mpMap || !mpMap->mpRoomDB) 
    {
        return;
    }

    areaTable->setRowCount(0);

    const auto& areaNames = mpMap->mpRoomDB->getAreaNamesMap();

    int row = 0;
    for (auto it = areaNames.begin(); it != areaNames.end(); ++it) 
    {
        const int areaId = it.key();
        const QString& areaName = it.value();

        int roomCount = 0;
        if (TArea* area = mpMap->mpRoomDB->getArea(areaId)) 
        {
            roomCount = area->getAreaRooms().size();
        }

        areaTable->insertRow(row);
        areaTable->setItem(row, ColAreaName, new QTableWidgetItem(areaName));
        areaTable->setItem(row, ColAreaId, new QTableWidgetItem(QString::number(areaId)));
        areaTable->setItem(row, ColRoomCount, new QTableWidgetItem(QString::number(roomCount)));
        ++row;
    }

    areaTable->resizeColumnsToContents();
}

void dlgConfigureAreas::slot_addArea()
{
    if (!mpMap || !mpMap->mpRoomDB)
    {
        return;
    }

    bool ok{};
    const QString name = QInputDialog::getText(this, tr("Add Area:"), tr("Area Name"), QLineEdit::Normal, QString(), &ok);

    if (!ok || name.trimmed().isEmpty()) 
    {
        return;
    }

    const int newId = mpMap->mpRoomDB->addArea(name);
    if (newId <= 0) 
    {
        QMessageBox::warning(this, tr("Cannot add area"), tr("Area with this name already exists."));
        return;
    }

    populateAreaList();
    refreshAreas();
}

void dlgConfigureAreas::slot_removeArea()
{
    if (!mpMap || !mpMap->mpRoomDB) 
    {
        return;
    }

    const int areaId = currentAreaId();
    if (areaId <= 0) 
    {
        return;
    }

    if (checkDefaultArea(mpMap, areaId)) 
    {
        QMessageBox::warning(this, tr("Cannot delete area"), tr("Default area cannot be deleted."));
        return;
    }

    TArea* area = mpMap->mpRoomDB->getArea(areaId);
    const int roomCount = area ? area->getAreaRooms().size() : 0;

    if (roomCount > 0) 
    {
        QMessageBox::warning(this, tr("Cannot delete area"), tr("This area contains %1 rooms.\n"
               "Move or delete the rooms first.").arg(roomCount));
        return;
    }

    const QString areaName = mpMap->mpRoomDB->getAreaNamesMap().value(areaId);

    const auto ret = QMessageBox::warning(this, tr("Delete Area"), tr("Delete empty area \"%1\"?").arg(areaName),
                                        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);

    if (ret != QMessageBox::Yes) 
    {
        return;
    }

    mpMap->mpRoomDB->removeArea(areaId);

    populateAreaList();
    refreshAreas();
}
