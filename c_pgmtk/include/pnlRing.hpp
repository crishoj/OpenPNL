/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                INTEL CORPORATION PROPRIETARY INFORMATION                //
//   This software is supplied under the terms of a license agreement or   //
//  nondisclosure agreement with Intel Corporation and may not be copied   //
//   or disclosed except in accordance with the terms of that agreement.   //
//       Copyright (c) 2003 Intel Corporation. All Rights Reserved.        //
//                                                                         //
//  File:      pnlRing.hpp                                                 //
//                                                                         //
//  Purpose:   Implementation of the algorithm                             //
//                                                                         //
//  Author(s):                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __PNLRING_HPP__
#define __PNLRING_HPP__

#pragma warning (push, 3 )
#pragma warning(disable: 4284)
#include <deque>
#pragma warning(pop)

//using namespace std;
PNL_BEGIN

template<class T>
class CRing {
public:
    typedef pnlVector<T> myDeque;

    class iterator;
    friend class iterator;
    class iterator : public std::iterator<
        std::bidirectional_iterator_tag,T,ptrdiff_t>
    {
        typename myDeque::iterator it;
        myDeque *r;
    public:

        iterator()
        {
        }
        iterator(myDeque& rng,
            const typename myDeque::iterator& i)
            : r(&rng), it(i) {}
        bool operator==(const iterator& x) const
        {
            return it == x.it;
        }

        bool operator!=(const iterator& x) const
        {
            return !(*this == x);
        }

        typename myDeque::reference operator*() const
        {
            return *it;
        }

        typename myDeque::reference operator[](int i) const
        {
            return *it+i;
        }

        iterator& operator++()
        {
            ++it;
            if(it == r->end())
                it = r->begin();
            return *this;
        }

        iterator operator++(int)
        {
            iterator tmp = *this;
            ++*this;
            return tmp;
        }

        iterator operator +(int t)
        {

            return iterator(*this) += t;
        }

        iterator& operator+=(int t)
        {

            int s= r->size();
            typename myDeque::iterator b = r->begin();
            if(!s)
            {
                it = b+t;
            }
            else
            {

                if(t<0)
                {
                    t=s+t%s;
                }

                int pos=it-b;
                it = b+(pos+t)%s;
            }
            return *this;
        }

        iterator operator -(int t)
        {
            return iterator(*this) -= t;
        }

        iterator& operator-=(int t)
        {
            return *this += -t;
        }

        iterator& operator--()
        {
            if(it == r->begin())
                it = r->end();
            --it;
            return *this;
        }

        iterator operator--(int)
        {
            iterator tmp = *this;
            --*this;
            return tmp;
        }

        iterator insert(const T& x)
        {
            return iterator(*r, r->insert(it, x));
        }
        iterator erase()
        {
            return iterator(*r, r->erase(it));
        }
    };

    void push_back(const T& x)
    {
        rng.push_back(x);
    }

    void clear()
    {
        rng.clear();
    }

    void resize(int n)
    {
        rng.resize(n);
    }

    iterator begin()
    {
        return iterator(rng, rng.begin());
    }

    typename myDeque::reference operator[](int i)
    {
        return *( iterator(rng, rng.begin())+i);
    }

    int size()
    {
        return rng.size();
    }

    void assign(int n, const T& x = T())
    {
        rng.assign(n, x);
    }

private:
    myDeque rng;
};
PNL_END
#endif
