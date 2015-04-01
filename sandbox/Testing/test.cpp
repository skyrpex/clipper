/*******************************************************************************
*                                                                              *
* Author    :  Angus Johnson                                                   *
* Version   :  0.9.1                                                           *
* Date      :  11 February 2014                                                *
* Website   :  http://www.angusj.com                                           *
* Copyright :  Angus Johnson 2010-2014                                         *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include "test.h"

namespace TestLib
{

  TestSuite::~TestSuite()
  {
    for (size_t i = 0; i < m_list.size(); ++i)
      delete m_list[i];
  }

  void TestSuite::AddTest(Test* t, const std::string& title)
  {
    t->Title(title);
    m_list.push_back(t);
  };

  void TestSuite::RunAll()
  {
    int fails = 0;
    for (size_t i = 0; i < m_list.size(); ++i)
    {
      bool result = Run(m_list[i]);
      if (result) 
        std::cout << "  Passed " << m_list[i]->Title() << std::endl;
      else
      {
        std::cout << "* Failed " << m_list[i]->Title() << std::endl;
        fails++;
      }
    }
    if (fails == 0) std::cout << std::endl << "All tests passed!" << std::endl;
    else if (fails == 1) std::cout << std::endl << "1 test failed!" << std::endl;
    else std::cout << std::endl << fails << " tests failed!" << std::endl;
  }

  void TestSuite::RunOne(const std::string& title)
  {
    for (size_t i = 0; i < m_list.size(); ++i)
      if (m_list[i]->Title() == title)
      {
        bool result = Run(m_list[i]);
        std::cout << "  Test on " << title << 
          (result ? " passed." : " failed.") << std::endl << std::endl;
        return;
      }
    std::cout << "  Test " << title << 
      " could not be found!" << std::endl << std::endl;
  }

  bool TestSuite::Run(Test* t)
  {
    try { return t->DoTest(); }
    catch(...){ return false; }
  }

}