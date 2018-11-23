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

#include "gui-base-tests.hpp"

// Testing infrastructure includes
#include <QtTest>
#include <QCoreApplication>

// Object includes
#include <QTimer>
#include <QEventLoop>

#include "../../src/gui-helpers/gui-generic-helper.hpp"

GUI_BASE_TESTS::GUI_BASE_TESTS()
{
    /* DO NOTHING */
}

GUI_BASE_TESTS::~GUI_BASE_TESTS()
{
    // Delete base test if allocated
    if (base_tester) delete base_tester;
}

void GUI_BASE_TESTS::initTestCase()
{
    // Create object for testing
    base_tester = new GUI_BASE_TEST_CLASS();
    QVERIFY(base_tester);
}

void GUI_BASE_TESTS::cleanupTestCase()
{
    // Delete test class
    if (base_tester)
    {
        delete base_tester;
        base_tester = nullptr;
    }
}

void GUI_BASE_TESTS::test_init_vals()
{
    // Verify all init values
    QVERIFY(base_tester->isClosable());
    QCOMPARE(base_tester->get_gui_key(), (uint8_t) MAJOR_KEY_ERROR);
    QCOMPARE(base_tester->get_gui_tab_name(), QString("GUI Base"));
}

void GUI_BASE_TESTS::test_basic_features()
{
    // Test toggle closable settings
    QVERIFY(base_tester->isClosable());
    base_tester->setClosable(false);
    QVERIFY(!base_tester->isClosable());
    base_tester->setClosable(true);
    QVERIFY(base_tester->isClosable());

    // Test accept all cmds (should return false)
    QVERIFY(!base_tester->acceptAllCMDs());

    // Test waitForDevice (should always return false)
    QVERIFY(!base_tester->waitForDevice(0));
    QVERIFY(!base_tester->waitForDevice((uint8_t) qrand()));
}

void GUI_BASE_TESTS::test_gui_key()
{
    // Fetch keys
    QFETCH(quint8, set_key);

    // Set and check result
    base_tester->set_gui_key_test(set_key);
    QCOMPARE(base_tester->get_gui_key(), set_key);
}

void GUI_BASE_TESTS::test_gui_key_data()
{
    QTest::addColumn<quint8>("set_key");

    QTest::newRow("Major Error Key") << (quint8) MAJOR_KEY_ERROR;
    QTest::newRow("General Settings Key") << (quint8) MAJOR_KEY_GENERAL_SETTINGS;
    QTest::newRow("IO Key") << (quint8) MAJOR_KEY_IO;
    QTest::newRow("Custom CMD Key") << (quint8) MAJOR_KEY_CUSTOM_CMD;
    QTest::newRow("RESET") << (quint8) base_tester->get_gui_key();
}

void GUI_BASE_TESTS::test_set_gui_name()
{
    // Fetch name
    QFETCH(QString, set_name);

    // Set and check result
    base_tester->set_gui_name_test(set_name);
    QCOMPARE(base_tester->get_gui_name(), set_name);
}

void GUI_BASE_TESTS::test_set_gui_name_data()
{
    QTest::addColumn<QString>("set_name");

    QTest::newRow("Blank") << "";
    QTest::newRow("Short") << "IO";
    QTest::newRow("Shimple") << "Welcome";
    QTest::newRow("Space") << "Data Transmit";
    QTest::newRow("Random Caps") << "RandOM GLESN";
    QTest::newRow("Random Chars") << "'-1923'/,.l][qw2487923-1!(*@!*$@!_";
    QTest::newRow("RESET") << base_tester->get_gui_name();
}

void GUI_BASE_TESTS::test_set_gui_tab_name()
{
    // Fetch name
    QFETCH(QString, set_name);

    // Set and check result
    base_tester->set_gui_tab_name(set_name);
    QCOMPARE(base_tester->get_gui_tab_name(), set_name);
}

void GUI_BASE_TESTS::test_set_gui_tab_name_data()
{
    QTest::addColumn<QString>("set_name");

    QTest::newRow("Blank") << "";
    QTest::newRow("Simple") << "New Tab";
    QTest::newRow("Random Caps") << "RandOM GLESN";
    QTest::newRow("Random Chars") << "'-1923'/,.l][qw2487923-1!(*@!*$@!_";
    QTest::newRow("RESET") << base_tester->get_gui_tab_name();
}

