/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * InterfacePickerTest.cpp
 * Test fixture for the InterfacePicker class
 * Copyright (C) 2005 Simon Newton
 */

#include <cppunit/extensions/HelperMacros.h>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "common/network/FakeInterfacePicker.h"
#include "ola/network/InterfacePicker.h"
#include "ola/Logging.h"
#include "ola/testing/TestUtils.h"


using ola::network::FakeInterfacePicker;
using ola::network::IPV4Address;
using ola::network::Interface;
using ola::network::InterfacePicker;
using ola::network::MACAddress;
using std::auto_ptr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

class InterfacePickerTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(InterfacePickerTest);
  CPPUNIT_TEST(testGetInterfaces);
  CPPUNIT_TEST(testGetLoopbackInterfaces);
  CPPUNIT_TEST(testChooseInterface);
  CPPUNIT_TEST_SUITE_END();

 public:
    void testGetInterfaces();
    void testGetLoopbackInterfaces();
    void testChooseInterface();
};

CPPUNIT_TEST_SUITE_REGISTRATION(InterfacePickerTest);


/*
 * Check that we find at least one candidate interface.
 */
void InterfacePickerTest::testGetInterfaces() {
  auto_ptr<InterfacePicker> picker(InterfacePicker::NewPicker());
  vector<Interface> interfaces = picker->GetInterfaces(true);
#ifndef _WIN32
  // If a Windows box is not on a network, and doesn't have it's loopback, there
  // may be zero interfaces present so we skip this check
  OLA_ASSERT_GT(interfaces.size(), 0);
#else
  OLA_WARN << "Windows found " << interfaces.size() << " interfaces";
#endif  // _WIN32

  vector<Interface>::iterator iter;
  cout << endl;
  for (iter = interfaces.begin(); iter != interfaces.end(); ++iter) {
    cout << iter->ToString("\n ") << endl;
    cout << endl;
    cout << "---------------" << endl;
  }
}


/*
 * Check that we find a loopback interface.
 */
void InterfacePickerTest::testGetLoopbackInterfaces() {
  auto_ptr<InterfacePicker> picker(InterfacePicker::NewPicker());
  vector<Interface> interfaces = picker->GetInterfaces(true);
#ifndef _WIN32
  // If a Windows box is not on a network, and doesn't have it's loopback, there
  // may be zero interfaces present so we skip this check
  OLA_ASSERT_GT(interfaces.size(), 0);
#else
  OLA_WARN << "Windows found " << interfaces.size() << " interfaces";
#endif  // _WIN32

  vector<Interface>::iterator iter;
  unsigned int loopback_count = 0;
  for (iter = interfaces.begin(); iter != interfaces.end(); ++iter) {
    if (iter->loopback)
      loopback_count++;
  }
#ifndef _WIN32
  OLA_ASSERT_GT(loopback_count, 0);
#endif  // _WIN32
}


