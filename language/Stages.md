Driver Stages
=============
This document describes the compilation stages of the driver. For each stage the operation is roughly sketched. This document should help debugging as it highlights where information in the repository is generated and how.

Operation
---------

### BindIdentifiers
Sets each IdentifierExpr's `bindingTarget` to a known named object, or generates an error if know such entities are known.

### ConfigureCalls
Prepares the *call* fragment of objects that have one.

- **BinaryOperatorExpr**:
	- `callName` = `operator`
	- `callArguments` = (`lhs`, `rhs`)

### FindCallCandidates
For each object implementing the `CallInterface`, gathers a list of function definitions whose name matches the `callName` of said object.

### EvaluateTypeExpressions
Tries to convert each `TypeExpr` object's `expr` into a valid type and stores it in the object's `evaluatedType` property.

### CalculatePossibleTypes
Calculates a rough estimate of each object's possible types.

- **AbstractFunctionArgument**: (`name`, `typeExpr.evaluatedType`)
- **AbstractFunctionArgumentTuple**: (`[arguments.*.possibleType]`)
- **AbstractFunctionDefinition**: (`inputs.possibleType`, `outputs.possibleType`)
- **CallInterface**: Set of output argument tuples of different length, where all argument tuples of the same length are merged into each other, producing arguments which have a set of types as type.
- **IdentifierExpr**: Type depends on target:
	- AbstractFunctionArgument: `possibleType.type`
	- otherwise: *null*
- **AssignmentExpr**: `lhs.possibleType`

### CalculateRequiredTypes
Calculates the type constraints objects impose on their children.

- **AssignmentExpr**: `rhs.requiredType` = `lhs.possibleType`
- **CallInterface**: Each argument's type is the set of types of the input arguments at this location of all candidates.

### CalculateActualTypes
Calculates the largest common set of types between an object's possible and required type. If no such type is found, the object is assigned the invalid type.

### NarrowCallCandidates
Eliminates unfeasible call candidates based on whether their types match the call argument's required types. Does not actually remove the call candidates but rather marks them as unfeasible. After this stage is finished and has not invalidated any dependencies each call is ready for call selection.

### SelectCallCandidate
Picks the best candidate marked as feasible for each call. In case no feasible entries are left an error may be produced here.


Requires Testing
----------------
- What if a call has no matching call candidates?
- What if a call has no return value? What happens to the parents if it is embedded somewhere that expects a type? 