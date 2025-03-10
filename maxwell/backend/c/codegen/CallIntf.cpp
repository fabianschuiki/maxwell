/* Copyright (c) 2013-2014 Fabian Schuiki */
#include "maxwell/ast/builtin/FuncDef.hpp"
#include "maxwell/backend/c/codegen/detail.hpp"

DEF_EXPR_INTF(Call)
{
	// Find the called function.
	const CallCandidate::Ptr& candidate = CallCandidate::needFrom(node->getSelectedCallCandidate()); // throws if null
	const NodePtr& funcNode = candidate->getFunc();
	const NodeVector& args = node->getCallArgs();

	FuncType::Ptr type;
	if (CallableInterface* func = funcNode->asCallable())
		type = FuncType::needFrom(func->getType());
	else
		type = FuncType::needFrom(stage::algorithm::type::resolve(funcNode->needType()->getActualType()));

	// If the called function is a builtin function, generate specialized code.
	if (const builtin::FuncDef::Ptr& bi = builtin::FuncDef::from(funcNode)) {
		const string& name = bi->getName();

		// Unary operator
		if (args.size() == 1) {
			CallArgInterface* arg = args[0]->needCallArg();
			ExprCode ec_arg;
			generateExpr(arg->getExpr(), ec_arg, ctx);
			out += ec_arg;

			// Synthesize the output.
			out.precedence = kPrefixPrec;
			out.code = name + precedenceWrap(ec_arg, out.precedence);
			out.isRef = false;
		}

		// Binary operator
		else if (args.size() == 2) {
			int p = -1;
			if (name == "||") p = kOrPrec;
			if (name == "&&") p = kAndPrec;
			if (name == "==" || name == "!=") p = kEqualityPrec;
			if (name == ">" || name == "<" || name == ">=" || name == "<=") p = kRelationalPrec;
			if (name == "+" || name == "-") p = kAdditivePrec;
			if (name == "*" || name == "/") p = kMultiplicativePrec;
			if (p == -1) {
				throw std::runtime_error("Precedence for builtin binary operator \"" + name + "\" cannot be determined.");
			}

			CallArgInterface* arg_lhs = args[0]->needCallArg();
			CallArgInterface* arg_rhs = args[1]->needCallArg();

			ExprCode ec_lhs, ec_rhs;
			generateExpr(arg_lhs->getExpr(), ec_lhs, ctx);
			generateExpr(arg_rhs->getExpr(), ec_rhs, ctx);
			out += ec_lhs;
			out += ec_rhs;

			// Synthesize the output.
			out.code = precedenceWrap(ec_lhs, p) + " " + name + " " + precedenceWrap(ec_rhs, p);
			out.isRef = false;
			out.precedence = p;
		}

		// Everything else is not supported.
		else {
			std::stringstream s;
			s << "Specialized code generation for builtin function " << funcNode->getId() << " not supported, as it takes " << args.size() << " input arguments instead of 1 or 2.";
			throw std::runtime_error(s.str());
		}
	}

	// If the called function is an implicit accessor, implement it directly.
	else if (const ImplAccessor::Ptr& iac = ImplAccessor::from(funcNode)) {

		// If the name ends in "=", this is a setter, otherwise we're dealing with a getter.
		std::string name = iac->getName();
		bool isSetter = (name[name.length()-1] == '=');
		if (isSetter)
			name = name.substr(0, name.length()-1);

		// Generate the expression for accessing the requested field of the struct.
		CallArgInterface* arg = args[0]->needCallArg();
		ExprCode ec_arg;
		generateExpr(arg->getExpr(), ec_arg, ctx);
		out += ec_arg;

		// Generate the accessor code.
		if (isSetter) {
			CallArgInterface* val = args[1]->needCallArg();
			ExprCode ec_val;
			generateExpr(val->getExpr(), ec_val, ctx);
			out += ec_val;

			out.precedence = kAssignmentPrec;
			out.code = ec_arg.code + "." + name + " = " + precedenceWrap(ec_val, kAssignmentPrec);
			out.isRef = false;
		} else {
			out.precedence = kPrefixPrec;
			out.code = ec_arg.code + "." + name;
			out.isRef = true;
		}
	}

	// Otherwise we implement a straightforward function call.
	else {
		std::string name;
		if (funcNode->isKindOf(kFuncDef)) {
			name = funcNode->getId().str() + "_decl";
			out.deps.insert(name);
			name = "%{" + name + "}";
		} else if (auto nf = NativeFuncDef::from(funcNode)) {
			name = nf->getName();
			const auto& d = nf->getDependencies();
			out.incs.insert(d.begin(), d.end());
		} else {
			name = funcNode->needNamed()->getName();
		}

		// Generate the code for each of the call arguments.
		out.code = name + "(";
		for (NodeVector::const_iterator it = args.begin(); it != args.end(); it++) {
			CallArgInterface* arg = (*it)->needCallArg();
			if (it != args.begin())
				out.code += ", ";

			ExprCode ec;
			generateExpr(arg->getExpr(), ec, ctx);
			out += ec;
			out.code += precedenceWrap(ec, kLowestPrec);
		}
		out.code += ")";

		// Synthesize the output.
		out.precedence = kPrimaryPrec;
		out.isRef = false;
	}
}
