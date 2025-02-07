//
// Contributors: Alfie
//

#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H
#include <vector>

template <typename T>
class PriorityQueue
{
public:
    explicit PriorityQueue(unsigned short int maxPriority);
    ~PriorityQueue();
    [[nodiscard]] T operator[] (int const i) const { return Get(i); }

    /**
     * @param atEnd Whether the new item should become the last item of the target priority (TRUE) or the first (FALSE)
     */
    void Insert(T item, unsigned short int priority, bool atEnd = true);

    void Remove(T item);

    [[nodiscard]]
    T Get(unsigned int index) const { return vec.at(index); }

    void Clear() { vec.clear(); }

    /**
     * @return Ordered std::vector of all values
     */
    [[nodiscard]]
    std::vector<T> GetValues() const { return vec; }

    [[nodiscard]]
    unsigned int GetLength() const { return vec.size(); }

protected:
    unsigned short int maxPriority;

    // Array of indices pointing to the first position of each priority level
    unsigned short int* priorityPointers;

    // Vector of values, ordered by priority.
    std::vector<T> vec;
};


template <typename T>
PriorityQueue<T>::PriorityQueue(unsigned short int const maxPriority) : maxPriority(maxPriority)
{
    priorityPointers = new unsigned short int [maxPriority] { }; // Create pointers array and assign all values to 0
    vec = std::vector<T>();
}


template<typename T>
PriorityQueue<T>::~PriorityQueue() {
    delete[] priorityPointers;
}


template<typename T>
void PriorityQueue<T>::Insert(T item, unsigned short int const priority, bool const atEnd) {
    // Insert the item into vec
    if (priority >= maxPriority) {
        vec.insert(atEnd ? vec.begin() + priorityPointers[maxPriority - 1] : vec.begin(), item);
    } else {
        vec.insert(atEnd ? (priority == 0 ? vec.end() : vec.begin() + priorityPointers[priority - 1])
            : vec.begin() + priorityPointers[priority], item);
    }

    // Update priorityPointers
    for (int i = 0; i < priority; i++) priorityPointers[i]++;
}


template<typename T>
void PriorityQueue<T>::Remove(T item) {
    // Check that the item is in vec, return if not
    auto it = find(vec.begin(), vec.end(), item);
    if (it == vec.end()) return;

    // Cache item's index in vec
    int const index = it - vec.begin();

    // Erase item from vec
    vec.erase(it);

    // Update priorityPointers
    for (int i = 0; i < maxPriority; i++) {
        if (priorityPointers[i] < index) break;
        priorityPointers[i]--;
    }
}

#endif //PRIORITYQUEUE_H
