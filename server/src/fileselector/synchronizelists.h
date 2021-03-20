/*
 * Copyright (C) 2013 Jolla Mobile <andrew.den.exter@jollamobile.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#ifndef SYNCHRONIZELISTS_H
#define SYNCHRONIZELISTS_H

// Helper utility to synchronize a cached list with some reference list with correct
// QAbstractItemModel signals and filtering.

// If the reference list is populated incrementally synchronizeList can be called multiple times with the
// same variables c and r to progressively synchronize the lists.  After the final call completeSynchronizeList
// can be called to remove or append any items which remain unsynchronized.
// The Filtered variants allow the reference list to be filtered by a callback function to
// exclude unwanted items from the synchronized list.

template <typename T>
bool compareIdentity(const T &item, const T &reference)
{
    return item == reference;
}

template <typename Agent, typename ReferenceList>
int insertRange(Agent *agent, int index, int count, const ReferenceList &source, int sourceIndex)
{
    agent->insertRange(index, count, source, sourceIndex);
    return count;
}

template <typename Agent>
int removeRange(Agent *agent, int index, int count)
{
    agent->removeRange(index, count);
    return 0;
}

template <typename Agent, typename ReferenceList>
int updateRange(Agent *agent, int index, int count, const ReferenceList &source, int sourceIndex)
{
    Q_UNUSED(agent);
    Q_UNUSED(index);
    Q_UNUSED(source);
    Q_UNUSED(sourceIndex);
    return count;
}

template <typename Agent, typename CacheList, typename ReferenceList>
class SynchronizeList
{
    typedef typename CacheList::const_reference CacheItem;
    typedef typename ReferenceList::const_reference ReferenceItem;

public:
    SynchronizeList(
            Agent *agent,
            const CacheList &cache,
            int &c,
            const ReferenceList &reference,
            int &r)
        : agent(agent), cache(cache), c(c), reference(reference), r(r)
    {
        int lastEqualC = c;
        int lastEqualR = r;
        for (; c < cache.count() && r < reference.count(); ++c, ++r) {
            if (compareIdentity(cache.at(c), reference.at(r))) {
                continue;
            }

            if (c > lastEqualC) {
                lastEqualC += updateRange(agent, lastEqualC, c - lastEqualC, reference, lastEqualR);
                c = lastEqualC;
                lastEqualR = r;
            }

            bool match = false;

            // Iterate through both the reference and cache lists in parallel looking for first
            // point of commonality, when that is found resolve the differences and continue
            // looking.
            int count = 1;
            for (; !match && c + count < cache.count() && r + count < reference.count(); ++count) {
                CacheItem cacheItem = cache.at(c + count);
                ReferenceItem referenceItem = reference.at(r + count);

                for (int i = 0; i <= count; ++i) {
                    if (cacheMatch(i, count, referenceItem) || referenceMatch(i, count, cacheItem)) {
                        match = true;
                        break;
                    }
                }
            }

            // Continue scanning the reference list if the cache has been exhausted.
            for (int re = r + count; !match && re < reference.count(); ++re) {
                ReferenceItem referenceItem = reference.at(re);
                for (int i = 0; i < count; ++i) {
                    if (cacheMatch(i, re - r, referenceItem)) {
                        match = true;
                        break;
                    }
                }
            }

            // Continue scanning the cache if the reference list has been exhausted.
            for (int ce = c + count; !match && ce < cache.count(); ++ce) {
                CacheItem cacheItem = cache.at(ce);
                for (int i = 0; i < count; ++i) {
                    if (referenceMatch(i, ce - c, cacheItem)) {
                        match = true;
                        break;
                    }
                }
            }

            if (!match)
                return;

            lastEqualC = c;
            lastEqualR = r;
        }

        if (c > lastEqualC) {
            updateRange(agent, lastEqualC, c - lastEqualC, reference, lastEqualR);
        }
    }

private:
    // Tests if the cached contact id at i matches a referenceId.
    // If there is a match removes all items traversed in the cache since the previous match
    // and inserts any items in the reference set found to to not be in the cache.
    bool cacheMatch(int i, int count, ReferenceItem referenceItem)
    {
        if (compareIdentity(cache.at(c + i),  referenceItem)) {
            if (i > 0)
                c += removeRange(agent, c, i);
            c += insertRange(agent, c, count, reference, r);
            r += count;
            return true;
        } else {
            return false;
        }
    }

    // Tests if the reference contact id at i matches a cacheId.
    // If there is a match inserts all items traversed in the reference set since the
    // previous match and removes any items from the cache that were not found in the
    // reference list.
    bool referenceMatch(int i, int count, CacheItem cacheItem)
    {
        if (compareIdentity(reference.at(r + i), cacheItem)) {
            c += removeRange(agent, c, count);
            if (i > 0)
                c += insertRange(agent, c, i, reference, r);
            r += i;
            return true;
        } else {
            return false;
        }
    }

    Agent * const agent;
    const CacheList &cache;
    int &c;
    const ReferenceList &reference;
    int &r;
};

template <typename Agent, typename CacheList, typename ReferenceList>
void completeSynchronizeList(
        Agent *agent,
        const CacheList &cache,
        int &cacheIndex,
        const ReferenceList &reference,
        int &referenceIndex)
{
    if (cacheIndex < cache.count()) {
        agent->removeRange(cacheIndex, cache.count() - cacheIndex);
    }
    if (referenceIndex < reference.count()) {
        agent->insertRange(cache.count(), reference.count() - referenceIndex, reference, referenceIndex);
    }

    cacheIndex = 0;
    referenceIndex = 0;
}

template <typename Agent, typename CacheList, typename ReferenceList>
void synchronizeList(
        Agent *agent,
        const CacheList &cache,
        int &cacheIndex,
        const ReferenceList &reference,
        int &referenceIndex)
{
    SynchronizeList<Agent, CacheList, ReferenceList>(
                agent, cache, cacheIndex, reference, referenceIndex);
}

template <typename Agent, typename CacheList, typename ReferenceList>
void synchronizeList(Agent *agent, const CacheList &cache, const ReferenceList &reference)
{
    int cacheIndex = 0;
    int referenceIndex = 0;
    synchronizeList(agent, cache, cacheIndex, reference, referenceIndex);
    completeSynchronizeList(agent, cache, cacheIndex, reference, referenceIndex);
}

template <typename Agent, typename ReferenceList>
ReferenceList filterList(
        Agent *agent,
        const ReferenceList &reference)
{
    ReferenceList filtered;
    filtered.reserve(reference.count());
    foreach (const typename ReferenceList::value_type &value, reference)
        if (agent->filterValue(value))
            filtered.append(value);

    return filtered;
}

template <typename Agent, typename CacheList, typename ReferenceList>
void synchronizeFilteredList(
        Agent *agent,
        const CacheList &cache,
        int &cacheIndex,
        const ReferenceList &reference,
        int &referenceIndex)
{
    ReferenceList filtered = filterList(agent, reference);
    synchronizeList(agent, cache, cacheIndex, filtered, referenceIndex);
}

template <typename Agent, typename CacheList, typename ReferenceList>
void synchronizeFilteredList(Agent *agent, const CacheList &cache, const ReferenceList &reference)
{
    int cacheIndex = 0;
    int referenceIndex = 0;
    ReferenceList filtered = filterList(agent, reference);
    synchronizeList(agent, cache, cacheIndex, filtered, referenceIndex);
    completeSynchronizeList(agent, cache, cacheIndex, filtered, referenceIndex);
}

#endif
