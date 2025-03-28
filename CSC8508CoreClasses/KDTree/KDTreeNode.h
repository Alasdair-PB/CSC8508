//
// Contributors: Alfie
//

#ifndef KDTREENODE_H
#define KDTREENODE_H

#define MAX_ITEMS 4

#include <vector>

#include "KDTreeEntry.h"

template <typename T>
class KDTreeNode {
public:
    virtual ~KDTreeNode() = default;

    /**
    * Inserts a new entry into this section of the KDTree
    * @param entry target KDTreeEntry
    * @return a leaf will return FALSE if it's full and needs dividing. Otherwise, if successful, it'll return TRUE.
    * Branches always return TRUE
    */
    virtual bool Insert(KDTreeEntry<T> entry) = 0;

    /**
    * Gets all entries from leaves that have possible colliding values
    * @param query KDTreeQuery query
    * @param output a std::vector that will hold all entries that collide with the query
    */
    virtual void Get(KDTreeQuery const& query, std::vector<KDTreeEntry<T>>& output) const = 0;

    /**
     * Remove a target entry from this section of the KDTree
     * @param entry target KDTreeEntry
     * @return A leaf will return TRUE if an object has been removed and FALSE otherwise. A branch will always return
     * FALSE
     */
    virtual bool Remove(KDTreeEntry<T> const& entry) = 0;

    virtual void Print() = 0;
};

#endif //KDTREENODE_H
