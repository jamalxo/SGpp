CC = g++
DEBUG = 
SRCDIR = src/sgpp/
CFLAGS = -Wall -pedantic -ansi -c -Wno-long-long -fno-strict-aliasing -fopenmp -O3 -funroll-loops -pthread -DUSEOMP -Isrc/sgpp/
LFLAGS = -Wall -pedantic -ansi -fopenmp -O3 -pthread
OBJS = DataVector.o Grid.o LinearBoundaryGrid.o LinearTrapezoidBoundaryGrid.o LinearGrid.o ModLinearGrid.o ModPolyGrid.o PolyGrid.o StandardGridGenerator.o BoundaryGridGenerator.o TrapezoidBoundaryGridGenerator.o OperationBLinear.o OperationEvalLinear.o OperationHierarchisationLinear.o OperationBModLinear.o OperationEvalModLinear.o OperationHierarchisationModLinear.o OperationBModPoly.o OperationEvalModPoly.o OperationHierarchisationModPoly.o OperationBPoly.o OperationEvalPoly.o OperationHierarchisationPoly.o OperationHierarchisationLinearTrapezoidBoundary.o OperationEvalLinearTrapezoidBoundary.o OperationBLinearTrapezoidBoundary.o OperationHierarchisationLinearBoundary.o OperationEvalLinearBoundary.o OperationBLinearBoundary.o ARFFTools.o DMSystemMatrix.o Classifier.o NativeCppClassifier.o ConjugateGradients.o SGppStopwatch.o BoundingBox.o OperationRiskfreeRateLinearTrapezoidBoundary.o OperationDeltaLinearTrapezoidBoundary.o OperationGammaPartOneLinearTrapezoidBoundary.o OperationGammaPartTwoLinearTrapezoidBoundary.o OperationGammaPartThreeLinearTrapezoidBoundary.o
OBJSSRC = tmp/build_native/DataVector.o tmp/build_native/Grid.o tmp/build_native/LinearBoundaryGrid.o tmp/build_native/LinearTrapezoidBoundaryGrid.o tmp/build_native/LinearGrid.o tmp/build_native/ModLinearGrid.o tmp/build_native/ModPolyGrid.o tmp/build_native/PolyGrid.o tmp/build_native/StandardGridGenerator.o tmp/build_native/BoundaryGridGenerator.o tmp/build_native/TrapezoidBoundaryGridGenerator.o tmp/build_native/OperationBLinear.o tmp/build_native/OperationEvalLinear.o tmp/build_native/OperationHierarchisationLinear.o tmp/build_native/OperationBModLinear.o tmp/build_native/OperationEvalModLinear.o tmp/build_native/OperationHierarchisationModLinear.o tmp/build_native/OperationBModPoly.o tmp/build_native/OperationEvalModPoly.o tmp/build_native/OperationHierarchisationModPoly.o tmp/build_native/OperationBPoly.o tmp/build_native/OperationEvalPoly.o tmp/build_native/OperationHierarchisationPoly.o tmp/build_native/OperationHierarchisationLinearTrapezoidBoundary.o tmp/build_native/OperationEvalLinearTrapezoidBoundary.o tmp/build_native/OperationBLinearTrapezoidBoundary.o tmp/build_native/OperationHierarchisationLinearBoundary.o tmp/build_native/OperationEvalLinearBoundary.o tmp/build_native/OperationBLinearBoundary.o tmp/build_native/ARFFTools.o tmp/build_native/DMSystemMatrix.o tmp/build_native/Classifier.o tmp/build_native/NativeCppClassifier.o tmp/build_native/ConjugateGradients.o tmp/build_native/SGppStopwatch.o tmp/build_native/BoundingBox.o tmp/build_native/OperationRiskfreeRateLinearTrapezoidBoundary.o tmp/build_native/OperationDeltaLinearTrapezoidBoundary.o tmp/build_native/OperationGammaPartOneLinearTrapezoidBoundary.o tmp/build_native/OperationGammaPartTwoLinearTrapezoidBoundary.o tmp/build_native/OperationGammaPartThreeLinearTrapezoidBoundary.o

NativeCppClassifier : $(OBJS)
	mkdir -p tmp/build_native
	\mv *.o tmp/build_native/
	$(CC) $(LFLAGS) $(OBJSSRC) -o bin/NativeCppClassifier

