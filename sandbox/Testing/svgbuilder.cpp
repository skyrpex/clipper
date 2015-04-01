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

#include "clipper.hpp"
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include "svgbuilder.h"

namespace SvgBuilder
{

const std::string SvgBase::svg_xml_start [] =
  {"<?xml version=\"1.0\" standalone=\"no\"?>\n"
   "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\"\n"
   "\"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\n\n"
   "<svg width=\"",
   "\" height=\"",
   "\" viewBox=\"0 0 ",
   "\" version=\"1.0\" xmlns=\"http://www.w3.org/2000/svg\">\n\n"
  };
const std::string SvgBase::path_end_poly [] =
  {"\"\n style=\"fill:",
   "; fill-opacity:",
   "; fill-rule:",
   "; stroke:",
   "; stroke-opacity:",
   "; stroke-width:",
   ";\"/>\n\n"
  };
const std::string SvgBase::path_end_line [] =
  {"\"\n style=\"fill:none; stroke:",
   "; stroke-opacity:",
   "; stroke-width:",
   "; ",
   "marker-start:url(#TriArrowSt",
   "); ",
   "marker-end:url(#TriArrow",
   "); ",
   "\" />\n\n"
  };

const std::string SvgBase::arrow_defs[] = 
  {"<defs>\
  \n  <marker id=\"TriArrow",
  "\" viewBox=\"0 0 10 10\" refX=\"2.0\" refY=\"5.0\" fill=\"",
  "\" markerUnits=\"strokeWidth\" markerHeight=\"10\" markerWidth=\"10\" orient=\"auto\"> \
  \n    <path d=\"M 0.0 0.0 L 10.0 5.0 L 0.0 10.0 z\" />\n  </marker> \
  \n  <marker id=\"TriArrowSt",
  "\" viewBox=\"0 0 10 10\" refX=\"8.0\" refY=\"5.0\" fill=\"",
  "\" markerUnits=\"strokeWidth\" markerHeight=\"10\" markerWidth=\"10\" orient=\"auto\"> \
  \n    <path d=\"M 10.0 0.0 L 0.0 5.0 L 10.0 10.0 z\" />\n  </marker> \
  \n</defs>\n\n"};

  using namespace ClipperLib;
  using namespace std;

  string ColorToHtml(unsigned clr)
  {
    stringstream ss;
    ss << '#' << hex << std::setfill('0') << setw(6) << (clr & 0xFFFFFF);
    return ss.str();
  }
  //------------------------------------------------------------------------------

  float GetAlphaAsFrac(unsigned clr)
  {
    return ((float)(clr >> 24) / 255);
  }
  //------------------------------------------------------------------------------

void SimpleSVG(const string filename, Polygons& subj, Polygons& clip, Polygons& solution,
  int width, int height)
{
  SvgBase svg;
  svg.style.pft = pftEvenOdd; 
  svg.AddPath(subj, 0x206666AC,0xCCD0D0DD, true);
  svg.AddPath(clip, 0x24666600, 0xCCDDDD80, true);
  svg.AddPath(solution, 0xFF99FF99, 0x40009900, true);
  //svg.SetFont("Verdana", 16, 0xFF0000AA);
  //svg.AddText(svg.bounds.Left, svg.bounds.Top + 20, "Clipper Benchmarks");
  //svg.SetFont("Verdana", 12, 0xFFAA0000);
  //svg.AddText(svg.bounds.Left, svg.bounds.Top + 36, "&#0169; Angus Johnson 2013");
  svg.SaveToFile(filename, width, height);
}
//------------------------------------------------------------------------------

}
