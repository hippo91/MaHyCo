#include <Kokkos_Core.hpp>
#include "Vnr.h"               // for VnrRemap
#include "../includes/Freefunctions.h"
#include "mesh/CartesianMesh2D.h"   // for CartesianMesh2D
#include "utils/Utils.h"  // for Indexof

/**
 * Job updateVelocity called @2.0 in executeTimeLoopN method.
 * In variables: C, Q_nplus1, deltat_n, deltat_nplus1, m, p_n, u_n
 * Out variables: u_nplus1
 */
void Vnr::updateVelocityBoundaryConditions() noexcept
{
  const double dt(0.5 * (gt->deltat_nplus1 + gt->deltat_n));
  const auto bottomNodes(mesh->getBottomNodes());
  const size_t nbBottomNodes(bottomNodes.size());
  Kokkos::parallel_for(nbBottomNodes, KOKKOS_LAMBDA(const size_t& pBottomNodes)
    {
      const Id pId(bottomNodes[pBottomNodes]);
      const size_t pNodes(pId);
			     
      if (cdl->bottomBC == cdl->symmetry)
	{
	  RealArray1D<2> reduction1({0.0, 0.0});
	  {
	    const auto cellsOfNodeP(mesh->getCellsOfNode(pId));
	    const size_t nbCellsOfNodeP(cellsOfNodeP.size());
	    for (size_t cCellsOfNodeP=0; cCellsOfNodeP<nbCellsOfNodeP; cCellsOfNodeP++)
	      {
		const Id cId(cellsOfNodeP[cCellsOfNodeP]);
		const size_t cCells(cId);
		const size_t pNodesOfCellC(utils::indexOf(mesh->getNodesOfCell(cId), pId));
		reduction1 = sumR1(reduction1, (p_n(cCells) +
						Q_n(cCells)) * (
								C(cCells,pNodesOfCellC)
								+ symmetricVector(C(cCells,pNodesOfCellC), {1.0, 0.0})));
	      }
	  }
	  u_nplus1(pNodes) = u_n(pNodes) + dt / (m(pNodes)) * reduction1;
	}
      else if (cdl->bottomBC == cdl->imposedVelocity)
	{
	  u_nplus1(pNodes) = cdl->bottomBCValue;
	}
    });
  const auto topNodes(mesh->getTopNodes());
  const size_t nbTopNodes(topNodes.size());
  Kokkos::parallel_for(nbTopNodes, KOKKOS_LAMBDA(const size_t& pTopNodes)
    {
      const Id pId(topNodes[pTopNodes]);
      const size_t pNodes(pId);
      
      if (cdl->topBC == cdl->symmetry)
	{
	  RealArray1D<2> reduction2({0.0, 0.0});
	  {
	    const auto cellsOfNodeP(mesh->getCellsOfNode(pId));
	    const size_t nbCellsOfNodeP(cellsOfNodeP.size());
	    for (size_t cCellsOfNodeP=0; cCellsOfNodeP<nbCellsOfNodeP; cCellsOfNodeP++)
	      {
		const Id cId(cellsOfNodeP[cCellsOfNodeP]);
		const size_t cCells(cId);
		const size_t pNodesOfCellC(utils::indexOf(mesh->getNodesOfCell(cId), pId));
		reduction2 = sumR1(reduction2, (p_n(cCells) + Q_n(cCells)) * (
			     C(cCells,pNodesOfCellC)
			      + symmetricVector(C(cCells,pNodesOfCellC), {1.0, 0.0})));
	      }
	  }
	  u_nplus1(pNodes) = u_n(pNodes) + dt / (m(pNodes)) * reduction2;
	}
      else if (cdl->topBC == cdl->imposedVelocity)
	{
	  u_nplus1(pNodes) = cdl->topBCValue;
	}
    });
  const auto leftNodes(mesh->getLeftNodes());
  const size_t nbLeftNodes(leftNodes.size());
  Kokkos::parallel_for(nbLeftNodes, KOKKOS_LAMBDA(const size_t& pLeftNodes)
    {
      const Id pId(leftNodes[pLeftNodes]);
      const size_t pNodes(pId);
      if (cdl->leftBC == cdl->symmetry)
	{
	  RealArray1D<2> reduction3({0.0, 0.0});
	  {
	    const auto cellsOfNodeP(mesh->getCellsOfNode(pId));
	    const size_t nbCellsOfNodeP(cellsOfNodeP.size());
	    for (size_t cCellsOfNodeP=0; cCellsOfNodeP<nbCellsOfNodeP; cCellsOfNodeP++)
	      {
		const Id cId(cellsOfNodeP[cCellsOfNodeP]);
		const size_t cCells(cId);
		const size_t pNodesOfCellC(utils::indexOf(mesh->getNodesOfCell(cId), pId));
		reduction3 = sumR1(reduction3, (p_n(cCells) + Q_n(cCells)) * (
			     C(cCells,pNodesOfCellC)
			     + symmetricVector(C(cCells,pNodesOfCellC), {0.0, 1.0})));
	      }
	  }
	  u_nplus1(pNodes) = u_n(pNodes) + dt / (m(pNodes)) * reduction3;
	}
      else if (cdl->leftBC == cdl->imposedVelocity)
	{
	  u_nplus1(pNodes) = cdl->leftBCValue;
	}
    });
  const auto rightNodes(mesh->getRightNodes());
  const size_t nbRightNodes(rightNodes.size());
  Kokkos::parallel_for(nbRightNodes, KOKKOS_LAMBDA(const size_t& pRightNodes)
    {
      const Id pId(rightNodes[pRightNodes]);
      const size_t pNodes(pId);
      if (cdl->rightBC == cdl->symmetry)
	{
	  RealArray1D<2> reduction4({0.0, 0.0});
	  {
	    const auto cellsOfNodeP(mesh->getCellsOfNode(pId));
	    const size_t nbCellsOfNodeP(cellsOfNodeP.size());
	    for (size_t cCellsOfNodeP=0; cCellsOfNodeP<nbCellsOfNodeP; cCellsOfNodeP++)
	      {
		const Id cId(cellsOfNodeP[cCellsOfNodeP]);
		const size_t cCells(cId);
		const size_t pNodesOfCellC(utils::indexOf(mesh->getNodesOfCell(cId), pId));
		reduction4 = sumR1(reduction4, (p_n(cCells) + Q_n(cCells)) * (
			     C(cCells,pNodesOfCellC)
			     + symmetricVector(C(cCells,pNodesOfCellC), {0.0, 1.0})));
	      }
	  }
	  u_nplus1(pNodes) = u_n(pNodes) + dt / (m(pNodes)) * reduction4;
	}
      else if (cdl->rightBC == cdl->imposedVelocity)
	{
	  u_nplus1(pNodes) = cdl->rightBCValue;
	}
    });
  const auto topLeftNode(mesh->getTopLeftNode());
  const size_t nbTopLeftNode(mesh->getNbTopLeftNode());
  Kokkos::parallel_for("computeBoundaryNodeVelocities", nbTopLeftNode,
    KOKKOS_LAMBDA(const int& pTopLeftNode)
      {
	int pId(topLeftNode[pTopLeftNode]);
	int pNodes(pId);
	if (cdl->topBC == cdl->symmetry && cdl->leftBC == cdl->symmetry)
	  u_nplus1(pNodes) = zeroVect;
	else if (cdl->topBC == cdl->imposedVelocity
		 && cdl->leftBC == cdl->imposedVelocity)
	  u_nplus1(pNodes) = cdl->leftBCValue;
      });
  const auto topRightNode(mesh->getTopRightNode());
  const size_t nbTopRightNode(mesh->getNbTopRightNode());
  Kokkos::parallel_for("computeBoundaryNodeVelocities", nbTopRightNode,
    KOKKOS_LAMBDA(const int& pTopRightNode)
      {
	int pId(topRightNode[pTopRightNode]);
	int pNodes(pId);
	if (cdl->topBC == cdl->symmetry && cdl->rightBC == cdl->symmetry)
	  u_nplus1(pNodes) = zeroVect;
	else if (cdl->topBC == cdl->imposedVelocity
		 && cdl->rightBC == cdl->imposedVelocity)
	  u_nplus1(pNodes) = cdl->rightBCValue;
      });
  const auto bottomLeftNode(mesh->getBottomLeftNode());
  const size_t nbBottomLeftNode(mesh->getNbBottomLeftNode());
  Kokkos::parallel_for("computeBoundaryNodeVelocities", nbBottomLeftNode,
    KOKKOS_LAMBDA(const int& pBottomLeftNode)
      {
	int pId(bottomLeftNode[pBottomLeftNode]);
	int pNodes(pId);
	if (cdl->bottomBC == cdl->symmetry && cdl->leftBC == cdl->symmetry)
	  u_nplus1(pNodes) = zeroVect;
	else if (cdl->bottomBC == cdl->imposedVelocity
		 && cdl->leftBC == cdl->imposedVelocity)
	  u_nplus1(pNodes) = cdl->leftBCValue;
      });
  const auto bottomRightNode(mesh->getBottomRightNode());
  const size_t nbBottomRightNode(mesh->getNbBottomRightNode());
  Kokkos::parallel_for("computeBoundaryNodeVelocities", nbBottomRightNode,
    KOKKOS_LAMBDA(const int& pBottomRightNode)
      {
	int pId(bottomRightNode[pBottomRightNode]);
	int pNodes(pId);
	if (cdl->bottomBC == cdl->symmetry && cdl->rightBC == cdl->symmetry)
	  u_nplus1(pNodes) = zeroVect;
	else if (cdl->bottomBC == cdl->imposedVelocity
		 && cdl->rightBC == cdl->imposedVelocity)
	  u_nplus1(pNodes) = cdl->rightBCValue;		     
      });
}
void Vnr::updatePeriodicBoundaryConditions() noexcept
{
  Kokkos::parallel_for(nbCells, KOKKOS_LAMBDA(const size_t& cCell)
    {
      if (cdl->rightCellBC == cdl->periodic) {
	int LeftCell = mesh->getLeftCellfromRight(cCell);
	if (LeftCell != -1) {
	  rho_nplus1(LeftCell) = rho_n0(LeftCell);
	  e_nplus1(LeftCell) = e_n0(LeftCell);
	  rho_nplus1(cCell) = rho_nplus1(LeftCell);
	  e_nplus1(cCell) = e_nplus1(LeftCell);
	  int nbmat = options->nbmat;
	  for (int imat = 0; imat < nbmat; ++imat) {
	    rhop_nplus1(LeftCell)[imat] = rhop_n0(LeftCell)[imat];
	    ep_nplus1(LeftCell)[imat] = ep_n0(LeftCell)[imat];
	    rhop_nplus1(cCell)[imat] = rhop_nplus1(LeftCell)[imat];
	    ep_nplus1(cCell)[imat] = ep_nplus1(LeftCell)[imat];
	  }
	}
      }
    });
}
		   
