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

#include "gui-base-test-class.hpp"

// Object includes
#include <QString>

GUI_BASE_TEST_CLASS::GUI_BASE_TEST_CLASS(QWidget *parent) :
    GUI_BASE(parent)
{
    /* DO NOTHING */
}

GUI_BASE_TEST_CLASS::~GUI_BASE_TEST_CLASS()
{
    /* DO NOTHING */
}

void GUI_BASE_TEST_CLASS::set_gui_key(uint8_t new_key)
{
    // Set new gui key
    gui_key = new_key;
}
