/*******************************************************************************
*                                                                              *
* Author    :  Angus Johnson                                                   *
* Version   :  0.9                                                             *
* Date      :  11 February 2014                                                *
* Website   :  http://www.angusj.com                                           *
* Copyright :  Angus Johnson 2010-2014                                         *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/

#ifndef svgbuilder_hpp
#define svgbuilder_hpp

#include "clipper.hpp"
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

namespace SvgBuilder
{

  using namespace ClipperLib;
  using namespace std;

  typedef Path Polygon;
  typedef Paths Polygons;

string ColorToHtml(unsigned clr);

float GetAlphaAsFrac(unsigned clr);

#ifdef use_int32 
const IntRect irMax = {LONG_MAX, LONG_MAX, -LONG_MAX, -LONG_MAX};
#else
const IntRect irMax = {LLONG_MAX, LLONG_MAX, -LLONG_MAX, -LLONG_MAX};
#endif


//a simple class to build SVG files that displays polygons
class SvgBase
{

  class StyleInfo
  {
  public:
    PolyFillType pft;
    unsigned brushClr;
    unsigned penClr;
    double penWidth;
    bool closePath;
    bool showStartArrow;
    bool showEndArrow;
    bool showCoords;
    bool showZ;
  };

  struct FontInfo
  {
  public:
    std::string family;
    int size;
    unsigned fillColor;
  };
  typedef std::vector<FontInfo> FontInfoList;

  class TextInfo
  {
  public:
    const std::string text;
    double  x;
    double  y;
    unsigned fontIdx;
    TextInfo(double  _x, double  _y, unsigned _fi, const std::string& _text): x(_x), y(_y), fontIdx(_fi),text(_text) {}
  };
  typedef std::vector<TextInfo> TextInfoList;

  class PolyInfo
  {
    public:
      Polygons polygons;
      StyleInfo si;

      PolyInfo(const Polygons& p, StyleInfo style)
      {
        this->polygons = p;
        this->si = style;
      }
  };
  typedef std::vector<PolyInfo> PolyInfoList;

private:
  PolyInfoList polyInfos;
  FontInfoList fontInfos;
  TextInfoList textInfos;

  static const std::string svg_xml_start[];
  static const std::string path_end_poly[];
  static const std::string path_end_line[];
  static const std::string arrow_defs[];

  void CheckFonts()
  {
    //if no font has been specified create a default font...
    if (!fontInfos.empty()) return;
    FontInfo fi;
    fi.family = "Verdana";
    fi.size = 10;
    fi.fillColor = 0xFF000000;
    fontInfos.push_back(fi);
  }

  void UpdateBounds(const Polygons& p)
  {
    IntRect r = GetBounds(p);
    if (r.left < bounds.left) bounds.left = r.left;
    if (r.top < bounds.top) bounds.top = r.top;
    if (r.right > bounds.right) bounds.right = r.right;
    if (r.bottom > bounds.bottom) bounds.bottom = r.bottom;
  }
  //---------------------------------------------------------------------------

public:
  StyleInfo style;
  IntRect bounds;

  SvgBase()
  {
    style.pft = pftNonZero;
    style.brushClr = 0xFFFFFFCC;
    style.penClr = 0xFF000000;
    style.penWidth = 0.8;
    style.closePath = true;
    style.showCoords = false;
    style.showZ = false;
    style.showStartArrow = false;
    style.showEndArrow = false;
    bounds = irMax;
  }

  IntRect GetBounds(const Polygons& p)
  {
    IntRect result;
    result = irMax;
    for (size_t i = 0; i < p.size(); i++)
      for (size_t j = 0; j < p[i].size(); j++)
      {
        if (p[i][j].X < result.left) result.left = p[i][j].X;
        if (p[i][j].X > result.right) result.right = p[i][j].X;
        if (p[i][j].Y < result.top) result.top = p[i][j].Y;
        if (p[i][j].Y > result.bottom) result.bottom = p[i][j].Y;
      }
    return result;
  }

  void AddPath(const Polygons& Polygons, unsigned brushClr, unsigned penClr, bool closed)
  {
    if (Polygons.size() == 0) return;
    CheckFonts();
    if (closed)
      style.brushClr = brushClr; 
    else
      style.brushClr = 0;
    style.penClr = penClr;
    style.closePath = closed;
    PolyInfo pi = PolyInfo(Polygons, style);
    polyInfos.push_back(pi);
    UpdateBounds(Polygons);
  }
  //---------------------------------------------------------------------------

  void SetFont(std::string family, int size = 10, unsigned fillColor = 0xFF000000)
  {
    FontInfo fi;
    fi.family = family;
    fi.size = size;
    fi.fillColor = fillColor;
    fontInfos.push_back(fi);
  }
  //---------------------------------------------------------------------------

  void AddText(double x, double y, const std::string& text)
  {
    CheckFonts();
    TextInfo ti = TextInfo(x, y, fontInfos.size() -1, text);
    textInfos.push_back(ti);
  }
  //---------------------------------------------------------------------------

  bool SaveToFile(const std::string filename, int width = 0, int height = 0, int margin = 10) const
  {
    if (margin < 0) margin = 0;
    double scale = 1.0;
    if (width > 0 && height > 0) 
      scale = 1.0 / max((double)(bounds.right - bounds.left)/width, 
        (double)(bounds.bottom-bounds.top)/height);
    ofstream file;
    file.open(filename);
    if (!file.is_open()) return false;
    file.setf(ios::fixed);
    file.precision(0);
    file << svg_xml_start[0] <<
      (int)((bounds.right - bounds.left) *scale + margin*2) << "px" << svg_xml_start[1] <<
      (int)((bounds.bottom-bounds.top) *scale + margin*2) << "px" << svg_xml_start[2] <<
      (int)((bounds.right - bounds.left) *scale + margin*2) << " " <<
      (int)((bounds.bottom-bounds.top) *scale + margin*2) << svg_xml_start[3];
    setlocale(LC_NUMERIC, "C");
    file.precision(1);

    for (PolyInfoList::size_type i = 0; i < polyInfos.size(); ++i)
    {
      if (!polyInfos[i].si.closePath && (polyInfos[i].si.showStartArrow || polyInfos[i].si.showEndArrow)) 
        file << arrow_defs[0] << i+1 << arrow_defs[1] << 
        ColorToHtml(polyInfos[i].si.brushClr) << arrow_defs[2] << i+1 << arrow_defs[3] << 
        ColorToHtml(polyInfos[i].si.brushClr)  << arrow_defs[4];
      file << "<path d=\"";
      for (Polygons::size_type j = 0; j < polyInfos[i].polygons.size(); ++j)
      {      
        if (polyInfos[i].polygons[j].empty()) continue;
        file << " M " << ((double)(polyInfos[i].polygons[j][0].X-bounds.left) * scale + margin) <<
          " " << ((double)(polyInfos[i].polygons[j][0].Y-bounds.top) * scale + margin);
        for (Polygon::size_type k = 1; k < polyInfos[i].polygons[j].size(); ++k)
        {
          IntPoint ip = polyInfos[i].polygons[j][k];
          double x = (ip.X - bounds.left) * scale + margin;
          double y = (ip.Y - bounds.top) * scale + margin;
          file << " L " << x << " " << y;
        }
        if (polyInfos[i].si.closePath) file << " z";
      }
      if (polyInfos[i].si.closePath)
        file << 
          path_end_poly[0] << ColorToHtml(polyInfos[i].si.brushClr) <<
          path_end_poly[1] << GetAlphaAsFrac(polyInfos[i].si.brushClr) <<
          path_end_poly[2] << (polyInfos[i].si.pft == pftEvenOdd ? "evenodd" : "nonzero") <<
          path_end_poly[3] << ColorToHtml(polyInfos[i].si.penClr) <<
          path_end_poly[4] << GetAlphaAsFrac(polyInfos[i].si.penClr) <<
          path_end_poly[5] << polyInfos[i].si.penWidth << 
          path_end_poly[6];
      else{
        file << 
          path_end_line[0] << ColorToHtml(polyInfos[i].si.penClr) <<
          path_end_line[1] << GetAlphaAsFrac(polyInfos[i].si.penClr) <<
          path_end_line[2] << polyInfos[i].si.penWidth << 
          path_end_line[3];
        if (polyInfos[i].si.showStartArrow) file << path_end_line[4] << i+1 << path_end_line[5];
        if (polyInfos[i].si.showEndArrow) file << path_end_line[6] << i+1 << path_end_line[7];
        file << path_end_line[8];
      }
    }
    bool showCoords = false;
    for(size_t i = 0; i < polyInfos.size(); i++)
      if (polyInfos[i].si.showCoords) {showCoords = true; break;}

    if (!textInfos.empty() || showCoords) 
    {
      if (showCoords)
      {
        FontInfo fontInfo = fontInfos.front();
        file << "<g font-family=\"" << fontInfo.family << "\" font-size=\""<< 
          (int)fontInfo.size << "\" fill=\"" << ColorToHtml(fontInfo.fillColor) << "\">\n";
        for(size_t i = 0; i < polyInfos.size(); i++)
          if (polyInfos[i].si.showCoords)
          {
            for (Polygons::size_type j = 0; j < polyInfos[i].polygons.size(); ++j)
            {
              if (polyInfos[i].polygons[j].size() < 3) continue;
              for (Polygon::size_type k = 0; k < polyInfos[i].polygons[j].size(); ++k)
              {
                IntPoint ip = polyInfos[i].polygons[j][k];
                //if (polyInfos[i].si.showZ) 
                //  file << "  <text x=\"" << (int)((ip.X- bounds.Left) * scale + margin) <<
                //    "\" y=\"" << (int)((ip.Y- bounds.Top) * scale + margin) << "\">" <<
                //    " " << ip.Z << "</text>\n";
                //else
                  file << "  <text x=\"" << (int)((ip.X- bounds.left) * scale + margin) <<
                    "\" y=\"" << (int)((ip.Y- bounds.top) * scale + margin) << "\">" <<
                    ip.X << "," << ip.Y << "</text>\n";
              }
            }
          }
      }
      if (showCoords)  file << "</g>\n";

      unsigned fi = INT_MAX;
      for (size_t i = 0; i < textInfos.size(); ++i)
      {
        TextInfo ti = textInfos[i];
        if (ti.fontIdx != fi)
        {
          if (fi != INT_MAX) file << "</g>\n";
          fi = ti.fontIdx;
          FontInfo fontInfo = fontInfos[fi];
          file << "<g font-family=\"" << fontInfo.family << "\" font-size=\""<< 
            (fontInfo.size* scale) << "\" fill=\"" << ColorToHtml(fontInfo.fillColor) << "\">\n";
        }
        file << "  <text x=\"" << (int)((ti.x- bounds.left) * scale + margin) <<
          "\" y=\"" << (int)((ti.y- bounds.top) * scale + margin) << "\">" <<
          ti.text << "</text>\n";
      }
      if (fi != INT_MAX) file << "</g>\n";
    }
    file << "</svg>\n";
    file.close();
    setlocale(LC_NUMERIC, "");
    return true;
  }
  //---------------------------------------------------------------------------

};
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void SimpleSVG(const string filename, Polygons& subj, Polygons& clip, Polygons& solution,
  int width = 800, int height = 600);

}

#endif //svgbuilder_hpp
