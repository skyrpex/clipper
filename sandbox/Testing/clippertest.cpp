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

#include <vector>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include "clipper.hpp"
#include "svgbuilder.h"
#include "clippertest.h"

namespace ClipperTestLib
{
  using namespace ClipperLib;

    ClipperTestSuite cts;

    bool ClipperTestSuite::Run(Test* t)
    {
      bool result = TestSuite::Run(t);
      if (ShowSVG == svgAlways || (!result && ShowSVG == svgFail)) 
        DoSVG(dynamic_cast<ClipperTest*>(t));
      return result;
    }

    void ClipperTestSuite::DoSVG(ClipperTest* t)
    {
      SvgBuilder::SvgBase svg;
      //svg.style.showCoords =true;
      svg.style.penWidth = 0.8;
      svg.style.pft = t->pft;
      svg.SetFont("Arial", 7, 0xFF000000);
      if (!t->subj.empty())
        svg.AddPath(t->subj, 0x1800009C, 0xFFB3B3DA, true);
      if (!t->subj2.empty())
        svg.AddPath(t->subj2, 0x1800009C, 0xFFB3B3DA, false);
      if (!t->clip.empty())
        svg.AddPath(t->clip, 0x209C0000, 0xFFFFA07A, true);  
      if (t->solution.empty())
      {
        if (t->polytree.ChildCount() > 0)
        {
          OpenPathsFromPolyTree(t->polytree, t->solution);
          if (!t->solution.empty())
            svg.AddPath(t->solution, 0, 0xFF000000, false);
          ClosedPathsFromPolyTree(t->polytree, t->solution);
          if (!t->solution.empty())
            svg.AddPath(t->solution, 0x20009C00, 0xFF000000, false);
        }
      }
      else  
      {
        svg.AddPath(t->solution, 0x6080ff9C, 0xFF003300, true);
      
        Paths holes;
        for (size_t i = 0; i < t->solution.size(); ++i)
          if (!Orientation(t->solution[i]))
            holes.push_back(t->solution[i]);
        svg.AddPath(holes, 0x0, 0xFFFF0000, true);
      }
      std::string svgFilename = t->Title() + ".svg";
      svg.SaveToFile(svgFilename, 600, 400, 50);
      system(svgFilename.c_str());  
    }

    void AddTest(ClipperTest* t, const std::string& title) {cts.AddTest(t, title);}
    void TestAll(EnumSVG svg) 
    {
      std::cout << std::endl << "Testing Clipper ..." << std::endl << std::endl;
      cts.ShowSVG = svg; 
      cts.RunAll();
    }
    void TestOne(const string& title, EnumSVG svg) 
    {
      std::cout << std::endl << "Testing Clipper ..." << std::endl << std::endl;
      cts.ShowSVG = svg; 
      cts.RunOne(title);
    }

  //------------------------------------------------------------------------------
  //------------------------------------------------------------------------------

  void MakePolygonFromInts(cInt *ints, int size, Path &p, double scale = 1.0)
  {
    p.clear();
    p.reserve(size / 2);
    for (int i = 0; i < size; i +=2)
      p.push_back(IntPoint((cInt)(ints[i] * scale), (cInt)(ints[i+1] * scale)));
  }
  //---------------------------------------------------------------------------

  void MakeSquarePolygons(int size, int totalWidth, int totalHeight, Paths &p)
  {
    int cols = totalWidth / size;
    int rows = totalHeight / size;
    p.resize(cols * rows);
    for (int i = 0; i < rows; ++i) 
      for (int j = 0; j < cols; ++j) 
      {
        cInt ints[] = {j * size, i * size, (j+1) * size, i * size,
          (j+1) * size, (i+1) * size, j * size, (i+1) * size};
        MakePolygonFromInts(ints, sizeof(ints)/sizeof(cInt), p[j * rows + i] );
      }
  }
  //------------------------------------------------------------------------------

  void MakeDiamondPolygons(int size, int totalWidth, int totalHeight, Paths &p)
  {
    int halfSize = size / 2;
    size = halfSize * 2;
    int cols = totalWidth / size;
    int rows = totalHeight * 2 / size;
    p.resize(cols * rows);
    int dx = 0;
    for (int i = 0; i < rows; ++i) 
    {
      if (dx == 0) dx = halfSize; else dx = 0;
      for (int j = 0; j < cols; ++j) 
      {
        cInt ints[] = {dx + j * size,    i * halfSize + halfSize,
            dx + j * size + halfSize,   i * halfSize,
            dx + (j+1) * size,          i * halfSize + halfSize,
            dx + j * size + halfSize,   i * halfSize + halfSize *2};
        MakePolygonFromInts(ints, sizeof(ints)/sizeof(cInt), p[j * rows + i] );
      }
    }
  }
  //------------------------------------------------------------------------------

  //------------------------------------------------------------------------------

  class Difference1: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;
      cInt ints1[] = {29, 342, 115, 68, 141, 86};
      cInt ints2[] = {128, 160, 99, 132, 97, 174};
      cInt ints3[] = {99, 212, 128, 160, 97, 174, 58, 160};
      cInt ints4[] = {97, 174, 99, 132, 60, 124, 58, 160};

