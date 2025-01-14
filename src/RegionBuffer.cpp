/*
Copyright (C) 2006 - 2023 Evan Teran
						  evan.teran@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "RegionBuffer.h"
#include "IDebugger.h"
#include "IProcess.h"
#include "edb.h"

//------------------------------------------------------------------------------
// Name: RegionBuffer
// Desc:
//------------------------------------------------------------------------------
RegionBuffer::RegionBuffer(const std::shared_ptr<IRegion> &region)
	: QIODevice(), region_(region) {

	setOpenMode(QIODevice::ReadOnly);
}

//------------------------------------------------------------------------------
// Name: RegionBuffer
// Desc:
//------------------------------------------------------------------------------
RegionBuffer::RegionBuffer(const std::shared_ptr<IRegion> &region, QObject *parent)
	: QIODevice(parent), region_(region) {

	setOpenMode(QIODevice::ReadOnly);
}

//------------------------------------------------------------------------------
// Name: set_region
// Desc:
//------------------------------------------------------------------------------
void RegionBuffer::setRegion(const std::shared_ptr<IRegion> &region) {
	region_ = region;
	reset();
}

//------------------------------------------------------------------------------
// Name: readData
// Desc:
//------------------------------------------------------------------------------
qint64 RegionBuffer::readData(char *data, qint64 maxSize) {

	if (region_) {
		if (IProcess *process = edb::v1::debugger_core->process()) {
			const edb::address_t start = region_->start() + pos();
			const edb::address_t end   = region_->start() + region_->size();

			if (start + maxSize > end) {
				maxSize = end - start;
			}

			if (maxSize == 0) {
				return 0;
			}

			if (process->readBytes(start, data, maxSize)) {
				return maxSize;
			} else {
				return -1;
			}
		}
	}

	return -1;
}

//------------------------------------------------------------------------------
// Name: writeData
// Desc:
//------------------------------------------------------------------------------
qint64 RegionBuffer::writeData(const char *, qint64) {
	return -1;
}
