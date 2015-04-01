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

#ifndef test_h
#define test_h

#include <cstdlib>
#include <cstdio>
#include <vector>

namespace TestLib
{

  class Test;

  class TestSuite
  {
  public:
    virtual ~TestSuite();
    void AddTest(Test* t, const std::string& title);
    void RunAll();
    void RunOne(const std::string& title);
  protected:
    virtual bool Run(Test* t);
  private:
    std::vector<Test*> m_list;
  };

  class Test
  {
  public:
    virtual ~Test() {}
    std::string Title() const {return m_title;}
    void Title(std::string title){m_title = title;}
    virtual bool DoTest(){return false;}
  private:
    std::string m_title;
  };
}
#endif test_h