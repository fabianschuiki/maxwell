/* Copyright (c) 2013-2015 Fabian Schuiki */
#include "maxwell/base64.hpp"
#include "maxwell/ast/Coder.hpp"
#include "maxwell/ast/nodes/nodes.hpp"
#include "maxwell/ast/Serializer.hpp"
#include <sstream>
#include <stdexcept>

using ast::Serializer;
using ast::Node;
using ast::NodePtr;
using ast::NodeRef;
using ast::NodeId;
using ast::NodeVector;
using ast::Encoder;
using ast::Decoder;
using maxwell::SourceId;
using maxwell::SourceRange;
using std::string;
using std::stringstream;
using std::ostream;
using std::istream;
using boost::shared_ptr;

/// Concrete implementation of the Encoder interface. An instance of this class
/// is created upon serialization and handed down the tree for serializing.
class EncoderImpl : public Encoder {
public:
	std::ostream& out;
	EncoderImpl(std::ostream& o) : out(o) {}

	virtual void encode(const NodePtr& node) {
		if (node) {
			out << node->getClassName() << " { ";
			node->encode(*this);
			out << "} ";
		} else {
			out << "@ ";
		}
	}

	virtual void encode(const NodeRef& node) {
		if (node) {
			out << node.id << " ";
		} else {
			out << "@ ";
		}
	}

	virtual void encode(const std::string& str) {
		if (str.empty()) {
			out << "$ ";
		} else {
			out << base64::encode(str) << ' ';
		}
	}

	virtual void encode(const std::vector<std::string>& strs) {
		if (strs.empty()) {
			out << "[] ";
		} else {
			out << "[ ";
			bool first = true;
			for (const auto& s : strs) {
				if (first) {
					first = false;
				} else {
					out << ", ";
				}
				encode(s);
			}
			out << "] ";
		}
	}

	virtual void encode(const bool& b) {
		out << (b ? "1 " : "0 ");
	}

	virtual void encode(const int& i) {
		out << i << ' ';
	}

	virtual void encode(const NodeVector& nodes) {
		if (nodes.empty()) {
			out << "[] ";
		} else {
			out << "[ ";
			bool first = true;
			for (NodeVector::const_iterator it = nodes.begin(); it != nodes.end(); it++) {
				if (first) {
					first = false;
				} else {
					out << ", ";
				}
				encode(*it);
			}
			out << "] ";
		}
	}

	virtual void encode(const SourceRange& rng) {
		out << rng.getSourceId().getId() << ' ' << rng.getOffset() << ' ' << rng.getLength() << ' ';
	}
};

/**
 * Concrete implementation of the Decoder interface. An instance of this class
 * is created upon unserialization and handed down the tree for decoding.
 */
class DecoderImpl : public Decoder
{
public:
	istream& in;
	DecoderImpl(istream& i) : in(i) {}

	virtual void decode(NodePtr& node) {
		string className;
		in >> className;
		if (className == "@") {
			node.reset();
		} else {
			string rd;
			in >> rd;
			if (rd != "{") {
				throw std::runtime_error("Node excepted to commence with an opening brace {, got '" + rd + "' instead.");
			}

			node = ast::makeNode(className);
			node->decode(*this);

			in >> rd;
			if (rd != "}") {
				throw std::runtime_error("Node excepted to terminate with a closing brace }, got '" + rd + "' instead.");
			}
		}
	}

	virtual void decode(NodeRef& node) {
		string rd;
		in >> rd;
		if (rd == "@") {
			node.reset();
		} else {
			node.set(NodeId(rd));
		}
	}

	virtual void decode(string& str) {
		string rd;
		in >> rd;
		if (rd == "$") {
			str.clear();
		} else {
			str = base64::decode(rd);
		}
	}

	virtual void decode(std::vector<std::string>& strs) {
		string rd;
		in >> rd;
		if (rd == "[]") {
			strs.clear();
		} else {
			if (rd != "[") {
				throw std::runtime_error("Vector of strings expected to commence with an opening bracket [, got '" + rd + "' instead.");
			}
			do {
				std::string str;
				decode(str);
				strs.push_back(str);
				in >> rd;
			} while (rd == ",");
			if (rd != "]") {
				throw std::runtime_error("Vector of strings expected to terminate with a closing bracket ], got '" + rd + "' instead.");
			}
		}
	}

	virtual void decode(bool& b) {
		string rd;
		in >> rd;
		if (rd == "0") {
			b = false;
		} else if (rd == "1") {
			b = true;
		} else {
			throw std::runtime_error("Boolean value must either be 1 or 0, got '" + rd + "' instead.");
		}
	}

	virtual void decode(int& i) {
		in >> i;
	}

	virtual void decode(NodeVector& nodes) {
		string rd;
		in >> rd;
		if (rd == "[]") {
			nodes.clear();
		} else {
			if (rd != "[") {
				throw std::runtime_error("Vector of nodes expected to commence with an opening bracket [, got '" + rd + "' instead.");
			}
			do {
				NodePtr node;
				decode(node);
				nodes.push_back(node);
				in >> rd;
			} while (rd == ",");
			if (rd != "]") {
				throw std::runtime_error("Vector of nodes expected to terminate with a closing bracket ], got '" + rd + "' instead.");
			}
		}
	}

	virtual void decode(SourceRange& rng) {
		uint32_t id;
		uint32_t offset;
		uint32_t length;
		in >> id >> offset >> length;
		rng = SourceRange(SourceId(id), offset, length);
	}
};

/**
 * @brief Serializes the given node to the given stream.
 */
void Serializer::encode(ostream& out, const NodePtr& node)
{
	EncoderImpl enc(out);
	enc.encode(node);
}

/**
 * @brief Serializes the given node into a string.
 *
 * Simply calls encode() with a stringstream and the node, then returns the
 * generated string.
 */
string Serializer::encode(const NodePtr& node)
{
	stringstream s;
	encode(s, node);
	return s.str();
}

/**
 * @brief Unserializes a node from the given stream.
 */
NodePtr Serializer::decode(istream& in)
{
	DecoderImpl dec(in);
	NodePtr node;
	dec.decode(node);
	return node;
}

/**
 * @brief Unserializes the string and returns the resulting node.
 *
 * Simply calls decode() with a string stream and the node, then returns the
 * generated node.
 */
NodePtr Serializer::decode(const std::string& str)
{
	stringstream s(str);
	return decode(s);
}