      subj.resize(1);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      clip.resize(3);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), clip[1]);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[2]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && solution.size() == 2 &&
        Orientation(solution[0]) && Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Difference2: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;
      
      cInt ints1[] = {-103,-219, -103,-136, -115,-136};
      cInt ints2[] = {-110,-174, -70,-174, -110,-155};

      subj.resize(1);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && solution.size() == 1;
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Horz1: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;
      
      cInt ints1[] = {380,280, 450,280, 130,400, 490,430, 320,200, 450,260};
      cInt ints2[] = {350,240, 520,470, 100,300};

      subj.resize(1);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctIntersection, solution, pft, pft);
      
      res = res && solution.size() <= 2;
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Horz2: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;
      
      cInt ints1[] = {120,400, 350,380, 340,140};
      cInt ints2[] = {350,370, 150,370, 560,20, 350,390, 340,150, 570,230, 390,40};

      subj.resize(1);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctIntersection, solution, pft, pft);
      
      res = res && solution.size() == 2;
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Horz3: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;
      
      cInt ints1[] = {470,190, 100,520, 280,270, 380,270, 460,170};
      cInt ints2[] = {170,70, 500,350, 110,90};

      subj.resize(1);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctIntersection, solution, pft, pft);
      
      res = res && solution.size() == 1;
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Horz4: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      cInt ints1[] = {904, 901, 1801, 901, 1801, 1801, 902, 1803};
      cInt ints2[] = {2, 1800, 902, 1800, 902, 2704, 4, 2701};
      cInt ints3[] = {902, 1802, 902, 2704, 1804, 2703, 1801, 1804};

      subj.resize(3);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      MakePolygonFromInts(ints3, sizeof(ints2)/sizeof(cInt), subj[2]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      bool res = c.Execute(ctUnion, solution, pft, pft);
      
      res = res && solution.size() == 2 && 
        Orientation(solution[0]) && !Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Horz5: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {93, 92, 183, 93, 184, 184, 94, 183};
      cInt ints2[] = {184, 1, 270, 2, 272, 91, 183, 94};
      cInt ints3[] = {92, 2, 91, 91, 184, 91, 184, 0};
      cInt ints4[] = {183, 93, 184, 184, 271, 182, 274, 94};

      subj.resize(2);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      clip.resize(2);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), clip[0]);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[1]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && solution.size() == 2;
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Horz6: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {14,15,16,12,10,12};
      cInt ints2[] = {15,14,11,14,13,16,17,10,10,17,18,13};

      subj.resize(1);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctIntersection, solution, pft, pft);
      
      res = res && solution.size() == 1;
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Horz7: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {11,19,19,15,15,12,13,19,15,13,10,14,13,18,16,13};
      cInt ints2[] = {16,10,14,17,18,10,15,18,14,14,15,14,11,16};

      subj.resize(1);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctIntersection, solution, pft, pft);
      
      res = res && solution.size() == 2;
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Horz8: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {12,11,15,15,18,16,16,18,15,14,14,14,19,15};
      cInt ints2[] = {13,12,17,17,19,15};

      subj.resize(1);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      return c.Execute(ctIntersection, solution, pft, pft);
    } 
  };
  //---------------------------------------------------------------------------

  class Horz9: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;
      
      cInt ints1[] = {380,140, 430,120, 180,120, 430,120, 190,150};
      cInt ints2[] = {430,130, 210,70, 20,260};

      subj.resize(1);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      return c.Execute(ctIntersection, solution, pft, pft);
    } 
  };
  //---------------------------------------------------------------------------

  class Horz10: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;
      
      cInt ints1[] = {40,310, 410,110, 460,110, 260,200};
      cInt ints2[] = {120,260, 450,220, 330,220, 240,220, 50,380};

      subj.resize(1);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      return c.Execute(ctIntersection, solution, pft, pft);
    } 
  };
  //---------------------------------------------------------------------------

  class Orientation1: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;
      
      cInt ints1[] = {470, 130, 330, 10, 370, 10,
        290, 190, 290, 280, 190, 10, 70, 370, 10, 400, 310, 10, 490, 220,
        130, 10, 150, 400, 490, 150, 250, 60, 410, 320, 430, 410,
        470, 10, 10, 10, 250, 220, 10, 180, 250, 160, 490, 130, 190, 320,
        170, 240, 290, 280, 370, 240, 350, 90, 450, 190, 10, 370,
        110, 180, 290, 160, 190, 350, 490, 360, 190, 190, 370, 230,
        90, 220, 270, 10, 70, 190, 10, 270, 430, 100, 190, 140, 370, 80,
        10, 40, 250, 260, 430, 40, 130, 350, 190, 420, 10, 10, 130, 50,
        90, 400, 530, 50, 150, 90, 250, 150, 390, 310, 250, 180,
        310, 220, 350, 280, 30, 140, 430, 260, 130, 10, 430, 310,
        10, 60, 190, 60, 490, 320, 190, 360, 430, 130, 210, 220,
        270, 190, 10, 10, 510, 10, 150, 210, 90, 400, 110, 10, 130, 110,
        130, 80, 130, 30, 430, 190, 190, 380, 90, 300, 10, 340, 10, 70,
        250, 380, 310, 370, 370, 240, 190, 130, 490, 100, 470, 70,
        10, 420, 190, 20, 430, 290, 430, 10, 330, 70, 450, 140, 430, 40,
        150, 220, 170, 190, 10, 110, 470, 310, 510, 160, 10, 200};
      cInt ints2[] = {50, 420, 10, 180, 190, 160,
        50, 40, 490, 40, 450, 130, 450, 290, 290, 310, 430, 110,
        370, 250, 490, 220, 430, 230, 410, 220, 10, 200, 530, 130, 
        50, 350, 370, 290, 130, 130, 110, 390, 10, 350, 210, 340,
        370, 220, 530, 280, 370, 170, 190, 370, 330, 310, 510, 280, 
        90, 10, 50, 250, 170, 100, 110, 40, 310, 370, 430, 80, 390, 40, 
        250, 360, 350, 150, 130, 310, 10, 260, 390, 90, 370, 280,
        70, 100, 530, 190, 10, 250, 470, 340, 110, 180, 10, 10, 70, 380, 
        370, 60, 190, 290, 250, 70, 10, 150, 70, 120, 490, 340, 330, 40, 
        90, 10, 210, 40, 50, 10, 450, 370, 310, 390, 10, 10, 10, 270, 
        250, 180, 130, 120, 10, 150, 10, 220, 150, 280, 490, 10, 
        150, 370, 370, 220, 10, 310, 10, 330, 450, 150, 310, 80, 
        410, 40, 530, 290, 110, 240, 70, 140, 190, 410, 10, 250, 
        270, 230, 370, 380, 270, 280, 230, 220, 430, 110, 10, 290, 
        130, 250, 190, 40, 170, 320, 210, 220, 290, 40, 370, 380, 
        30, 380, 130, 50, 370, 340, 130, 190, 70, 250, 310, 270, 
        250, 290, 310, 280, 230, 150};

      subj.resize(1);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctIntersection, solution, pft, pft);
      
      if (res) for (size_t i = 0; i < solution.size(); ++i)
        if (!Orientation(solution[i])) return false;
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Orientation2: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {370, 150, 130, 400, 490, 290,
    490, 400, 170, 10, 130, 130, 270, 90, 430, 230, 310, 230,
    10, 80, 390, 110, 370, 20, 190, 210, 370, 410, 110, 100,
    410, 230, 370, 290, 350, 190, 350, 100, 230, 290};
      cInt ints2[] = {510, 400, 250, 100, 410, 410,
    170, 210, 390, 100, 10, 100, 10, 250, 10, 220, 130, 90, 410, 330,
    450, 160, 50, 180, 110, 100, 210, 320, 410, 220, 190, 30,
    370, 70, 270, 260, 450, 250, 90, 280};

      subj.resize(1);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctIntersection, solution, pft, pft);
      int cnt = 0;
      if (res) 
        for (size_t i = 0; i < solution.size(); ++i)
        {
          if (!Orientation(solution[i])) cnt++;
        }
      return res && cnt == 4;
    } 
  };
  //---------------------------------------------------------------------------

  class Orientation3: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;
      
      cInt ints1[] = {70, 290, 10, 410, 10, 220};
      cInt ints2[] = {430, 20, 10, 30, 10, 370, 250, 300,
    190, 10, 10, 370, 30, 220, 490, 100, 10, 370};

      subj.resize(1);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctIntersection, solution, pft, pft);
      
      if (res) 
        for (size_t i = 0; i < solution.size(); ++i)
          if (!Orientation(solution[i])) return false;
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Orientation4: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {40, 190, 400, 10, 510, 450,
    300, 50, 440, 230, 340, 290, 260, 510, 110, 50, 500, 90,
    450, 410, 550, 70, 70, 130, 410, 110, 130, 130, 470, 50,
    410, 10, 360, 50, 460, 90, 170, 270, 400, 210, 240, 370,
    50, 370, 350, 270, 530, 330, 170, 250, 440, 170, 40, 430,
    410, 90, 170, 510, 470, 130, 290, 390, 510, 410, 500, 230,
    490, 490, 430, 430, 10, 250, 240, 190, 80, 370, 60, 190,
    570, 490, 110, 270, 550, 290, 90, 10, 200, 10, 580, 450,
    500, 450, 370, 210, 10, 250, 60, 70, 220, 10, 530, 130, 190, 10,
    350, 170, 440, 330, 260, 50, 320, 10, 570, 10, 350, 170,
    130, 470, 350, 370, 40, 130, 540, 50, 10, 50, 320, 450, 270, 470,
    460, 10, 60, 110, 280, 170, 300, 410, 300, 370, 520, 170,
    460, 410, 180, 270, 270, 450, 50, 110, 490, 490, 10, 150,
    240, 490, 200, 190, 10, 10, 30, 370, 170, 410, 560, 290,
    140, 10, 350, 190, 290, 10, 460, 210, 70, 290, 300, 270,
    570, 450, 250, 330, 250, 290, 300, 410, 210, 330, 320, 390,
    160, 290, 70, 190, 40, 170, 490, 70, 70, 50};
      cInt ints2[] = {160, 510, 440, 90, 400, 510,
    220, 250, 480, 210, 80, 410, 530, 170, 10, 50, 220, 290,
    110, 490, 110, 10, 350, 130, 510, 330, 10, 410, 190, 30,
    90, 10, 380, 270, 50, 250, 510, 50, 580, 10, 50, 130, 540, 330,
    120, 250, 440, 250, 10, 430, 10, 410, 150, 190, 510, 490,
    400, 170, 200, 10, 170, 470, 300, 10, 130, 130, 190, 10,
    500, 350, 40, 10, 400, 230, 20, 370, 230, 510, 140, 10, 220, 490,
    90, 370, 490, 190, 520, 210, 180, 70, 440, 490, 510, 10,
    420, 210, 340, 410, 80, 10, 100, 190, 100, 250, 340, 390,
    360, 10, 170, 70, 300, 290, 110, 370, 160, 330, 210, 10,
    300, 10, 540, 410, 380, 490, 550, 290, 170, 450, 580, 390,
    360, 10, 450, 370, 520, 330, 100, 30, 160, 450, 160, 190,
    300, 90, 400, 270, 40, 170, 40, 90, 210, 330, 450, 50, 430, 370,
    290, 370, 150, 10, 340, 170, 10, 90, 180, 150, 530, 450,
    310, 490, 400, 450, 340, 10, 420, 210, 500, 70, 100, 10,
    400, 470, 40, 490, 550, 190, 30, 90, 100, 130, 70, 490, 20, 270,
    490, 410, 570, 370, 220, 90};

      subj.resize(1);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      int cnt = 0;
      if (res) 
        for (size_t i = 0; i < solution.size(); ++i)
        {
          if (!Orientation(solution[i])) cnt++;
        }
      return res && cnt == 2;
    } 
  };  
  //---------------------------------------------------------------------------
  
  class Orientation5: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {5237, 5237, 68632, 5164, 10315, 61247,
    10315, 20643, 16045, 29877, 24374, 11012, 10359, 19690, 10315, 20643,
    10315, 67660};

      subj.resize(1);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      bool res = c.Execute(ctUnion, solution, pft, pft);
      
      res = res && solution.size() == 2 && !Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Orientation6: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {0, 0, 100, 0, 101, 116, 0, 109};
      cInt ints2[] = {110, 112, 200, 106, 200, 200, 111, 200};
      cInt ints3[] = {0, 106, 101, 114, 107, 200, 0, 200};
      cInt ints4[] = {117, 0, 200, 0, 200, 110, 115, 102};

      subj.resize(2);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      clip.resize(2);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), clip[0]);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[1]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && solution.size() == 2 
        && Orientation(solution[1]) && Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Orientation7: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {0, 0, 100, 0, 104, 116, 0, 118};
      cInt ints2[] = {111, 115, 200, 103, 200, 200, 105, 200};
      cInt ints3[] = {0, 103, 112, 111, 105, 200, 0, 200};
      cInt ints4[] = {116, 0, 200, 0, 200, 113, 101, 110};

      subj.resize(2);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      clip.resize(2);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), clip[0]);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[1]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && solution.size() == 2 
        && Orientation(solution[1]) && Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Orientation8: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {0, 0, 112, 0, 111, 116, 0, 108};
      cInt ints2[] = {112, 114, 200, 108, 200, 200, 116, 200};
      cInt ints3[] = {0, 102, 118, 111, 117, 200, 0, 200};
      cInt ints4[] = {109, 0, 200, 0, 200, 117, 105, 110};

      subj.resize(2);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      clip.resize(2);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), clip[0]);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[1]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && solution.size() == 2 
        && Orientation(solution[1]) && Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Orientation9: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {0, 0, 114, 0, 113, 110, 0, 117};
      cInt ints2[] = {109, 114, 200, 106, 200, 200, 104, 200};
      cInt ints3[] = {0, 100, 118, 106, 103, 200, 0, 200};
      cInt ints4[] = {110, 0, 200, 0, 200, 116, 101, 105};

      subj.resize(2);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      clip.resize(2);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), clip[0]);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[1]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && solution.size() == 2 
        && Orientation(solution[1]) && Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Orientation10: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {0, 0, 102, 0, 103, 118, 0, 106};
      cInt ints2[] = {110, 115, 200, 108, 200, 200, 113, 200};
      cInt ints3[] = {0, 110, 103, 117, 109, 200, 0, 200};
      cInt ints4[] = {118, 0, 200, 0, 200, 108, 116, 101};

      subj.resize(2);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      clip.resize(2);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), clip[0]);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[1]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && solution.size() == 2 
        && Orientation(solution[1]) && Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Orientation11: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {0, 0, 100, 0, 107, 116, 0, 104};
      cInt ints2[] = {116, 100, 200, 115, 200, 200, 118, 200};
      cInt ints3[] = {0, 115, 107, 115, 115, 200, 0, 200};
      cInt ints4[] = {101, 0, 200, 0, 200, 100, 100, 100};

      subj.resize(2);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      clip.resize(2);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), clip[0]);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[1]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && solution.size() == 2 
        && Orientation(solution[1]) && Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Orientation12: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {0, 0, 119, 0, 113, 105, 0, 100};
      cInt ints2[] = {117, 103, 200, 105, 200, 200, 106, 200};
      cInt ints3[] = {0, 112, 116, 104, 108, 200, 0, 200};
      cInt ints4[] = {101, 0, 200, 0, 200, 117, 104, 112};

      subj.resize(2);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      clip.resize(2);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), clip[0]);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[1]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && solution.size() == 2 
        && Orientation(solution[1]) && Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Orientation13: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {0, 0, 119, 0, 109, 108, 0, 101};
      cInt ints2[] = {115, 100, 200, 103, 200, 200, 101, 200};
      cInt ints3[] = {0, 117, 110, 100, 103, 200, 0, 200};
      cInt ints4[] = {115, 0, 200, 0, 200, 109, 119, 102};

      subj.resize(2);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      clip.resize(2);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), clip[0]);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[1]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && solution.size() == 2 
        && Orientation(solution[1]) && Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Orientation14: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {0, 0, 102, 0, 119, 107, 0, 101};
      cInt ints2[] = {116, 110, 200, 114, 200, 200, 107, 200};
      cInt ints3[] = {0, 108, 117, 106, 111, 200, 0, 200};
      cInt ints4[] = {112, 0, 200, 0, 200, 117, 101, 112};

      subj.resize(2);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      clip.resize(2);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), clip[0]);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[1]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && solution.size() == 2 
        && Orientation(solution[1]) && Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Orientation15: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {0, 0, 106, 0, 107, 111, 0, 102};
      cInt ints2[] = {119, 116, 200, 118, 200, 200, 117, 200};
      cInt ints3[] = {0, 101, 107, 106, 111, 200, 0, 200};
      cInt ints4[] = {113, 0, 200, 0, 200, 114, 117, 117};

      subj.resize(2);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      clip.resize(2);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), clip[0]);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[1]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && solution.size() == 2 
        && Orientation(solution[1]) && Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------
 
  class SelfInt1: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {0, 0, 201, 0, 203, 217, 0, 207};
      cInt ints2[] = {204, 214, 400, 217, 400, 400, 205, 400};
      cInt ints3[] = {0, 211, 203, 214, 208, 400, 0, 400};
      cInt ints4[] = {207, 0, 400, 0, 400, 208, 218, 200};

      subj.resize(2);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      clip.resize(2);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), clip[0]);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[1]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && (solution.size() == 2) && 
        Orientation(solution[0]) && Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class SelfInt2: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {0, 0, 200, 0, 219, 207, 0, 200};
      cInt ints2[] = {201, 207, 400, 200, 400, 400, 200, 400};
      cInt ints3[] = {0, 200, 214, 207, 200, 400, 0, 400};
      cInt ints4[] = {200, 0, 400, 0, 400, 200, 209, 215};

      subj.resize(2);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      clip.resize(2);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), clip[0]);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[1]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && (solution.size() == 2) && 
        Orientation(solution[0]) && Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class SelfInt3: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {0, 0, 201, 0, 207, 214, 0, 207};
      cInt ints2[] = {209, 211, 400, 206, 400, 400, 214, 400};
      cInt ints3[] = {0, 211, 207, 208, 213, 400, 0, 400};
      cInt ints4[] = {213, 0, 400, 0, 400, 210, 213, 200};

      subj.resize(2);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      clip.resize(2);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), clip[0]);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[1]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && (solution.size() == 2) && 
        Orientation(solution[0]) && Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class SelfInt4: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {0, 0, 214, 0, 209, 206, 0, 201};
      cInt ints2[] = {205, 208, 400, 207, 400, 400, 200, 400};
      cInt ints3[] = {201, 0, 400, 0, 400, 217, 205, 217};
      cInt ints4[] = {0, 205, 215, 206, 217, 400, 0, 400};

      subj.resize(2);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      clip.resize(2);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), clip[0]);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[1]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && (solution.size() == 2) && 
        Orientation(solution[0]) && Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class SelfInt5: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;
      
      cInt ints1[] = {0,0, 219,0, 217,217, 0,200};
      cInt ints2[] = {214,219, 400,200, 400,400, 219,400};
      cInt ints3[] = {0,207, 205,211, 214,400, 0,400};
      cInt ints4[] = {202, 0, 400, 0, 400, 217, 205, 217};

      subj.resize(2);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      clip.resize(2);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), clip[0]);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[1]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctDifference, solution, pft, pft);
      
      res = res && (solution.size() == 2) && 
        Orientation(solution[0]) && Orientation(solution[1]);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class SelfInt6: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;
      
      cInt ints[6] = {182,179, 477,123, 25,55};
      cInt ints2[8] = {477,122, 485,103, 122,265, 55,207};

      subj.resize(1);
      MakePolygonFromInts(ints, sizeof(ints)/sizeof(cInt), subj[0]);
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctIntersection, solution, pft, pft);
      res = res && (solution.size() == 1) && Orientation(solution[0]);
      
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Union1: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;
      
      cInt ints1[] = {1026,1126, 1026,235, 4505,401,
    4522,1145, 4503,1162, 2280,1129};
      cInt ints2[] = {4501,1100, 4501,866, 1146,462,
    1071,1067, 4469,1000};
      cInt ints3[] = {4499, 1135, 3360, 1050, 3302, 1107};
      cInt ints4[] = {3360, 1050, 3291, 1118, 4512, 1136};

      subj.resize(3);
      MakePolygonFromInts(ints1, sizeof(ints1)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      MakePolygonFromInts(ints3, sizeof(ints3)/sizeof(cInt), subj[2]);
      clip.resize(1);
      MakePolygonFromInts(ints4, sizeof(ints4)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      bool res = c.Execute(ctUnion, solution, pft, pft);
      res = res && (solution.size() == 2) && Orientation(solution[0]) &&
        !Orientation(solution[1]);
      
      return res;
    }
  };
  //---------------------------------------------------------------------------

  class Union2: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;
      
      cInt ints[11][8] = {{10, 10, 20, 10, 20, 20, 10, 20},
        {20, 10, 30, 10, 30, 20, 20, 20},
        {30, 10, 40, 10, 40, 20, 30, 20},
        {40, 10, 50, 10, 50, 20, 40, 20},
        {50, 10, 60, 10, 60, 20, 50, 20},
        {10, 20, 20, 20, 20, 30, 10, 30},
        {30, 20, 40, 20, 40, 30, 30, 30},
        {10, 30, 20, 30, 20, 40, 10, 40},
        {20, 30, 30, 30, 30, 40, 20, 40},
        {30, 30, 40, 30, 40, 40, 30, 40},
        {40, 30, 50, 30, 50, 40, 40, 40}};

      subj.resize(11);
      for (size_t i = 0; i < 11; ++i)
        MakePolygonFromInts(ints[i], 8, subj[i]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      bool res = c.Execute(ctUnion, solution, pft, pft);
      res = res && (solution.size() == 2);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class Union3: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;     
      cInt ints[2][6] = {{1,3, 2,4, 2,5}, {1,3, 3,3, 2,4}};
      subj.resize(2);
      for (size_t i = 0; i < 2; ++i)
        MakePolygonFromInts(ints[i], sizeof(ints[i])/sizeof(cInt), subj[i]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      bool res = c.Execute(ctUnion, solution, pft, pft);
      res = res && (solution.size() == 1);
      return res;
    } 
  };
  //---------------------------------------------------------------------------

  class AddPath1: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][12] = {{480,20, 480,110, 320,30, 480,30, 250,250, 480,30}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 12, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath2: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][18] = {{60,320, 390,320, 100,320,
    220,120, 120,10, 20,380, 120,20, 280,20, 480,20}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 18, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath3: public virtual ClipperTest
  {
    bool DoTest()
    {      
      cInt ints[1][16] = {{320,70, 420,370, 250,170, 60,290,
    10,290, 210,290, 400,150, 410,340}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 16, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath4: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][12] = {{300,80, 280,220, 
        180,220, 170,220, 290,220, 40,180}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 12, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath5: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][10] = {{170,340, 280,230, 160,50, 430,370, 280,230}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 10, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath6: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][10] = {{30,380, 70,160, 170,220, 70,160, 240,160}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 10, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath7: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][8] = {{440,300, 40,40, 440,300, 80,360}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 8, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath8: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][10] = {{260,10, 260,240, 190,100, 260,10, 420,120}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 10, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath9: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][10] = {{60,240, 30,10, 460,170, 110,280, 30,10}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 10, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath10: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][12] = {{430,270, 440,260, 470,30, 280,30, 430,270, 450,40}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 12, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath11: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][10] = {{320,10, 240,300, 260,140, 320,10, 240,300}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 10, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath12: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][10] = {{270,340, 130,50, 50,350, 270,340, 290,40}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 10, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath13: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][10] = {{430,330, 280,10, 210,280, 430,330, 280,10}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 10, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath14: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][8] = {{50,30, 410,330, 50,30, 310,50}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 8, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath15: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][6] = {{230,50, 10,50, 110,50}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 6, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath16: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][12] = {{260,320, 40,130, 100,30, 80,360, 260,320, 40,50}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 12, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath17: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][10] = {{190,170, 350,290, 110,290, 250,290, 430,90}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 10, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath18: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][10] = {{150,330, 210,70, 90,70, 210,70, 150,330}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 10, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath19: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][10] = {{170,290, 50,290, 170,290, 410,310, 170,290}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 10, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class AddPath20: public virtual ClipperTest
  {
    bool DoTest()
    {
      
      cInt ints[1][8] = {{430,10, 150,110, 430,10, 230,50}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 8, subj2[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      return true;
    }
  };
  //---------------------------------------------------------------------------

  class OpenPath1: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;
      
      cInt ints[1][10] = {{290,370, 160,150, 230,150, 160,150, 250,280}};
      subj2.resize(1);
      MakePolygonFromInts(ints[0], 10, subj2[0]);
      cInt ints2[1][8] = {{150,10, 160,290, 200,80, 50,340}};
      clip.resize(1);
      MakePolygonFromInts(ints2[0], 8, clip[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      c.AddPaths(clip, ptClip, true);
      return c.Execute(ctIntersection, 
        polytree, pft, pft);
    }
  };
  //---------------------------------------------------------------------------

  class OpenPath2: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;
      
      cInt ints[] = {50, 310, 210, 110, 260, 110, 170, 110, 350, 200};
      subj2.resize(1);
      MakePolygonFromInts(ints, sizeof(ints)/sizeof(cInt), subj2[0]);
      cInt ints2[] = {310, 30, 90, 90, 370, 130};
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      c.AddPaths(clip, ptClip, true);
      return c.Execute(ctIntersection, 
        polytree, pft, pft);
    }
  };
  //---------------------------------------------------------------------------

  class OpenPath3: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;
      
      cInt ints[] = {40,360,  260,50,  180,270,  180,250,  410,250,  140,250,  350,380};
      subj2.resize(1);
      MakePolygonFromInts(ints, sizeof(ints)/sizeof(cInt), subj2[0]);
      cInt ints2[] = {30, 110, 330, 90, 20, 370};
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      c.AddPaths(clip, ptClip, true);
      return c.Execute(ctIntersection, 
        polytree, pft, pft);
    }
  };
  //---------------------------------------------------------------------------

  class OpenPath4: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;
      
      cInt ints[] = {10,50, 200,50};
      subj2.resize(1);
      MakePolygonFromInts(ints, sizeof(ints)/sizeof(cInt), subj2[0]);
      cInt ints2[] = {50,10, 150,10, 150,100, 50,100};
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj2, ptSubject, false);
      c.AddPaths(clip, ptClip, true);
      return c.Execute(ctIntersection, polytree, pft, pft) && 
        (polytree.ChildCount() == 1);
    }
  };
  //---------------------------------------------------------------------------

  class Simplify1: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;    
      cInt ints[] = {5048400, 1719180, 5050250, 1717630, 5049070,
        1717320, 5049150, 1717200, 5049350, 1717570};
      subj.resize(1);
      MakePolygonFromInts(ints, sizeof(ints)/sizeof(cInt), subj[0]);
      Clipper c;
      c.StrictlySimple(true); 
      c.AddPaths(subj, ptSubject, true);
      return c.Execute(ctUnion, solution, pft, pft) &&
        (solution.size() == 2);
    }
  };
  //---------------------------------------------------------------------------

  class Simplify2: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;    
      cInt ints[] = {220,720, 420,720, 420,520, 320,520, 320,480,
        480,480, 480,800, 180,800, 180,480, 320,480, 320,520, 220,520};
      cInt ints2[] = {440,520, 620,520, 620,420, 440,420};
      subj.resize(2);
      MakePolygonFromInts(ints, sizeof(ints)/sizeof(cInt), subj[0]);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[1]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      return c.Execute(ctUnion, solution, pft, pft) && 
        (solution.size() == 3);
    }
  };
  //---------------------------------------------------------------------------

  class Joins1: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;    
      cInt ints[8][8] = {
        {0, 0, 0, 32, 32, 32, 32, 0},
        {32, 0, 32, 32, 64, 32, 64, 0},
        {64, 0, 64, 32, 96, 32, 96, 0},
        {96, 0, 96, 32, 128, 32, 128, 0},
        {0, 32, 0, 64, 32, 64, 32, 32},
        {64, 32, 64, 64, 96, 64, 96, 32},
        {0, 64, 0, 96, 32, 96, 32, 64},
        {32, 64, 32, 96, 64, 96, 64, 64}
      };
      subj.resize(8);
      for (size_t i = 0; i < 8; ++i)
        MakePolygonFromInts(ints[i], 8, subj[i]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      return c.Execute(ctUnion, solution, pft, pft) && 
        (solution.size() == 1);
    }
  };
  //---------------------------------------------------------------------------

  class Joins2: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;    
      cInt ints[12][8] = {
        {100, 100, 100, 91, 200, 91, 200, 100},
        {200, 91, 209, 91, 209, 250, 200, 250},
        {209, 250, 209, 259, 100, 259, 100, 250},
        {100, 250, 109, 250, 109, 300, 100, 300},
        {109, 300, 109, 309, 50, 309, 50, 300},
        {50, 309, 41, 309, 41, 250, 50, 250},
        {50, 250, 50, 259, 0, 259, 0, 250},
        {0, 259, -9, 259, -9, 100, 0, 100},
        {-9, 100, -9, 91, 50, 91, 50, 100},
        {50, 100, 41, 100, 41, 50, 50, 50},
        {41, 50, 41, 41, 100, 41, 100, 50},
        {100, 41, 109, 41, 109, 100, 100, 100}
      };
      subj.resize(12);
      for (size_t i = 0; i < 12; ++i)
        MakePolygonFromInts(ints[i], 8, subj[i]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      return c.Execute(ctUnion, solution, pft, pft) && 
        (solution.size() == 2) && 
        (Orientation(solution[0]) != Orientation(solution[1]));
    }
  };
  //---------------------------------------------------------------------------

  class Joins3: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftNonZero;    
      cInt ints[] = {220,720,  420,720,  420,520,  320,520,  320,480,  
        480,480,  480,800, 180,800,  180,480,  320,480,  320,520,  220,520};
      subj.resize(1);
      MakePolygonFromInts(ints, sizeof(ints)/sizeof(cInt), subj[0]);
      cInt ints2[] = {440,520,  620,520,  620,420,  440,420};
      clip.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), clip[0]);
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      c.AddPaths(clip, ptClip, true);
      return c.Execute(ctUnion, solution, pft, pft) && 
        (solution.size() == 2) && 
        (Orientation(solution[0]) != Orientation(solution[1]));
    }
  };
  //---------------------------------------------------------------------------

  class Joins4: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;    
      int ints[120] = {
        1172, 318, 337, 1066, 154, 639, 479, 448, 1197, 545, 1041, 773, 30, 888,
        444, 308, 1051, 552, 1109, 102, 658, 683, 394, 596, 972, 1145, 442, 179,
        470, 441, 227, 564, 1179, 1037, 213, 379, 1072, 872, 587, 171, 723, 329,
        272, 242, 952, 1121, 714, 1148, 91, 217, 735, 561, 903, 1009, 664, 1168,
        1160, 847, 9, 7, 619, 142, 1139, 1116, 1134, 369, 760, 647, 372, 134,
        1106, 183, 311, 103, 265, 185, 1062, 856, 453, 944, 44, 653, 766, 527,
        334, 965, 443, 971, 474, 36, 397, 1138, 901, 841, 775, 612, 222, 465,
        148, 955, 417, 540, 997, 472, 666, 802, 754, 32, 907, 638, 927, 42, 990,
        406, 99, 682, 17, 281, 106, 848};
      MakeDiamondPolygons(20, 600, 400, subj);
      for (int i = 0; i < 120; ++i) subj[ints[i]].clear();
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      return c.Execute(ctUnion, solution, pft, pft) && 
        (solution.size() == 69);
    }
  };
  //---------------------------------------------------------------------------

  class Joins5: public virtual ClipperTest
  {
    bool DoTest()
    {
      pft = pftEvenOdd;    
      int ints[60] = {
        553, 388, 574, 20, 191, 26, 461, 258, 509, 19, 466, 257, 90, 269, 373, 516,
    350, 333, 288, 141, 47, 217, 247, 519, 535, 336, 504, 497, 344, 341, 293,
    177, 558, 598, 399, 286, 482, 185, 266, 24, 27, 118, 338, 413, 514, 510,
    366, 46, 593, 465, 405, 32, 449, 6, 326, 59, 75, 173, 127, 130};
      MakeSquarePolygons(20, 600, 400, subj);
      for (int i = 0; i < 60; ++i) subj[ints[i]].clear();
      Clipper c;
      c.AddPaths(subj, ptSubject, true);
      return c.Execute(ctUnion, solution, pft, pft) && 
        (solution.size() == 37);
    }
  };
  //---------------------------------------------------------------------------

  class OffsetPoly1: public virtual ClipperTest
  {
    bool DoTest()
    {
      const double scale = 10;
      cInt ints2[] = {348,257, 364,148, 362,148, 326,241, 295,219, 258,88, 440,129, 370,196, 372,275};
      subj.resize(1);
      MakePolygonFromInts(ints2, sizeof(ints2)/sizeof(cInt), subj[0], scale);
      ClipperOffset co = ClipperOffset();
      co.AddPaths(subj, jtRound, etClosedPolygon);
      solution.clear();
      co.Execute(solution, -7.0 * scale);
      return solution.size() == 2;
    }
  };
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  
  void LoadTests()
  {
    AddTest(new Difference1(), "Difference1");
    AddTest(new Difference2(), "Difference2");
    AddTest(new Horz1(), "Horz1");
    AddTest(new Horz2(), "Horz2");
    AddTest(new Horz3(), "Horz3");
    AddTest(new Horz4(), "Horz4");
    AddTest(new Horz5(), "Horz5");
    AddTest(new Horz6(), "Horz6");
    AddTest(new Horz7(), "Horz7");
    AddTest(new Horz8(), "Horz8");
    AddTest(new Horz9(), "Horz9");
    AddTest(new Horz10(), "Horz10");
    AddTest(new Orientation1(), "Orientation1");
    AddTest(new Orientation2(), "Orientation2");
    AddTest(new Orientation3(), "Orientation3");
    AddTest(new Orientation4(), "Orientation4");
    AddTest(new Orientation5(), "Orientation5");
    AddTest(new Orientation6(), "Orientation6");
    AddTest(new Orientation7(), "Orientation7");
    AddTest(new Orientation8(), "Orientation8");
    AddTest(new Orientation9(), "Orientation9");
    AddTest(new Orientation10(), "Orientation10");
    AddTest(new Orientation11(), "Orientation11");
    AddTest(new Orientation12(), "Orientation12");
    AddTest(new Orientation13(), "Orientation13");
    AddTest(new Orientation14(), "Orientation14");
    AddTest(new Orientation15(), "Orientation15");
    AddTest(new SelfInt1(), "SelfInt1");
    AddTest(new SelfInt2(), "SelfInt2");
    AddTest(new SelfInt3(), "SelfInt3");
    AddTest(new SelfInt4(), "SelfInt4");
    AddTest(new SelfInt5(), "SelfInt5");
    AddTest(new SelfInt6(), "SelfInt6");
    AddTest(new Union1(), "Union1");
    AddTest(new Union2(), "Union2");
    AddTest(new Union3(), "Union3");
    AddTest(new AddPath1(), "AddPath1");
    AddTest(new AddPath2(), "AddPath2");
    AddTest(new AddPath3(), "AddPath3");
    AddTest(new AddPath4(), "AddPath4");
    AddTest(new AddPath5(), "AddPath5");
    AddTest(new AddPath6(), "AddPath6");
    AddTest(new AddPath7(), "AddPath7");
    AddTest(new AddPath8(), "AddPath8");
    AddTest(new AddPath9(), "AddPath9");
    AddTest(new AddPath10(), "AddPath10");
    AddTest(new AddPath11(), "AddPath11");
    AddTest(new AddPath12(), "AddPath12");
    AddTest(new AddPath13(), "AddPath13");
    AddTest(new AddPath14(), "AddPath14");
    AddTest(new AddPath15(), "AddPath15");
    AddTest(new AddPath16(), "AddPath16");
    AddTest(new AddPath17(), "AddPath17");
    AddTest(new AddPath18(), "AddPath18");
    AddTest(new AddPath19(), "AddPath19");
    AddTest(new AddPath20(), "AddPath20");
    AddTest(new OpenPath1(), "OpenPath1");
    AddTest(new OpenPath2(), "OpenPath2");
    AddTest(new OpenPath3(), "OpenPath3");
    AddTest(new OpenPath4(), "OpenPath4");
    AddTest(new Simplify1(), "Simplify1");
    AddTest(new Simplify2(), "Simplify2");
    AddTest(new Joins1(), "Joins1");
    AddTest(new Joins2(), "Joins2");
    AddTest(new Joins3(), "Joins3");
    AddTest(new Joins4(), "Joins4");
    AddTest(new Joins5(), "Joins5");
    AddTest(new OffsetPoly1(), "OffsetPoly1");
  }
  //---------------------------------------------------------------------------

}