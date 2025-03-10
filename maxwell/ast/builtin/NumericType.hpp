/* Copyright (c) 2013 Fabian Schuiki */
#pragma once
#include "maxwell/ast/builtin/BuiltinNode.hpp"
#include "maxwell/ast/nodes/interfaces.hpp"
#include <string>

namespace ast {

class BuiltinRepository;

namespace builtin {

using std::string;

class NumericType : public BuiltinNode
{
public:
	NumericType() : interfaceNamed(this) {}
	virtual string getClassName() const { return "BuiltinNumericType"; }
	virtual const string& getName(bool required = true) const { return name; }
	virtual void setName(const string& s) { throw std::runtime_error("Cannot set name of builtin type " + name + "."); }
	virtual bool equalTo(const NodePtr& other)
	{
		NumericType* nt = dynamic_cast<NumericType*>(other.get());
		if (!nt) return false;
		return (name == nt->name);
	}
	virtual bool implements(Interface i)
	{
		if (BuiltinNode::implements(i)) return true;
		if (i == kNamedInterface) return true;
		return false;
	}

	// Interfaces
	virtual NamedInterface* asNamed() { return &this->interfaceNamed; }

	typedef boost::shared_ptr<NumericType> Ptr;
	template<typename T> static Ptr from(const T& n) { return boost::dynamic_pointer_cast<NumericType>(n); }
	template<typename T> static Ptr needFrom(const T& n) { Ptr r = boost::dynamic_pointer_cast<NumericType>(n); if (!r) throw std::runtime_error("Node " + n->getId().str() + " cannot be dynamically casted to NumericType."); return r; }

protected:
	friend class ast::BuiltinRepository;
	string name;

	// Interfaces
	NamedInterfaceImpl<NumericType> interfaceNamed;
};

} // namespace builtin
} // namespace ast
