// Gmsh - Copyright (C) 1997-2016 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to the public mailing list <gmsh@geuz.org>.

#include <limits>
#include <cmath>
#include <algorithm>
#include "nodalBasis.h"
#include "BasisFactory.h"
#include "pointsGenerators.h"

namespace ClosureGen {
  inline double pow2(double x)
  {
    return x * x;
  }

  void rotateHex(int iFace, int iRot, int iSign, double uI, double vI,
                 double &uO, double &vO, double &wO)
  {
    if (iSign<0){
      double tmp=uI;
      uI=vI;
      vI=tmp;
    }
    for (int i=0; i < iRot; i++){
      double tmp=uI;
      uI=-vI;
      vI=tmp;
    }
    switch (iFace) {
      case 0: uO = vI; vO = uI; wO=-1; break;
      case 1: uO = uI; vO = -1; wO=vI; break;
      case 2: uO =-1;  vO = vI; wO=uI; break;
      case 3: uO = 1;  vO = uI; wO=vI; break;
      case 4: uO =-uI; vO = 1;  wO=vI; break;
      case 5: uO =uI;  vO = vI; wO=1; break;
    }
  }

  void rotateHexFull(int iFace, int iRot, int iSign, double uI, double vI,
                     double wI, double &uO, double &vO, double &wO)
  {
    switch (iFace) {
      case 0: uO = uI; vO = vI; wO = wI; break;
      case 1: uO = wI; vO = uI; wO = vI; break;
      case 2: uO = vI; vO = wI; wO = uI; break;
      case 3: uO = wI; vO = vI; wO =-uI; break;
      case 4: uO = wI; vO =-uI; wO =-vI; break;
      case 5: uO = vI; vO = uI; wO =-wI; break;
    }
    for (int i = 0; i < iRot; i++){
      double tmp = uO;
      uO = -vO;
      vO = tmp;
    }
    if (iSign<0){
      double tmp = uO;
      uO = vO;
      vO = tmp;
    }
  }

  void generate1dVertexClosure(nodalBasis::clCont &closure, int order)
  {
    closure.clear();
    closure.resize(2);
    closure[0].push_back(0);
    closure[1].push_back(order == 0 ? 0 : 1);
    closure[0].type = MSH_PNT;
    closure[1].type = MSH_PNT;
  }

  void generate1dVertexClosureFull(nodalBasis::clCont &closure,
                                   std::vector<int> &closureRef, int order)
  {
    closure.clear();
    closure.resize(2);
    closure[0].push_back(0);
    if (order != 0) {
      closure[0].push_back(1);
      closure[1].push_back(1);
    }
    closure[1].push_back(0);
    for (int i = 0; i < order - 1; i++) {
      closure[0].push_back(2 + i);
      closure[1].push_back(2 + order - 2 - i);
    }
    closureRef.resize(2);
    closureRef[0] = 0;
    closureRef[1] = 0;
  }

  void getFaceClosureTet(int iFace, int iSign, int iRotate,
                         nodalBasis::closure &closure, int order)
  {
    closure.clear();
    closure.resize((order + 1) * (order + 2) / 2);
    closure.type = ElementType::getTag(TYPE_TRI, order, false);

    switch (order){
    case 0:
      closure[0] = 0;
      break;
    default:
      int face[4][3] = {{-3, -2, -1}, {1, -6, 4}, {-4, 5, 3}, {6, 2, -5}};
      int order1node[4][3] = {{0, 2, 1}, {0, 1, 3}, {0, 3, 2}, {3, 1, 2}};
      for (int i = 0; i < 3; ++i){
        int k = (3 + (iSign * i) + iRotate) % 3;  //- iSign * iRotate
        closure[i] = order1node[iFace][k];
      }
      for (int i = 0;i < 3; ++i){
        int edgenumber = iSign *
          face[iFace][(6 + i * iSign + (-1 + iSign) / 2 + iRotate) % 3];  //- iSign * iRotate
        for (int k = 0; k < (order - 1); k++){
          if (edgenumber > 0)
            closure[3 + i * (order - 1) + k] =
              4 + (edgenumber - 1) * (order - 1) + k;
          else
            closure[3 + i * (order - 1) + k] =
              4 + (-edgenumber) * (order - 1) - 1 - k;
        }
      }
      int fi = 3 + 3 * (order - 1);
      int ti = 4 + 6 * (order - 1);
      int ndofff = (order - 3 + 2) * (order - 3 + 1) / 2;
      ti = ti + iFace * ndofff;
      for (int k = 0; k < order / 3; k++){
        int orderint = order - 3 - k * 3;
        if (orderint > 0){
          for (int ci = 0; ci < 3 ; ci++){
            int  shift = (3 + iSign * ci + iRotate) % 3;  //- iSign * iRotate
            closure[fi + ci] = ti + shift;
          }
          fi = fi + 3; ti = ti + 3;
          for (int l = 0; l < orderint - 1; l++){
            for (int ei = 0; ei < 3; ei++){
              int edgenumber = (6 + ei * iSign + (-1 + iSign) / 2 + iRotate) % 3;
                       //- iSign * iRotate
              if (iSign > 0)
                closure[fi + ei * (orderint - 1) + l] =
                  ti + edgenumber * (orderint - 1) + l;
              else
                closure[fi + ei * (orderint - 1) + l] =
                  ti + (1 + edgenumber) * (orderint - 1) - 1 - l;
            }
          }
          fi = fi + 3 * (orderint - 1); ti = ti + 3 * (orderint - 1);
        }
        else {
          closure[fi] = ti;
          ti++;
          fi++;
        }
      }
      break;
    }
  }

