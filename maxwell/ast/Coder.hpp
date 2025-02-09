/* Copyright (c) 2013 Fabian Schuiki */
#pragma once
#include "maxwell/ast/Node.hpp"
#include "maxwell/location.hpp"
#include <string>
#include <boost/smart_ptr.hpp>

namespace ast {

using std::string;
using boost::shared_ptr;

class Node;

/// Encoder interface.
class Encoder
{
public:
	virtual void encode(const NodePtr& node) = 0;
	virtual void encode(const NodeRef& node) = 0;
	virtual void encode(const string& str) = 0;
	virtual void encode(const std::vector<std::string>& strs) = 0;
	virtual void encode(const bool& b) = 0;
	virtual void encode(const int& i) = 0;
	virtual void encode(const NodeVector& nodes) = 0;
	virtual void encode(const SourceRange& rng) = 0;
};

/// Decoder interface.
class Decoder
{
public:
	virtual void decode(NodePtr& node) = 0;
	virtual void decode(NodeRef& node) = 0;
	virtual void decode(string& str) = 0;
	virtual void decode(std::vector<std::string>& strs) = 0;
	virtual void decode(bool& b) = 0;
	virtual void decode(int& i) = 0;
	virtual void decode(NodeVector& nodes) = 0;
	virtual void decode(SourceRange& rng) = 0;
};

} // namespace ast
