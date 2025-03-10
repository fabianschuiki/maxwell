/* Copyright (c) 2013 Fabian Schuiki */
#include "maxwell/stage/stages.hpp"
#include "maxwell/stage/algorithm/type.hpp"
#include <iostream>

using namespace ast;
using stage::FindCallCandidates;

void FindCallCandidates::process(const NodePtr& node)
{
	// Find call candidates for the node's children.
	const NodeVector& children = node->getChildren();
	for (NodeVector::const_iterator it = children.begin(); it != children.end(); it++) {
		process(*it);
	}

	// Operate on nodes implementing the CallInterface only.
	if (CallInterface *intf = node->asCall())
	{
		// Find nodes with this call's name.
		NodeVector nodes;
		const string& name = intf->getCallName();
		const Repository::ExternalNames& externals = repository.getExternalNamesForNodeId(node->getId());

		for (Repository::ExternalNames::const_iterator it = externals.begin(); it != externals.end(); it++) {
			if (it->second != name) continue; // discard other names
			const NodePtr& n = repository.getNode(it->first);
			CallableInterface *callable = n->asCallable();
			if (!callable) continue; // discard anything but Callable nodes
			const FuncType::Ptr& funcType = FuncType::needFrom(callable->getType());
			const NodePtr& funcInType = funcType->getIn();
			const TupleType::Ptr& funcInTuple = TupleType::from(funcInType);
			if (!funcInTuple && !funcInType->isKindOf(kNilType)) continue; // nil invalid argument type type
			if (funcInTuple && funcInTuple->getArgs().size() != intf->getCallArgs().size()) continue; // discard functions with incorrect number of arguments
			nodes.push_back(n);
		}

		NodePtr current(node->asGraph()->getGraphPrev());
		while (current) {
			if (VariableInterface *var = current.get()->asVariable()) {
				if (var->getName() == name) {
					const NodePtr& type = algorithm::type::resolve(var->getActualType(false));
					if (!type || type->isKindOf(kFuncType)) {
						nodes.push_back(current);
					}
				}
			}
			current = current->asGraph()->getGraphPrev(false);
		}

		// Wrap each node into a CallCandidate node.
		NodeVector candidates;
		for (NodeVector::iterator it = nodes.begin(); it != nodes.end(); it++) {
			const NodePtr& funcNode = *it;
			println(0, "Wrapping candidate " + funcNode->getId().str(), funcNode);

			// Create the candidate object to hold the function.
			CallCandidate::Ptr candidate(new CallCandidate);
			candidate->setFunc(funcNode);
			candidate->setFeasible(true);
			candidate->setCost(0);
			candidate->setGraphPrev(node);

			// Wrap the input arguments into pairing nodes.
			const NodeVector& callArgs = intf->getCallArgs();
			NodeVector args;

			for (NodeVector::const_iterator it = callArgs.begin(); it != callArgs.end(); it++) {
				const NodePtr& arg = *it;
				println(1, "  argument " + arg->getId().str(), funcNode);
				CallCandidateArg::Ptr cca(new CallCandidateArg);
				cca->setArg(arg);
				cca->setGraphPrev(candidate);
				args.push_back(cca);
			}

			candidate->setArgs(args);

			// Add the candidate to the call's list of candidates.
			candidates.push_back(candidate);
		}

		// Store the list of candidates in the call node.
		intf->setCallCandidates(candidates);
		if (candidates.empty()) {
			throw std::runtime_error("Called function '" + name + "' is not known.");
		}
		node->updateHierarchyOfChildren(); // since we added new children to node we need to update the node's hierarchy
	}
}