  void generate2dEdgeClosureFull(nodalBasis::clCont &closure,
                                 std::vector<int> &closureRef,
                                 int order, int nNod, bool serendip)
  {
    closure.clear();
    closure.resize(2*nNod);
    closureRef.resize(2*nNod);
    int shift = 0;
    for (int corder = order; corder>=0; corder -= (nNod == 3 ? 3 : 2)) {
      if (corder == 0) {
        for (int r = 0; r < nNod ; r++){
          closure[r].push_back(shift);
          closure[r+nNod].push_back(shift);
        }
        break;
      }
      for (int r = 0; r < nNod ; r++){
        for (int j = 0; j < nNod; j++) {
          closure[r].push_back(shift + (r + j) % nNod);
          closure[r + nNod].push_back(shift + (r - j + 1 + nNod) % nNod);
        }
      }
      shift += nNod;
      int n = nNod*(corder-1);
      for (int r = 0; r < nNod ; r++){
        for (int j = 0; j < n; j++){
          closure[r].push_back(shift + (j + (corder - 1) * r) % n);
          closure[r + nNod].push_back(shift + (n - j - 1 + (corder - 1) * (r + 1)) % n);
        }
      }
      shift += n;
      if (serendip) break;
    }
    for (int r = 0; r < nNod*2 ; r++) {
      closure[r].type = ElementType::getTag(TYPE_LIN, order);
      closureRef[r] = 0;
    }
  }

  void addEdgeNodes(nodalBasis::clCont &closureFull, const int *edges, int order)
  {
    if (order < 2)
      return;
    int numNodes = 0;
    for (int i = 0; edges[i] >= 0; ++i) {
      numNodes = std::max(numNodes, edges[i] + 1);
    }
    std::vector<std::vector<int> > nodes2edges(numNodes, std::vector<int>(numNodes, -1));
    for (int i = 0; edges[i] >= 0; i += 2) {
      nodes2edges[edges[i]][edges[i + 1]] = i;
      nodes2edges[edges[i + 1]][edges[i]] = i + 1;
    }
    for (unsigned int iClosure = 0; iClosure < closureFull.size(); iClosure++) {
      std::vector<int> &cl = closureFull[iClosure];
      for (int iEdge = 0; edges[iEdge] >= 0; iEdge += 2) {
        if (cl.empty())
          continue;
        int n0 = cl[edges[iEdge]];
        int n1 = cl[edges[iEdge + 1]];
        int oEdge = nodes2edges[n0][n1];
        if (oEdge == -1)
          Msg::Error("invalid p1 closure or invalid edges list");
        for (int i = 0 ; i < order - 1; i++)
          cl.push_back(numNodes + oEdge/2 * (order - 1) + ((oEdge % 2) ?  order - 2 - i : i));
      }
    }
  }

  void generateFaceClosureTet(nodalBasis::clCont &closure, int order)
  {
    closure.clear();
    for (int iRotate = 0; iRotate < 3; iRotate++){
      for (int iSign = 1; iSign >= -1; iSign -= 2){
        for (int iFace = 0; iFace < 4; iFace++){
          nodalBasis::closure closure_face;
          getFaceClosureTet(iFace, iSign, iRotate, closure_face, order);
          closure.push_back(closure_face);
        }
      }
    }
  }

