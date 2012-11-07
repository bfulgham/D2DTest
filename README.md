# D2DTest

This repository contains the source code for a test of various rendering libraries under Windows.

# Use

When you run the executable, it renders a very simple Cairo clock example from the [[Cairo graphics
project] http://www.cairographics.org].  I ported the logic to the CoreGraphics and Direct2D APIs
to get a feel for each.

# Building

By default, the project will build the Cairo and Direct2D targets, and will exclude Apple's
proprietary CoreGraphics library.  If you have the link libraries and binaries (for example,
if you are set up to build the [[WebKit project] http://www.webkit.org/], you can add the
necessary include and library paths to the project and build that target, too.

## Important

I have only tried this on Windows 7 using Visual Studio 2010 (both Professional and Express
editions).  Other platforms and compilers may not build cleanly.