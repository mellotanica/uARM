/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2010 Tomislav Jonjic
 * Copyright (C) 2014 Marco Melletti
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef UMPS_STOPPOINT_H
#define UMPS_STOPPOINT_H

#include <vector>
#include <map>
#include <string>
#include <cassert>

#include <QObject>
//#include <sigc++/sigc++.h>

#include "armProc/types.h"
#include "services/lang.h"

class processor;

enum AccessMode {
    AM_EXEC       = 1 << 0,
    AM_WRITE      = 1 << 1,
    AM_READ       = 1 << 2,
    AM_READ_WRITE = AM_WRITE | AM_READ
};

class AddressRange {
public:
    AddressRange(Word asid, Word start, Word end)
        : asid(asid),
          start(start),
          end(end)
    {
        assert(start <= end);
    }

    Word getASID() const { return asid; }
    Word getStart() const { return start; }
    Word getEnd() const { return end; }

    bool LessThan(const AddressRange& other) const
    {
        return (asid < other.asid) || (asid == other.asid && start < other.start);
    }

    bool operator<(const AddressRange& other) const
    {
        return LessThan(other);
    }

    bool Overlaps(const AddressRange& r) const
    {
        return !(asid != r.asid || r.end < start || r.start > end);
    }

    bool Contains(Word asid, Word addr) const
    {
        return asid == this->asid && start <= addr && addr <= end;
    }

private:
    Word asid, start, end;
};

class Stoppoint : public boost::enable_shared_from_this<Stoppoint> {
public:
    typedef boost::shared_ptr<Stoppoint> Ptr;
    typedef boost::shared_ptr<Stoppoint const> ConstPtr;

    Stoppoint(unsigned int id, const AddressRange& range, AccessMode mode)
        : id(id),
          enabled(true),
          range(range),
          accessMode(mode)
    {}

    unsigned int getId() const { return id; }

    void SetEnabled(bool setting) { enabled = setting; }
    bool IsEnabled() const { return enabled; }

    const AddressRange& getRange() const { return range; }

    void setAccessMode(AccessMode mode) { accessMode = mode; }
    AccessMode getAccessMode() const { return accessMode; }

    bool Matches(Word asid, Word addr, AccessMode mode) const
    {
        return (enabled &&
                range.Contains(asid, addr) &&
                (accessMode & mode));
    }

    std::string ToString() const;

private:
    unsigned int id;
    bool enabled;
    AddressRange range;
    AccessMode accessMode;
};


class StoppointSet : public QObject{
    Q_OBJECT
public:
    virtual ~StoppointSet();

    size_t Size() const { return points.size(); }
    bool IsEmpty() const { return points.empty(); }

    Stoppoint* Get(size_t index);
    const Stoppoint* Get(size_t index) const;

    Stoppoint* Find(Word asid, Word addr);

    bool CanInsert(const AddressRange& range) const;
    bool Add(const AddressRange& range, AccessMode mode);
    bool Add(const AddressRange& range, AccessMode mode, unsigned int id, bool enabled = true);
    void Remove(size_t index);
    void Clear();

    void SetEnabled(size_t index, bool setting);

    Stoppoint* Probe(Word asid, Word addr, AccessMode mode, const processor* cpu);

    template<typename OutputIterator>
    void GetStoppointsInRange(Word asid, Word start, Word end, OutputIterator out);

    std::string ToString(bool sorted = false) const;

signals:
    void SignalStoppointInserted();
    void SignalStoppointRemoved(size_t index);
    void SignalEnabledChanged(size_t index);
    void SignalHit(size_t index, const Stoppoint* sp, Word address, const processor* proc);

private:
    unsigned int nextId() const;

    typedef std::vector<Stoppoint::Ptr> StoppointVector;
    StoppointVector points;

    typedef std::map<AddressRange, Stoppoint*> StoppointMap;
    StoppointMap addressMap;

public:
    typedef StoppointVector::const_iterator const_iterator;
    typedef const_iterator iterator;
    const_iterator begin() const { return points.begin(); }
    const_iterator end() const { return points.end(); }
};

inline Stoppoint* StoppointSet::Get(size_t index)
{
    assert(index < Size());
    return points[index].get();
}

inline const Stoppoint* StoppointSet::Get(size_t index) const
{
    assert(index < Size());
    return points[index].get();
}

template<typename OutputIterator>
void StoppointSet::GetStoppointsInRange(Word asid, Word start, Word end, OutputIterator out)
{
    AddressRange r1(asid, start, start);
    AddressRange r2(asid, end, end);

    StoppointMap::const_iterator it = addressMap.lower_bound(r1);
    for (it = addressMap.lower_bound(r1);
         it != addressMap.end() && (it->first < r2 || !(r2 < it->first));
         ++it)
    {
        *out++ = it->second;
    }
}

#endif // UMPS_STOPPOINT_H