  void generateFaceClosureTetFull(nodalBasis::clCont &closureFull,
                                  std::vector<int> &closureRef,
                                  int order, bool serendip)
  {
    closureFull.clear();
    //input :
    static const short int faces[4][3] = {{0,1,2}, {0,3,1}, {0,2,3}, {3,2,1}};
    static const int edges[] = {0, 1,  1, 2,  2, 0,  3, 0,  3, 2,  3, 1,  -1};
    static const int faceOrientation[6] = {0,1,2,5,3,4};
    closureFull.resize(24);
    closureRef.resize(24);
    for (int i = 0; i < 24; i ++)
      closureRef[i] = 0;
    if (order == 0) {
      for (int i = 0; i < 24; i ++) {
        closureFull[i].push_back(0);
      }
      return;
    }
    //Mapping for the p1 nodes
    nodalBasis::clCont closure;
    generateFaceClosureTet(closure, 1);
    for (unsigned int i = 0; i < closureFull.size(); i++) {
      std::vector<int> &clFull = closureFull[i];
      std::vector<int> &cl = closure[i];
      clFull.resize(4, -1);
      closureRef[i] = 0;
      for (unsigned int j = 0; j < cl.size(); j ++)
        clFull[closure[0][j]] = cl[j];
      for (int j = 0; j < 4; j ++)
        if (clFull[j] == -1)
          clFull[j] = (6 - clFull[(j + 1) % 4] - clFull[(j + 2) % 4] - clFull[(j + 3) % 4]);
    }
    int nodes2Faces[4][4][4];
    for (int i = 0; i < 4; i++) {
      for (int iRotate = 0; iRotate < 3; iRotate ++) {
        short int n0 = faces[i][(3 - iRotate) % 3];
        short int n1 = faces[i][(4 - iRotate) % 3];
        short int n2 = faces[i][(5 - iRotate) % 3];
        nodes2Faces[n0][n1][n2] = i * 6 + iRotate;
        nodes2Faces[n0][n2][n1] = i * 6 + iRotate + 3;
      }
    }
    nodalBasis::clCont closureTriangles;
    std::vector<int> closureTrianglesRef;
    if (order >= 3)
      generate2dEdgeClosureFull(closureTriangles, closureTrianglesRef, order - 3, 3, false);
    addEdgeNodes(closureFull, edges, order);
    for (unsigned int iClosure = 0; iClosure < closureFull.size(); iClosure++) {
      //faces
      std::vector<int> &cl = closureFull[iClosure];
      if (order >= 3) {
        for (int iFace = 0; iFace < 4; iFace ++) {
          int n0 = cl[faces[iFace][0]];
          int n1 = cl[faces[iFace][1]];
          int n2 = cl[faces[iFace][2]];
          short int id = nodes2Faces[n0][n1][n2];
          short int iTriClosure = faceOrientation [id % 6];
          short int idFace = id/6;
          int nOnFace = closureTriangles[iTriClosure].size();
          for (int i = 0; i < nOnFace; i++) {
            cl.push_back(4 + 6 * (order - 1) + idFace * nOnFace +
                         closureTriangles[iTriClosure][i]);
          }
        }
      }
    }
    if (order >= 4 && !serendip) {
      nodalBasis::clCont insideClosure;
      std::vector<int> fakeClosureRef;
      generateFaceClosureTetFull(insideClosure, fakeClosureRef, order - 4, false);
      for (unsigned int i = 0; i < closureFull.size(); i ++) {
        unsigned int shift = closureFull[i].size();
        for (unsigned int j = 0; j < insideClosure[i].size(); j++)
          closureFull[i].push_back(insideClosure[i][j] + shift);
      }
    }
  }

  /*
  void checkClosure(int tag) {
    printf("TYPE = %i\n", tag);
    const nodalBasis &fs = *nodalBases::find(tag);
    for (int i = 0; i < fs.closures.size(); ++i) {
      const std::vector<int> &clRef = fs.closures[fs.closureRef[i]];
      const std::vector<int> &cl = fs.closures[i];
      const std::vector<int> &clFull = fs.fullClosures[i];
      printf("i = %i\n", i);
      for (int j = 0; j < cl.size(); ++j) {
        printf("%3i ", clFull[clRef[j]]);
      }
      printf("\n");
      for (int j = 0; j < cl.size(); ++j) {
        printf("%3i ", cl[j]);
      }
      printf("\n");
    }
  }
  */

