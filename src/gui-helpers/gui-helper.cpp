/*
 * uC Interface - A GUI for Programming & Interfacing with Microcontrollers
 * Copyright (C) 2018  Mitchell Oleson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "gui-helper.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QSettings>
#include <QTemporaryFile>

const float GUI_HELPER::S2MS = 1000.0f;

GUI_HELPER::GUI_HELPER(QObject *parent) :
    QObject(parent)
{
}

GUI_HELPER::~GUI_HELPER()
{
}

bool GUI_HELPER::showMessage(QString msg)
{
    // Create a new message box & set text
    QMessageBox *n = new QMessageBox();
    if (n)
    {
        // Set messagebox attributes
        n->setText(msg);
        n->setModal(true);

        // Set delete on close attribute
        n->setAttribute(Qt::WA_DeleteOnClose);

        // Show message box
        n->show();
    }

    // Return true (for error cases)
    return true;
}

bool GUI_HELPER::getUserString(QString *str, QString title, QString label)
{
    bool ok;
    *str = QInputDialog::getText(nullptr, title, label,
                                 QLineEdit::Normal, *str, &ok);
    return (ok && !str->isEmpty());
}

bool GUI_HELPER::getOpenFilePath(QString *filePath, QString fileTypes)
{
    *filePath = QFileDialog::getOpenFileName(nullptr, tr("Open"),
                                             "", fileTypes);

    return !filePath->isEmpty();
}

bool GUI_HELPER::getSaveFilePath(QString *filePath, QString fileTypes)
{
    *filePath = QFileDialog::getSaveFileName(nullptr, tr("Save Location"),
                                             "", fileTypes);

    return !filePath->isEmpty();
}

bool GUI_HELPER::saveFile(QString filePath, QByteArray data)
{
    // Check to make sure path valid
    if (filePath.isEmpty())
        return false;

    // Open file
    QFile sFile(filePath);
    if (!sFile.open(QIODevice::WriteOnly))
        return false;

    // Write data to file
    qint64 res = sFile.write(data);
    sFile.close();

    // Check result
    return (0 <= res);
}

uint32_t GUI_HELPER::getFileSize(QString filePath)
{
    QFileInfo file(filePath);
    return file.size();
}

QByteArray GUI_HELPER::loadFile(QString filePath)
{
    // Check to make sure path valid
    if (filePath.isEmpty())
        return QByteArray();

    // Open file
    QFile sFile(filePath);
    if (!sFile.open(QIODevice::ReadOnly))
        return QByteArray();

    // Read file
    QByteArray data = sFile.readAll();

    // Close file
    sFile.close();

    // Return read data
    return data;
}

QMap<QString, QMap<QString, QVariant>*> *GUI_HELPER::readConfigINI(QString config)
{
    // Reset & load the GUI settings file
    QSettings config_settings(config, QSettings::IniFormat);

    QMap<QString, QVariant> *groupMap;
    QMap<QString, QMap<QString, QVariant>*> *configMap;
    configMap = new QMap<QString, QMap<QString, QVariant>*>();
    if (!configMap) return nullptr;

    // Loop through all child groups
    foreach (QString childGroup, config_settings.childGroups())
    {
        // Create new group map
        groupMap = new QMap<QString, QVariant>();
        if (!groupMap) continue;

        // Begin GUI group settings
        config_settings.beginGroup(childGroup);
        foreach (QString childKey, config_settings.childKeys())
        {
            groupMap->insert(childKey, config_settings.value(childKey));
        }

        // Exit GUI group settings
        config_settings.endGroup();

        // Add groupMap to configMap
        configMap->insert(childGroup, groupMap);
    }

    // Handle no settings in ini (return nullptr)
    if (configMap->isEmpty())
    {
        // Show error message
        GUI_HELPER::showMessage(QString("Error: Failed to load INI file!\n") + config);

        // Delete empty map
        delete configMap;

        // return a null pointer
        return nullptr;
    }

    return configMap;
}

QString GUI_HELPER::encode_configMap(QMap<QString, QMap<QString, QVariant>*> *configMap)
{
    // Verify configMap
    if (!configMap) return "";

    // Create qstring to hold config
    QString configMap_str;
    QVariant arg_str;

    // Write all values in the configMap
    QMap<QString, QVariant>* groupMap;
    foreach (QString group, configMap->keys())
    {
        // Get groupMap, skip if nullptr
        groupMap = configMap->value(group);
        if (!groupMap) continue;

        // Add group map to string conversion
        configMap_str += "[" + group + "]\n";

        // Write key and value for each entry in groupMap
        foreach (QString groupKey, groupMap->keys())
        {
            // Add setting key name and start assignment to string
            configMap_str += groupKey + "=\"";

            // Get the current value
            arg_str = groupMap->value(groupKey);

            // Parse value
            if (arg_str.canConvert(QMetaType::QString))
            {
                // Add value to assignment
                configMap_str += arg_str.toString();
            } else if (arg_str.canConvert(QMetaType::QStringList))
            {
                // Join the list with "," and add to assignment
                configMap_str += arg_str.toStringList().join("\",\"");
            }

            // Add end assignment to string
            configMap_str += "\"\n";
        }

        // Add ending newline
        configMap_str += "\n";
    }

    // Return the config data
    return configMap_str;
}

QMap<QString, QMap<QString, QVariant>*> *GUI_HELPER::decode_configMap(QString configMap)
{
    // Create temporary file & set autoremove
    QTemporaryFile tmpINI;
    tmpINI.setAutoRemove(true);

    // Open temporary file for writing data
    if (!tmpINI.open())
    {
        // Show error message
        GUI_HELPER::showMessage("Error: Unable to open temp file!");

        // Return out of function
        return nullptr;
    }

    // Write data to temporary file & close
    tmpINI.write(configMap.toLatin1());
    QString tmpName = tmpINI.fileName();
    tmpINI.close();

    // Read in file, create config, and return pointer
    return GUI_HELPER::readConfigINI(tmpName);
}

void GUI_HELPER::deleteConfigMap(QMap<QString, QMap<QString, QVariant>*> **configMap)
{
    // Verify valid pointer
    if (!configMap) return;

    // Get direct pointer (instead of pointer to a pointer
    QMap<QString, QMap<QString, QVariant>*> *configMap_ptr = *configMap;
    if (!configMap_ptr) return;

    // Loop through elements deleting
    QMap<QString, QVariant> *groupMap;
    foreach (QString group, configMap_ptr->keys())
    {
        groupMap = configMap_ptr->value(group);
        configMap_ptr->remove(group);
        if (groupMap) delete groupMap;
    }

    // Delete main config map
    delete *configMap;

    // Assign pointer to null
    *configMap = nullptr;
}

QByteArray GUI_HELPER::initList_to_byteArray(std::initializer_list<uint8_t> initList)
{
    QByteArray init_array;
    foreach (char i, initList)
    {
        init_array.append(i);
    }

    return init_array;
}

uint32_t GUI_HELPER::byteArray_to_uint32(QByteArray data)
{
    uint32_t ret_data = 0;
    uint8_t data_len = data.length();
    for (uint8_t i = 0; ((i < 4) && (i < data_len)); i++)
    {
        ret_data = (ret_data << 8) | (uchar) data.at(i);
    }
    return ret_data;
}

QByteArray GUI_HELPER::uint32_to_byteArray(uint32_t data)
{
    QByteArray ret_data;
    for (uint8_t i = 0; i < 4; i++)
    {
        ret_data.prepend((char) (data & 0xFF));
        data = data >> 8;
    }
    return ret_data;
}

QByteArray GUI_HELPER::encode_byteArray(QByteArray data, uint8_t base, char sep)
{
    // If base is 0, return data as is
    if (base == 0) return data;

    // Otherwise, build return string from data
    QByteArray ret_data;
    foreach (uint8_t elem, data)
    {
        // Append new element + sep
        ret_data += QString::number(elem, base) + sep;
    }
    return ret_data;
}

QByteArray GUI_HELPER::decode_byteArray(QByteArray data, uint8_t base, char sep)
{
    // If base is 0, return data as is
    if (base == 0) return data;

    // Otherwise, build return array from data
    // Splits array using sep char and trys to convert each string
    // into a number
    QByteArray ret_data;
    foreach (QString elem, data.split(sep))
    {
        ret_data.append((char) elem.toInt(nullptr, base));
    }
    return ret_data;
}
