/* Copyright (c) 2013 Fabian Schuiki */
#pragma once
#include "maxwell/ast/builtin/BuiltinNode.hpp"
#include "maxwell/ast/nodes/interfaces.hpp"
#include <string>

namespace ast {

class BuiltinRepository;

namespace builtin {

using std::string;

class FuncDef : public BuiltinNode
{
public:
	FuncDef() :
		interfaceNamed(this),
		interfaceCallable(this) {}

	const string& getName(bool required = true) const { return name; }
	const NodePtr& getType(bool required = true) { return type; }

	void setName(const string& s) { throw std::runtime_error("Cannot set name of builtin function " + name + "."); }
	void setType(const NodePtr& v) { throw std::runtime_error("Cannot set type of builtin function " + name + "."); }

	virtual bool equalTo(const NodePtr& other)
	{
		FuncDef* nt = dynamic_cast<FuncDef*>(other.get());
		if (!nt) return false;
		return (name == nt->name);
	}
	virtual bool implements(Interface i)
	{
		if (BuiltinNode::implements(i)) return true;
		if (i == kCallableInterface) return true;
		if (i == kNamedInterface) return true;
		return false;
	}

	// Interfaces
	virtual CallableInterface* asCallable() { return &this->interfaceCallable; }
	virtual NamedInterface* asNamed() { return &this->interfaceNamed; }

	typedef boost::shared_ptr<FuncDef> Ptr;
	template<typename T> static Ptr from(const T& n) { return boost::dynamic_pointer_cast<FuncDef>(n); }
	template<typename T> static Ptr needFrom(const T& n) { Ptr r = boost::dynamic_pointer_cast<FuncDef>(n); if (!r) throw std::runtime_error("Node " + n->getId().str() + " cannot be dynamically casted to FuncDef."); return r; }

protected:
	friend class ast::BuiltinRepository;
	string name;
	NodePtr type;

	// Interfaces
	NamedInterfaceImpl<FuncDef> interfaceNamed;
	CallableInterfaceImpl<FuncDef> interfaceCallable;
};

} // namespace builtin
} // namespace ast
