/*******************************************************************************
 * Copyright (c) 2020 CEA
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
	 * Contributors: see AUTHORS file
 *******************************************************************************/
#include "mesh/CartesianMesh2D.h"
#include <stdexcept>
#include <sstream>

namespace nablalib
{

CartesianMesh2D::CartesianMesh2D(
  MeshGeometry<2>* geometry, const vector<Id>& inner_nodes_ids,
  const vector<Id>& top_nodes_ids, const vector<Id>& bottom_nodes_ids,
  const vector<Id>& left_nodes_ids, const vector<Id>& right_nodes_ids,
  const Id top_left_node_id, const Id top_right_node_id,
  const Id bottom_left_node_id, const Id bottom_right_node_id)
: m_geometry(geometry)
, m_inner_nodes(inner_nodes_ids)
, m_top_nodes(top_nodes_ids)
, m_bottom_nodes(bottom_nodes_ids)
, m_left_nodes(left_nodes_ids)
, m_right_nodes(right_nodes_ids)
, m_top_left_node(top_left_node_id)
, m_top_right_node(top_right_node_id)
, m_bottom_left_node(bottom_left_node_id)
, m_bottom_right_node(bottom_right_node_id)
, m_top_faces(0)
, m_bottom_faces(0)
, m_left_faces(0)
, m_right_faces(0)
, m_nb_x_quads(bottom_nodes_ids.size() - 1)
, m_nb_y_quads(left_nodes_ids.size() - 1)
{
  // outer faces
	auto edges = m_geometry->getEdges();
	for (size_t edgeId(0); edgeId < edges.size(); ++edgeId) {
    m_faces.emplace_back(edgeId);
		if (!isInnerEdge(edges[edgeId])) {
			m_outer_faces.emplace_back(edgeId);
		} else {
      m_inner_faces.emplace_back(edgeId);
			if (isInnerVerticalEdge(edges[edgeId])) {
				m_inner_vertical_faces.emplace_back(edgeId);
			}	else if (isInnerHorizontalEdge(edges[edgeId])) {
				m_inner_horizontal_faces.emplace_back(edgeId);
			}	else {
        stringstream msg;
        msg << "The inner edge " << edgeId << " should be either vertical or horizontal" << endl;
        throw runtime_error(msg.str());
			}
		}
	}

	// Construction of boundary cell sets
	m_top_cells = cellsOfNodeCollection(m_top_nodes);
	m_bottom_cells = cellsOfNodeCollection(m_bottom_nodes);
	m_left_cells = cellsOfNodeCollection(m_left_nodes);
	m_right_cells = cellsOfNodeCollection(m_right_nodes);

	// Construction of boundary cell faces
	for(auto&& cellId: m_top_cells)    m_top_faces.emplace_back(getTopFaceOfCell(cellId));
	for(auto&& cellId: m_bottom_cells) m_bottom_faces.emplace_back(getBottomFaceOfCell(cellId));
	for(auto&& cellId: m_left_cells)   m_left_faces.emplace_back(getLeftFaceOfCell(cellId));
	for(auto&& cellId: m_right_cells)  m_right_faces.emplace_back(getRightFaceOfCell(cellId));
}

const array<Id, 4>&
CartesianMesh2D::getNodesOfCell(const Id& cellId) const noexcept
{
	return m_geometry->getQuads()[cellId].getNodeIds();
}

const array<Id, 2>&
CartesianMesh2D::getNodesOfFace(const Id& faceId) const noexcept
{
	return m_geometry->getEdges()[faceId].getNodeIds();
}

Id
CartesianMesh2D::getFirstNodeOfFace(const Id& faceId) const noexcept
{
	return m_geometry->getEdges()[faceId].getNodeIds()[0];
}

Id
CartesianMesh2D::getSecondNodeOfFace(const Id& faceId) const noexcept
{
	return m_geometry->getEdges()[faceId].getNodeIds()[1];
}

vector<Id>
CartesianMesh2D::getCellsOfNode(const Id& nodeId) const noexcept
{
	vector<Id> cells;
	size_t i,j;
  tie(i, j) = id2IndexNode(nodeId);
	if (i < m_nb_y_quads && j < m_nb_x_quads) cells.emplace_back(index2IdCell(i, j));
	if (i < m_nb_y_quads && j > 0)            cells.emplace_back(index2IdCell(i, j-1));
	if (i > 0            && j < m_nb_x_quads) cells.emplace_back(index2IdCell(i-1, j));
	if (i > 0            && j > 0)            cells.emplace_back(index2IdCell(i-1, j-1));
  return cells;
}

vector<Id>
CartesianMesh2D::getCellsOfFace(const Id& faceId) const
{
	vector<Id> cells;	
	size_t i_f = static_cast<size_t>(faceId) / (2 * m_nb_x_quads + 1);
	size_t k_f = static_cast<size_t>(faceId) - i_f * (2 * m_nb_x_quads + 1);
	
	if (i_f < m_nb_y_quads) {  // all except upper bound faces
	  if (k_f == 2 * m_nb_x_quads) {  // right bound edge 
	    cells.emplace_back(index2IdCell(i_f, m_nb_x_quads-1));
	  }
    else if (k_f == 1) {  // left bound edge
	    cells.emplace_back(index2IdCell(i_f, 0));
	  }
	  else if (k_f % 2 == 0) {  // horizontal edge
	    cells.emplace_back(index2IdCell(i_f, k_f/2));
	    if (i_f > 0)  // Not bottom bound edge
	      cells.emplace_back(index2IdCell(i_f-1, k_f/2));
	  }
    else {  // vertical edge (neither left bound nor right bound)
	    cells.emplace_back(index2IdCell(i_f, (k_f-1)/2 - 1));
	    cells.emplace_back(index2IdCell(i_f, (k_f-1)/2));
	  }
	} else {  // upper bound faces
	  cells.emplace_back(index2IdCell(i_f-1, k_f));
	}
  return cells;
  /*
   * Old code just in case there are still flaws in new code...
   * 
  std::vector<Id> cellsOfFace;
	const auto& nodes(getNodesOfFace(faceId));
	for (auto nodeId : nodes)
	{
		auto adjacentCells(getCellsOfNode(nodeId));
		for(auto quadId : adjacentCells)
			if (getNbCommonIds(nodes, m_geometry->getQuads()[quadId].getNodeIds()) == 2)
				cellsOfFace.emplace_back(quadId);
	}
	std::sort(cellsOfFace.begin(), cellsOfFace.end());
	cellsOfFace.erase(std::unique(cellsOfFace.begin(), cellsOfFace.end()), cellsOfFace.end());
	return cellsOfFace;
  */
}

vector<Id>
CartesianMesh2D::getNeighbourCells(const Id& cellId) const
{
  std::vector<Id> neighbours;
	size_t i,j;
	tie(i, j) = id2IndexCell(cellId);
	if (i >= 1) neighbours.emplace_back(index2IdCell(i-1, j));
	if (i < m_nb_y_quads-1) neighbours.emplace_back(index2IdCell(i+1, j));
	if (j >= 1) neighbours.emplace_back(index2IdCell(i, j-1));
	if (j < m_nb_x_quads-1) neighbours.emplace_back(index2IdCell(i, j+1));
  return neighbours;
}

vector<Id>
CartesianMesh2D::getFacesOfCell(const Id& cellId) const
{
	size_t i,j;
	tie(i, j) = id2IndexCell(cellId);
  Id bottom_face(static_cast<Id>(2 * j + i * (2 * m_nb_x_quads + 1)));
	Id left_face(bottom_face + 1);
	Id right_face(bottom_face + static_cast<Id>(j == m_nb_x_quads-1 ? 2 : 3));
	Id top_face(bottom_face + static_cast<Id>(i < m_nb_y_quads-1 ? 2 * m_nb_x_quads + 1 : 2 * m_nb_x_quads + 1 - j));
	return vector<Id>({bottom_face, left_face, right_face, top_face});
}

Id
CartesianMesh2D::getCommonFace(const Id& cellId1, const Id& cellId2) const
{
	auto cell1Faces{getFacesOfCell(cellId1)};
	auto cell2Faces{getFacesOfCell(cellId2)};
	auto result = find_first_of(cell1Faces.begin(), cell1Faces.end(), cell2Faces.begin(), cell2Faces.end());
	if (result == cell1Faces.end()) {
    stringstream msg;
    msg << "No common faces found between cell " << cellId1 << " and cell " << cellId2 << endl;
	  throw runtime_error(msg.str());
	} else {
	  return *result;
  }
}

// Id
// CartesianMesh2D::getBackCell(const Id& faceId) const
// {
//   vector<Id> cells(move(getCellsOfFace(faceId)));
//   if (cells.size() < 2) {
//     stringstream msg;
//     msg << "Error in getBackCell(" << faceId << "): please consider using this method with inner face only." << endl;
// 	  throw runtime_error(msg.str());
//   } else {
//     return cells[0];
//   }
// }

// Id
// CartesianMesh2D::getFrontCell(const Id& faceId) const
// {
//   vector<Id> cells(move(getCellsOfFace(faceId)));
//   if (cells.size() < 2) {
//     stringstream msg;
//     msg << "Error in getFrontCell(" << faceId << "): please consider using this method with inner face only." << endl;
// 	  throw runtime_error(msg.str());
//   } else {
//     return cells[1];
//   }
// }
Id
CartesianMesh2D::getBackCell(const Id& faceId) const 
{
  vector<Id> cells = (move(getCellsOfFace(faceId)));
  if (cells.size() >= 1)
    return cells[0];
  else
    return -1;
}

Id
CartesianMesh2D::getFrontCell(const Id& faceId) const 
{
  vector<Id> cells = (move(getCellsOfFace(faceId)));
  if (cells.size() >= 2)
    return cells[1];
  else
    return -1;
}

size_t
CartesianMesh2D::getNbCommonIds(const vector<Id>& as, const vector<Id>& bs) const noexcept
{
	size_t nbCommonIds = 0;
	for (auto a : as)
		for (auto b : bs)
			if (a == b) nbCommonIds++;
	return nbCommonIds;
}

bool
CartesianMesh2D::isInnerEdge(const Edge& e) const noexcept
{
	return (find(m_inner_nodes.begin(), m_inner_nodes.end(), e.getNodeIds()[0]) != m_inner_nodes.end()) ||
	       (find(m_inner_nodes.begin(), m_inner_nodes.end(), e.getNodeIds()[1]) != m_inner_nodes.end());
}

bool
CartesianMesh2D::isInnerVerticalEdge(const Edge& e) const noexcept
{
	if (!isInnerEdge(e)) return false;
	return (e.getNodeIds()[0] == e.getNodeIds()[1] + m_nb_x_quads + 1 ||
			    e.getNodeIds()[1] == e.getNodeIds()[0] + m_nb_x_quads + 1);
}

bool
CartesianMesh2D::isInnerHorizontalEdge(const Edge& e) const noexcept
{
	if (!isInnerEdge(e)) return false;
	return (e.getNodeIds()[0] == e.getNodeIds()[1] + 1 ||
			    e.getNodeIds()[1] == e.getNodeIds()[0] + 1);
}

Id
CartesianMesh2D::getBottomFaceOfCell(const Id& cellId) const noexcept
{
  size_t i,j;
  tie(i, j) = id2IndexCell(cellId);
  Id bottom_face(static_cast<Id>(2 * j + i * (2 * m_nb_x_quads + 1)));
  return bottom_face;
}

Id
CartesianMesh2D::getLeftFaceOfCell(const Id& cellId) const noexcept
{
  Id left_face(getBottomFaceOfCell(cellId) + 1);
  return left_face;
}

Id
CartesianMesh2D::getRightFaceOfCell(const Id& cellId) const noexcept
{
  size_t i,j;
  tie(i, j) = id2IndexCell(cellId);
  Id bottom_face(static_cast<Id>(2 * j + i * (2 * m_nb_x_quads + 1)));
  Id right_face(bottom_face + static_cast<Id>(j == m_nb_x_quads - 1 ? 2 : 3));
  return right_face;
}

Id
CartesianMesh2D::getTopFaceOfCell(const Id& cellId) const noexcept
{
  size_t i,j;
  tie(i, j) = id2IndexCell(cellId);
  Id bottom_face(static_cast<Id>(2 * j + i * (2 * m_nb_x_quads + 1)));
  Id top_face(bottom_face + static_cast<Id>(i < m_nb_y_quads - 1 ? 2 * m_nb_x_quads + 1 : 2 * m_nb_x_quads + 1 - j));
  return top_face;
}

inline Id
CartesianMesh2D::index2IdCell(const size_t& i, const size_t& j) const noexcept
{
  return static_cast<Id>(i * m_nb_x_quads + j);
}
  
inline pair<size_t, size_t>
CartesianMesh2D::id2IndexCell(const Id& k) const noexcept
{
  size_t i(static_cast<size_t>(k) / m_nb_x_quads);
  size_t j(static_cast<size_t>(k) - i * m_nb_x_quads);
  return make_pair(i, j);
}

inline Id
CartesianMesh2D::index2IdNode(const size_t& i, const size_t&j) const noexcept
{
  return static_cast<Id>(i * (m_nb_x_quads + 1) + j);
}
  
inline pair<size_t, size_t>
CartesianMesh2D::id2IndexNode(const Id& k) const noexcept
{
  size_t i(static_cast<size_t>(k) / (m_nb_x_quads + 1));
  size_t j(static_cast<size_t>(k) - i * (m_nb_x_quads + 1));
  return make_pair(i, j); 
}


inline vector<Id>
CartesianMesh2D::cellsOfNodeCollection(const vector<Id>& nodes)
{
  vector<Id> cells(0);
  for (auto&& node_id : nodes)
    for (auto&& cell_id : getCellsOfNode(node_id))
      cells.emplace_back(cell_id);
  // Deleting duplicates
  std::sort(cells.begin(), cells.end());
  cells.erase(std::unique(cells.begin(), cells.end()), cells.end());
  return cells;
}
Id CartesianMesh2D::getLeftNode(const int node) const noexcept
{
   size_t i,j;
   tie(i,j) = id2IndexNode(node);
   if (j!=0)
      return index2IdNode(i,j-1);
   else return -1;
}
Id CartesianMesh2D::getRightNode(const int node) const noexcept
{
   size_t i,j;
   tie(i,j) = id2IndexNode(node); 
   if (j!=(m_nb_x_quads)) {
     return index2IdNode(i,j+1);
   } else return -1;
   
}
Id
CartesianMesh2D::getBottomNode(const int node) const noexcept
{
   size_t i,j;
   tie(i,j) = id2IndexNode(node);
   if (i!=0)
     return index2IdNode(i-1,j);
   else return -1;
}
Id CartesianMesh2D::getTopNode(const int node) const noexcept
{
   size_t i,j;
   tie(i,j) = id2IndexNode(node);
   if (i!=(m_nb_y_quads))
     return index2IdNode(i+1,j);
   else return -1;
   
}
Id CartesianMesh2D::getTopCellfromBottom(const int cell) const noexcept
{
   size_t i,j;
   tie(i,j) = id2IndexCell(cell);
   if (i == 0)
     return index2IdCell((m_nb_y_quads-1),j);
   else
     return -1;   
}
Id CartesianMesh2D::getBottomCellfromTop(const int cell) const noexcept
{
   size_t i,j;
   tie(i,j) = id2IndexCell(cell);
   if (i == (m_nb_y_quads-1))
     return index2IdCell(0,j);
   else
     return -1;   
}
Id CartesianMesh2D::getLeftCellfromRight(const int cell) const noexcept
{
   size_t i,j;
   tie(i,j) = id2IndexCell(cell);
   if (j == (m_nb_x_quads-1))
     return index2IdCell(i,0);
   else
     return -1;   
}
Id CartesianMesh2D::getRightCellfromLeft(const int cell) const noexcept
{
   size_t i,j;
   tie(i,j) = id2IndexCell(cell);
   if (j == 0)
     return index2IdCell(i, (m_nb_x_quads-1));
   else
     return -1;   
}    
}
