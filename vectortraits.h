#pragma once


/*#include "OwnPtr.h"
#include "RefPtr.h"
#include "TypeTraits.h"*/

#include <utility>
#include <memory>

using std::pair;

    template<bool isPod, typename T>
    struct VectorTraitsBase;

    template<typename T>
    struct VectorTraitsBase<false, T>
    {
        static const bool needsDestruction = true;
        static const bool needsInitialization = true;
        static const bool canInitializeWithMemset = false;
        static const bool canMoveWithMemcpy = false;
        static const bool canCopyWithMemcpy = false;
        static const bool canFillWithMemset = false;
        static const bool canCompareWithMemcmp = false;
    };

    template<typename T>
    struct VectorTraitsBase<true, T>
    {
        static const bool needsDestruction = false;
        static const bool needsInitialization = false;
        static const bool canInitializeWithMemset = false;
        static const bool canMoveWithMemcpy = true;
        static const bool canCopyWithMemcpy = true;
        static const bool canFillWithMemset = sizeof(T) == sizeof(char);
        static const bool canCompareWithMemcmp = true;
    };

    template<typename T>
    struct VectorTraits /*: VectorTraitsBase<IsPod<T>::value, T>*/ { };

    struct SimpleClassVectorTraits
    {
        static const bool needsDestruction = true;
        static const bool needsInitialization = true;
        static const bool canInitializeWithMemset = true;
        static const bool canMoveWithMemcpy = true;
        static const bool canCopyWithMemcpy = false;
        static const bool canFillWithMemset = false;
        static const bool canCompareWithMemcmp = true;
    };

    // we know OwnPtr and RefPtr are simple enough that initializing to 0 and moving with memcpy
    // (and then not destructing the original) will totally work
    /*template<typename P>
    struct VectorTraits<RefPtr<P> > : SimpleClassVectorTraits { };

    template<typename P>
    struct VectorTraits<OwnPtr<P> > : SimpleClassVectorTraits { };*/

    template<typename First, typename Second>
    struct VectorTraits<pair<First, Second> >
    {
        typedef VectorTraits<First> FirstTraits;
        typedef VectorTraits<Second> SecondTraits;

        static const bool needsDestruction = FirstTraits::needsDestruction || SecondTraits::needsDestruction;
        static const bool needsInitialization = FirstTraits::needsInitialization || SecondTraits::needsInitialization;
        static const bool canInitializeWithMemset = FirstTraits::canInitializeWithMemset && SecondTraits::canInitializeWithMemset;
        static const bool canMoveWithMemcpy = FirstTraits::canMoveWithMemcpy && SecondTraits::canMoveWithMemcpy;
        static const bool canCopyWithMemcpy = FirstTraits::canCopyWithMemcpy && SecondTraits::canCopyWithMemcpy;
        static const bool canFillWithMemset = false;
        static const bool canCompareWithMemcmp = FirstTraits::canCompareWithMemcmp && SecondTraits::canCompareWithMemcmp;
    };
