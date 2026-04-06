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

#include <QInputDialog>
#include <QMessageBox>

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

    areaTable->setColumnCount(3);
    areaTable->setHorizontalHeaderLabels({tr("Area Name"), tr("Area ID"), tr("Rooms")});
    areaTable->setColumnHidden(1, true);
    areaTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    areaTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    areaTable->horizontalHeader()->setStretchLastSection(true);

    connect(addAreaBtn, &QPushButton::clicked, this, &dlgConfigureAreas::slot_addArea);
    connect(removeAreaBtn, &QPushButton::clicked, this, &dlgConfigureAreas::slot_removeArea);

    populateAreaList();
}

int dlgConfigureAreas::currentAreaId() const
{
    const auto selected = areaTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return -1;
    }

    return areaTable->item(selected.first().row(), ColAreaId)->text().toInt();
}

void dlgConfigureAreas::refreshAreas(bool added, const QString& areaName)
{
    if(!mpMap || !mpMap->mpMapper)
        return;

    auto mapper = mpMap->mpMapper;
    mapper->updateAreaComboBox();
    QComboBox* combo = mapper->findChild<QComboBox*>();

    QString targetArea;
    if(added) {
        targetArea = areaName;
    }
    else {
        targetArea = mpMap->getDefaultAreaName();
    }

    const int index = combo->findText(targetArea);
    if(index >= 0) {
        mapper->slot_switchArea(index);
        combo->setCurrentIndex(index);
    }

    mpMap->setUnsaved(__func__);
}

void dlgConfigureAreas::populateAreaList()
{
    if (!mpMap || !mpMap->mpRoomDB) {
        return;
    }

    areaTable->setRowCount(0);
    const auto& areaNames = mpMap->mpRoomDB->getAreaNamesMap();
    int row = 0;
    for (auto it = areaNames.begin(); it != areaNames.end(); ++it) {
        const int areaId = it.key();
        const QString& areaName = it.value();
        int roomCount = 0;
        if (TArea* area = mpMap->mpRoomDB->getArea(areaId)) {
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
    if (!mpMap || !mpMap->mpRoomDB) {
        return;
    }

    bool ok{};
    const QString rawName = QInputDialog::getText(this, tr("Add Area:"), tr("Area Name"), QLineEdit::Normal, QString(), &ok);
    const QString name = rawName.trimmed();
    if (!ok || name.isEmpty()) {
        return;
    }

    if (mpMap->mpRoomDB->getAreaNamesMap().values().count(name) > 0) {
        statusLabel->setText(tr("Area with this name already exists."));
        return;
    }

    mpMap->mpRoomDB->addArea(name);
    statusLabel->clear();

    populateAreaList();
    refreshAreas(true, name);
}

void dlgConfigureAreas::slot_removeArea()
{
    if (!mpMap || !mpMap->mpRoomDB) {
        return;
    }

    const int areaId = currentAreaId();
    if (areaId == -1) {
        statusLabel->setText(tr("Default area cannot be deleted."));
        return;
    }

    TArea* area = mpMap->mpRoomDB->getArea(areaId);
    const int roomCount = area ? area->getAreaRooms().size() : 0;
    if (roomCount > 0) {
        const auto reply = QMessageBox::warning(this, tr("Delete area"), tr("This will also delete %1 rooms.\nDo you want to continue?").arg(roomCount),
                           QMessageBox::Yes | QMessageBox::No,
                           QMessageBox::No);

        if (reply != QMessageBox::Yes) {
            return;
        }
    }

    mpMap->mpRoomDB->removeArea(areaId);
    statusLabel->clear();

    populateAreaList();
    refreshAreas(false, QString());
}

