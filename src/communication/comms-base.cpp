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

#include "comms-base.h"

COMMS_BASE::COMMS_BASE(QObject *parent) :
    QObject(parent)
{
    readLock = new QMutex(QMutex::Recursive);
    writeLock = new QMutex(QMutex::Recursive);
}

COMMS_BASE::~COMMS_BASE()
{
    delete readLock;
    delete writeLock;
}

void COMMS_BASE::open()
{
    // Default never allow connection
    connected = false;
    emit deviceDisconnected();
}

bool COMMS_BASE::isConnected()
{
    return connected;
}

void COMMS_BASE::close()
{
    connected = false;
}

void COMMS_BASE::write(QByteArray writeData)
{
    writeLock->lock();

    qDebug() << "DS: " << writeData;

    writeLock->unlock();
}

void COMMS_BASE::read()
{
    readLock->lock();

    QByteArray recvData;
    emit readyRead(recvData);
    qDebug() << "DR: " << recvData;

    readLock->unlock();
}