DataVector.o : src/sgpp/data/DataVector.cpp
	$(CC) $(CFLAGS) src/sgpp/data/DataVector.cpp

Grid.o : src/sgpp/grid/Grid.cpp
	$(CC) $(CFLAGS) src/sgpp/grid/Grid.cpp

LinearBoundaryGrid.o : src/sgpp/grid/type/LinearBoundaryGrid.cpp
	$(CC) $(CFLAGS) src/sgpp/grid/type/LinearBoundaryGrid.cpp

LinearTrapezoidBoundaryGrid.o : src/sgpp/grid/type/LinearTrapezoidBoundaryGrid.cpp
	$(CC) $(CFLAGS) src/sgpp/grid/type/LinearTrapezoidBoundaryGrid.cpp
	
LinearGrid.o : src/sgpp/grid/type/LinearGrid.cpp
	$(CC) $(CFLAGS) src/sgpp/grid/type/LinearGrid.cpp
	
ModLinearGrid.o : src/sgpp/grid/type/ModLinearGrid.cpp
	$(CC) $(CFLAGS) src/sgpp/grid/type/ModLinearGrid.cpp

ModPolyGrid.o : src/sgpp/grid/type/ModPolyGrid.cpp
	$(CC) $(CFLAGS) src/sgpp/grid/type/ModPolyGrid.cpp

PolyGrid.o : src/sgpp/grid/type/PolyGrid.cpp
	$(CC) $(CFLAGS) src/sgpp/grid/type/PolyGrid.cpp
	
StandardGridGenerator.o : src/sgpp/grid/generation/StandardGridGenerator.cpp
	$(CC) $(CFLAGS) src/sgpp/grid/generation/StandardGridGenerator.cpp
	
BoundaryGridGenerator.o : src/sgpp/grid/generation/BoundaryGridGenerator.cpp
	$(CC) $(CFLAGS) src/sgpp/grid/generation/BoundaryGridGenerator.cpp

TrapezoidBoundaryGridGenerator.o : src/sgpp/grid/generation/TrapezoidBoundaryGridGenerator.cpp
	$(CC) $(CFLAGS) src/sgpp/grid/generation/TrapezoidBoundaryGridGenerator.cpp

OperationBLinear.o : src/sgpp/basis/linear/operation/classification/OperationBLinear.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/linear/operation/classification/OperationBLinear.cpp

OperationEvalLinear.o : src/sgpp/basis/linear/operation/common/OperationEvalLinear.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/linear/operation/common/OperationEvalLinear.cpp

OperationHierarchisationLinear.o : src/sgpp/basis/linear/operation/common/OperationHierarchisationLinear.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/linear/operation/common/OperationHierarchisationLinear.cpp

OperationBModLinear.o : src/sgpp/basis/modlinear/operation/classification/OperationBModLinear.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/modlinear/operation/classification/OperationBModLinear.cpp

OperationEvalModLinear.o : src/sgpp/basis/modlinear/operation/common/OperationEvalModLinear.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/modlinear/operation/common/OperationEvalModLinear.cpp

OperationHierarchisationModLinear.o : src/sgpp/basis/modlinear/operation/common/OperationHierarchisationModLinear.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/modlinear/operation/common/OperationHierarchisationModLinear.cpp

OperationBModPoly.o : src/sgpp/basis/modpoly/operation/classification/OperationBModPoly.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/modpoly/operation/classification/OperationBModPoly.cpp

OperationEvalModPoly.o : src/sgpp/basis/modpoly/operation/common/OperationEvalModPoly.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/modpoly/operation/common/OperationEvalModPoly.cpp

OperationHierarchisationModPoly.o : src/sgpp/basis/modpoly/operation/common/OperationHierarchisationModPoly.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/modpoly/operation/common/OperationHierarchisationModPoly.cpp

OperationBPoly.o : src/sgpp/basis/poly/operation/classification/OperationBPoly.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/poly/operation/classification/OperationBPoly.cpp

OperationEvalPoly.o : src/sgpp/basis/poly/operation/common/OperationEvalPoly.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/poly/operation/common/OperationEvalPoly.cpp

OperationHierarchisationPoly.o : src/sgpp/basis/poly/operation/common/OperationHierarchisationPoly.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/poly/operation/common/OperationHierarchisationPoly.cpp

OperationHierarchisationLinearTrapezoidBoundary.o : src/sgpp/basis/lineartrapezoidboundary/operation/common/OperationHierarchisationLinearTrapezoidBoundary.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/lineartrapezoidboundary/operation/common/OperationHierarchisationLinearTrapezoidBoundary.cpp

