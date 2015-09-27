#include <utility>
#include <iterator>
#include <functional>

#ifndef _AGENDA_H
#define _AGENDA_H

template<typename T>
class BeamIterator {
public:
    typedef BeamIterator<T> self;

    typedef T value_type;
    typedef T *pointer;
    typedef T &reference;

public:
    T *node;

public:
    BeamIterator() = default;

    BeamIterator(T *x) : node(x) { }

    BeamIterator(const self &b) : node(b.node) { }

    bool operator==(const self &x) const { return node == x.node; }

    bool operator!=(const self &x) const { return node != x.node; }

    reference operator*() { return *node; }

    pointer operator->() { return node; }

    reference operator*() const { return *node; }

    pointer operator->() const { return node; }

    self &operator++() {
        ++node;
        return *this;
    }

    self &operator++(int) {
        self tmp = *this;
        ++*this;
        return tmp;
    }

    self &operator--() {
        --node;
        return *this;
    }

    self &operator--(int) {
        self tmp = *this;
        ++*this;
        return tmp;
    }
};

template<typename KEY_TYPE, int SIZE, typename COMPARE = std::greater<KEY_TYPE> >
class AgendaBeam {
public:
    typedef BeamIterator<KEY_TYPE> iterator;
    typedef BeamIterator<const KEY_TYPE> const_iterator;

private:
    int m_nBeamSize;
    bool m_bItemSorted;
    KEY_TYPE m_lBeam[SIZE];
    COMPARE comparer;

    void pop_heap();

    void push_heap(int base);

public:

    AgendaBeam() : m_nBeamSize(0), m_bItemSorted(false) { }

    ~AgendaBeam() = default;

    int size();

    void clear();

    void insertItem(const KEY_TYPE &item);

    void sortItems();

    const KEY_TYPE &bestItem(const int &index = 0);

    iterator begin() { return &m_lBeam[0]; }

    iterator end() { return &m_lBeam[m_nBeamSize]; }

    const_iterator begin() const { return &m_lBeam[0]; }

    const_iterator end() const { return &m_lBeam[m_nBeamSize]; }

    const KEY_TYPE &operator[](int index) { return m_lBeam[index]; }
};

template<typename KEY_TYPE, int SIZE, typename COMPARE>
void AgendaBeam<KEY_TYPE, SIZE, COMPARE>::push_heap(int base) {
    while (base > 0) {
        int next_base = (base - 1) >> 1;
        if (m_lBeam[next_base] > m_lBeam[base]) {
            std::swap(m_lBeam[next_base], m_lBeam[base]);
            base = next_base;
        }
        else {
            break;
        }
    }
}

template<typename KEY_TYPE, int SIZE, typename COMPARE>
void AgendaBeam<KEY_TYPE, SIZE, COMPARE>::pop_heap() {
    if (m_nBeamSize <= 0) {
        return;
    }
    std::swap(m_lBeam[0], m_lBeam[--m_nBeamSize]);
    KEY_TYPE smallest(m_lBeam[0]);
    int index = 0, child = 1;
    while (child < m_nBeamSize) {
        if (child + 1 < m_nBeamSize && !(m_lBeam[child + 1] > m_lBeam[child])) {
            ++child;
        }
        m_lBeam[index] = m_lBeam[child];
        index = child;
        child = (child << 1) + 1;
    }
    m_lBeam[index] = smallest;
    push_heap(index);
}

template<typename KEY_TYPE, int SIZE, typename COMPARE>
void AgendaBeam<KEY_TYPE, SIZE, COMPARE>::clear() {
    m_nBeamSize = 0;
    m_bItemSorted = false;
}

template<typename KEY_TYPE, int SIZE, typename COMPARE>
int AgendaBeam<KEY_TYPE, SIZE, COMPARE>::size() {
    return m_nBeamSize;
}

template<typename KEY_TYPE, int SIZE, typename COMPARE>
void AgendaBeam<KEY_TYPE, SIZE, COMPARE>::insertItem(const KEY_TYPE &item) {
    if (m_nBeamSize == SIZE) {
        if (comparer(item, m_lBeam[0])) {
            pop_heap();
        }
        else {
            return;
        }
    }
    m_lBeam[m_nBeamSize] = item;
    push_heap(m_nBeamSize++);
}

template<typename KEY_TYPE, int SIZE, typename COMPARE>
void AgendaBeam<KEY_TYPE, SIZE, COMPARE>::sortItems() {
    if (m_bItemSorted) {
        return;
    }
    int size = m_nBeamSize;
    while (m_nBeamSize > 0) {
        pop_heap();
    }
    m_nBeamSize = size;
    m_bItemSorted = true;
}

template<typename KEY_TYPE, int SIZE, typename COMPARE>
const KEY_TYPE &AgendaBeam<KEY_TYPE, SIZE, COMPARE>::bestItem(const int &index) {
    if (!m_bItemSorted) {
        sortItems();
    }
    return m_lBeam[index];
}

#endif