void GUI_BASE_TESTS::test_gui_config_1()
{
    // Get current config string
    QString config_str_before = base_tester->get_gui_config();

    // Check handling of null config maps (no changes)
    base_tester->parseConfigMap(nullptr);
    QCOMPARE(base_tester->get_gui_config(), config_str_before);

    // Setup empty variables
    CONFIG_MAP empty_gui_config;
    QMap<QString, QVariant> empty_gui_map;
    empty_gui_config.insert(base_tester->get_gui_name(), &empty_gui_map);
    QString expected_config_str = GUI_GENERIC_HELPER::encode_configMap(&empty_gui_config);

    // Check handling of empty config maps (clears all)
    base_tester->parseConfigMap(&empty_gui_map);
    QCOMPARE(base_tester->get_gui_config(), expected_config_str);
}

void GUI_BASE_TESTS::test_gui_config_2()
{
    // Build config string
    // String is set to how GUI_GENERIC_HELPER::encode_configMap() would
    // generate it from a map object as get_gui_config() calls this
    QString config_str = "[";
    config_str += base_tester->get_gui_name() + "]\n";
    config_str += "tab_name=\"\"\n\n";

    // Create config map
    QMap<QString, QMap<QString, QVariant>*> *config_map = \
            GUI_GENERIC_HELPER::decode_configMap(config_str);

    // Set config map
    base_tester->parseConfigMap(config_map->value(base_tester->get_gui_name()));

    // Delete config map
    GUI_GENERIC_HELPER::delete_configMap(&config_map);

    // Verify changes
    QCOMPARE(base_tester->get_gui_config(), config_str);
    QCOMPARE(base_tester->get_gui_tab_name(), QString(""));
}

void GUI_BASE_TESTS::test_recv_length()
{
    // Fetch data
    QFETCH(quint32, expected_recv_len);
    QFETCH(quint32, recv_len);
    QFETCH(QString, recv_len_str);

    // Set start info
    base_tester->set_expected_recv_length_test(expected_recv_len);
    base_tester->update_current_recv_length_test(recv_len);

    // Verify the updates
    QCOMPARE(base_tester->get_expected_recv_length_test(), expected_recv_len);
    QCOMPARE(base_tester->get_current_recv_length_test(), recv_len);
    QCOMPARE(base_tester->get_expected_recv_length_str_test(), recv_len_str);
}

void GUI_BASE_TESTS::test_recv_length_data()
{
    QTest::addColumn<quint32>("expected_recv_len");
    QTest::addColumn<quint32>("recv_len");
    QTest::addColumn<QString>("recv_len_str");

    QTest::newRow("Zero") << (quint32) 0 << (quint32) 0 << "";
    QTest::newRow("One") << (quint32) 1 << (quint32) 1 << "/0.001KB";
    QTest::newRow("500") << (quint32) 500 << (quint32) 5 << "/0.5KB";
    QTest::newRow("1000") << (quint32) 1000 << (quint32) 100 << "/1KB";
    QTest::newRow("RESET") << base_tester->get_expected_recv_length_test() \
                           << base_tester->get_current_recv_length_test() \
                           << base_tester->get_expected_recv_length_str_test();
}

void GUI_BASE_TESTS::test_reset_gui_1()
{
    // Set some info
    base_tester->set_expected_recv_length_test(500);
    base_tester->update_current_recv_length_test(100);

    // Verify the updates
    QCOMPARE(base_tester->get_expected_recv_length_test(), (uint32_t) 500);
    QCOMPARE(base_tester->get_current_recv_length_test(), (uint32_t) 100);
    QCOMPARE(base_tester->get_expected_recv_length_str_test(), QString("/0.5KB"));

    // Reset the gui
    base_tester->reset_gui();

    // Verify the reset
    QCOMPARE(base_tester->get_expected_recv_length_test(), (uint32_t) 0);
    QCOMPARE(base_tester->get_current_recv_length_test(), (uint32_t) 0);
    QCOMPARE(base_tester->get_expected_recv_length_str_test(), QString(""));
}