void InterfacePickerTest::testChooseInterface() {
  vector<Interface> interfaces;
  FakeInterfacePicker picker(interfaces);

  // Some test interfaces
  Interface iface1;
  iface1.name = "eth0";
  iface1.index = 1;
  OLA_ASSERT_TRUE(IPV4Address::FromString("10.0.0.1", &iface1.ip_address));

  Interface iface2;
  iface2.name = "eth1";
  iface2.index = 2;
  OLA_ASSERT_TRUE(IPV4Address::FromString("192.168.1.1", &iface2.ip_address));

  Interface iface2_alt;
  iface2_alt.name = "eth1";
  iface2_alt.index = 2;
  OLA_ASSERT_TRUE(IPV4Address::FromString("192.168.1.1",
                                          &iface2_alt.ip_address));

  // no interfaces
  Interface iface;
  OLA_ASSERT_FALSE(picker.ChooseInterface(&iface, ""));
  // no interfaces, with string default
  OLA_ASSERT_FALSE(picker.ChooseInterface(&iface, "", ""));
  OLA_ASSERT_FALSE(picker.ChooseInterface(&iface, "", "foo"));
  OLA_ASSERT_FALSE(picker.ChooseInterface(&iface, "foo", ""));
  OLA_ASSERT_FALSE(picker.ChooseInterface(&iface, "foo", "bar"));
  // no interfaces, with Interface default
  Interface blank_iface;
  OLA_ASSERT_FALSE(picker.ChooseInterface(&iface, "", blank_iface));
  OLA_ASSERT_FALSE(picker.ChooseInterface(&iface, "", iface1));
  OLA_ASSERT_FALSE(picker.ChooseInterface(&iface, "foo", blank_iface));
  OLA_ASSERT_FALSE(picker.ChooseInterface(&iface, "foo", iface2));
  // no interfaces, by index
  OLA_ASSERT_FALSE(picker.ChooseInterface(&iface, 0));

  // now with one iface that doesn't match
  interfaces.push_back(iface1);

  FakeInterfacePicker picker2(interfaces);
  OLA_ASSERT_TRUE(picker2.ChooseInterface(&iface, "192.168.1.1"));
  OLA_ASSERT_EQ(iface1, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker2.ChooseInterface(&iface, "192.168.1.1", "172.16.0.1"));
  OLA_ASSERT_EQ(iface1, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker2.ChooseInterface(&iface, "192.168.1.1", ""));
  OLA_ASSERT_EQ(iface1, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker2.ChooseInterface(&iface, "", "172.16.0.1"));
  OLA_ASSERT_EQ(iface1, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker2.ChooseInterface(&iface, "192.168.1.1", iface2));
  OLA_ASSERT_EQ(iface1, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker2.ChooseInterface(&iface, "192.168.1.1", blank_iface));
  OLA_ASSERT_EQ(iface1, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker2.ChooseInterface(&iface, "", iface1));
  OLA_ASSERT_EQ(iface1, iface);

  // check that preferred works
  interfaces.push_back(iface2);

  FakeInterfacePicker picker3(interfaces);
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "192.168.1.1"));
  OLA_ASSERT_EQ(iface2, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "192.168.1.1", ""));
  OLA_ASSERT_EQ(iface2, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "", "192.168.1.1"));
  OLA_ASSERT_EQ(iface2, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "192.168.1.1", "foo"));
  OLA_ASSERT_EQ(iface2, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "192.168.1.1", blank_iface));
  OLA_ASSERT_EQ(iface2, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "", iface2_alt));
  OLA_ASSERT_EQ(iface2, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "192.168.1.1", iface1));
  OLA_ASSERT_EQ(iface2, iface);

  // an invalid address should fallback to default
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "foo", "192.168.1.1"));
  OLA_ASSERT_EQ(iface2, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "foo", iface2_alt));
  OLA_ASSERT_EQ(iface2, iface);

  // a valid address should ignore the default
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "192.168.1.1", "10.0.0.1"));
  OLA_ASSERT_EQ(iface2, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "192.168.1.1", iface1));
  OLA_ASSERT_EQ(iface2, iface);

  // now check for iface name
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "eth0"));
  OLA_ASSERT_EQ(iface1, iface);

  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "eth1"));
  OLA_ASSERT_EQ(iface2, iface);

  // an invalid or empty address should fallback to default
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "", "eth1"));
  OLA_ASSERT_EQ(iface2, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "foo", "eth1"));
  OLA_ASSERT_EQ(iface2, iface);

  // a valid address should ignore the default
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "eth1", "eth0"));
  OLA_ASSERT_EQ(iface2, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "eth1", iface1));
  OLA_ASSERT_EQ(iface2, iface);

  // a invalid address should return the first one
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "foo"));
  OLA_ASSERT_EQ(iface1, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "foo", "bar"));
  OLA_ASSERT_EQ(iface1, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "foo", ""));
  OLA_ASSERT_EQ(iface1, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "", "bar"));
  OLA_ASSERT_EQ(iface1, iface);
  iface = Interface();
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, "foo", blank_iface));
  OLA_ASSERT_EQ(iface1, iface);

  // now check by iface index
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, 2));
  OLA_ASSERT_EQ(iface2, iface);

  // an invalid index should return the first one
  OLA_ASSERT_TRUE(picker3.ChooseInterface(&iface, 3));
  OLA_ASSERT_EQ(iface1, iface);
}
