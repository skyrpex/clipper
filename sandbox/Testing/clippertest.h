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

#ifndef clippertest_h
#define clippertest_h

#include "clipper.hpp"
#include "svgbuilder.h"
#include "test.h"
#include <cstdlib>

namespace ClipperTestLib
{

  using namespace ClipperLib;
  using namespace SvgBuilder;
  using namespace TestLib;

  enum EnumSVG {svgNever, svgFail, svgAlways};

  class ClipperTest;

  class ClipperTestSuite: public virtual TestSuite
  {
  public:
    EnumSVG ShowSVG;
    void DoSVG(ClipperTest* t);
  protected:
    bool Run(Test* t);
  };

  class ClipperTest: public virtual Test
  {
  public:
    Paths subj, subj2, clip, solution;
    PolyTree polytree;
    PolyFillType pft;
  };

  //////////////////////////////////////////////////////////////
  void LoadTests();
  void AddTest(ClipperTest* t, const std::string& title);

  void TestAll(EnumSVG svg = svgFail);
  void TestOne(const string& title, EnumSVG svg);
  //////////////////////////////////////////////////////////////

}


#endif clippertest_h