// Gmsh - Copyright (C) 1997-2018 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// issues on https://gitlab.onelab.info/gmsh/gmsh/issues

#ifndef _READ_IMG_
#define _READ_IMG_

#include <string>

int read_pnm(const std::string &fileName);
int read_jpeg(const std::string &fileName);
int read_png(const std::string &fileName);
int read_bmp(const std::string &fileName);

#endif
