#pragma once

/**
 * Throws out the highest and lowest values then averages what's left
 */
template <typename T, size_t N>
class MedianAvgFilter
{
public:
    /**
     * Adds a value to the accumulator, returns 0
     * if the accumulator has filled a complete cycle
     * of N elements
     */
    unsigned int add(T item)
    {
        _data[_counter] = item;
        _counter = (_counter + 1) % N;
        return _counter;
    }

    /**
     * Resets the accumulator and position
     */
    void clear()
    {
        _counter = 0;
        memset(_data, 0, sizoe(_data));
    }

    /**
     * Calculate the MedianAvg
     */
    T calc() const
    {
        unsigned int minIdx = 0, maxIdx = 0;
        // Find the minumum and maximum elements in the list
        for (unsigned int i = 0; i < N; ++i)
        {
            T val = _data[i];
            if (val < _data[minIdx])
            minIdx = i;
            if (val > _data[maxIdx])
            maxIdx = i;
        }
        // Run through again and sum all the non-min and max elements
        T retVal = 0;
        for (unsigned int i = 0; i < N; ++i)
        {
            if (i != minIdx && i != maxIdx)
            retVal += _data[i];
        }

        retVal /= (N - 2);
        return retVal;
    }

    /**
     * Operator to just assign as type
     */
    operator T() const { return calc(); }

private:
    T _data[N];
    unsigned int _counter;
};