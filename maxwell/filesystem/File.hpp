/* Copyright (c) 2014 Fabian Schuiki */
#pragma once
#include "maxwell/Buffer.hpp"
#include "maxwell/filesystem/Node.hpp"
#include "maxwell/filesystem/Path.hpp"
#include "maxwell/types.hpp"
#include <vector>

namespace maxwell {
namespace filesystem {

/// An abstract interface for a file resource. Most likely implemented by a disk
/// file class and a mock file class.
class File : public Node {
public:
	/// Reads the contents of the file into \a dst in the most efficient manner
	/// implemented. The data is *appended* to \a dst.
	virtual void read(std::vector<Byte>& dst) const = 0;

	/// Writes the contents of \a src to the file, replacing its existing
	/// content.
	virtual void write(const Buffer<const Byte>& src) = 0;

	/// Writes the contents of \a src to the file, replacing its existing
	/// content.
	virtual void write(const std::vector<Byte>& src) {
		write(Buffer<const Byte>(&src[0], &src[0] + src.size()));
	}

	/// Returns the time at which the file was modified the last time.
	virtual time_t getModificationTime() const = 0;
};

} // namespace filesystem
} // namespace maxwell