OperationEvalLinearTrapezoidBoundary.o : src/sgpp/basis/lineartrapezoidboundary/operation/common/OperationEvalLinearTrapezoidBoundary.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/lineartrapezoidboundary/operation/common/OperationEvalLinearTrapezoidBoundary.cpp

OperationBLinearTrapezoidBoundary.o : src/sgpp/basis/lineartrapezoidboundary/operation/classification/OperationBLinearTrapezoidBoundary.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/lineartrapezoidboundary/operation/classification/OperationBLinearTrapezoidBoundary.cpp
	
OperationRiskfreeRateLinearTrapezoidBoundary.o : src/sgpp/basis/lineartrapezoidboundary/operation/finance/OperationRiskfreeRateLinearTrapezoidBoundary.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/lineartrapezoidboundary/operation/finance/OperationRiskfreeRateLinearTrapezoidBoundary.cpp

OperationDeltaLinearTrapezoidBoundary.o : src/sgpp/basis/lineartrapezoidboundary/operation/finance/OperationDeltaLinearTrapezoidBoundary.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/lineartrapezoidboundary/operation/finance/OperationDeltaLinearTrapezoidBoundary.cpp
	
OperationGammaPartOneLinearTrapezoidBoundary.o : src/sgpp/basis/lineartrapezoidboundary/operation/finance/OperationGammaPartOneLinearTrapezoidBoundary.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/lineartrapezoidboundary/operation/finance/OperationGammaPartOneLinearTrapezoidBoundary.cpp
	
OperationGammaPartTwoLinearTrapezoidBoundary.o : src/sgpp/basis/lineartrapezoidboundary/operation/finance/OperationGammaPartTwoLinearTrapezoidBoundary.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/lineartrapezoidboundary/operation/finance/OperationGammaPartTwoLinearTrapezoidBoundary.cpp

OperationGammaPartThreeLinearTrapezoidBoundary.o : src/sgpp/basis/lineartrapezoidboundary/operation/finance/OperationGammaPartThreeLinearTrapezoidBoundary.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/lineartrapezoidboundary/operation/finance/OperationGammaPartThreeLinearTrapezoidBoundary.cpp
		
OperationHierarchisationLinearBoundary.o : src/sgpp/basis/linearboundary/operation/common/OperationHierarchisationLinearBoundary.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/linearboundary/operation/common/OperationHierarchisationLinearBoundary.cpp

OperationEvalLinearBoundary.o : src/sgpp/basis/linearboundary/operation/common/OperationEvalLinearBoundary.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/linearboundary/operation/common/OperationEvalLinearBoundary.cpp

OperationBLinearBoundary.o : src/sgpp/basis/linearboundary/operation/classification/OperationBLinearBoundary.cpp
	$(CC) $(CFLAGS) src/sgpp/basis/linearboundary/operation/classification/OperationBLinearBoundary.cpp
	
ARFFTools.o : src/sgpp/tools/classification/ARFFTools.cpp
	$(CC) $(CFLAGS) src/sgpp/tools/classification/ARFFTools.cpp
	
DMSystemMatrix.o : src/sgpp/algorithm/classification/DMSystemMatrix.cpp
	$(CC) $(CFLAGS) src/sgpp/algorithm/classification/DMSystemMatrix.cpp
	
ConjugateGradients.o : src/sgpp/solver/cg/ConjugateGradients.cpp
	$(CC) $(CFLAGS) src/sgpp/solver/cg/ConjugateGradients.cpp

Classifier.o : src/sgpp/application/classification/Classifier.cpp
	$(CC) $(CFLAGS) src/sgpp/application/classification/Classifier.cpp

SGppStopwatch.o : src/sgpp/common/SGppStopwatch.cpp
	$(CC) $(CFLAGS) src/sgpp/common/SGppStopwatch.cpp
	
BoundingBox.o : src/sgpp/grid/common/BoundingBox.cpp
	$(CC) $(CFLAGS) src/sgpp/grid/common/BoundingBox.cpp
	
NativeCppClassifier.o : src/sgpp/application/classification/NativeCppClassifier.cpp
	$(CC) $(CFLAGS) src/sgpp/application/classification/NativeCppClassifier.cpp
	
clean:
	\rm tmp/build_native/*.o *.o
