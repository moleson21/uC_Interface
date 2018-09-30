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

#ifndef GUI_PIN_BASE_H
#define GUI_PIN_BASE_H

#include <QGridLayout>
#include <QLayoutItem>
#include <QMap>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QLineEdit>
#include <QFile>
#include <QTimer>

#include "gui-base.h"

typedef enum {
    PIN_TYPES_DIO = 0,
    PIN_TYPES_AIO,
    PIN_TYPES_REMOTE
} PIN_TYPES;

struct RangeList {
    int min;
    int max;
    int step;
    float div;
};
#define EMPTY_RANGE RangeList{.min=0, .max=0, .step=0, .div=1.0}

struct PinTypeInfo {
    QGridLayout *grid;
    uint8_t pinType;
    uint8_t cols;
    uint8_t rows;
    uint8_t numButtons;
    uint8_t numPins_GUI;
    uint8_t numPins_DEV;
    uint8_t numPins_START;
};
#define EMPTY_PIN_TYPE_INFO PinTypeInfo{.grid=nullptr, .pinType=0, \
    .cols=0, .rows=0, .numButtons=0, \
    .numPins_GUI=0, .numPins_DEV=0, .numPins_START=0}


class GUI_PIN_BASE : public GUI_BASE
{
    Q_OBJECT

public:
    GUI_PIN_BASE(QWidget *parent = 0);
    ~GUI_PIN_BASE();

    void addNewPinSettings(uint8_t pinType, QList<QString> newSettings);

protected:
    QMap<uint8_t, QMap<QString, uint8_t>*> controlMap;
    QMap<uint8_t, QList<uint8_t>*> disabledValueSet;
    QMap<uint8_t, QMap<uint8_t, RangeList*>*> rangeMap;

    QTimer DIO_READ;
    QTimer AIO_READ;

    QFile *logFile;
    QTextStream *logStream;
    QTimer logTimer;
    bool logIsRecording;

    bool devConnected;
    QMap<QString, QStringList> devSettings;

    QByteArray currData;
    uint8_t bytesPerPin;

    uint8_t num_AIOpins_GUI;
    uint8_t num_AIOpins_DEV;
    uint8_t num_AIOrows;
    uint8_t num_AIOcols;
    uint8_t num_AIObuttons;
    uint8_t num_AIOpins_START;

    uint8_t num_DIOpins_GUI;
    uint8_t num_DIOpins_DEV;
    uint8_t num_DIOrows;
    uint8_t num_DIOcols;
    uint8_t num_DIObuttons;
    uint8_t num_DIOpins_START;

    uint8_t labelPos;
    uint8_t comboPos;
    uint8_t slideValuePos;
    uint8_t textValuePos;

    void inputsChanged(PinTypeInfo *pInfo, uint8_t colOffset);
    void updateSliderRange(QSlider *slider, RangeList *rList);

    void setPinAttribute(PinTypeInfo *pInfo, uint8_t pinNum, Qt::WidgetAttribute attribute, bool on);
    void setPinNumbers(PinTypeInfo *pInfo, uint8_t start_num);

    bool getItemWidget(QWidget** itemWidget, QGridLayout *grid, uint8_t row, uint8_t col);
    void getPinLocation(uint8_t *row, uint8_t* col, PinTypeInfo *pInfo, uint8_t pin);
    bool getPinTypeInfo(uint8_t pinType, PinTypeInfo *infoPtr);

private:
    RangeList* makeRangeList(QString rangeInfo);
    void addPinControls(uint8_t pinType, QList<QString> keys);
    void addPinRangeMap(uint8_t pinType, QList<QString> keys, QList<RangeList*> values);
};

#endif // GUI_PIN_BASE_H