  void generateFaceClosureHex(nodalBasis::clCont &closure, int order,
                              bool serendip, const fullMatrix<double> &points)
  {
    closure.clear();
    const nodalBasis &fsFace = *BasisFactory::getNodalBasis
      (ElementType::getTag(TYPE_QUA, order, serendip));
    for (int iRotate = 0; iRotate < 4; iRotate++){
      for (int iSign = 1; iSign >= -1; iSign -= 2){
        for (int iFace = 0; iFace < 6; iFace++) {
          nodalBasis::closure cl;
          cl.type = fsFace.type;
          cl.resize(fsFace.points.size1());
          for (unsigned int iNode = 0; iNode < cl.size(); ++iNode) {
            double u,v,w;
            rotateHex(iFace, iRotate, iSign, fsFace.points(iNode, 0),
                      fsFace.points(iNode, 1), u, v, w);
            cl[iNode] = 0;
            double D = std::numeric_limits<double>::max();
            for (int jNode = 0; jNode < points.size1(); ++jNode) {
              double d = pow2(points(jNode, 0) - u) + pow2(points(jNode, 1) - v) +
                pow2(points(jNode, 2) - w);
              if (d < D) {
                cl[iNode] = jNode;
                D = d;
              }
            }
          }
          closure.push_back(cl);
        }
      }
    }
  }

  void generateFaceClosureHexFull(nodalBasis::clCont &closure,
                                  std::vector<int> &closureRef,
                                  int order, bool serendip,
                                  const fullMatrix<double> &points)
  {
    closure.clear();
    int clId = 0;
    for (int iRotate = 0; iRotate < 4; iRotate++){
      for (int iSign = 1; iSign >= -1; iSign -= 2){
        for (int iFace = 0; iFace < 6; iFace++) {
          nodalBasis::closure cl;
          cl.resize(points.size1());
          for (int iNode = 0; iNode < points.size1(); ++iNode) {
            double u,v,w;
            rotateHexFull(iFace, iRotate, iSign, points(iNode, 0), points(iNode, 1),
                          points(iNode, 2), u, v, w);
            int J = 0;
            double D = std::numeric_limits<double>::max();
            for (int jNode = 0; jNode < points.size1(); ++jNode) {
              double d = pow2(points(jNode, 0) - u) + pow2(points(jNode, 1) - v) +
                pow2(points(jNode, 2) - w);
              if (d < D) {
                J = jNode;
                D = d;
              }
            }
            cl[J] = iNode;
          }
          closure.push_back(cl);
          closureRef.push_back(0);
          clId ++;
        }
      }
    }
  }

  void getFaceClosurePrism(int iFace, int iSign, int iRotate,
                           nodalBasis::closure &closure, int order)
  {
    //if (order > 2)
    //  Msg::Error("FaceClosure not implemented for prisms of order %d",order);
    bool isTriangle = iFace<2;
    int nNodes = isTriangle ? (order+1)*(order+2)/2 : (order+1)*(order+1);
    closure.clear();
    if (isTriangle && iRotate > 2) return;
    closure.resize(nNodes);
    if (order==0) {
      closure[0] = 0;
      return;
    }
    int order1node[5][4] = {{0, 2, 1, -1}, {3, 4, 5, -1}, {0, 1, 4, 3}, {0, 3, 5, 2},
                            {1, 2, 5, 4}};
    int order2node[5][5] = {{7, 9, 6, -1, -1}, {12, 14, 13, -1, -1}, {6, 10, 12, 8, 15},
                            {8, 13, 11, 7, 16}, {9, 11, 14, 10, 17}};
    // int order2node[5][4] = {{7, 9, 6, -1}, {12, 14, 13, -1}, {6, 10, 12, 8},
    //                         {8, 13, 11, 7}, {9, 11, 14, 10}};
    int nVertex = isTriangle ? 3 : 4;
    closure.type = ElementType::getTag(isTriangle ? TYPE_TRI : TYPE_QUA, order);
    for (int i = 0; i < nVertex; ++i){
      int k = (nVertex + (iSign * i) + iRotate) % nVertex;  //- iSign * iRotate
      closure[i] = order1node[iFace][k];
    }
    if (order==2) {
      for (int i = 0; i < nVertex; ++i){
        int k = (nVertex + (iSign==-1?-1:0) + (iSign * i) + iRotate) % nVertex;
                  //- iSign * iRotate
        closure[nVertex+i] = order2node[iFace][k];
      }
      if (!isTriangle)
        closure[nNodes-1] = order2node[iFace][4]; // center
    }
  }

