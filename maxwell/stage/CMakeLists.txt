add_library(stage
	Stage.cpp
	BuildGraph.cpp
	BindIdentifiers.cpp
	BindNamedTypes.cpp
	EvalTypeExprs.cpp
	InitRootTypes.cpp
	GenImplicitAccessors.cpp
	ConfigureCalls.cpp
	FindCallCandidates.cpp
	CalcPossibleTypes.cpp
	CalcRequiredTypes.cpp
	CalcActualTypes.cpp
	NarrowCallCandidates.cpp
	SelectCallCandidate.cpp

	algorithm/type.cpp
)

add_dependencies(stage stage-gen)
add_dependencies(stage ast)