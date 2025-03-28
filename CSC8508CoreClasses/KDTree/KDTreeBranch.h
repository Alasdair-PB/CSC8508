//
// Contributors: Alfie
//

#ifndef KDTREEBRANCH_H
#define KDTREEBRANCH_H
#include <cfloat>
#include <cmath>
#include <iostream>

#include "KDTreeLeaf.h"
#include "KDTreeNode.h"
#include "../../NCLCoreClasses/Axis.h"

template <typename T>
class KDTreeBranch : public KDTreeNode<T> {
public:
    /**
     * Divisible KDTreeBranch
     */
    KDTreeBranch(Axis const division_axis, float const division, KDTreeNode<T>* more_node, KDTreeNode<T>* less_node)
        : moreNode(more_node), lessNode(less_node), division(division),
        divisionAxis(division_axis), isIndivisible(false) { }

    /**
     * Indivisible KDTreeBranch
     */
    KDTreeBranch(KDTreeNode<T>* fixed_node, KDTreeNode<T>* less_node) : moreNode(fixed_node), lessNode(less_node),
                                                                       divisionAxis(x), division(FLT_MAX), isIndivisible(true) { }

    bool Insert(KDTreeEntry<T> entry) override;

    void Get(KDTreeQuery const& query, std::vector<KDTreeEntry<T>>& output) const override;

    bool Remove(KDTreeEntry<T> const& entry) override;

    static KDTreeBranch* Split(KDTreeNode<T>* node);

    void SetIsIndivisible(bool value) { this->isIndivisible = value; }

protected:
    KDTreeNode<T> * moreNode, * lessNode;
    float division;
    Axis divisionAxis;

    /**
     * If indivisible, it means both leaves need checking for everything since there's no division
     */
    bool isIndivisible;

    [[nodiscard]]
    bool IsAboveDivision(KDTreeQuery const& query) const;
    [[nodiscard]]
    static bool IsAboveDivision(KDTreeQuery const& query, float const& division, Axis const& axis);
    [[nodiscard]]
    bool IsBeneathOrOnDivision(KDTreeQuery const& query) const;
    [[nodiscard]]
    static bool IsBeneathOrOnDivision(KDTreeQuery const& query, float const& division, Axis const& axis);
};


template<typename T>
bool KDTreeBranch<T>::Insert(KDTreeEntry<T> const entry) {

    if (IsAboveDivision(entry)) {
        if (!moreNode->Insert(entry)) {
            moreNode = Split(moreNode);
            moreNode->Insert(entry);
        }
    }

    if (IsBeneathOrOnDivision(entry))  // In an indivisible branch, it'll always go here
        if (!lessNode->Insert(entry)) {
            lessNode = Split(lessNode);
            lessNode->Insert(entry);
        }

    return true;
}


template<typename T>
void KDTreeBranch<T>::Get(KDTreeQuery const& query, std::vector<KDTreeEntry<T>>& output) const {

    if (IsAboveDivision(query) || isIndivisible)
        moreNode->Get(query, output);

    if (IsBeneathOrOnDivision(query)) // In an indivisible branch, it'll already call this
        lessNode->Get(query, output);
}


template<typename T>
bool KDTreeBranch<T>::Remove(KDTreeEntry<T> const& entry) {

    if (IsAboveDivision(entry) || isIndivisible)
        if (moreNode->Remove(entry)) isIndivisible = false;

    if (IsBeneathOrOnDivision(entry)) { // In an indivisible branch, it'll already go here
        lessNode->Remove(entry);
    }

    return false;
}


template<typename T>
KDTreeBranch<T>* KDTreeBranch<T>::Split(KDTreeNode<T>* node) {

    std::vector<KDTreeEntry<T>> vec;
    auto query = KDTreeQuery(Vector3(0.0f, 0.0f, 0.0f), Vector3(FLT_MAX, FLT_MAX, FLT_MAX));
    node->Get(query, vec);

    float bestDivision;
    int bestOverlaps = INT_MAX;
    int bestSplit = INT_MAX;
    Axis bestAxis = x;

    for (Axis splittingAxis = x; splittingAxis <= z; splittingAxis++) {

        // Get 1 dimensional bounds of each entry
        float points[MAX_ITEMS][2]; // [0] = min, [1] = max
        for (int i = 0; i < vec.size(); i++) {
            KDTreeEntry<T> e = vec.at(i);
            points[i][0] = e.position[splittingAxis] - e.halfDimensions[splittingAxis];
            points[i][1] = e.position[splittingAxis] + e.halfDimensions[splittingAxis];
        }

        // Try a division on each 1D vertex and cache it if it's the best
        for (auto const& point: points) {
            float divisionOnMin = point[0] - FLT_MIN;
            float divisionOnMax = point[1];

            int less = 0;
            int more = 0;
            for (KDTreeEntry<T> e: vec) {
                if (IsBeneathOrOnDivision(e, divisionOnMin, splittingAxis)) less++;
                if (IsAboveDivision(e, divisionOnMin, splittingAxis)) more++;
            }
            int overlaps = less + more - vec.size();
            int split = std::abs(less - more);

            if ((overlaps < bestOverlaps && split != vec.size()) // If this division has fewer overlaps
                || (overlaps == bestOverlaps && split < bestSplit) ) { // If it has the same amount of overlaps but a more even split
                bestDivision = divisionOnMin;
                bestOverlaps = overlaps;
                bestSplit = split;
                bestAxis = splittingAxis;
            }

            less = 0;
            more = 0;
            for (KDTreeEntry<T> e: vec) {
                if (IsBeneathOrOnDivision(e, divisionOnMax, splittingAxis)) less++;
                if (IsAboveDivision(e, divisionOnMax, splittingAxis)) more++;
            }
            overlaps = less + more - vec.size();
            split = std::abs(less - more);

            if ((overlaps < bestOverlaps && split != vec.size()) // If this division has fewer overlaps
                || (overlaps == bestOverlaps && split < bestSplit) ) { // If it has the same amount of overlaps but a more even split
                bestDivision = divisionOnMax;
                bestOverlaps = overlaps;
                bestSplit = split;
                bestAxis = splittingAxis;
            }
        }
    }

    if (bestOverlaps == MAX_ITEMS) {
        auto* newLessNode = new KDTreeLeaf<T>();
        auto* newMoreNode = node;
        auto* newBranch = new KDTreeBranch(newMoreNode, newLessNode);
        return newBranch;
    }

    // Split on the best division
    auto* newLessNode = new KDTreeLeaf<T>();
    auto* newMoreNode = new KDTreeLeaf<T>();
    auto* newBranch = new KDTreeBranch(bestAxis, bestDivision, newMoreNode, newLessNode);
    for (KDTreeEntry<T> e : vec) newBranch->Insert(e);
    return newBranch;
}


template<typename T>
bool KDTreeBranch<T>::IsAboveDivision(KDTreeQuery const& query) const {
    return IsAboveDivision(query, this->division, this->divisionAxis);
}


template<typename T>
bool KDTreeBranch<T>::IsAboveDivision(KDTreeQuery const& query, float const& division, Axis const& axis) {
    return query.position[axis] + query.halfDimensions[axis] > division;
}


template<typename T>
bool KDTreeBranch<T>::IsBeneathOrOnDivision(KDTreeQuery const& query) const {
    return IsBeneathOrOnDivision(query, this->division, this->divisionAxis);
}


template<typename T>
bool KDTreeBranch<T>::IsBeneathOrOnDivision(KDTreeQuery const& query, float const& division, Axis const& axis) {
    return query.position[axis] - query.halfDimensions[axis] <= division;
}


#endif //KDTREEBRANCH_H