  void generateFaceClosurePrism(nodalBasis::clCont &closure, int order)
  {
    if (order > 2)
      Msg::Error("FaceClosure not implemented for prisms of order %d",order);
    closure.clear();
    for (int iRotate = 0; iRotate < 4; iRotate++){
      for (int iSign = 1; iSign >= -1; iSign -= 2){
        for (int iFace = 0; iFace < 5; iFace++){
          nodalBasis::closure closure_face;
          getFaceClosurePrism(iFace, iSign, iRotate, closure_face, order);
          closure.push_back(closure_face);
        }
      }
    }
  }

  void generateFaceClosurePrismFull(nodalBasis::clCont &closureFull,
                                    std::vector<int> &closureRef, int order)
  {
    nodalBasis::clCont closure;
    closureFull.clear();
    closureFull.resize(40);
    closureRef.resize(40);
    generateFaceClosurePrism(closure, 1);
    int ref3 = -1, ref4a = -1, ref4b = -1;
    for (unsigned int i = 0; i < closure.size(); i++) {
      std::vector<int> &clFull = closureFull[i];
      std::vector<int> &cl = closure[i];
      if (cl.size() == 0)
        continue;
      clFull.resize(6, -1);
      int &ref = cl.size() == 3 ? ref3 : (cl[0] / 3 + cl[1] / 3) % 2 ? ref4b : ref4a;
      if (ref == -1)
        ref = i;
      closureRef[i] = ref;
      for (unsigned int j = 0; j < cl.size(); j ++)
        clFull[closure[ref][j]] = cl[j];
      for (int j = 0; j < 6; j ++) {
        if (clFull[j] == -1) {
          int k = ((j / 3) + 1) % 2 * 3;
          int sum = (clFull[k + (j + 1) % 3] + clFull[k + (j + 2) % 3]);
          clFull[j] = ((sum / 6 + 1) % 2) * 3 + (12 - sum) % 3;
        }
      }
    }
    static const int edges[] = {0, 1,  0, 2,  0, 3,  1, 2,  1, 4,  2, 5,
                                3, 4,  3, 5,  4, 5,  -1};
    addEdgeNodes(closureFull, edges, order);
    if ( order < 2 )
      return;
    // face center nodes for p2 prism
    static const int faces[5][4] = {{0, 2, 1, -1}, {3, 4, 5, -1}, {0, 1, 4,  3},
                                    {0, 3, 5,  2}, {1, 2, 5,  4}};

    if ( order == 2 ) {
      int nextFaceNode = 15;
      int numFaces = 5;
      int numFaceNodes = 4;
      std::map<int,int> nodeSum2Face;
      for (int iFace = 0; iFace < numFaces ; iFace ++) {
        int nodeSum = 0;
        for (int iNode = 0; iNode < numFaceNodes; iNode++ ) {
          nodeSum += faces[iFace][iNode];
        }
        nodeSum2Face[nodeSum] = iFace;
      }
      for (unsigned int i = 0; i < closureFull.size(); i++ ) {
        if (closureFull[i].empty())
          continue;
        for (int iFace = 0; iFace < numFaces; iFace++ ) {
          int nodeSum = 0;
          for (int iNode = 0; iNode < numFaceNodes; iNode++)
            nodeSum += faces[iFace][iNode] < 0 ? faces[iFace][iNode] :
              closureFull[i][ faces[iFace][iNode] ];
          std::map<int,int>::iterator it = nodeSum2Face.find(nodeSum);
          if (it == nodeSum2Face.end() )
            Msg::Error("Prism face not found");
          int mappedFaceId = it->second;
          if ( mappedFaceId > 1) {
            closureFull[i].push_back(nextFaceNode + mappedFaceId - 2);
          }
        }
      }
    } else {
      Msg::Error("FaceClosureFull not implemented for prisms of order %d",order);
    }

  }

