/* Copyright (c) 2013 Fabian Schuiki */
#pragma once
#include "maxwell/ast/NodeId.hpp"
#include "maxwell/ast/nodes/BaseNode.hpp" // for ast::BaseNode
#include "maxwell/ast/nodes/types.hpp" // for ast::Kind
#include "maxwell/location.hpp"
#include <boost/enable_shared_from_this.hpp>
#include <boost/smart_ptr.hpp>
#include <iostream>
#include <string>
#include <vector>

namespace ast {

using std::string;
using std::vector;
using maxwell::SourceRange;
using boost::shared_ptr;

class Encoder;
class Decoder;
class Repository;

class Node;
class NodeRef;
typedef shared_ptr<Node> NodePtr;
typedef vector<NodePtr> NodeVector;

/**
 * @brief Base class for all nodes in the abstract syntax tree.
 */
class Node : public boost::enable_shared_from_this<Node>, public BaseNode {
public:
	Node();

	/// Overridden by subclasses to indicate a node reference.
	virtual bool isReference() { return false; }

	/// Overridden by subclasses to indicate what type of node they are.
	virtual bool isKindOf(Kind k) { return false; }
	/// Overridden by subclasses to indicate what interfaces they implement.
	virtual bool implements(Interface i) { return false; }

	/// Overridden by subclasses to generate an exact copy of them.
	virtual NodePtr copy() { return NodePtr(); }
	/// Convenience helper function to create a copy of an object which is potentially null.
	static NodePtr copy(const NodePtr& n) { return n ? n->copy() : NodePtr(); }

	/// Returns a description of this node and all its subnodes for debugging purposes.
	virtual string describe(int depth = -1) { return "Node"; }

	/// Returns the class name of this node used in serialization.
	virtual string getClassName() const = 0;

	/// Overridden by subclasses to encode their member variables.
	virtual void encode(Encoder& e) = 0;
	/// Overridden by subclasses to reconstruct their member variables.
	virtual void decode(Decoder& d) = 0;

	/// Overridden by subclasses to contain child nodes.
	virtual vector<shared_ptr<Node> > getChildren() { return vector<shared_ptr<Node> >(); }

	/// Overridden by subclasses to descend into the tree to lookup a node.
	virtual const shared_ptr<Node>& resolvePath(const string& path) = 0;

	/// Overridden by subclasses to indicate whether %this is equal to %other.
	virtual bool equalTo(const NodePtr& other) = 0;

	/// Returns the node's id.
	const NodeId& getId() const { return id; }
	/// Returns the node's parent node, or an empty pointer if this is a root node.
	Node* getParent() const { return parent; }
	/// Returns the node's repository, or an empty pointer if it is not part of one.
	Repository* getRepository() const { return repository; }

	virtual void updateHierarchy(const NodeId& id, Repository* repository = NULL, Node* parent = NULL);
	/// Overridden by subclasses to propagate hierarchy changes to child nodes.
	virtual void updateHierarchyOfChildren() {}

	/// Sets the range this node covers in the source file.
	void setRange(SourceRange r) { range = r; }
	/// Sets the range that shall be used when referring to this node from a
	/// diagnostic message.
	void setReferenceRange(SourceRange r) { referenceRange = r; }

	/// Returns the range this node covers in the source file.
	SourceRange getRange() const {
		return range;
	}

	/// Returns the range that should be used to refer to this node. If no
	/// reference range has been set by a previous call to setReferenceRange(),
	/// returns the result of calling getRange() instead.
	SourceRange getReferenceRange() const {
		return referenceRange ? referenceRange : range;
	}

protected:
	bool modified;
	NodeId id;
	Node* parent;
	Repository* repository;
	SourceRange range;
	SourceRange referenceRange;

	void modify(const string& attribute);
	string indent(const string& in);

	string describeVector(const vector<shared_ptr<Node> >& nodes, int depth = -1);
	string describeVector(const vector<string>& strings, int depth = -1);

	static bool equal(const string& a, const string& b) { return a == b; }
	static bool equal(const vector<string>& a, const vector<string>& b) { return a == b; }
	static bool equal(const bool& a, const bool& b) { return a == b; }
	static bool equal(const int& a, const int& b) { return a == b; }
	static bool equal(const NodePtr& a, const NodePtr& b);
	static bool equal(const NodeVector& a, const NodeVector& b);

	static void copy(const string& s, string& d) { d = s; }
	static void copy(const vector<string>& s, vector<string>& d) { d = s; }
	static void copy(const bool& s, bool& d) { d = s; }
	static void copy(const int& s, int& d) { d = s; }
	static void copy(const NodePtr& s, NodePtr& d);
	static void copy(const NodeVector& s, NodeVector& d);
	static void copy(const NodeRef& s, NodeRef& d);
};

/**
 * @brief Reference to another node in the repository.
 *
 * A NodeRef acts a special node wrapping a by-id-reference to another node
 * in the NodeRef's repository. Concrete nodes should be aware of this class
 * and transparently use get() to resolve the reference.
 */
class NodeRef
{
public:
	NodeRef() {}
	NodeRef(const NodeId& id) : id(id) {}

	NodeId id;
	NodePtr resolved;

	const NodePtr& get(Repository* repository);
	void set(const NodePtr& node);
	void set(const NodeId& id);
	void reset();
	bool empty() const { return id.empty(); }
	operator bool() const { return !id.empty(); }
};

} // namespace ast
