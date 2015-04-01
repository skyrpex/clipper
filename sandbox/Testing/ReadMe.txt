  #include "clippertest.h";
  
  using namespace ClipperTestLib;
  
  //when to produce SVG files ...
  //enum EnumSVG {svgNever, svgFail, svgAlways};
  //load all tests ...
  //void LoadTests();
  //add a test (or an extra test if LoadTests() is also called) ...
  //void AddTest(ClipperTest* t, const std::string& title);
  //do the testing using either of the following ...
  //void TestAll(EnumSVG svg = svgFail);
  //void TestOne(const string& title, EnumSVG svg);

  //Example:
  LoadTests();
  TestAll();
  
  //That's it!