  void generate2dEdgeClosure(nodalBasis::clCont &closure, int order, int nNod = 3)
  {
    closure.clear();
    closure.resize(2*nNod);
    for (int j = 0; j < nNod ; j++){
      closure[j].push_back(j);
      closure[j].push_back((j+1)%nNod);
      closure[nNod+j].push_back((j+1)%nNod);
      closure[nNod+j].push_back(j);
      for (int i=0; i < order-1; i++){
        closure[j].push_back( nNod + (order-1)*j + i );
        closure[nNod+j].push_back(nNod + (order-1)*(j+1) -i -1);
      }
      closure[j].type = closure[nNod+j].type = ElementType::getTag(TYPE_LIN, order);
    }
  }

  void generateClosureOrder0(nodalBasis::clCont &closure, int nb)
  {
    closure.clear();
    closure.resize(nb);
    for (int i=0; i < nb; i++) {
      closure[i].push_back(0);
      closure[i].type = MSH_PNT;
    }
  }
}

nodalBasis::nodalBasis(int tag)
{
  using namespace ClosureGen;
  type = tag;
  parentType = ElementType::ParentTypeFromTag(tag);
  order = ElementType::OrderFromTag(tag);
  serendip = ElementType::SerendipityFromTag(tag) > 1;
  dimension = ElementType::DimensionFromTag(tag);

  switch (parentType) {
  case TYPE_PNT :
    numFaces = 1;
    points = gmshGeneratePointsLine(0);
    break;
  case TYPE_LIN :
    numFaces = 2;
    points = gmshGeneratePointsLine(order);
    generate1dVertexClosure(closures, order);
    generate1dVertexClosureFull(fullClosures, closureRef, order);
    break;
  case TYPE_TRI :
    numFaces = 3;
    points = gmshGeneratePointsTriangle(order, serendip);
    if (order == 0) {
      generateClosureOrder0(closures, 6);
      generateClosureOrder0(fullClosures, 6);
      closureRef.resize(6, 0);
    }
    else {
      generate2dEdgeClosure(closures, order);
      generate2dEdgeClosureFull(fullClosures, closureRef, order, 3, serendip);
    }
    break;
  case TYPE_QUA :
    numFaces = 4;
    points = gmshGeneratePointsQuadrangle(order, serendip);
    if (order == 0) {
      generateClosureOrder0(closures, 8);
      generateClosureOrder0(fullClosures, 8);
      closureRef.resize(8, 0);
    }
    else {
      generate2dEdgeClosure(closures, order, 4);
      generate2dEdgeClosureFull(fullClosures, closureRef, order, 4, serendip);
    }
    break;
  case TYPE_TET :
    numFaces = 4;
    points = gmshGeneratePointsTetrahedron(order, serendip);
    if (order == 0) {
      generateClosureOrder0(closures,24);
      generateClosureOrder0(fullClosures, 24);
      closureRef.resize(24, 0);
    }
    else {
      generateFaceClosureTet(closures, order);
      generateFaceClosureTetFull(fullClosures, closureRef, order, serendip);
    }
    break;
  case TYPE_PRI :
    numFaces = 5;
    points = gmshGeneratePointsPrism(order, serendip);
    if (order == 0) {
      generateClosureOrder0(closures,48);
      generateClosureOrder0(fullClosures,48);
      closureRef.resize(48, 0);
    }
    else {
      generateFaceClosurePrism(closures, order);
      generateFaceClosurePrismFull(fullClosures, closureRef, order);
    }
    break;
  case TYPE_HEX :
    numFaces = 6;
    points = gmshGeneratePointsHexahedron(order, serendip);
    generateFaceClosureHex(closures, order, serendip, points);
    generateFaceClosureHexFull(fullClosures, closureRef, order, serendip, points);
    break;
  case TYPE_PYR :
    numFaces = 5;
    points = gmshGeneratePointsPyramid(order, serendip);
    break;
  }

}

void nodalBasis::getReferenceNodesForBezier(fullMatrix<double> &nodes) const
{
  if (parentType != TYPE_PYR && !serendip) {
    nodes = points;
  }
  else {
    const bool serendipSpace = false;
    if (parentType != TYPE_PYR) {
      FuncSpaceData data(true, type, order, &serendipSpace);
      gmshGeneratePoints(data, nodes);
    }
    else {
      FuncSpaceData data(true, type, false, order, order, &serendipSpace);
      gmshGeneratePoints(data, nodes);
    }
  }
}
