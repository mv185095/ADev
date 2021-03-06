// Copyright 2020 Michael Vlach
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "pch.hpp"

#include "FileRecords.hpp"

#include <numeric>

namespace afile
{
FileRecords::FileRecords(FileData *data) :
    mData{data}
{
    initialize();
}

auto FileRecords::clear() -> void
{
    mCount = 0;
    mFreeIndex = acore::INVALID_INDEX;
    mRecords.clear();
}

auto FileRecords::contains(acore::size_type index) const noexcept -> bool
{
    if (0 <= index && index < static_cast<acore::size_type>(mRecords.size()))
    {
        return isValid(index);
    }

    return false;
}

auto FileRecords::count() const noexcept -> acore::size_type
{
    return mCount;
}

auto FileRecords::endPos(acore::size_type index) const noexcept -> acore::size_type
{
    return mRecords[index].pos + mRecords[index].size;
}

auto FileRecords::indexes() const -> std::vector<acore::size_type>
{
    std::vector<acore::size_type> idxs;
    idxs.reserve(mRecords.size());

    for (const Index &idx : mRecords)
    {
        if (isValid(idx))
        {
            idxs.push_back(static_cast<acore::size_type>(std::distance(mRecords.data(), &idx)));
        }
    }

    return idxs;
}

auto FileRecords::invalidate(acore::size_type index) -> void
{
    update(recordPos(index), Index{acore::INVALID_INDEX, -mRecords[index].size});
}

auto FileRecords::isLast(acore::size_type index) const -> bool
{
    return mData->size() == endPos(index);
}

auto FileRecords::isValid(acore::size_type index) const noexcept -> bool
{
    return isValid(mRecords[index]);
}

auto FileRecords::newIndex() -> acore::size_type
{
    acore::size_type index = mFreeIndex;
    Index recordIndex{logicalRecordPos(mData->size()), 0};

    if (index == acore::INVALID_INDEX)
    {
        index = static_cast<acore::size_type>(mRecords.size());
        mRecords.push_back(recordIndex);
        saveRecordsCount();
    }
    else
    {
        mFreeIndex = mRecords[index].pos;
        mRecords[index] = recordIndex;
    }

    update(mData->size(), Index{index, 0});
    mCount++;
    return index;
}

auto FileRecords::pos(acore::size_type index) const noexcept -> acore::size_type
{
    return mRecords[index].pos;
}

auto FileRecords::recordPos(acore::size_type index) const noexcept -> acore::size_type
{
    return mRecords[index].pos - static_cast<acore::size_type>(sizeof(Index));
}

auto FileRecords::remove(acore::size_type index) -> void
{
    updateRemovedIndex(index);
    mCount--;
}

auto FileRecords::set(acore::size_type index, Index record) -> void
{
    mRecords[index] = record;
}

auto FileRecords::size(acore::size_type index) const noexcept -> acore::size_type
{
    return mRecords[index].size;
}

auto FileRecords::sortedIndexes() -> std::vector<acore::size_type>
{
    std::vector<acore::size_type> indexes(mRecords.size());
    std::iota(indexes.begin(), indexes.end(), 0);
    std::sort(indexes.begin(), indexes.end(), [&](acore::size_type left, acore::size_type right) {
        return mRecords[left].pos < mRecords[right].pos;
    });

    return indexes;
}

auto FileRecords::update(Index index) -> void
{
    mRecords[index.pos].size = index.size;
    update(recordPos(index.pos), index);
}

auto FileRecords::update(acore::size_type pos, Index index) -> void
{
    mData->write(pos, (acore::DataStream{} << index).buffer().data());
}

auto FileRecords::buildFreeList() -> void
{
    for (Index &index : mRecords)
    {
        if (index.size < 0)
        {
            index.pos = mFreeIndex;
            mFreeIndex = recordIndex(&index);
        }
    }
}

auto FileRecords::createIndex() -> void
{
    const auto count = mData->load<acore::size_type>(0);

    if (count < 0)
    {
        throw acore::Exception() << "Index corrupted";
    }

    mRecords.resize(count);
}

auto FileRecords::initialize() -> void
{
    if (mData->size() != 0)
    {
        loadRecords();
    }
    else
    {
        mData->save(0, acore::size_type{0});
    }
}

auto FileRecords::isValid(Index index) noexcept -> bool
{
    return 0 <= index.size;
}

auto FileRecords::loadIndex() -> void
{
    acore::size_type pos = mData->pos();

    while (pos != mData->size())
    {
        pos = processIndex(mData->load<Index>(pos));
    }
}

auto FileRecords::loadRecords() -> void
{
    try
    {
        createIndex();
        loadIndex();
        buildFreeList();
    }
    catch (...)
    {
        throw acore::Exception() << "Could not load file: invalid or corrupted file data";
    }
}

auto FileRecords::logicalRecordPos(acore::size_type pos) noexcept -> acore::size_type
{
    return pos + static_cast<acore::size_type>(sizeof(Index));
}

auto FileRecords::processIndex(Index index) -> acore::size_type
{
    if (isValid(index))
    {
        mRecords[index.pos] = Index{mData->pos(), index.size};
        mCount++;
    }

    return recordEnd(Index{mData->pos(), index.size});
}

auto FileRecords::recordEnd(Index index) -> acore::size_type
{
    if (0 <= index.size)
    {
        return index.pos + index.size;
    }

    return index.pos - index.size;
}

auto FileRecords::recordIndex(const Index *idx) const noexcept -> acore::size_type
{
    return std::distance(mRecords.data(), idx);
}

auto FileRecords::saveRecordsCount() -> void
{
    mData->save(0, static_cast<acore::size_type>(mRecords.size()));
}

auto FileRecords::updateRemovedIndex(acore::size_type recordIndex) -> void
{
    mRecords[recordIndex] = {mFreeIndex, acore::INVALID_INDEX};
    mFreeIndex = recordIndex;
}
}
