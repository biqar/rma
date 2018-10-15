/*
 * test_bh07_v2.cpp
 *
 *  Created on: Jul 10, 2018
 *      Author: dleo@cwi.nl
 */



#include <climits>

#define CATCH_CONFIG_MAIN
#include "third-party/catch/catch.hpp"

#include "pma/driver.hpp"
#include "pma/adaptive/bh07_v2/packed_memory_array.hpp"
#include "pma/adaptive/bh07_v2/predictor.hpp"

using namespace pma;
using namespace pma::adaptive::bh07_v2;
using namespace std;

TEST_CASE("predictor"){
    Predictor predictor{4, 2}; // 4 positions, max count = 2

    REQUIRE(predictor.empty());
    REQUIRE(predictor.size() == 0);

    predictor.update(1); // [k=1, c=1], k = key, c = count
    predictor.update(1); // [k=1, c=2]
    REQUIRE(!predictor.empty());
    REQUIRE(predictor.size() == 1);
    /*SECTION("pred_010")*/{
        // the results are always sorted by the field `key'
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 1);
        REQUIRE(v[0].m_pointer == 1);
        REQUIRE(v[0].m_count == 2);
    }

    predictor.update(1); // [k=1, c=1]
    /*SECTION("pred_020")*/{
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 1);
        REQUIRE(v[0].m_pointer == 1);
        REQUIRE(v[0].m_count == 1);
    }

    predictor.update(1); // [k=1, c=2]
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 1);
        REQUIRE(v[0].m_pointer == 1);
        REQUIRE(v[0].m_count == 2);
    }

    predictor.update(2); // [k=1, c=2], [k=2, c=1]
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 2);
        REQUIRE(v[0].m_pointer == 1);
        REQUIRE(v[0].m_count == 2);
        REQUIRE(v[1].m_pointer == 2);
        REQUIRE(v[1].m_count == 1);
    }

    predictor.update(3); // [k=1, c=2], [k=2, c=1], [k=3, c=1]
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 3);
        REQUIRE(v[0].m_pointer == 1);
        REQUIRE(v[0].m_count == 2);
        REQUIRE(v[1].m_pointer == 2);
        REQUIRE(v[1].m_count == 1);
        REQUIRE(v[2].m_pointer == 3);
        REQUIRE(v[2].m_count == 1);
    }

    predictor.update(4); // [k=1, c=2], [k=2, c=1], [k=3, c=1], [k=4, c=1]
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 4);
        REQUIRE(v[0].m_pointer == 1);
        REQUIRE(v[0].m_count == 2);
        REQUIRE(v[1].m_pointer == 2);
        REQUIRE(v[1].m_count == 1);
        REQUIRE(v[2].m_pointer == 3);
        REQUIRE(v[2].m_count == 1);
        REQUIRE(v[3].m_pointer == 4);
        REQUIRE(v[3].m_count == 1);
    }

    predictor.update(3); // [k=1, c=2], [k=2, c=1], [k=4, c=1], [k=3, c=2]
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 4);
        REQUIRE(v[0].m_pointer == 1);
        REQUIRE(v[0].m_count == 2);
        REQUIRE(v[1].m_pointer == 2);
        REQUIRE(v[1].m_count == 1);
        REQUIRE(v[2].m_pointer == 3);
        REQUIRE(v[2].m_count == 2);
        REQUIRE(v[3].m_pointer == 4);
        REQUIRE(v[3].m_count == 1);
    }

    // no space left, decrease the count at the tail
    predictor.update(5); // [k=1, c=1], [k=2, c=1], [k=4, c=1], [k=3, c=2]
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 4);
        REQUIRE(v[0].m_pointer == 1);
        REQUIRE(v[0].m_count == 1);
        REQUIRE(v[1].m_pointer == 2);
        REQUIRE(v[1].m_count == 1);
        REQUIRE(v[2].m_pointer == 3);
        REQUIRE(v[2].m_count == 2);
        REQUIRE(v[3].m_pointer == 4);
        REQUIRE(v[3].m_count == 1);
    }
    predictor.update(5); // [k=2, c=1], [k=4, c=1], [k=3, c=2]
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 3);
        REQUIRE(v[0].m_pointer == 2);
        REQUIRE(v[0].m_count == 1);
        REQUIRE(v[1].m_pointer == 3);
        REQUIRE(v[1].m_count == 2);
        REQUIRE(v[2].m_pointer == 4);
        REQUIRE(v[2].m_count == 1);
    }

    // finally add <5, 5> at the front
    predictor.update(5); // [k=2, c=1], [k=4, c=1], [k=3, c=2], [k=5, c=1]
    {
        auto v1 = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v1.size() == 4);
        REQUIRE(v1[0].m_pointer == 2);
        REQUIRE(v1[0].m_count == 1);
        REQUIRE(v1[1].m_pointer == 3);
        REQUIRE(v1[1].m_count == 2);
        REQUIRE(v1[2].m_pointer == 4);
        REQUIRE(v1[2].m_count == 1);
        REQUIRE(v1[3].m_pointer == 5);
        REQUIRE(v1[3].m_count == 1);

        /**
         * Multiply by 10 the pointer of all elements
         */
        for(size_t i = 0; i < v1.size(); i++){
            predictor.reset_ptr(v1[i].m_permuted_position, v1[i].m_pointer * 10);
        }

        /**
         * Check the content of the updated pointers
         */
        auto v2 = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v2.size() == 4);
        REQUIRE(v2[0].m_pointer == 20);
        REQUIRE(v2[0].m_count == 1);
        REQUIRE(v2[1].m_pointer == 30);
        REQUIRE(v2[1].m_count == 2);
        REQUIRE(v2[2].m_pointer == 40);
        REQUIRE(v2[2].m_count == 1);
        REQUIRE(v2[3].m_pointer == 50);
        REQUIRE(v2[3].m_count == 1);
    }


    // add twice <6, 60>. The first time remove the tail, the second time add the element at the front
    predictor.update(60); // [k=40, c=1], [k=30, c=2], [k=50, c=1]
    predictor.update(60); // [k=40, c=1], [k=30, c=2], [k=50, c=1], [k=60, c=1]
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 4);
        REQUIRE(v[0].m_pointer == 30);
        REQUIRE(v[0].m_count == 2);
        REQUIRE(v[1].m_pointer == 40);
        REQUIRE(v[1].m_count == 1);
        REQUIRE(v[2].m_pointer == 50);
        REQUIRE(v[2].m_count == 1);
        REQUIRE(v[3].m_pointer == 60);
        REQUIRE(v[3].m_count == 1);
    }

    // increase the count at both the front and the tail by one. Swap the tail with its next element
    predictor.update(40); // [k=30, c=2], [k=40, c=2], [k=50, c=1], [k=60, c=1]
    predictor.update(60); // [k=30, c=2], [k=40, c=2], [k=50, c=1], [k=60, c=2]
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 4);
        REQUIRE(v[0].m_pointer == 30);
        REQUIRE(v[0].m_count == 2);
        REQUIRE(v[1].m_pointer == 40);
        REQUIRE(v[1].m_count == 2);
        REQUIRE(v[2].m_pointer == 50);
        REQUIRE(v[2].m_count == 1);
        REQUIRE(v[3].m_pointer == 60);
        REQUIRE(v[3].m_count == 2);
    }

    // update the front. The count is already at the max (2), so decrease the tail by 1
    predictor.update(60); // [k=30, c=1], [k=40, c=2], [k=50, c=1], [k=60, c=2]
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 4);
        REQUIRE(v[0].m_pointer == 30);
        REQUIRE(v[0].m_count == 1);
        REQUIRE(v[1].m_pointer == 40);
        REQUIRE(v[1].m_count == 2);
        REQUIRE(v[2].m_pointer == 50);
        REQUIRE(v[2].m_count == 1);
        REQUIRE(v[3].m_pointer == 60);
        REQUIRE(v[3].m_count == 2);
    }

    // again update the front, this time it removes the element at the tail
    predictor.update(60); // [k=40, c=2], [k=50, c=1], [k=60, c=2]
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 3);
        REQUIRE(v[0].m_pointer == 40);
        REQUIRE(v[0].m_count == 2);
        REQUIRE(v[1].m_pointer == 50);
        REQUIRE(v[1].m_count == 1);
        REQUIRE(v[2].m_pointer == 60);
        REQUIRE(v[2].m_count == 2);
    }

    // resize by copying all elements (next power of 2)
    predictor.resize(5);
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 3);
        REQUIRE(v[0].m_pointer == 40);
        REQUIRE(v[0].m_count == 2);
        REQUIRE(v[1].m_pointer == 50);
        REQUIRE(v[1].m_count == 1);
        REQUIRE(v[2].m_pointer == 60);
        REQUIRE(v[2].m_count == 2);
    }

    predictor.update(70); // [k=40, c=2], [k=50, c=1], [k=60, c=2], [k=70, c=1]
    predictor.update(80); // [k=40, c=2], [k=50, c=1], [k=60, c=2], [k=70, c=1], [k=80, c=1]
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 5);
        REQUIRE(v[0].m_pointer == 40);
        REQUIRE(v[0].m_count == 2);
        REQUIRE(v[1].m_pointer == 50);
        REQUIRE(v[1].m_count == 1);
        REQUIRE(v[2].m_pointer == 60);
        REQUIRE(v[2].m_count == 2);
        REQUIRE(v[3].m_pointer == 70);
        REQUIRE(v[3].m_count == 1);
        REQUIRE(v[4].m_pointer == 80);
        REQUIRE(v[4].m_count == 1);
    }

    // remove the element at the tail
    predictor.update(90); // [k=40, c=1], [k=50, c=1], [k=60, c=2], [k=70, c=1], [k=80, c=1]
    predictor.update(90); // [k=50, c=1], [k=60, c=2], [k=70, c=1], [k=80, c=1]
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 4);
        REQUIRE(v[0].m_pointer == 50);
        REQUIRE(v[0].m_count == 1);
        REQUIRE(v[1].m_pointer == 60);
        REQUIRE(v[1].m_count == 2);
        REQUIRE(v[2].m_pointer == 70);
        REQUIRE(v[2].m_count == 1);
        REQUIRE(v[3].m_pointer == 80);
        REQUIRE(v[3].m_count == 1);
    }

    // resize again, it should only shift by one position the capacity, i.e. no need to perform any copy
    predictor.resize(6);
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 4);
        REQUIRE(v[0].m_pointer == 50);
        REQUIRE(v[0].m_count == 1);
        REQUIRE(v[1].m_pointer == 60);
        REQUIRE(v[1].m_count == 2);
        REQUIRE(v[2].m_pointer == 70);
        REQUIRE(v[2].m_count == 1);
        REQUIRE(v[3].m_pointer == 80);
        REQUIRE(v[3].m_count == 1);
    }

    // add two more elements
    predictor.update(90); // [k=50, c=1], [k=60, c=2], [k=70, c=1], [k=80, c=1], [k=90, c=1]
    predictor.update(100); // [k=50, c=1], [k=60, c=2], [k=70, c=1], [k=80, c=1], [k=90, c=1], [k=100, c=1]
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 6);
        REQUIRE(v[0].m_pointer == 50);
        REQUIRE(v[0].m_count == 1);
        REQUIRE(v[1].m_pointer == 60);
        REQUIRE(v[1].m_count == 2);
        REQUIRE(v[2].m_pointer == 70);
        REQUIRE(v[2].m_count == 1);
        REQUIRE(v[3].m_pointer == 80);
        REQUIRE(v[3].m_count == 1);
        REQUIRE(v[4].m_pointer == 90);
        REQUIRE(v[4].m_count == 1);
        REQUIRE(v[5].m_pointer == 100);
        REQUIRE(v[5].m_count == 1);
    }

    // no room left, remove the last element
    predictor.update(110); // [k=60, c=2], [k=70, c=1], [k=80, c=1], [k=90, c=1], [k=100, c=1]
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 5);
        REQUIRE(v[0].m_pointer == 60);
        REQUIRE(v[0].m_count == 2);
        REQUIRE(v[1].m_pointer == 70);
        REQUIRE(v[1].m_count == 1);
        REQUIRE(v[2].m_pointer == 80);
        REQUIRE(v[2].m_count == 1);
        REQUIRE(v[3].m_pointer == 90);
        REQUIRE(v[3].m_count == 1);
        REQUIRE(v[4].m_pointer == 100);
        REQUIRE(v[4].m_count == 1);
    }

    // remove the keys 7, 8
    predictor.update(60); // [k=60, c=2], [k=80, c=1], [k=90, c=1], [k=100, c=1]
    predictor.update(60); // [k=60, c=2], [k=90, c=1], [k=100, c=1]
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 3);
        REQUIRE(v[0].m_pointer == 60);
        REQUIRE(v[0].m_count == 2);
        REQUIRE(v[1].m_pointer == 90);
        REQUIRE(v[1].m_count == 1);
        REQUIRE(v[2].m_pointer == 100);
        REQUIRE(v[2].m_count == 1);
    }

    // add two more keys
    predictor.update(10); // [k=60, c=2], [k=90, c=1], [k=100, c=1], [k=10,c=1]
    predictor.update(80); // [k=60, c=2], [k=90, c=1], [k=100, c=1], [k=10,c=1], [k=80,c=1]
    {
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 5);
        REQUIRE(v[0].m_pointer == 10);
        REQUIRE(v[0].m_count == 1);
        REQUIRE(v[1].m_pointer == 60);
        REQUIRE(v[1].m_count == 2);
        REQUIRE(v[2].m_pointer == 80);
        REQUIRE(v[2].m_count == 1);
        REQUIRE(v[3].m_pointer == 90);
        REQUIRE(v[3].m_count == 1);
        REQUIRE(v[4].m_pointer == 100);
        REQUIRE(v[4].m_count == 1);
    }

    // resize by shifting the elements
    predictor.resize(8);
    {   // same result as above
        auto v = predictor.items(numeric_limits<size_t>::min(), numeric_limits<size_t>::max());
        REQUIRE(v.size() == 5);
        REQUIRE(v[0].m_pointer == 10);
        REQUIRE(v[0].m_count == 1);
        REQUIRE(v[1].m_pointer == 60);
        REQUIRE(v[1].m_count == 2);
        REQUIRE(v[2].m_pointer == 80);
        REQUIRE(v[2].m_count == 1);
        REQUIRE(v[3].m_pointer == 90);
        REQUIRE(v[3].m_count == 1);
        REQUIRE(v[4].m_pointer == 100);
        REQUIRE(v[4].m_count == 1);
    }

    // filtering on [k=60, c=2], [k=90, c=1], [k=100, c=1], [k=10,c=1], [k=80,c=1]
    { // no results
        auto v = predictor.items(20, 40);
        REQUIRE(v.empty());
    }
    { // one result, k = 60
        auto v = predictor.items(20, 60);
        REQUIRE(v.size() == 1);
        REQUIRE(v[0].m_pointer == 60);
        REQUIRE(v[0].m_count == 2);
    }
    { // as above, one result, k = 60
        auto v = predictor.items(20, 70);
        REQUIRE(v.size() == 1);
        REQUIRE(v[0].m_pointer == 60);
        REQUIRE(v[0].m_count == 2);
    }
    { // three results, k = 10, 60, 80
        auto v = predictor.items(10, 80);
        REQUIRE(v.size() == 3);
        REQUIRE(v[0].m_pointer == 10);
        REQUIRE(v[0].m_count == 1);
        REQUIRE(v[1].m_pointer == 60);
        REQUIRE(v[1].m_count == 2);
        REQUIRE(v[2].m_pointer == 80);
        REQUIRE(v[2].m_count == 1);
    }
}

// copy & paste from test_static_abtree.cpp
TEST_CASE("sanity"){
    initialise();

    auto driver0 = make_shared<APMA_BH07_v2>(/* index block size */ 8, /* segment size */ 32, /* extent size */ 8);
    REQUIRE(driver0->size() == 0);
    driver0->insert(1, 10);
    driver0->insert(3, 30);
    driver0->insert(2, 20);
    driver0->insert(4, 40);
    driver0->insert(5, 50);
    driver0->insert(6, 60);
    driver0->insert(10, 100);
    driver0->insert(11, 110);
    driver0->insert(9, 90);
    REQUIRE(driver0->size() == 9);

    auto it = driver0->iterator();
    int64_t prev = -1;
    while(it->hasNext()){
        auto p = it->next();
        REQUIRE(prev < p.first);
        prev = p.first;
    }

    for(int64_t i = 0; i <= 12; i++){
        auto res = driver0->find(i);
        // Elements 1, 2, 3, 4, 5, 6, 9, 10, 11 are present and should be found
        if((i >= 1 && i <= 6) || (i >= 9 && i <= 11)){
            REQUIRE(res == i * 10);
        } else {
            REQUIRE(res == -1);
        }
    }
}

// copy & paste from test_static_abtree.cpp
TEST_CASE("find_range_dense"){
    initialise();

    constexpr size_t sz = 344; // 7 * 7 * 7 + 1

    APMA_BH07_v2 pma{/* index block size */ 8, /* segment size */ 32, /* extent size */ 8};
    for(int i = 1; i <= sz; i++){
        pma.insert(i, i * 100);
    }
    REQUIRE(pma.size() == sz);

    for(int i = 0; i <= pma.size() +1; i++){
        for(int j = i; j <= pma.size() +1; j++){
            auto it = pma.find(i,j);
            int64_t min = -1, max = -1;
            if(it->hasNext()) min = max = it->next().first;
            while(it->hasNext()) max = it->next().first;

//            cout << "find(" << i << ", " << j << ") min: " << min << ", max: " << max << endl;

            // check min
            if(i == 0) {
                if(j == 0){
                    REQUIRE(min == -1);
                } else {
                    REQUIRE(min == 1);
                }
            } else if (i > pma.size()){ // as the first case
                REQUIRE(min == -1);
            } else {
                REQUIRE(min == i);
            }

            // check max
            if(i == 0 && j == 0){
                REQUIRE(max == -1);
            } else if (j > pma.size()){
                if(i > pma.size()){ // as the first case
                    REQUIRE(max == -1);
                } else {
                    REQUIRE(max == pma.size());
                }
            } else {
                REQUIRE(max == j);
            }
        }
    }

}

// copy & paste from test_static_abtree.cpp
TEST_CASE("find_range_with_gaps"){
    initialise();

    constexpr size_t sz = 344; // 7 * 7 * 7 + 1

    APMA_BH07_v2 pma{/* index block size */ 8, /* segment size */ 32, /* extent size */ 8};
    for(int i = 1; i <= sz; i++){
        pma.insert(2 * i, 2 * i * 100);
    }
    REQUIRE(pma.size() == sz);

    for(int i = 0; i <= 2 * pma.size() +1; i++){
        for(int j = i; j <= 2 * pma.size() +1; j++){
            auto it = pma.find(i,j);
            int64_t min = -1, max = -1;
            if(it->hasNext()) min = max = it->next().first;
            while(it->hasNext()) max = it->next().first;

//            cout << "find(" << i << ", " << j << ") min: " << min << ", max: " << max << endl;

            // check min
            if(i == 0) {
                if(j < 2){
                    REQUIRE(min == -1);
                } else {
                    REQUIRE(min == 2);
                }
            } else if (i > 2 * pma.size()){ // as the first case
                REQUIRE(min == -1);
            } else {
                if (i % 2 == 0){
                    REQUIRE(min == i);
                } else if ( i == j ) { // find(x, x) with x odd
                    REQUIRE(min == -1);
                } else { // find( 2*k -1, y ) implies min == 2*k
                    REQUIRE(min == i+1);
                }
            }

            // check max
            if(j < 2){ // 2 is the first key
                REQUIRE(max == -1);
            } else if ( i > 2 * pma.size() ){ // as above
                REQUIRE(max == -1);
            } else if ( j > 2 * pma.size() ){ // scan [i, 2 * B.size()] with i <= 2 * B.size()
                REQUIRE(max == 2 * pma.size());
            } else if ( j % 2 == 0 ) { // j is in the interval of the B+Tree [min,max] and is even
                REQUIRE(max == j);
            } else if ( i == j ) { // both i && j are odd, i.e. find (x,x)
                REQUIRE(max == -1);
            } else { // j is odd, but i is even and i < j
                REQUIRE(max == j -1);
            }
        }
    }
}

// copy & paste from test_static_abtree.cpp
TEST_CASE("duplicates"){
    initialise();

    APMA_BH07_v2 pma{/* index block size */ 8, /* segment size */ 32, /* extent size */ 8};

    // insert the elements
    size_t cardinality = 0;
    for(size_t i = 0; i < 8; i++){
        pma.insert(2, 200 + i);
        cardinality++;
    }
    for(size_t i = 0; i < 32; i++){
        pma.insert(1, 100 + i);
        cardinality++;
    }
    for(size_t i = 0; i < 3; i++){
        pma.insert(4, 400 + i);
        cardinality++;
    }
    pma.insert(3, 300); cardinality++;
    for(size_t i = 0; i < 64; i++){
        pma.insert(5, 500 + i);
        cardinality++;
    }
    REQUIRE(pma.size() == cardinality);

    { // check all elements have been inserted!
        size_t index = 0;
        int64_t previous = -1;
        auto it = pma.iterator();
        while(it->hasNext()){
            auto p = it->next();
            REQUIRE(p.first >= previous);
            previous = p.first;
            index++;
        }
        REQUIRE(pma.size() == index);
    }

    { // search the 3 elements with key = 4
        auto it = pma.find(4, 4);
        size_t index = 0;
        while(it->hasNext()){
            auto p = it->next();
            REQUIRE(p.first == 4);

            index++;
        }
        REQUIRE(index == 3);
    }

    { // search the 4 elements with keys = 3, 4
        auto it = pma.find(3, 4);
        size_t index = 0;
        while(it->hasNext()){
            auto p = it->next();
            bool check = p.first == 3 || p.first == 4;
            REQUIRE(check);
            index++;
        }
        REQUIRE(index == 4);
    }

    { // search the 40 elements with keys = 1, 2
        auto it = pma.find(1, 2);
        size_t index = 0;
        while(it->hasNext()){
            auto p = it->next();
            bool check = p.first == 1 || p.first == 2;
            REQUIRE(check);
            index++;
        }
        REQUIRE(index == 40);
    }

    { // search the 64 elements with key = 5
        auto it = pma.find(5, 5);
        size_t index = 0;
        while(it->hasNext()){
            auto p = it->next();
            REQUIRE(p.first == 5);
            REQUIRE(p.second >= 500);
            REQUIRE(p.second < 600);
            index++;
        }
        REQUIRE(index == 64);
    }
}

//// copy & paste from test_static_abtree.cpp
TEST_CASE("find_range_with_duplicates"){
    initialise();

    constexpr size_t num_duplicates = 100;
    constexpr size_t num_keys = 17;
    APMA_BH07_v2 tree{/* index block size */ 8, /* segment size */ 32, /* extent size */ 8};

    for(int i = 0; i < num_duplicates; i++){
        for(int j = 0; j < num_keys; j++){
            tree.insert(j, j * 100 + i);
        }
    }

    for(int j = 0; j < num_keys; j++){
        auto it = tree.find(j,j);
        auto sum = 0;
        while(it->hasNext()){
            auto e = it->next();
            REQUIRE(e.first == j);
            REQUIRE(e.second / 100 == j);
            sum++;
        }
        REQUIRE(sum == num_duplicates);
    }
}

void rewiring_check(const vector<int64_t>& entries, bool check_all_entries = false, size_t block_size = 32){
    initialise();

    APMA_BH07_v2 tree{/* index block size */ block_size, /* segment size */ block_size, /* extent size */ 2};
    REQUIRE(tree.size() == 0);
    REQUIRE(tree.empty());

    for(size_t i = 0; i < entries.size(); i++){
        tree.insert(entries[i], entries[i] + 100);

        if(check_all_entries){
            for(size_t j = 0; j <= i; j++){
                auto v = tree.find(entries[j]);
                REQUIRE(v == entries[j] + 100);
            }
        }
    }

    for(size_t i = 0; i < entries.size(); i++){
        auto v = tree.find(entries[i]);
        REQUIRE(v == entries[i] + 100);
    }

    REQUIRE(!tree.empty());
    REQUIRE(tree.size() == entries.size());
}


TEST_CASE("rew_sequential"){
    const size_t cardinality = 10000;
    vector<int64_t> keys;
    keys.reserve(cardinality);
    for(size_t i = 1; i <= cardinality; i++){
        keys.push_back(i);
    }
    rewiring_check(keys, /* check all keys after an insert ? */ false);
}

TEST_CASE("rew_sequential_rev"){
    const size_t cardinality = 10000;
    vector<int64_t> keys;
    keys.reserve(cardinality);
    for(int64_t i = cardinality; i >= 1; i--){
        keys.push_back(i);
    }
    rewiring_check(keys);
}

TEST_CASE("rew_uniform"){
    // vector<int64_t> keys{ ... } would take ages to compile
    int64_t array_keys[] = {16268, 9640, 5961, 705, 2465, 15317, 13106, 12091, 5505, 14689,
    3605, 6903, 13363, 12727, 9320, 926, 3149, 16096, 11899, 13243,
    16247, 12061, 5071, 11493, 15319, 15575, 9608, 9287, 3868, 9462,
    11849, 5327, 11901, 819, 11091, 6487, 10626, 5671, 9301, 14123, 4234,
    7060, 14183, 15475, 230, 3831, 13826, 2730, 14445, 9099, 8421, 14199,
    3202, 11123, 7029, 6960, 2672, 12399, 1661, 7908, 7447, 4007, 7176,
    10256, 4679, 1553, 190, 393, 6196, 8683, 14013, 9568, 3193, 4848,
    3613, 4084, 11713, 4622, 7188, 3870, 4590, 624, 6232, 599, 12523,
    14785, 807, 9728, 3791, 8745, 10482, 11024, 15789, 1300, 11868,
    10770, 11100, 2771, 8702, 7091, 13563, 13723, 14217, 2948, 3694,
    15967, 12425, 1337, 8244, 12232, 14872, 4820, 5791, 16205, 7299, 883,
    12245, 6884, 14713, 2801, 7621, 6650, 4090, 13770, 3733, 14152, 8721,
    12056, 14234, 10311, 1734, 13258, 540, 13198, 6492, 2661, 10431,
    3693, 8812, 2094, 11802, 4043, 12025, 4275, 10457, 13495, 8285,
    10617, 13346, 16037, 13985, 5479, 5445, 5263, 2722, 3842, 10160,
    5013, 8527, 85, 14299, 3136, 2009, 2248, 2734, 3992, 12499, 13088,
    8223, 3002, 12334, 10534, 10821, 6605, 14401, 13729, 13508, 14832,
    4927, 3573, 5478, 4626, 10970, 10666, 16053, 12037, 13006, 7774,
    14981, 2486, 782, 2630, 15073, 13128, 2902, 4277, 15047, 1740, 16314,
    9364, 5016, 5337, 10182, 14766, 750, 6748, 13866, 1032, 12608, 3550,
    2827, 16051, 8136, 14963, 25, 16318, 12131, 7170, 5749, 12225, 14387,
    14674, 4133, 10399, 8111, 9272, 4155, 5395, 14776, 14235, 13288,
    4406, 8114, 127, 7330, 13553, 1587, 5077, 653, 3510, 13488, 11654,
    2795, 16062, 4367, 14911, 4472, 15907, 11833, 15673, 6124, 901, 4309,
    6631, 6971, 5992, 7429, 8661, 8196, 6802, 12364, 15607, 3218, 4268,
    11769, 13273, 8124, 2204, 11697, 5447, 2641, 3675, 7711, 2459, 6296,
    11164, 6583, 3970, 13556, 3116, 7442, 8885, 13634, 10603, 9157,
    10073, 13075, 16089, 6095, 12658, 14541, 3334, 708, 12150, 6966,
    14353, 8294, 371, 14418, 990, 10993, 7986, 6195, 15942, 8697, 15536,
    1200, 11013, 12178, 2706, 14300, 7361, 12669, 8790, 7408, 1955, 3102,
    9499, 9900, 15402, 13315, 864, 14696, 6790, 2896, 14355, 13199,
    12660, 11009, 13892, 9701, 11167, 12309, 4462, 1812, 13268, 10422,
    10702, 7260, 10700, 3798, 201, 13007, 16175, 1517, 3557, 5477, 2985,
    767, 11105, 7191, 13816, 11443, 12118, 779, 15594, 625, 387, 1003,
    5905, 7967, 8163, 1782, 12934, 10151, 4442, 15252, 14424, 7094, 5359,
    11954, 2737, 9695, 13451, 13932, 6403, 7580, 2815, 1839, 10624, 9235,
    7655, 8386, 14497, 972, 4920, 4153, 5242, 8041, 11415, 13506, 1010,
    13812, 13440, 12826, 8013, 3029, 14854, 13841, 4178, 13473, 2682,
    1478, 2994, 1357, 6397, 14273, 13361, 1168, 103, 9170, 11283, 512,
    10196, 4215, 9250, 4904, 4616, 1439, 7197, 14630, 13430, 2550, 8171,
    8910, 4173, 9459, 13885, 4302, 15931, 1248, 4438, 5080, 15952, 4244,
    5155, 1080, 6040, 11353, 14109, 1605, 1140, 1789, 1487, 4997, 7878,
    8069, 11112, 4692, 2781, 8634, 2071, 2817, 15172, 14996, 7223, 4807,
    2063, 8994, 14040, 9278, 9471, 1610, 14519, 11428, 1062, 8209, 12168,
    2255, 532, 6489, 3109, 10593, 3083, 609, 12538, 9212, 2951, 7619,
    12564, 5599, 11192, 15642, 13426, 263, 14737, 1613, 14720, 1958,
    11888, 116, 11835, 14580, 1094, 5986, 11369, 14036, 5157, 2162, 3070,
    13193, 11910, 15200, 8653, 1910, 14688, 14313, 15588, 6159, 3664,
    3158, 8345, 3360, 8552, 3131, 9291, 9899, 1705, 4030, 4149, 12836,
    7205, 12982, 9293, 13921, 3916, 12265, 7388, 7688, 5177, 8032, 11341,
    13234, 15239, 10210, 11064, 6249, 8499, 3388, 8999, 13659, 6722,
    6571, 3729, 10823, 7757, 16295, 2433, 10242, 626, 14193, 14589,
    15086, 1383, 16026, 12411, 11247, 13297, 3579, 10305, 6326, 4140,
    11140, 16331, 13654, 12096, 13368, 11229, 1554, 7881, 10550, 5230,
    5668, 3186, 13064, 15315, 140, 3406, 5586, 12692, 9977, 15986, 9244,
    13919, 13262, 15606, 10762, 13599, 3361, 15727, 12146, 11685, 15464,
    14978, 2363, 10342, 14491, 9191, 8346, 1681, 5826, 7355, 12812, 4356,
    5439, 4735, 9030, 15426, 4744, 13863, 13791, 1908, 11870, 12734,
    8831, 9978, 5050, 4313, 5322, 6265, 1223, 15364, 4152, 15933, 1303,
    9965, 14754, 15067, 2472, 6806, 13177, 2294, 11892, 273, 12675, 4251,
    16186, 6482, 6695, 2718, 8870, 11890, 11420, 757, 4831, 14, 7682,
    3355, 11507, 14150, 15820, 3636, 8338, 5535, 15743, 14143, 11495,
    5565, 13026, 8276, 3990, 6935, 14201, 5104, 236, 15988, 9202, 1942,
    12397, 12566, 9880, 6254, 2851, 2367, 5611, 731, 15755, 9826, 15868,
    15370, 3421, 12730, 13801, 7880, 9363, 9338, 15891, 2150, 13086,
    12641, 13158, 9609, 8427, 5891, 361, 6521, 15717, 2678, 2636, 3635,
    1663, 6704, 10066, 13104, 6000, 12639, 10366, 6256, 15248, 2298,
    8144, 3985, 480, 3263, 10332, 13212, 7412, 1987, 7748, 15029, 12805,
    110, 2662, 7954, 4216, 3489, 15209, 4764, 8359, 3522, 7576, 16196,
    6529, 4506, 7108, 3851, 678, 9445, 12784, 15882, 1949, 3899, 13888,
    8038, 2499, 1597, 4345, 10464, 2876, 13441, 8678, 15493, 464, 8553,
    9199, 16211, 16057, 10735, 11393, 5121, 10344, 14915, 14492, 4986,
    14598, 5446, 9996, 7764, 14795, 16164, 8579, 7393, 2114, 13879, 4047,
    15729, 15971, 3767, 15865, 13117, 8391, 3785, 15934, 5501, 2924,
    6400, 13955, 10162, 6467, 1612, 5994, 3596, 5887, 7972, 3923, 13604,
    4522, 13602, 8675, 10417, 4801, 11371, 5643, 445, 6781, 7138, 3915,
    7318, 13852, 11530, 4477, 7174, 15360, 11148, 5346, 2171, 7775, 6161,
    16185, 10742, 15691, 426, 10314, 10589, 4857, 7117, 2447, 5179,
    15199, 13217, 4076, 10321, 3096, 15863, 7745, 15557, 8509, 15020,
    2018, 7951, 5704, 6362, 929, 16199, 12796, 8770, 12262, 9906, 2807,
    1602, 14169, 4723, 10907, 12406, 6950, 14323, 13635, 13303, 677,
    15331, 6614, 13603, 12324, 8526, 14975, 11272, 15806, 12656, 2993,
    2932, 741, 14733, 1467, 16345, 11187, 2278, 13060, 9712, 6590, 8857,
    9643, 4748, 1301, 4308, 14057, 9289, 13185, 10428, 13744, 12562,
    16005, 11108, 145, 5224, 14367, 11533, 3542, 14153, 6156, 4424,
    14993, 8738, 7625, 13393, 12453, 8991, 7111, 536, 341, 5369, 14740,
    6208, 5548, 16070, 11227, 325, 6680, 7171, 9240, 10980, 13997, 11905,
    1946, 5519, 4599, 6179, 14003, 7349, 12598, 11349, 4846, 14075,
    13090, 10171, 2930, 10599, 12086, 9104, 15809, 2259, 9774, 9933, 623,
    235, 5602, 6476, 14927, 12450, 12082, 3805, 3631, 594, 307, 5069,
    7902, 6998, 1833, 10454, 436, 12663, 6452, 7334, 15963, 792, 3731,
    15091, 12123, 3525, 2164, 4385, 2205, 14079, 3483, 5871, 3692, 6480,
    11751, 810, 379, 11879, 4684, 5028, 8200, 9942, 5537, 6306, 10449,
    4957, 13862, 15057, 12427, 500, 9091, 14399, 6230, 15034, 10727,
    16305, 3433, 7303, 14014, 10130, 8378, 14458, 3991, 13492, 5581, 290,
    3659, 9409, 2321, 7328, 15602, 7427, 6003, 4908, 12437, 15945, 352,
    6970, 14581, 14970, 5620, 8920, 14205, 434, 11332, 8179, 8792, 15180,
    4289, 5714, 5000, 9800, 15030, 3661, 5217, 8008, 940, 15192, 7077,
    386, 3089, 7420, 366, 14769, 14503, 9083, 15810, 4455, 10644, 7982,
    14773, 3212, 13836, 14856, 13588, 14351, 4057, 4732, 1628, 10037,
    11675, 16311, 2222, 6423, 1710, 10581, 5148, 11198, 11803, 15846,
    5098, 9442, 11088, 14252, 4882, 4458, 8250, 12159, 3566, 3929, 4808,
    428, 5936, 13554, 7405, 2903, 10584, 5754, 13348, 7519, 13960, 984,
    8696, 13663, 14475, 2233, 12139, 3743, 16338, 2107, 8010, 13498,
    5940, 12239, 4368, 1630, 8587, 14255, 2638, 1707, 7784, 13245, 7770,
    11510, 2729, 13797, 11284, 15392, 10959, 9665, 11955, 12707, 4115,
    10720, 13311, 5500, 3594, 7438, 7934, 9544, 10293, 15253, 861, 6070,
    6657, 8848, 6877, 9846, 4086, 5456, 3740, 11631, 14388, 10092, 14463,
    2024, 1790, 3197, 7175, 11177, 10356, 9263, 1414, 2050, 15790, 14164,
    570, 1427, 15314, 14836, 1188, 2684, 787, 7616, 1379, 11330, 4018,
    5675, 1913, 50, 1504, 5005, 10974, 1444, 4383, 4675, 1408, 11621,
    812, 1979, 8583, 12351, 5152, 10715, 9711, 4577, 6937, 6993, 11146,
    4985, 5239, 10280, 8789, 3761, 3749, 6283, 11843, 10173, 7776, 3014,
    3440, 10200, 1513, 253, 15936, 1270, 3038, 2227, 11796, 4556, 7161,
    14824, 2516, 9785, 8462, 2191, 4260, 783, 12610, 11857, 8199, 5367,
    12277, 14647, 1395, 11220, 4327, 15611, 1111, 14347, 6507, 15182,
    16133, 2149, 15767, 10904, 3340, 5247, 2411, 4595, 12768, 12107,
    11721, 11256, 6325, 13042, 3163, 9153, 9226, 5363, 7869, 14718, 4210,
    11223, 12899, 14116, 7857, 2, 1645, 13944, 15106, 15851, 5055, 470,
    5189, 16142, 13880, 16113, 9624, 12718, 15101, 14450, 5366, 1334,
    13280, 9189, 13387, 3768, 7583, 4941, 6368, 1924, 10396, 12534,
    10789, 7452, 14187, 12733, 15136, 9652, 8327, 8478, 9842, 557, 2108,
    146, 11368, 1508, 10063, 14032, 4360, 14527, 8490, 8689, 3508, 3364,
    5303, 4739, 1350, 6915, 4042, 492, 13143, 12529, 11189, 8943, 10031,
    14462, 14930, 4982, 3839, 5769, 15099, 16266, 6440, 8572, 6432, 686,
    15407, 7703, 1066, 5837, 5873, 4546, 13206, 14827, 13248, 3875, 3772,
    8651, 7542, 10817, 12440, 8998, 15103, 5313, 7907, 4128, 7502, 9200,
    12976, 9439, 13202, 1185, 8879, 2238, 3515, 13408, 5588, 16256, 5934,
    15, 5541, 3940, 12735, 16380, 68, 16114, 1078, 3266, 12704, 4564,
    6415, 12117, 5657, 15843, 11786, 3599, 10831, 5046, 4330, 13711,
    3021, 11465, 5723, 9806, 11618, 7516, 11644, 6726, 15477, 15681,
    7500, 6823, 15710, 11668, 16351, 950, 8872, 6318, 14277, 2581, 6584,
    14241, 1561, 3878, 401, 216, 13714, 12911, 11742, 10843, 9093, 11222,
    4989, 5522, 602, 134, 3628, 3254, 4226, 8647, 7819, 16219, 12898,
    12979, 10188, 11280, 14119, 13586, 4879, 11142, 4014, 14118, 8977,
    2334, 12027, 300, 4701, 13184, 7257, 8217, 1779, 9095, 5459, 13263,
    7462, 452, 3896, 45, 6028, 12612, 7609, 4720, 12325, 8311, 10443,
    16344, 13166, 11262, 5648, 7548, 716, 1818, 16285, 8131, 1378, 10570,
    3011, 15960, 8554, 2090, 1257, 8374, 1828, 6548, 2625, 5952, 6560,
    1391, 13676, 9296, 913, 11035, 12136, 15510, 11451, 8815, 16161,
    3807, 11720, 8471, 12762, 13255, 3432, 10289, 7766, 10318, 2906,
    4885, 10466, 8497, 13775, 16045, 3910, 15926, 13575, 7722, 9111,
    1004, 7541, 754, 10733, 6204, 572, 14091, 14628, 10869, 10246, 10064,
    749, 13900, 10874, 12632, 14816, 4973, 15595, 12506, 13624, 15707,
    10323, 13050, 5141, 13335, 6964, 446, 14860, 14552, 9312, 14357,
    12203, 3658, 12544, 11022, 13507, 14723, 10233, 3233, 11680, 11007,
    8016, 5145, 8017, 14156, 12418, 871, 6514, 15082, 4824, 12817, 12571,
    15022, 5859, 7847, 10250, 7019, 8498, 11331, 10013, 8226, 2555, 4457,
    6734, 4717, 12471, 9147, 6500, 6760, 13694, 9141, 2746, 2362, 8235,
    3390, 10850, 11681, 8939, 7488, 3900, 15362, 10381, 3347, 11919,
    6755, 15504, 3018, 9274, 5472, 8508, 3316, 8113, 4518, 4348, 898,
    9421, 3860, 1755, 7809, 66, 16166, 9205, 9762, 15580, 10927, 13358,
    9710, 6828, 368, 5967, 863, 957, 4845, 13760, 9525, 6456, 10853, 496,
    6609, 7390, 1927, 15803, 10509, 3376, 11602, 11764, 5526, 16307,
    14145, 9605, 6801, 4553, 3004, 1578, 16222, 4423, 5667, 604, 13279,
    11325, 12992, 2323, 10261, 8495, 9298, 1031, 11866, 11914, 3022,
    15349, 1430, 6413, 13926, 13973, 8461, 825, 4910, 2387, 2508, 6233,
    6547, 4174, 11929, 11163, 2397, 3876, 14550, 9063, 13727, 11665,
    5468, 7131, 6319, 6337, 7344, 11603, 3648, 10120, 16043, 11684, 4396,
    12377, 12400, 12815, 15516, 9904, 10755, 7008, 14102, 504, 12543,
    6188, 10084, 8188, 4718, 1544, 11422, 12603, 9057, 16071, 3318,
    15991, 5876, 3702, 220, 13893, 11344, 13857, 10445, 11171, 13099,
    316, 6177, 16163, 7884, 508, 15701, 3671, 11772, 14661, 7357, 12683,
    11489, 1331, 701, 2049, 15283, 10301, 12670, 13419, 9997, 13523, 702,
    6641, 13187, 5532, 4074, 221, 5007, 2624, 4315, 4601, 4305, 10569,
    884, 14830, 13305, 1599, 5422, 3094, 3656, 6, 2767, 3416, 12084,
    9034, 11026, 15633, 10944, 7648, 9026, 168, 14103, 2421, 7983, 8305,
    4135, 15745, 5659, 7469, 11450, 14679, 11783, 14627, 15737, 8373,
    7342, 8009, 1838, 3601, 5330, 10001, 11158, 8101, 10429, 7540, 13043,
    2575, 9277, 7286, 7634, 8100, 1343, 4432, 2326, 7323, 6063, 5606,
    15635, 3506, 11709, 14570, 5499, 5977, 3422, 14862, 11049, 6414,
    12787, 8578, 7369, 2258, 11625, 5451, 16357, 9598, 11967, 6961, 4710,
    7646, 10376, 7220, 7834, 14655, 13471, 8726, 12791, 14742, 9850,
    11196, 13357, 13223, 14685, 5788, 15142, 9958, 15260, 14958, 6132,
    16287, 12457, 5556, 2410, 11044, 14777, 9566, 11116, 4940, 9982, 947,
    8687, 14960, 8856, 10076, 12678, 1649, 1438, 4137, 9353, 117, 2300,
    2880, 7137, 848, 2435, 16220, 2809, 2886, 4884, 14381, 1759, 2372,
    1048, 5170, 16317, 2286, 2565, 10016, 4962, 15191, 3697, 11206,
    14518, 13344, 6422, 3705, 11199, 10487, 16183, 12163, 5023, 9619,
    2473, 10744, 4006, 1352, 9267, 9112, 16302, 14261, 1728, 14386, 3775,
    14869, 2591, 12055, 11686, 9700, 7685, 14690, 15365, 4688, 9370,
    8459, 4975, 773, 1879, 1145, 7981, 15662, 12876, 12555, 5777, 13429,
    566, 2644, 15779, 1898, 6870, 9792, 15215, 14590, 14226, 1563, 10495,
    9344, 7185, 7428, 734, 6250, 14417, 5587, 519, 3629, 11979, 14023,
    4337, 12064, 7601, 7324, 3272, 902, 7620, 9716, 8243, 14736, 8600,
    14296, 13906, 9721, 5955, 16140, 10251, 10629, 1679, 5084, 13216,
    5238, 4003, 6625, 5138, 12824, 12631, 14667, 14946, 1911, 431, 9372,
    10146, 3337, 12498, 13952, 1258, 7845, 11998, 2925, 11411, 6543,
    8801, 93, 4105, 2786, 3150, 3720, 1704, 4724, 1876, 6995, 10940,
    9918, 3973, 1878, 6182, 15674, 13942, 12320, 4641, 15888, 1173, 1709,
    10859, 13530, 3090, 16179, 10682, 7476, 15211, 16316, 6054, 12350,
    4460, 668, 237, 13615, 8993, 13011, 1155, 14371, 7886, 12170, 14921,
    12119, 12756, 530, 5106, 12464, 7212, 423, 3793, 6719, 15042, 10436,
    16281, 6958, 6517, 6824, 9822, 12914, 6531, 186, 8556, 5131, 199,
    4069, 9236, 5021, 312, 2327, 8496, 3299, 10951, 10195, 4025, 14721,
    5904, 14142, 3111, 10632, 7236, 3191, 16226, 4809, 2688, 8960, 1521,
    12383, 3345, 2228, 14088, 11708, 16269, 2605, 8724, 11178, 15066,
    2844, 13433, 243, 477, 2514, 7726, 12722, 4170, 10966, 15151, 6875,
    4508, 3797, 4027, 15676, 8292, 11877, 15730, 12624, 2756, 14584,
    5139, 336, 10426, 12880, 13655, 5886, 8699, 15000, 6065, 14000, 4113,
    4623, 4212, 5585, 4, 5471, 9892, 8442, 5917, 15886, 10647, 5901,
    9412, 12681, 12187, 10846, 9053, 9659, 11197, 7826, 13513, 158, 5287,
    12270, 11042, 12702, 16217, 3391, 5782, 4336, 257, 13664, 14971,
    10577, 13352, 14287, 3776, 4401, 5569, 5617, 3085, 5236, 14964,
    16233, 472, 7923, 12850, 5646, 8868, 2236, 1896, 9521, 16364, 13598,
    9770, 3517, 16112, 14821, 6874, 11912, 9611, 15842, 1821, 10852,
    4868, 4901, 15339, 4228, 6203, 909, 5334, 15285, 2314, 9590, 9196,
    5695, 14804, 10364, 6624, 16255, 3005, 7867, 2402, 9211, 15302, 8205,
    3687, 13341, 12124, 12647, 3986, 9932, 14008, 5811, 5685, 14644,
    13547, 5761, 15777, 9000, 6842, 14404, 3099, 12243, 11114, 1836,
    6660, 3681, 5527, 12872, 10339, 3790, 7538, 7581, 1474, 13722, 2741,
    13983, 10187, 4791, 14105, 8491, 795, 12891, 2133, 736, 9854, 10139,
    11263, 11693, 2789, 12509, 11909, 11960, 7437, 10887, 4222, 12764,
    1892, 14370, 11364, 1947, 14056, 7855, 12343, 13691, 7167, 2573,
    11752, 5732, 13584, 3494, 3952, 7763, 13053, 15019, 3874, 7673,
    13049, 13811, 15974, 4266, 14544, 3000, 12739, 13171, 14067, 4436,
    9221, 873, 12199, 11867, 4952, 6037, 16319, 8007, 10788, 4521, 3866,
    16309, 7118, 5728, 12849, 13418, 8212, 7359, 8065, 2811, 1577, 3652,
    5162, 11728, 14803, 90, 14879, 10899, 816, 13616, 11150, 357, 10413,
    12016, 10455, 14480, 4038, 14435, 214, 5942, 5020, 3039, 958, 8456,
    14104, 1462, 5867, 13916, 12714, 550, 16134, 2158, 6675, 8326, 3466,
    1719, 12922, 5149, 5398, 12224, 11824, 7038, 7700, 11715, 15118,
    7721, 9589, 13325, 5630, 13628, 9555, 12073, 8031, 9726, 571, 8648,
    13219, 2104, 13750, 8832, 11862, 13995, 11390, 11512, 1832, 7675,
    4089, 1848, 13355, 2769, 8368, 5661, 1590, 4604, 8286, 6626, 10409,
    5368, 9332, 280, 2019, 12202, 9585, 12447, 2448, 16363, 10838, 8599,
    14329, 13670, 9683, 2585, 12519, 5549, 11101, 16042, 5765, 14636,
    351, 13145, 14454, 1547, 9231, 15554, 12023, 9845, 14779, 9684, 2580,
    8325, 5878, 5684, 2370, 7187, 4056, 16171, 12536, 11508, 1956, 12839,
    15377, 6544, 3786, 13564, 12960, 13843, 4822, 12649, 2763, 13370,
    6679, 4883, 1285, 3485, 2087, 3796, 9588, 8329, 4812, 2701, 3396,
    16209, 13017, 14240, 14701, 4382, 15628, 10878, 8787, 1891, 10018,
    11486, 5344, 6344, 10315, 7825, 11426, 5822, 8880, 6331, 4893, 917,
    3612, 11554, 88, 12480, 4915, 15125, 8778, 8488, 13482, 3072, 13105,
    12909, 2086, 12246, 8095, 15290, 5169, 808, 14933, 12777, 10137,
    1732, 10807, 212, 13804, 6572, 11378, 14065, 12408, 9951, 15417,
    8534, 9473, 15574, 3963, 9907, 12623, 10989, 3662, 4913, 561, 2621,
    2603, 1642, 450, 4280, 6602, 16082, 6537, 9704, 8863, 9752, 14822,
    11212, 15726, 1381, 4044, 11304, 7906, 13646, 6275, 2869, 4542, 9713,
    7839, 11366, 9969, 3946, 11248, 15696, 2897, 15010, 4999, 9204,
    11379, 12527, 7226, 6472, 13131, 13181, 3810, 6688, 15605, 5317,
    11032, 7460, 7441, 12575, 15890, 9886, 10987, 4640, 4379, 16195,
    14635, 15330, 12112, 4785, 2373, 15132, 15076, 7132, 8073, 10858,
    6015, 2858, 13039, 4714, 14222, 12693, 815, 14962, 10748, 12840, 400,
    185, 15708, 13078, 2089, 16131, 9276, 6026, 9152, 7468, 14656, 13674,
    13442, 16138, 5424, 6327, 7941, 2115, 12776, 5460, 9466, 13281,
    15982, 4746, 4870, 7186, 9703, 1526, 7765, 2524, 11176, 6019, 15532,
    14038, 16124, 11202, 8004, 12790, 13179, 12478, 15003, 10674, 409,
    1179, 15568, 5710, 14375, 7989, 373, 3447, 5254, 2140, 5163, 13209,
    3853, 11305, 7998, 458, 3358, 3836, 15450, 11019, 8316, 9470, 10427,
    2325, 8948, 1314, 12485, 11031, 8926, 7329, 6435, 14658, 3498, 2648,
    7479, 13065, 824, 9065, 15892, 14349, 11526, 133, 14602, 4562, 8565,
    3465, 10451, 7657, 3279, 4220, 8213, 6212, 13579, 9160, 15088, 11804,
    6887, 16012, 11622, 7031, 10879, 9038, 8504, 11853, 5614, 5997, 451,
    7023, 6388, 3593, 8259, 13696, 14605, 6732, 6088, 13432, 6034, 12810,
    7689, 5816, 11904, 2270, 6756, 9180, 15525, 11363, 1457, 1858, 11494,
    398, 6713, 5574, 740, 14900, 6659, 8107, 4121, 535, 9814, 8942, 1589,
    15004, 7025, 11211, 11743, 2898, 10841, 9181, 3724, 6430, 10179,
    15060, 2312, 5129, 4531, 14908, 1199, 9546, 12299, 15660, 9603, 9004,
    6163, 2416, 11295, 2234, 7009, 9480, 3231, 8308, 5018, 14588, 3560,
    7241, 5323, 1122, 10154, 3229, 7088, 12694, 14060, 15361, 4738, 4878,
    3257, 14034, 2272, 14468, 3747, 11038, 16197, 13771, 4095, 10253,
    6893, 1724, 13242, 11, 13253, 13686, 8641, 11001, 15139, 2612, 1501,
    16008, 9446, 3614, 5017, 10783, 589, 7196, 12609, 12029, 10721, 6033,
    6762, 8986, 3551, 10386, 5673, 12794, 10787, 12097, 886, 10506,
    15324, 13746, 4956, 2893, 12354, 13790, 3533, 8768, 2073, 12908, 817,
      15435, 8409, 12720, 13559, 14782, 11878, 4964, 4181, 8743, 14210,
    8839, 14354, 6471, 3122, 11320, 5232, 15573, 10564, 3562, 10568,
    10889, 1902, 8159, 7750, 2878, 1382, 1212, 9078, 14376, 10552, 654,
    9549, 12294, 466, 3977, 2719, 7313, 10291, 16075, 8283, 5030, 13196,
    2301, 2218, 11885, 1543, 8957, 9388, 9808, 10127, 12906, 282, 11429,
    7082, 1557, 13950, 3043, 2709, 8384, 11911, 9593, 7208, 198, 119,
    14182, 7242, 7504, 8269, 12226, 406, 5868, 14140, 6460, 13118, 5396,
    6930, 1132, 16036, 2320, 3825, 8026, 15670, 13332, 9345, 9985, 15958,
    7278, 15229, 2418, 3252, 3368, 12771, 10025, 13859, 10271, 6241,
    2153, 1683, 7182, 12220, 7804, 10481, 8964, 11707, 3487, 11144, 6281,
    10698, 3703, 8536, 15647, 1966, 14845, 777, 153, 11271, 1669, 6977,
    4254, 14181, 363, 275, 358, 2677, 2643, 10223, 2726, 8271, 2543,
    14613, 2614, 10094, 4005, 11797, 6407, 3437, 10316, 7156, 4533,
    16234, 3879, 14403, 5203, 12195, 6215, 476, 4630, 15140, 13375, 9047,
    8847, 7288, 3735, 13883, 9962, 13653, 13384, 15721, 14325, 11939,
    13115, 13675, 12247, 14043, 9970, 14597, 15552, 10294, 2631, 13497,
    15592, 8887, 2531, 5674, 2874, 15725, 11086, 10695, 6725, 49, 6412,
    9448, 14197, 7848, 11012, 6576, 13081, 13833, 4397, 12210, 11456,
    2126, 13837, 9998, 11041, 3452, 9540, 2892, 12748, 9893, 2753, 1183,
    15734, 370, 3190, 11632, 15358, 6234, 1520, 7604, 6694, 3093, 14204,
    6561, 14334, 13641, 4597, 6084, 2134, 7097, 4917, 15715, 10460, 6343,
    7515, 11342, 4654, 5523, 7513, 4088, 9475, 7687, 8074, 7071, 9543,
    9552, 15716, 7752, 9295, 656, 95, 1355, 7613, 5253, 11023, 14520,
    14617, 2209, 7160, 704, 13200, 3502, 5128, 2800, 12162, 12094, 16300,
    16066, 15213, 11137, 5039, 6369, 12962, 1715, 8612, 9694, 11822,
    8854, 14756, 15949, 3410, 1758, 12286, 11848, 3569, 3172, 15618,
    14893, 5976, 3300, 16383, 3476, 10686, 9596, 16291, 4991, 8932,
    11975, 1551, 11179, 12204, 1877, 4316, 971, 3505, 4825, 15565, 6024,
    396, 15198, 4324, 7570, 2940, 10134, 637, 7744, 15123, 6213, 9070,
    10176, 6171, 733, 14887, 6918, 9389, 6711, 13555, 12441, 2502, 8837,
    8402, 14639, 12487, 14877, 5965, 15373, 16122, 8761, 3852, 6185,
    10792, 5469, 7699, 1308, 403, 3861, 9206, 10471, 2792, 5404, 1803,
    16046, 4154, 16327, 510, 11705, 1220, 6720, 13308, 7102, 11745, 8198,
    7550, 4138, 5916, 11093, 16286, 15005, 1139, 8273, 839, 7072, 785,
    1750, 15474, 11282, 5524, 5252, 9303, 7095, 3987, 8122, 9820, 384,
    1718, 8800, 8034, 4071, 12205, 3712, 16210, 9577, 10141, 11942, 17,
    2916, 11372, 1312, 8568, 13631, 6929, 6763, 11424, 14317, 6350,
    11115, 10197, 11386, 614, 14866, 6504, 5662, 955, 1780, 675, 9039,
    5015, 15234, 7893, 5418, 10776, 10281, 13277, 10717, 7101, 996, 7517,
    13792, 14902, 2978, 7553, 9237, 11087, 14941, 12613, 3139, 5293,
    15312, 9606, 8663, 8331, 3978, 8019, 8826, 98, 4649, 16093, 5742,
    11240, 14115, 12378, 121, 4322, 8042, 7758, 13697, 5804, 6744, 5906,
    1125, 14743, 13454, 6059, 4621, 13986, 6364, 5112, 13620, 13707,
    1776, 9337, 9979, 8669, 11565, 15148, 8798, 2138, 6258, 539, 7178,
    8809, 13351, 7806, 4760, 9131, 12214, 11812, 13365, 13573, 5092,
    8624, 12098, 15381, 505, 11932, 7246, 2061, 8203, 1233, 2790, 13291,
    590, 333, 5978, 12213, 13183, 13541, 4922, 16151, 13968, 5944, 13751,
    10830, 7508, 14914, 8393, 5566, 1887, 11841, 51, 1954, 13321, 7471,
    6380, 12201, 6242, 9926, 14147, 517, 12156, 171, 8825, 1965, 4871,
    1810, 3837, 7904, 10953, 6346, 15915, 9299, 8901, 13681, 4620, 13945,
    8646, 11435, 5833, 781, 3204, 1945, 16260, 13095, 6540, 5266, 2731,
    4004, 7454, 15972, 4139, 9768, 304, 7047, 948, 11760, 8404, 7048,
    730, 4550, 14361, 3745, 12160, 13071, 5820, 9766, 5223, 2011, 6901,
    8206, 6682, 453, 7230, 9410, 3889, 8594, 8293, 4008, 15184, 3251,
    8929, 11698, 5848, 3669, 5115, 14215, 10219, 1348, 344, 8635, 11563,
    9290, 7136, 14778, 541, 6479, 3077, 8845, 10402, 1764, 7335, 9531,
    10740, 13192, 5068, 6187, 10790, 6002, 13274, 1940, 12173, 3979,
    12674, 441, 1112, 4180, 16176, 14265, 5003, 10267, 2759, 13838, 5930,
    6692, 8609, 9732, 14248, 5979, 2175, 12997, 999, 10732, 7190, 7367,
    4274, 682, 9741, 4479, 11225, 7769, 13595, 1115, 14389, 1574, 4839,
    841, 6078, 6546, 12474, 631, 1644, 15194, 6044, 2251, 10613, 4021,
    14298, 8270, 5227, 3059, 5809, 7554, 13182, 3846, 5880, 6644, 12421,
    5787, 10441, 15551, 12445, 9595, 12390, 10865, 14987, 2881, 1413,
    8475, 6697, 6807, 14548, 16306, 12368, 3872, 1999, 4151, 4373, 10354,
    13276, 6809, 12345, 6047, 1882, 2998, 12403, 11864, 14549, 3540,
    12798, 6868, 1469, 4364, 3253, 13887, 8596, 7606, 1490, 15508, 11762,
    6011, 4552, 5616, 9324, 7062, 16100, 5988, 4575, 3537, 11180, 13517,
    14186, 1655, 14618, 14433, 12893, 13951, 14154, 7595, 5072, 3429,
    10667, 5628, 6831, 7440, 7787, 3415, 6908, 10372, 15121, 14719,
    12835, 9757, 8135, 3075, 8911, 8484, 14292, 5443, 13496, 2468, 5022,
    14459, 6222, 13953, 7859, 448, 14496, 8457, 1238, 11152, 4145, 4144,
    945, 1368, 11532, 7415, 9897, 10234, 9463, 3243, 7828, 13963, 8740,
    14218, 7199, 5264, 3939, 549, 3653, 89, 3301, 13004, 15680, 6046,
    269, 16325, 10848, 13762, 9215, 5786, 2408, 14575, 13383, 4359, 5319,
    10935, 9258, 13028, 9330, 8752, 6812, 12099, 6434, 1279, 4618, 685,
    9617, 8728, 9963, 2034, 7552, 4905, 6982, 7918, 9436, 10259, 11591,
    7281, 2915, 5480, 11313, 3934, 7842, 6618, 7466, 7827, 16074, 3467,
    647, 311, 10421, 2023, 10958, 4967, 13578, 10078, 7905, 7406, 15078,
    10209, 10619, 8352, 11572, 14385, 14093, 6092, 7696, 5506, 12452,
    615, 14765, 101, 13510, 900, 10678, 10069, 9927, 7928, 10623, 2072,
    7733, 9673, 7195, 4651, 11329, 13695, 11471, 1935, 10041, 13907, 905,
    4773, 887, 3994, 14487, 15925, 6119, 12887, 6381, 10304, 6810, 4377,
    7708, 154, 10112, 1095, 6401, 15498, 890, 5377, 15390, 13322, 13087,
    1558, 13066, 14196, 12189, 8758, 12858, 1399, 276, 10483, 6027, 8796,
    16293, 10204, 4499, 2944, 8908, 5452, 15329, 4706, 9625, 3120, 5126,
    8816, 5423, 6288, 13613, 367, 11481, 9487, 9804, 10991, 8420, 9359,
    9648, 5100, 9760, 5699, 10563, 14421, 5343, 10635, 15350, 6622,
    12622, 9505, 5090, 8674, 10241, 12743, 11482, 1394, 14044, 15984,
    10191, 1434, 6542, 5689, 1753, 15418, 12054, 5309, 1456, 1068, 7916,
    9807, 14486, 10620, 10330, 11094, 14697, 6335, 8688, 15379, 11972,
    2393, 11845, 15597, 15429, 7039, 8182, 481, 671, 14266, 16167, 14055,
    14157, 1739, 5881, 321, 9582, 12153, 9749, 6607, 11133, 2105, 15998,
    9675, 9503, 11920, 13424, 15250, 6896, 14885, 9533, 2694, 1187, 4873,
    15827, 6596, 6445, 7844, 10165, 2692, 12802, 10688, 5677, 14029,
    13717, 11928, 8623, 5764, 14522, 528, 8145, 15928, 11959, 16145,
    11099, 10796, 5853, 4778, 12801, 3333, 12009, 15811, 4863, 16081,
    13803, 1344, 3232, 5933, 6106, 14947, 9101, 13169, 16315, 2671, 3372,
    10759, 7356, 329, 13465, 10861, 3650, 11418, 15159, 1034, 2698, 6262,
    7586, 2117, 1373, 9567, 1797, 13379, 8447, 14441, 16103, 13151,
    16342, 10469, 8803, 1693, 4431, 3998, 7830, 13747, 2201, 15410,
    11103, 10394, 14931, 13445, 14953, 15401, 2315, 9254, 10180, 13317,
    15061, 5494, 3928, 8511, 10194, 3040, 7565, 4148, 3555, 14868, 14925,
    6979, 5803, 5283, 5724, 7627, 10860, 2629, 9690, 9924, 4757, 7312,
    11275, 14151, 2798, 8240, 14831, 10880, 16129, 12083, 6107, 9, 7966,
    326, 4127, 7992, 14007, 8436, 3426, 10961, 9275, 16243, 13247, 8795,
    3371, 10129, 8279, 4412, 5475, 10761, 2441, 15107, 13283, 5743,
    14768, 11201, 6313, 4197, 8021, 4745, 14553, 3833, 10478, 6057, 7900,
    490, 8690, 4317, 1274, 5571, 4660, 1667, 13605, 10725, 6570, 12882,
    9198, 7234, 14645, 7336, 3425, 15544, 10988, 10155, 2337, 850, 13211,
    5349, 6378, 5757, 7940, 13665, 14542, 16034, 11375, 9641, 14346,
    5268, 7143, 10567, 10368, 11381, 2999, 7501, 15511, 15831, 5001,
    2843, 3482, 11194, 2157, 2096, 16267, 2303, 14526, 12183, 15423,
    1601, 5921, 627, 9610, 14610, 7856, 2547, 4369, 6836, 9018, 15027,
    3302, 14916, 8282, 6539, 10419, 684, 3604, 12640, 3679, 15672, 3908,
    13472, 12333, 15155, 13377, 7373, 11655, 13048, 12236, 12665, 3166,
    1978, 11903, 801, 10361, 8806, 1786, 7422, 1824, 2388, 5912, 7364,
    2436, 5897, 13301, 5354, 778, 11242, 9959, 8954, 12646, 14071, 7391,
    11657, 9284, 1600, 8642, 5414, 12152, 3412, 11125, 3024, 3258, 5888,
    4233, 8531, 8735, 397, 8000, 545, 14897, 15228, 14185, 4235, 12218,
    12180, 8460, 3883, 15062, 13703, 10814, 4031, 6351, 15083, 3168,
    12357, 9509, 2229, 14651, 6118, 1077, 15844, 8861, 13990, 10416,
    4548, 2275, 15207, 15371, 11385, 14564, 27, 44, 14969, 9858, 10694,
    8896, 9454, 14250, 160, 8562, 5594, 15460, 10090, 12842, 15287,
    16150, 1113, 3439, 9708, 9583, 4939, 8230, 9827, 1691, 8399, 5681,
    8898, 4853, 8607, 885, 8757, 8852, 12379, 10105, 6112, 2527, 936,
    1745, 11629, 16004, 744, 15901, 6656, 9356, 8543, 15683, 193, 9418,
    13512, 603, 11641, 4988, 11586, 16041, 14695, 3405, 1101, 6840,
    15459, 6994, 15929, 5306, 10992, 9314, 12525, 6081, 12330, 2927,
    8149, 6710, 14254, 6183, 11243, 1572, 1349, 5696, 3748, 16324, 9429,
    878, 11907, 5693, 109, 170, 6869, 7988, 5729, 9599, 11181, 8941,
    5854, 8720, 120, 9098, 9096, 224, 7958, 15941, 11015, 4866, 16192,
    14264, 3863, 14516, 161, 4110, 4519, 10444, 5199, 10098, 4386, 13160,
    6292, 12110, 11449, 16078, 10142, 8818, 827, 14058, 8664, 546, 5926,
    12896, 12046, 6426, 16047, 7956, 11917, 4689, 14319, 2826, 4890,
    1424, 10657, 12682, 4514, 1654, 9373, 4158, 4390, 11653, 3950, 4075,
    7051, 11614, 7309, 10360, 11682, 2213, 1165, 8630, 6642, 8330, 12344,
    11860, 15603, 5775, 9482, 11139, 11937, 1121, 16365, 374, 13191,
    6745, 5918, 5736, 15534, 12001, 15881, 6822, 14710, 7446, 3133, 5122,
    16086, 15678, 4697, 16009, 12584, 10625, 15038, 14574, 9839, 14587,
    6068, 14494, 1529, 3688, 12599, 1657, 9656, 4101, 5335, 9411, 9638,
    6796, 7719, 9214, 11490, 2690, 7154, 349, 5202, 6438, 1007, 6243,
    4517, 12556, 10044, 14709, 9857, 6808, 15893, 11004, 15243, 13175,
    2409, 4444, 7927, 3317, 497, 7321, 14132, 9680, 16080, 7293, 7056,
    7944, 10777, 14009, 11398, 14599, 3941, 5836, 15636, 13979, 9722,
    3146, 5914, 8684, 14966, 16149, 5119, 14814, 3590, 15321, 7033, 7021,
    3847, 4183, 4763, 722, 10554, 7930, 12878, 2971, 13581, 1243, 4475,
    14505, 4156, 8489, 10867, 3121, 8237, 11154, 8302, 10032, 266, 11528,
    13925, 365, 3137, 3541, 16054, 8301, 12028, 1181, 9342, 3600, 12676,
    960, 4787, 13251, 8731, 2361, 7888, 1951, 252, 4033, 14724, 11958,
    1025, 5521, 8186, 9417, 12765, 12434, 10693, 8047, 3114, 11518,
    10440, 8295, 14377, 10365, 8938, 2557, 11454, 10231, 15824, 6934,
    16326, 6154, 6493, 3153, 2505, 7141, 1116, 7266, 7506, 9094, 1340,
    10017, 7378, 12721, 1967, 667, 13139, 9178, 9407, 1429, 655, 11399,
    6305, 6323, 15075, 4269, 1638, 6795, 2534, 930, 4296, 6847, 3968,
    6266, 13742, 16072, 9115, 7931, 6816, 985, 4304, 11630, 4403, 7193,
    7761, 9852, 16102, 6640, 13511, 12773, 12886, 15409, 8222, 1417,
    12988, 13965, 6898, 3814, 2964, 1253, 2065, 3581, 10610, 1468, 7503,
    12314, 5120, 6921, 10186, 10319, 2777, 6881, 11337, 16353, 3960,
    9866, 14935, 2148, 2704, 4768, 12987, 11183, 13020, 1299, 15644,
    8343, 7651, 1318, 14485, 15861, 3036, 13739, 12196, 16180, 11980,
    6457, 6907, 15956, 14681, 16244, 5164, 3927, 7915, 1990, 16118, 8116,
    14850, 11566, 12502, 5103, 5453, 9281, 8479, 16069, 8204, 3044,
    14896, 13214, 1236, 15794, 9129, 13031, 7382, 10286, 7850, 8307,
    5564, 13867, 15048, 11981, 115, 16158, 10053, 1445, 13261, 13467,
    980, 12831, 879, 1235, 7005, 15585, 9110, 6712, 2041, 1317, 6957,
    4687, 11477, 14937, 5593, 4051, 8371, 1017, 10894, 424, 15282, 205,
    2823, 5281, 1067, 15835, 6271, 15872, 3103, 15507, 10684, 5087, 2128,
    3704, 9919, 7155, 9414, 9748, 1341, 4570, 5895, 10661, 11740, 6499,
    420, 1102, 8403, 6775, 8838, 15857, 8539, 3534, 1309, 8278, 10401,
    4486, 509, 5212, 13284, 4193, 7300, 13110, 16156, 543, 15058, 8043,
    11412, 894, 14643, 9849, 15876, 7738, 6707, 7140, 5669, 7768, 4971,
    11957, 10608, 2084, 6525, 5175, 14015, 12402, 11607, 1388, 5360,
    14825, 9264, 11296, 10963, 11747, 5975, 10065, 12813, 9789, 11346,
    8387, 6316, 9156, 13912, 15990, 5925, 5362, 13525, 918, 10238, 84,
    715, 9072, 14536, 10872, 15281, 7686, 1021, 10024, 6880, 11231,
    15540, 560, 11500, 13220, 1860, 7493, 8450, 7777, 7680, 4924, 3260,
    5956, 12958, 7643, 15092, 11861, 12752, 1859, 10353, 4488, 11595,
    11209, 12710, 5166, 8155, 15291, 11290, 10371, 11696, 10012, 15586,
    4179, 2853, 6128, 7631, 4161, 2751, 8051, 202, 8823, 13395, 15685,
    11052, 8785, 12013, 15747, 13450, 969, 6526, 3645, 14657, 15433,
    15309, 15428, 1019, 10756, 15465, 10414, 7135, 15384, 12269, 4250,
    665, 4141, 6793, 15327, 15617, 14992, 36, 6899, 3835, 15818, 11790,
    12751, 6494, 35, 9646, 10672, 7432, 15918, 12516, 8193, 14198, 13851,
    10475, 10274, 2509, 10582, 4374, 7946, 11651, 12741, 11090, 1194,
    6231, 9784, 1091, 5849, 7917, 13435, 15906, 12698, 10996, 15556,
    1332, 8947, 6270, 11704, 12455, 13909, 12596, 11558, 1411, 10769,
    11667, 6633, 5780, 6855, 5047, 13726, 7712, 7179, 14077, 5756, 13098,
    5058, 13228, 2918, 15711, 925, 1164, 10642, 10948, 4168, 5982, 4112,
    9844, 12045, 6928, 9222, 2860, 1136, 13933, 12490, 14477, 12782,
    7823, 12237, 2212, 8611, 16377, 3226, 5598, 9950, 10290, 162, 10192,
    14113, 3893, 5256, 3365, 7046, 16288, 7283, 7351, 1996, 9666, 9404,
    4060, 4834, 16273, 8672, 13164, 15752, 114, 7301, 8835, 12750, 2841,
    3547, 11647, 10535, 642, 4434, 15300, 8036, 3559, 10059, 13240,
    11887, 10840, 6330, 15938, 2831, 1678, 4979, 4949, 5578, 9167, 2606,
    16330, 2280, 5600, 4405, 11811, 12680, 16294, 14950, 1974, 6184,
    1425, 8706, 6637, 7274, 13529, 15694, 6029, 292, 924, 7754, 12574,
    3171, 5812, 10565, 256, 7419, 9691, 683, 6854, 9650, 3393, 2919,
    8519, 1754, 8312, 5165, 9119, 4803, 13894, 11938, 7701, 3610, 1712,
    13236, 4523, 11439, 4728, 7767, 12280, 6652, 6645, 10812, 12069,
    6951, 8429, 6117, 5037, 4410, 2197, 1206, 14061, 7198, 6612, 11930,
    13705, 14615, 14732, 14907, 13404, 16310, 12318, 15430, 3356, 9008,
    11191, 1576, 13684, 6717, 12363, 3194, 10606, 14293, 3185, 4013,
    8256, 15044, 1976, 184, 7410, 5054, 12105, 1862, 13904, 9796, 674,
    10985, 15765, 12093, 6967, 165, 5601, 10328, 238, 16299, 9686, 6181,
    12781, 4756, 8106, 11753, 11394, 10573, 13457, 3144, 12346, 11452,
    4645, 9270, 7445, 15303, 8192, 11118, 16289, 5391, 4899, 14464,
    12429, 11943, 65, 10398, 8546, 4998, 4048, 13518, 8962, 12080, 7543,
    3259, 11373, 12912, 15368, 7592, 1815, 15478, 608, 3657, 10143,
    11984, 15077, 418, 100, 4661, 13725, 12772, 13856, 1904, 9441, 14942,
    2253, 12129, 891, 964, 7885, 15932, 3420, 5642, 3982, 7600, 449,
    14428, 3480, 4246, 338, 14304, 14232, 10592, 5481, 12257, 6603, 9393,
    8380, 1585, 13546, 11188, 6696, 15559, 2290, 15558, 7348, 13002,
    14666, 14303, 6727, 2894, 7084, 9405, 3046, 7818, 1889, 7392, 7081,
    5708, 13927, 6077, 5449, 15985, 13964, 6157, 3444, 3736, 6199, 11207,
    13795, 12990, 354, 2822, 15036, 2525, 8747, 5931, 4926, 12192, 7795,
    659, 14547, 14653, 6394, 5957, 12818, 6511, 14934, 9869, 7032, 3281,
    11925, 13636, 11254, 2062, 6076, 12644, 2352, 11809, 16203, 15214,
    9184, 7829, 86, 7133, 3701, 14307, 3912, 9071, 1218, 15156, 14406,
    2329, 4509, 12511, 10585, 10922, 14144, 2106, 2870, 3448, 16020,
    5702, 3418, 2778, 4658, 4540, 3911, 12888, 8649, 7011, 6599, 12501,
    9506, 13292, 14956, 14108, 15094, 14700, 5561, 3655, 14649, 4130,
    9245, 8164, 15957, 12431, 2440, 5823, 10746, 11726, 6716, 3710, 6859,
    6226, 12451, 7054, 14179, 5241, 14369, 3739, 2788, 12505, 10676,
    3479, 10804, 3454, 7535, 15889, 4270, 10964, 2495, 14945, 1550, 4335,
    7049, 3757, 7374, 6835, 8719, 15128, 611, 12939, 11312, 12998, 1284,
    2143, 15298, 10986, 7849, 14753, 9718, 2578, 15220, 11070, 12306,
    12033, 13577, 1934, 4574, 1721, 5101, 8525, 14894, 5572, 11352, 7130,
    11003, 6701, 9255, 4016, 11128, 15937, 108, 12342, 2599, 6357, 1618,
    9256, 3821, 4978, 4948, 4470, 1510, 9853, 14288, 793, 10308, 8367,
    12415, 15502, 9064, 73, 188, 1040, 14396, 13504, 11107, 1808, 1128,
    13614, 1666, 9346, 3178, 6909, 6454, 8918, 5234, 14089, 6439, 7549,
    12032, 2728, 2079, 8899, 10448, 3665, 15840, 2348, 4959, 13543,
    11545, 9981, 5010, 8175, 320, 7803, 14271, 1186, 15286, 15456, 6392,
    5206, 6404, 1519, 4417, 9340, 8272, 179, 2446, 4221, 13688, 709,
    5679, 15278, 7227, 14335, 14538, 1787, 16077, 4819, 7919, 10095,
    5798, 7561, 11859, 4929, 5650, 10891, 10917, 4134, 232, 4611, 14771,
    2324, 15443, 9557, 14761, 1374, 5488, 1371, 9391, 13858, 8398, 5312,
    3080, 9651, 11700, 4476, 9369, 6207, 14508, 6731, 14449, 11417, 6639,
    4823, 3955, 7558, 6669, 8341, 12179, 15855, 6235, 12832, 13698, 5181,
    7780, 2794, 2733, 12465, 4119, 12385, 272, 8912, 12547, 5025, 6506,
    9547, 6520, 8860, 11539, 16198, 1863, 3606, 5663, 3809, 8844, 7974,
    3700, 6321, 10438, 2137, 4218, 12456, 7225, 8593, 12077, 8208, 12194,
    3130, 8139, 6141, 217, 3095, 10432, 3145, 13587, 7305, 6051, 5248,
    128, 7837, 15772, 12332, 6004, 1777, 11334, 15461, 7478, 5467, 10517,
    588, 4543, 14796, 3919, 9621, 10058, 7860, 11234, 10845, 7652, 8927,
    2479, 13252, 15968, 1900, 8119, 9242, 2091, 5799, 10956, 13650,
    13849, 9529, 12030, 1760, 10979, 7551, 3980, 13431, 7664, 12588,
    10255, 14675, 14551, 2187, 15695, 7228, 4903, 15940, 3759, 5257,
    11581, 11923, 3383, 3867, 9945, 3974, 7086, 11059, 3822, 4019, 9227,
    11659, 14789, 8071, 9103, 14855, 14481, 6461, 11599, 10918, 3553,
    12542, 7180, 3311, 4037, 9668, 8781, 1897, 3595, 14808, 6769, 7953,
    295, 1497, 4420, 8319, 14233, 2246, 3880, 7396, 15543, 1570, 7960,
    1752, 11293, 3385, 12207, 7139, 8529, 8507, 3794, 13350, 4175, 14419,
    16352, 8333, 11136, 11690, 3386, 7491, 4786, 11219, 9856, 15608,
    9390, 1894, 10299, 2371, 7276, 12834, 15542, 1360, 8006, 12546, 4419,
    13810, 910, 11340, 7683, 137, 16128, 10254, 2668, 6759, 7338, 9394,
    13709, 8025, 3069, 13831, 14706, 1359, 6533, 1024, 1938, 10470, 6894,
    14517, 8959, 16297, 11343, 13627, 4974, 13132, 1735, 1811, 3269,
    15131, 4207, 14002, 1107, 713, 12942, 6916, 2592, 7838, 5785, 12653,
    15012, 2256, 7999, 5416, 9799, 13381, 8769, 16101, 9674, 1948, 5296,
    7403, 11522, 4496, 5387, 6658, 102, 2667, 2022, 11190, 7343, 10007,
    7125, 919, 12401, 4242, 7279, 6969, 16165, 4283, 5902, 2400, 13632,
    10111, 2296, 12894, 8505, 6303, 3751, 3762, 1634, 6465, 1703, 14162,
    174, 1928, 8093, 14951, 8808, 14995, 4061, 9024, 12444, 9485, 1579,
    10752, 5856, 11285, 10038, 8506, 2368, 15760, 10876, 4219, 3926,
    12100, 10202, 6149, 13896, 2697, 8172, 12167, 3413, 822, 2571, 2536,
    7695, 13233, 13730, 14984, 2657, 7310, 1781, 14514, 5403, 2215, 195,
    16275, 1708, 11143, 14924, 9968, 15380, 1684, 15530, 7124, 16039,
    5484, 14716, 1326, 5176, 12416, 1365, 2928, 11976, 10126, 5817, 9953,
    16098, 13257, 11339, 12356, 2478, 13399, 13869, 597, 1124, 10655,
    8012, 7375, 7400, 11408, 15700, 14398, 10110, 10056, 14446, 8228,
    7794, 10306, 7026, 8819, 1668, 12186, 13449, 2676, 9472, 11210,
    13939, 15313, 5461, 12463, 3561, 14662, 8160, 407, 5637, 7798, 13420,
    9286, 14457, 9676, 1217, 1443, 2689, 10855, 4814, 4976, 1596, 9612,
    8476, 10060, 9731, 976, 1714, 3654, 2206, 10124, 2890, 4677, 2064,
    12638, 15533, 4860, 8807, 5771, 3741, 15463, 3478, 10335, 6800, 7075,
    8705, 14788, 8229, 1616, 15704, 1690, 9494, 1177, 9376, 6846, 7665,
    3620, 13033, 11362, 9444, 1983, 756, 447, 302, 8426, 2196, 1502,
    12708, 4830, 3902, 1674, 11733, 13683, 723, 14170, 14513, 525, 2200,
    2567, 11600, 1986, 7573, 13527, 8487, 8670, 4698, 11757, 2804, 14282,
    15232, 10774, 15375, 8971, 3949, 835, 9246, 1798, 59, 9635, 10039,
    596, 5214, 10827, 13782, 15147, 1230, 1351, 4190, 2939, 14839, 15596,
    4333, 8836, 9921, 12482, 9169, 2415, 14016, 2913, 13533, 10510, 3951,
    14095, 8895, 12684, 5052, 14543, 14231, 11816, 6459, 167, 1075,
    11089, 3198, 12862, 4461, 4596, 1565, 9644, 4722, 3434, 11348, 765,
    1336, 9989, 15529, 15834, 12039, 7210, 14968, 15849, 3326, 6174,
    11525, 15126, 9232, 10507, 14221, 1442, 3622, 8783, 9066, 10945,
    1536, 2943, 7728, 9885, 4719, 13842, 15900, 9419, 3275, 1347, 4653,
    6830, 12570, 6247, 4487, 2642, 1083, 5243, 2355, 15775, 6370, 4278,
    2931, 9698, 595, 14642, 7255, 12394, 2743, 4966, 3220, 13619, 12161,
    6255, 8652, 3885, 2564, 7821, 11294, 8001, 2226, 383, 5062, 3811,
    16059, 6248, 4169, 7057, 9146, 7624, 6259, 15266, 2040, 1688, 14207,
    12488, 9883, 12048, 13494, 16028, 13486, 11376, 3526, 14251, 11553,
    6278, 156, 11464, 10995, 7282, 10408, 2632, 789, 1482, 7669, 3716,
    16265, 9107, 9512, 8756, 3484, 2311, 8619, 8353, 1237, 2849, 13469,
    4775, 2168, 7532, 10292, 5734, 11731, 3895, 13112, 12228, 5840, 6910,
    9810, 8291, 15189, 3082, 5432, 7707, 8419, 7443, 10329, 9164, 11540,
    6017, 4354, 13542, 4032, 10516, 9075, 7582, 1809, 3848, 7661, 5226,
    5135, 10915, 13438, 15514, 3464, 11543, 3025, 7865, 9116, 13475, 865,
    6827, 2744, 13427, 558, 4062, 4363, 5273, 5371, 210, 7346, 12311,
    7781, 9408, 14426, 15615, 12222, 2812, 3164, 12889, 1656, 15193,
    11779, 5487, 4097, 9740, 4339, 11869, 3472, 2864, 14272, 7920, 9514,
    1361, 4589, 5035, 474, 11228, 9930, 10634, 15564, 499, 359, 2882,
    3184, 15645, 8339, 14448, 11034, 11269, 4489, 10737, 1933, 6776,
    9801, 7773, 13678, 14411, 10278, 7203, 6706, 2584, 13570, 13422,
    5339, 9468, 1392, 10489, 2968, 6491, 4188, 4800, 8337, 953, 6340,
    4707, 8560, 14012, 9966, 13519, 3113, 498, 15801, 10664, 15154, 7556,
    3651, 7484, 11335, 11466, 3221, 15069, 8656, 8862, 9504, 15822,
    13878, 992, 2207, 4132, 11782, 9127, 10932, 11239, 14050, 4430, 6197,
    15206, 14811, 8749, 3074, 15104, 4282, 4972, 7018, 4255, 3242, 942,
    9874, 8115, 15141, 9688, 11104, 8178, 13439, 16088, 4511, 14336,
    5173, 291, 11172, 11014, 10108, 2935, 3349, 5778, 965, 13318, 11915,
    6628, 15441, 6240, 8766, 2562, 1545, 1964, 1982, 15584, 13197, 15411,
    4167, 11403, 8056, 12577, 3283, 9195, 5531, 4928, 7971, 1088, 1205,
    297, 13144, 10357, 2015, 10605, 5954, 11537, 3574, 11073, 8350, 3913,
    15922, 4646, 15964, 11754, 10019, 486, 1271, 10244, 1995, 14994,
    2687, 5290, 16332, 10465, 11768, 2953, 10500, 7525, 1401, 3834,
    11949, 5143, 11897, 15713, 2216, 4298, 12635, 8762, 9767, 6718,
    15297, 8344, 3886, 14596, 5440, 2748, 4751, 6518, 14225, 8894, 1506,
    12593, 949, 1720, 8772, 10502, 4613, 9887, 13155, 16068, 14841, 2237,
    15127, 15273, 4767, 15648, 12844, 12461, 15550, 14976, 9225, 11755,
    9027, 13195, 834, 7224, 1861, 15296, 2681, 13736, 13294, 15950,
    13210, 15257, 2825, 7004, 9464, 4792, 1296, 4332, 8102, 1407, 13414,
    11658, 9681, 16328, 7496, 13032, 8334, 12327, 11308, 7564, 2174,
    8452, 3115, 10387, 6216, 9501, 15310, 15325, 14081, 1622, 14903,
    16147, 10681, 2513, 12430, 3451, 4527, 14586, 3527, 576, 5860, 1330,
    7792, 7512, 286, 11982, 798, 14037, 7807, 4393, 1903, 1736, 5285,
    13282, 3469, 12846, 5608, 14895, 11195, 5142, 5559, 16079, 10504,
    6723, 6030, 2910, 13630, 2852, 3722, 15391, 3352, 14851, 6850, 2847,
    9530, 732, 4560, 13275, 14640, 3920, 7395, 12554, 6739, 3019, 12015,
    16058, 3718, 12902, 11689, 8606, 12943, 9253, 10005, 7822, 11692,
    15408, 14236, 15848, 2319, 1099, 8028, 12481, 12503, 4566, 1788,
    4414, 3840, 10345, 4209, 15328, 2963, 6172, 1386, 11609, 2715, 11102,
    15084, 5570, 15902, 3649, 8521, 6420, 1830, 2650, 6588, 8780, 6006,
    14039, 7074, 5740, 2803, 6742, 9786, 12701, 13260, 742, 2598, 1969,
    245, 13651, 10526, 261, 5631, 9336, 6632, 5752, 15241, 3196, 12652,
    2537, 9891, 13844, 9687, 14343, 12274, 4656, 2750, 15059, 12197,
    7637, 1162, 14959, 200, 15749, 12130, 5064, 8241, 10237, 10008, 548,
    1441, 10621, 10327, 4191, 1835, 7030, 7645, 9228, 11620, 14961, 8736,
    15927, 16146, 4817, 9269, 529, 5932, 113, 721, 1013, 9782, 634, 7861,
    15836, 2077, 11921, 6617, 8889, 13180, 1865, 1454, 1885, 5178, 11503,
    62, 5945, 4331, 14180, 14663, 8671, 5220, 2659, 15233, 13286, 1769,
    12410, 12703, 1854, 2850, 279, 3183, 12413, 6342, 4749, 8317, 15468,
    12971, 7868, 3844, 15442, 9451, 11997, 11953, 13890, 1292, 277,
    14554, 14810, 14101, 6101, 14982, 7945, 12537, 4273, 5032, 6750,
    10795, 1108, 2380, 15528, 7896, 13437, 10533, 6291, 8050, 12956,
    10230, 3369, 3904, 11575, 12662, 437, 752, 15776, 4217, 12174, 7522,
    12677, 3766, 13787, 8891, 9023, 9618, 12271, 13994, 5298, 12389,
    4262, 5124, 11738, 6785, 9833, 13331, 14192, 5576, 6862, 3674, 5903,
    4996, 7201, 3845, 181, 2553, 16340, 4625, 8262, 15560, 711, 1817,
    3495, 385, 1047, 11837, 7020, 7537, 14568, 7897, 4859, 5270, 8840,
    11784, 14436, 8935, 10716, 7788, 6121, 5345, 11043, 1918, 8644, 3855,
    149, 7184, 6478, 3686, 3804, 9862, 8522, 6360, 7172, 707, 10539,
    10373, 1823, 13565, 10377, 10268, 2192, 2640, 1944, 14731, 15115,
    9742, 2125, 10598, 6468, 11598, 14767, 14849, 2821, 13689, 1595,
    9706, 15054, 9361, 5275, 14136, 2458, 213, 14413, 14452, 5240, 12871,
    10768, 9559, 15359, 10524, 857, 10178, 15613, 4573, 15396, 4350,
    4441, 7411, 8395, 11814, 5562, 10822, 12671, 9209, 11437, 4925, 4073,
    2163, 3100, 6227, 10324, 11072, 1437, 5759, 6600, 1489, 15414, 13489,
    1422, 12068, 12932, 15796, 9733, 14524, 13412, 13203, 7539, 8715,
    14898, 10102, 6992, 7292, 467, 11624, 1845, 15393, 9889, 1246, 4963,
    13864, 14819, 46, 6093, 16109, 2582, 8621, 14011, 1970, 2052, 2656,
    1756, 9661, 12036, 6114, 14774, 8963, 11583, 14439, 2457, 15212,
    9128, 5340, 2194, 6043, 2127, 14078, 4894, 9497, 3431, 8005, 10184,
    4338, 9938, 6252, 13901, 922, 13569, 2824, 8372, 4439, 12977, 13552,
    15829, 14712, 12340, 8628, 8782, 1006, 11441, 7964, 4459, 13652,
    13969, 7451, 6193, 13840, 1676, 5801, 9238, 15071, 29, 15224, 5538,
    8210, 2403, 15175, 585, 12580, 10990, 12148, 3147, 12732, 5168, 4591,
    5511, 4721, 13928, 3969, 7736, 3435, 13085, 6496, 15269, 6406, 5770,
    7793, 9809, 8357, 15449, 16144, 8033, 12857, 14676, 5328, 8315, 9427,
    8830, 2193, 15666, 7731, 12171, 4342, 6448, 4867, 11267, 2496, 4098,
    6549, 2589, 4730, 16127, 10512, 5195, 10685, 1286, 3536, 12285, 8597,
    5260, 3278, 9490, 9360, 3010, 8077, 9432, 3646, 8900, 391, 3249,
    9474, 12264, 8322, 14114, 5592, 14437, 10892, 12137, 9657, 1909,
    5666, 4592, 16025, 1029, 2517, 3442, 123, 8253, 6379, 5156, 9884,
    5604, 6940, 13886, 16191, 13133, 15481, 12305, 7322, 7314, 11795,
    3056, 12628, 16023, 7090, 8928, 6965, 14495, 13227, 11361, 4783,
    2635, 8996, 3512, 2414, 1458, 12177, 1252, 10771, 16382, 9262, 10263,
    8888, 3937, 15665, 11350, 8082, 13455, 10374, 2745, 9062, 15653,
    5672, 9010, 9423, 1905, 15400, 11448, 5183, 15581, 15944, 6729,
    15437, 11232, 6127, 7426, 5067, 13713, 3162, 10367, 8989, 14699, 993,
    14042, 15356, 16279, 7034, 11333, 7384, 607, 12553, 10781, 1440,
    1540, 6162, 15021, 10767, 11670, 8396, 8580, 9848, 12155, 11947,
    11935, 3463, 4404, 9304, 13443, 3936, 9329, 3781, 6920, 15626, 8284,
    14752, 16283, 9037, 13850, 12757, 11036, 2707, 10718, 10062, 8617,
    7492, 6648, 14567, 13786, 12229, 4631, 14783, 9586, 15917, 1059,
    3475, 7623, 2279, 244, 8448, 7181, 8332, 12284, 8264, 2856, 4288,
    11345, 5425, 3086, 26, 13423, 10302, 12779, 650, 8563, 11825, 12621,
    9443, 7, 13743, 9148, 725, 7587, 10052, 16033, 15320, 14566, 6363,
    5333, 5265, 4796, 16367, 9484, 4951, 849, 12586, 7870, 9812, 14189,
    4797, 3064, 9248, 9068, 16007, 2529, 7973, 13882, 1548, 1170, 3764,
    4320, 9964, 11006, 15011, 4204, 770, 15499, 4271, 5762, 12071, 11351,
    6927, 14211, 16024, 4124, 9234, 5321, 4223, 11964, 7993, 14715,
    14297, 10498, 15346, 13069, 4131, 1018, 11663, 4655, 1957, 9173,
    2977, 15416, 13013, 7566, 8167, 14792, 456, 11204, 1870, 7790, 5984,
    10864, 10048, 12316, 4454, 5008, 12520, 6431, 7853, 11765, 8567,
    2111, 9539, 14137, 10391, 12317, 1665, 11978, 14010, 11840, 11413,
    8133, 2469, 4602, 7044, 5518, 7399, 11149, 3342, 12557, 3053, 6201,
    2810, 14572, 3864, 10690, 8443, 2392, 7311, 5545, 12672, 15832,
    10630, 2774, 3211, 14717, 9224, 15009, 13774, 6358, 1290, 10174, 337,
    10380, 12020, 5547, 11237, 939, 2908, 13018, 13343, 7997, 2006,
    12024, 1090, 5972, 12617, 10277, 15223, 8070, 1886, 14420, 4225,
    15258, 14711, 13735, 11744, 5463, 6889, 3310, 12769, 2595, 6956,
    4794, 12328, 10075, 5465, 15758, 9565, 5832, 14698, 2404, 12755,
    7610, 9843, 12930, 79, 14619, 8833, 10185, 11973, 2936, 6411, 10714,
    13470, 2419, 6619, 6152, 2832, 7059, 8802, 6164, 14977, 3020, 1539,
    15549, 7815, 3838, 5302, 4628, 1191, 12398, 1584, 13934, 2710, 14178,
    2957, 15431, 4793, 12266, 14158, 9092, 553, 14917, 6912, 3205, 8700,
    9461, 1, 13566, 15924, 6405, 12310, 14772, 7876, 3097, 12476, 1280,
    9315, 869, 9664, 3931, 11047, 4851, 6799, 2043, 8537, 8767, 194,
    10881, 8665, 7597, 6724, 11357, 2386, 350, 1806, 14141, 15336, 12648,
    482, 2912, 6126, 8967, 4779, 2365, 8750, 3563, 3227, 2594, 989, 4310,
    5158, 13484, 10218, 2232, 40, 13600, 12963, 5026, 4264, 12655, 6116,
    758, 3584, 5431, 10103, 3296, 14669, 3565, 411, 5907, 12484, 1020,
    1775, 10392, 4664, 11898, 6733, 9649, 1195, 7691, 7649, 14080, 10152,
    12700, 14622, 7119, 4107, 2221, 2600, 9871, 11732, 16125, 8659,
    13289, 11383, 13396, 13710, 13270, 951, 11884, 14184, 2658, 11823,
    13832, 15469, 1573, 9528, 9994, 1671, 2356, 11605, 7368, 11221,
    13148, 15434, 105, 14875, 2619, 12560, 1151, 1028, 1542, 11601,
    10553, 11433, 5372, 13731, 7267, 13755, 10724, 8454, 5551, 10199,
    9663, 15161, 12127, 7692, 6513, 1324, 11498, 8875, 12000, 11568,
    8351, 2382, 11636, 994, 16116, 13215, 4627, 9765, 5516, 13991, 5603,
    14922, 7975, 14309, 13025, 12466, 11889, 5893, 289, 112, 743, 9283,
    6390, 7926, 3611, 16052, 2838, 15686, 9946, 11338, 12966, 1620, 3999,
    10839, 11712, 15978, 10257, 3756, 12292, 13754, 11273, 7112, 4103,
    10122, 2560, 4012, 10883, 15164, 1272, 6277, 15033, 10916, 12497,
    15163, 6959, 6455, 3784, 10479, 2711, 4297, 13337, 4666, 299, 938,
    8691, 6740, 8510, 14322, 14112, 4593, 8588, 870, 12467, 10738, 11250,
    12955, 16212, 14510, 12062, 3055, 1410, 11737, 3723, 8340, 877, 5225,
    1640, 5664, 4449, 7991, 9724, 14228, 14237, 1744, 10492, 5082, 8413,
    2252, 763, 8211, 175, 3298, 2463, 13057, 3752, 2118, 12391, 11323,
    8729, 5575, 6991, 3470, 3428, 13302, 1327, 643, 639, 5990, 15732,
    15451, 6260, 16263, 9313, 8445, 3680, 8195, 7409, 11162, 1611, 3591,
    11097, 10950, 2665, 284, 4569, 8564, 14660, 11557, 14967, 15539,
    2102, 7296, 6925, 12585, 11124, 9450, 7693, 11640, 5295, 5850, 6945,
    10485, 7383, 14316, 8176, 2429, 14121, 1263, 11829, 12973, 4319, 896,
    14410, 16076, 14926, 13012, 16228, 4841, 15682, 13416, 12154, 8138,
    13113, 2716, 6473, 4526, 11224, 12404, 11687, 10415, 7173, 12691,
    12494, 1695, 3350, 8984, 13764, 12582, 4240, 4683, 780, 15736, 2014,
    6953, 14025, 14082, 10824, 7545, 8383, 1241, 2413, 13103, 11065, 242,
    988, 15110, 14889, 2456, 12919, 15484, 8905, 10156, 1857, 2130,
    13966, 6972, 13920, 14760, 11945, 10729, 14283, 12837, 15524, 7640,
    8287, 4795, 12517, 13618, 3017, 7461, 260, 10799, 9915, 13768, 6988,
    3154, 11246, 11370, 4888, 13173, 1673, 4392, 9397, 4534, 562, 8842,
    12551, 11646, 12149, 14268, 10388, 14571, 1207, 1960, 3107, 12789,
    618, 2720, 4711, 3514, 9576, 13772, 8604, 9374, 1204, 16378, 14859,
    14372, 3854, 5329, 15684, 15898, 13946, 1606, 9149, 15997, 10087,
    2504, 8140, 15334, 13019, 15879, 2528, 10068, 4351, 11279, 2602,
    14561, 4586, 13094, 2165, 12359, 8349, 14219, 10931, 3101, 310, 6018,
    4171, 4983, 8181, 15538, 10093, 10385, 314, 8764, 927, 7073, 7370,
    11410, 16003, 14269, 10226, 11475, 4585, 12230, 10906, 173, 8275,
    4040, 12493, 13313, 6261, 4361, 4400, 7965, 6784, 4614, 3047, 8776,
    298, 15864, 12297, 3497, 9108, 13788, 11806, 12528, 15877, 9860, 433,
    7113, 10934, 12830, 9537, 382, 1184, 142, 9851, 13658, 2887, 8627,
    820, 2483, 6419, 7633, 9991, 6597, 15817, 15495, 635, 15853, 13141,
    454, 12227, 3319, 5353, 7291, 4725, 3140, 5813, 7547, 4323, 2340,
    5694, 3624, 12972, 13817, 11453, 1941, 9993, 2766, 11317, 11430,
    10898, 12959, 2183, 1295, 1100, 7903, 15343, 412, 11536, 15448,
    10833, 7952, 14131, 6936, 9939, 1370, 1376, 9913, 3132, 6905, 12874,
    10493, 3523, 5589, 378, 13781, 6139, 13130, 9788, 13499, 2346, 5815,
    3964, 11253, 1035, 8263, 9578, 15480, 13956, 12135, 7520, 1633,
    14909, 11071, 14070, 1546, 15394, 4347, 14805, 7277, 15947, 5188,
    3373, 10456, 13728, 7122, 2770, 5970, 10021, 4463, 11292, 9309, 6598,
    16132, 7110, 9669, 7873, 14214, 10091, 11016, 5274, 15017, 13058,
    7262, 2859, 14238, 12690, 5061, 4813, 4843, 6417, 5341, 4231, 1988,
    9190, 15718, 16296, 8592, 2712, 8811, 10435, 2166, 14416, 8893,
    13796, 2558, 15973, 10815, 15562, 13333, 13540, 3255, 12607, 851,
    10797, 12374, 8425, 408, 15869, 2541, 2970, 10600, 5703, 5435, 16343,
    15347, 7472, 6891, 7994, 13666, 2538, 16040, 8389, 13534, 10699,
    10270, 8390, 2610, 16298, 8183, 1182, 2454, 1923, 9193, 4930, 4977,
    14555, 6138, 3857, 13623, 867, 3237, 12438, 12067, 6687, 1555, 414,
    3473, 3348, 4108, 7598, 6191, 8658, 13777, 11527, 15236, 7593, 16048,
    4960, 1126, 16235, 1409, 3462, 814, 1178, 15366, 11445, 1098, 6530,
    14502, 4571, 2066, 7295, 544, 6976, 11068, 14315, 2004, 12583, 14159,
    9629, 8682, 11711, 11588, 6297, 11774, 16141, 646, 1972, 8154, 7772,
    495, 10358, 228, 15546, 3336, 8916, 1621, 874, 15905, 3787, 5737,
    12052, 7100, 13056, 15582, 5763, 5276, 556, 7285, 511, 13232, 106,
    12578, 13538, 2768, 8974, 9292, 12550, 319, 2627, 14834, 14460, 9719,
    1677, 5059, 13947, 2092, 1446, 15124, 13891, 11916, 14257, 12864,
    10453, 10279, 764, 9905, 8194, 8569, 10719, 15114, 11793, 12754,
    13884, 9055, 2945, 15983, 14638, 14973, 15444, 5731, 12175, 10810,
    2289, 7663, 7662, 2949, 13295, 13769, 7265, 12075, 12753, 13168,
    12132, 8824, 5089, 1569, 1470, 5374, 2156, 5233, 10591, 4850, 2933,
    10555, 5718, 14302, 11531, 4287, 5792, 11613, 1831, 7911, 3826, 4418,
    12937, 11846, 12417, 10508, 9798, 1362, 13386, 1322, 16276, 7760,
    10633, 9081, 2412, 10307, 1961, 11236, 6167, 13993, 6274, 3378,
    14890, 6834, 3945, 6050, 6103, 7189, 1535, 11265, 2467, 2399, 150,
    13974, 14051, 13122, 5645, 4816, 6447, 11590, 166, 776, 13761, 5301,
    12563, 8523, 4639, 13163, 15912, 13272, 10523, 15825, 6863, 1698,
    9803, 10530, 9581, 1920, 4980, 2616, 10566, 6551, 435, 7239, 9620,
    6108, 1069, 2177, 9627, 16152, 12458, 12169, 9601, 15145, 7677, 8755,
    3277, 1158, 9132, 796, 2683, 12058, 11547, 4538, 15095, 5454, 10618,
    443, 14432, 12065, 5725, 9362, 1615, 8360, 16110, 11392, 6253, 9481,
    8267, 4921, 8084, 14124, 369, 15802, 6751, 14326, 14905, 14028,
    10010, 13382, 15948, 3773, 14671, 1189, 15271, 1494, 3511, 13719,
    11759, 2211, 12799, 8385, 6251, 13948, 3532, 16014, 1921, 1097, 5305,
    11082, 7937, 5896, 4856, 6638, 3961, 4938, 15709, 13152, 2519, 11578,
    13819, 8143, 13359, 15738, 3959, 5291, 4064, 10276, 13126, 1266,
    11020, 4705, 9079, 13030, 7697, 1791, 15341, 10908, 2952, 3520, 3755,
    6636, 494, 10000, 9602, 10522, 5953, 9100, 3213, 11623, 4515, 8937,
    11516, 9151, 13271, 2462, 5307, 9358, 10397, 8605, 14021, 12869,
    13647, 6220, 4164, 6450, 2477, 15812, 10627, 10547, 13822, 14294,
    3331, 9117, 3289, 13662, 30, 2883, 5753, 2699, 5457, 16278, 14493,
    9203, 11844, 7003, 15746, 15262, 15079, 9838, 15764, 7783, 6826,
    8668, 15116, 12208, 1912, 11126, 6670, 15604, 13000, 2654, 6485, 267,
    14833, 12003, 14762, 5779, 13349, 956, 8603, 1498, 1385, 12711, 483,
    1603, 7416, 14392, 1093, 13016, 15238, 4635, 14382, 11699, 9352,
    4484, 11175, 4840, 7887, 14431, 12642, 9948, 3548, 9171, 16347, 3914,
    577, 10272, 6424, 3110, 9952, 12491, 12978, 4829, 6486, 5995, 1530,
    10424, 12459, 13536, 11487, 753, 10149, 6681, 7123, 676, 7891, 6176,
    7345, 6354, 946, 2059, 8197, 7704, 5861, 10739, 7366, 13545, 250,
    5269, 2996, 12725, 6502, 2875, 4969, 9988, 5113, 2242, 14612, 9349,
    4258, 8849, 16240, 82, 10913, 11865, 11468, 15875, 11766, 1085, 2965,
    13903, 10976, 14883, 9308, 13045, 16208, 7486, 2377, 4909, 6825,
    12991, 2785, 8723, 12315, 521, 8601, 10943, 11514, 2135, 8850, 13459,
    7467, 6135, 8760, 468, 15446, 862, 14407, 11780, 10133, 15659, 11303,
    15579, 9105, 12561, 7252, 5525, 4911, 12005, 9003, 15133, 7320, 6140,
    11691, 2995, 13704, 5361, 10022, 7534, 13633, 7121, 13609, 11758,
    2449, 6554, 2808, 9403, 6627, 5539, 6359, 1297, 11548, 7889, 6075,
    6579, 11169, 5492, 10928, 1198, 15500, 471, 9524, 7702, 10446, 9089,
    4325, 5012, 977, 8608, 11669, 12370, 3667, 786, 8955, 3282, 2181,
    5219, 10882, 4467, 5835, 2123, 13807, 2820, 12969, 14912, 7470, 973,
    16155, 7229, 11286, 8486, 5, 6510, 10042, 15791, 12079, 8057, 3873,
    8027, 13102, 7811, 1992, 12428, 1906, 8865, 33, 4535, 6665, 6837,
    15631, 2840, 12868, 15288, 5686, 11166, 14853, 5738, 8979, 14430,
    966, 2546, 7168, 3346, 6410, 16002, 4965, 12947, 11367, 3236, 14120,
    258, 11839, 6666, 4446, 6933, 3802, 3841, 5381, 14469, 5259, 12252,
    4448, 3975, 12209, 254, 3535, 14693, 3877, 15526, 12454, 8589, 826,
    4557, 7727, 11200, 14844, 8169, 4201, 842, 3438, 7714, 11096, 9536,
    3958, 3771, 12728, 1491, 14604, 12184, 872, 11193, 7261, 9425, 10798,
    2085, 422, 15699, 15908, 10437, 10157, 10245, 5618, 16229, 9133,
    6667, 104, 8763, 9406, 2941, 2188, 4238, 5727, 11799, 13147, 13601,
    323, 6396, 10968, 7507, 15179, 1989, 14874, 12643, 7879, 728, 4481,
    4227, 784, 9923, 15063, 8951, 13121, 3461, 14331, 11066, 5793, 6497,
    7589, 15739, 10334, 1356, 4150, 3539, 16218, 1138, 12856, 3607, 7526,
    38, 10003, 8274, 3112, 12981, 1619, 2958, 11592, 13778, 5198, 15032,
    6885, 14213, 5528, 1058, 16136, 14471, 1706, 3312, 6122, 13235,
    15152, 10189, 14087, 15276, 3843, 3683, 10747, 4371, 15966, 6676,
    3589, 14395, 2485, 10340, 2306, 15415, 1509, 13814, 9311, 12060,
    13353, 16248, 6042, 4776, 4422, 12122, 7654, 2674, 491, 2297, 8746,
    12627, 775, 5127, 3924, 1968, 4686, 9347, 3409, 2666, 11858, 6577,
    14506, 1682, 10266, 5842, 13597, 13428, 9834, 982, 12182, 587, 4673,
    3944, 10300, 15965, 8170, 7816, 2045, 6236, 12088, 12819, 8207,
    13702, 15720, 14200, 4642, 6699, 13003, 15995, 2721, 11672, 6366,
    8309, 2705, 4544, 16374, 3274, 8638, 12549, 5060, 14989, 5221, 12688,
    4864, 1043, 6867, 718, 1064, 3398, 2532, 7636, 15181, 15887, 8820,
    13899, 3582, 13186, 6418, 14018, 14005, 6458, 5235, 7544, 8433, 3335,
    12883, 16104, 7801, 9302, 843, 14324, 2574, 3453, 11074, 8542, 3265,
    16215, 1001, 1696, 7871, 6503, 10866, 14291, 7166, 7254, 2132, 4010,
    11883, 2888, 8698, 12353, 6944, 9498, 4365, 16135, 7641, 3330, 12443,
    3777, 12565, 9060, 10847, 4968, 4099, 13324, 12625, 10779, 6429, 908,
    13998, 6922, 6001, 2262, 10611, 1201, 13657, 3315, 475, 9076, 6352,
    15798, 4329, 10708, 4923, 13895, 12890, 4790, 11276, 3060, 15332,
    9325, 11838, 2438, 3285, 1400, 3521, 9584, 3531, 4355, 2960, 9054,
    8087, 7177, 10468, 14523, 6376, 2145, 12637, 9197, 12376, 15185,
    3303, 8088, 11095, 2653, 10015, 13403, 5512, 15754, 15903, 5768,
    15490, 10348, 5989, 399, 11719, 7684, 1153, 15977, 7017, 967, 14621,
    5109, 4737, 8500, 1319, 9069, 16290, 14030, 12905, 15553, 4261, 3865,
    15728, 13984, 4256, 13699, 11940, 7724, 4685, 7251, 12532, 7459,
    8245, 13818, 7211, 3035, 14243, 9500, 14787, 3016, 2141, 8547, 13515,
    1264, 12446, 4109, 3503, 6941, 8654, 1012, 15413, 268, 9261, 13737,
    15664, 8157, 2623, 4407, 8141, 9483, 3640, 4694, 2693, 7280, 9730,
    5741, 1192, 12141, 5910, 13535, 13637, 7289, 5951, 13846, 4402, 1901,
    7895, 8677, 1416, 5282, 4011, 6613, 9086, 3699, 2344, 9420, 10528,
    2645, 2590, 8365, 322, 14383, 15316, 10645, 4429, 6309, 7863, 14368,
    4468, 7439, 8201, 15763, 8559, 15833, 9859, 14171, 6329, 5872, 10057,
    12904, 15080, 5024, 10405, 4445, 8707, 3685, 2169, 10097, 4443, 9052,
    1827, 791, 15847, 3208, 5014, 7495, 1650, 7670, 16372, 7737, 7052,
    4192, 10529, 14380, 2566, 6301, 14749, 13501, 15624, 10537, 5960,
    6317, 2452, 15016, 14985, 14673, 4045, 1026, 10763, 12667, 13520,
    5726, 4213, 5255, 5332, 7315, 8397, 2292, 13307, 4953, 8573, 11875,
    14847, 6566, 907, 12916, 3725, 12961, 14507, 11050, 10920, 3509,
    9257, 4529, 1840, 6387, 3481, 12366, 1950, 11881, 7063, 11831, 3174,
    8924, 7103, 12337, 8321, 3637, 5311, 5215, 14741, 11985, 1244, 2184,
    8247, 3753, 9931, 13872, 63, 10125, 6115, 14913, 15744, 2554, 11895,
    15404, 13576, 9044, 3032, 9685, 4580, 151, 3774, 2680, 1560, 12778,
    8557, 5991, 6509, 1471, 8925, 5043, 2007, 3216, 1580, 10541, 4307,
    5971, 7800, 13267, 416, 11078, 10313, 9401, 12300, 2846, 1505, 15884,
    11717, 14467, 10984, 9182, 10213, 10692, 16064, 12103, 2432, 15046,
    4572, 15284, 318, 12795, 7444, 222, 4629, 8586, 10183, 11141, 14812,
    13402, 10895, 6147, 14260, 15669, 2142, 809, 18, 12483, 4833, 4421,
    12104, 2284, 3965, 2144, 12975, 13793, 5216, 13908, 12535, 3058,
    1567, 15488, 2587, 6818, 16174, 2871, 8919, 7434, 14652, 12392, 9334,
    6879, 10849, 346, 1056, 13373, 11633, 4608, 14378, 6931, 3719, 11963,
    3028, 9613, 5949, 13583, 15910, 6246, 3571, 8636, 6746, 3887, 9310,
    1103, 7883, 5034, 12788, 12611, 13005, 7287, 3871, 14429, 12217,
    2956, 8260, 1880, 4034, 15496, 10558, 2618, 3003, 5810, 11950, 6094,
    13201, 14611, 6878, 538, 16107, 11544, 14694, 4891, 1751, 3148, 7430,
    13861, 11948, 6700, 3446, 1757, 5324, 1692, 554, 3157, 8907, 7533,
    10513, 2905, 6425, 598, 4416, 15031, 15471, 3375, 10298, 9048, 12087,
    4520, 5116, 5065, 11491, 15823, 1963, 10400, 10919, 9016, 8483, 6098,
    1985, 11302, 6981, 4163, 15388, 14530, 9941, 8029, 6553, 3169, 987,
    15638, 8514, 5515, 2420, 4186, 4736, 5563, 6955, 14314, 1725, 2862,
    7668, 7947, 8035, 15052, 9328, 13226, 1294, 12380, 10023, 12004, 813,
    3546, 3859, 10488, 1637, 620, 605, 7127, 11109, 2889, 1697, 13594,
    14888, 5935, 2563, 7585, 9042, 13667, 15226, 3829, 13938, 9745, 4353,
    2003, 3030, 8480, 8011, 9001, 13685, 5884, 1406, 14601, 1135, 6273,
    16312, 10379, 9523, 6373, 9288, 12190, 6064, 15025, 6053, 16333,
    9689, 11926, 14425, 13310, 6464, 11251, 2042, 9637, 7402, 8328,
    14359, 11098, 11061, 3983, 9738, 14758, 2736, 854, 7377, 10106,
    12825, 14864, 1256, 13853, 10706, 2552, 11077, 9251, 14138, 2103,
    14892, 11535, 9049, 617, 610, 983, 6333, 3407, 11218, 218, 8015,
    6477, 3078, 8692, 968, 13161, 2492, 3222, 954, 14117, 11971, 661,
    6214, 9763, 12504, 14528, 6200, 11473, 15072, 4143, 14278, 2782,
    10551, 12785, 8280, 16258, 8513, 8473, 14865, 241, 469, 14327, 15878,
    1389, 1418, 6797, 11550, 14337, 11573, 12256, 6664, 11226, 9831,
    9036, 5355, 14500, 13763, 12742, 11660, 8150, 15895, 12113, 15652,
    5146, 6175, 14573, 1743, 5827, 4050, 6788, 3455, 8869, 9631, 3666,
    6519, 12059, 13481, 1027, 578, 3247, 11988, 11596, 13954, 16202,
    5493, 1749, 15555, 11111, 11551, 6805, 4100, 1796, 2395, 13645,
    15518, 2491, 13369, 8061, 9594, 768, 12604, 11552, 8799, 11634, 1623,
    1670, 761, 3698, 6698, 5889, 9775, 1476, 7660, 12524, 12240, 3954,
    14451, 14515, 1825, 1792, 12792, 5250, 9957, 15667, 3935, 15819, 762,
    7163, 13123, 4380, 8091, 16321, 12335, 1639, 14729, 4900, 13911,
    10649, 3691, 6280, 6753, 15352, 2723, 14188, 14352, 4306, 14910,
    12166, 963, 802, 4474, 9729, 9572, 5558, 16355, 920, 13673, 881,
    8614, 8118, 12388, 4028, 6728, 7273, 855, 5691, 2051, 7709, 15268,
    6789, 11946, 14110, 15522, 7022, 9433, 10837, 4469, 13779, 11484,
    14873, 8464, 15049, 1648, 12619, 125, 15571, 2332, 8142, 5633, 296,
    12936, 6341, 14203, 8884, 7458, 12365, 3307, 8494, 97, 1686, 1267,
    12278, 11893, 4919, 866, 12216, 2136, 5474, 2057, 10643, 2223, 1261,
    4023, 3558, 13407, 11723, 180, 10347, 4536, 6573, 5530, 8242, 11479,
    1480, 2533, 7126, 7723, 15445, 2167, 11336, 4835, 13400, 16358,
    11650, 2277, 1419, 5004, 14125, 2884, 8704, 12948, 3678, 3288, 5351,
    3932, 794, 6709, 1148, 1353, 6198, 12244, 8936, 6190, 11555, 7079,
    8152, 858, 3322, 6758, 15263, 15969, 8463, 14533, 1680, 6653, 8469,
    7629, 6202, 1147, 7270, 3513, 15654, 591, 7743, 11161, 943, 15920,
    5625, 11962, 487, 136, 12545, 10701, 2954, 5739, 10519, 5615, 12281,
    6136, 2225, 1837, 12950, 3321, 11409, 12940, 2981, 15655, 14562,
    7339, 6483, 5698, 3813, 61, 7085, 9285, 9496, 15267, 4865, 15935,
    7555, 429, 9140, 14408, 6852, 13176, 1134, 9020, 11347, 5841, 1926,
    2307, 15372, 3492, 6272, 10780, 1631, 6385, 9341, 11676, 10937, 679,
    239, 13084, 933, 3585, 4285, 13362, 4136, 11391, 12541, 3627, 3037,
    5830, 14940, 14092, 251, 4166, 5744, 11374, 5682, 978, 4440, 15616,
    3160, 13865, 12260, 13296, 13464, 3441, 6102, 13319, 16323, 4826,
    11594, 9550, 11298, 12928, 16237, 10601, 9912, 12767, 10785, 15814,
    828, 14301, 10594, 6111, 1255, 3104, 5838, 10520, 15610, 1516, 9230,
    6180, 12843, 6433, 1733, 4362, 10835, 5613, 8874, 225, 502, 1014,
    2545, 7717, 10955, 4782, 7791, 13278, 10412, 11562, 3984, 14683,
    12967, 6209, 11567, 15002, 87, 13146, 14256, 14558, 16322, 5568,
    14155, 9633, 7480, 8083, 11781, 5029, 3125, 15085, 6293, 10418, 7560,
    9920, 1063, 6962, 5573, 12426, 6610, 8616, 12258, 12957, 2686, 10942,
    3799, 4177, 823, 15219, 147, 8024, 3742, 5310, 11002, 15249, 8533,
    15168, 6462, 13621, 7820, 4490, 11299, 13041, 4399, 10322, 4352,
    16239, 16277, 11502, 13924, 12970, 12495, 16230, 7308, 5624, 13159,
    4741, 632, 6643, 3575, 13093, 3623, 6564, 16349, 13448, 155, 7352,
    8961, 14342, 10480, 1157, 9872, 6983, 8958, 12989, 10941, 13668,
    7932, 4772, 542, 11145, 13592, 14738, 15816, 6484, 10983, 10275,
    2407, 507, 1625, 2481, 8584, 13366, 5543, 3129, 8821, 12606, 7771,
    3308, 2647, 15650, 11952, 13385, 3049, 13487, 13660, 16361, 14482,
    7350, 11574, 5079, 5190, 1287, 2732, 8637, 4789, 6864, 8356, 10531,
    2561, 3572, 3763, 8643, 9355, 15265, 8388, 4615, 5096, 1593, 11185,
    9139, 6395, 16094, 563, 16181, 8044, 3529, 15587, 53, 4229, 9925,
    6623, 12255, 1041, 5097, 2199, 4041, 13961, 10006, 3073, 5654, 8545,
    15593, 14274, 3377, 4665, 10924, 3314, 4328, 16030, 8502, 1527, 3328,
    9571, 1150, 10960, 14525, 9159, 3642, 14230, 7667, 748, 12133, 1853,
    5640, 6285, 9735, 13962, 6536, 6058, 4896, 9067, 3808, 6792, 10014,
    12507, 7256, 8414, 9102, 13977, 6829, 3696, 2060, 1874, 14229, 9677,
    9519, 11970, 4017, 16126, 10123, 12473, 9556, 13733, 3792, 7362,
    9761, 1033, 14362, 3672, 691, 1518, 4198, 14340, 8685, 11850, 2053,
    1523, 421, 7183, 3054, 9318, 2119, 7250, 13561, 8843, 11534, 13847,
    698, 10070, 11387, 3182, 2583, 11397, 5444, 5085, 14891, 8703, 5495,
    2038, 13298, 3443, 7424, 13524, 9354, 8666, 16320, 3898, 2266, 13015,
    13374, 12420, 343, 13648, 4394, 2522, 6968, 5783, 234, 10240, 2613,
    11384, 2559, 11695, 11808, 3765, 2359, 3819, 2837, 9326, 10691, 8424,
    5462, 11750, 4081, 11807, 1144, 9384, 7477, 8108, 10282, 681, 19,
    15828, 12387, 6585, 6872, 6821, 4752, 1372, 6466, 4674, 15503, 7105,
    3788, 4053, 13806, 7523, 4916, 10977, 57, 9736, 1260, 6586, 11281,
    10754, 4493, 11582, 5577, 9130, 3677, 9323, 12650, 9041, 15841, 6954,
    14559, 2424, 3849, 7877, 9781, 16271, 16360, 5773, 567, 9592, 13988,
    13264, 14270, 3367, 248, 7584, 1226, 4700, 13574, 11434, 5755, 7599,
    3639, 8161, 6634, 6391, 6286, 13874, 16238, 13935, 8828, 5621, 4659,
    8128, 10602, 15420, 4387, 12600, 8946, 1820, 8516, 9486, 1104, 4120,
    523, 7498, 1214, 6651, 636, 12774, 4781, 8298, 4644, 12034, 5981,
    2633, 9922, 324, 10705, 3092, 10, 9220, 7457, 7376, 14591, 5958,
    9014, 13458, 12917, 5105, 3214, 5946, 6104, 11559, 12760, 10169,
    15769, 15795, 830, 2390, 6689, 7977, 15135, 8081, 4349, 3244, 10473,
    6569, 7218, 10458, 7921, 7275, 8052, 4568, 6703, 15293, 11671, 11673,
    12724, 4126, 12319, 15354, 13452, 3325, 11961, 5450, 5583, 15807,
    4466, 4777, 4202, 7985, 2281, 13364, 526, 10653, 10868, 5002, 1119,
    6948, 13326, 837, 16177, 15333, 7578, 11564, 15567, 14923, 14474,
    6310, 10215, 5658, 203, 16221, 12572, 11322, 3165, 6947, 2376, 13572,
    2828, 1363, 9163, 7007, 6593, 7200, 4855, 11873, 7258, 13738, 13562,
    13672, 11637, 7464, 13596, 11820, 564, 16044, 518, 1146, 2035, 10026,
    1586, 1369, 1871, 1731, 6087, 9260, 2973, 9992, 12804, 10903, 14308,
    6754, 1096, 5218, 8774, 157, 14886, 7089, 12666, 9179, 8063, 11896,
    10596, 15105, 8300, 8952, 11249, 10061, 7510, 6008, 4558, 8125,
    15081, 7414, 12895, 14045, 16348, 7671, 15657, 3856, 3524, 805, 3170,
    5996, 8639, 6229, 13129, 1209, 8598, 13854, 10407, 11324, 5857,
    12944, 4849, 9513, 1844, 11311, 14623, 915, 5831, 7423, 7546, 3006,
    16022, 2572, 3901, 7557, 13389, 737, 4182, 3177, 1660, 13625, 3370,
    13047, 4340, 8158, 5943, 3830, 575, 3801, 7962, 12176, 11147, 11913,
    15221, 13138, 10224, 6194, 669, 738, 10901, 4587, 15483, 11131, 9574,
    2873, 11761, 13172, 4498, 7159, 2776, 7730, 1461, 15167, 3366, 6984,
    6691, 8970, 2067, 2740, 12140, 15797, 6165, 8251, 4895, 15722, 3597,
    12559, 9452, 16087, 2210, 5863, 8576, 163, 15237, 4184, 5180, 8096,
    6882, 8472, 15398, 15690, 2984, 2762, 12223, 1005, 5070, 14949, 3306,
    14290, 6314, 15548, 1592, 12435, 5338, 8853, 8512, 7098, 4248, 7387,
    15733, 11134, 6975, 7272, 7858, 3882, 10099, 658, 10227, 12807, 8930,
    774, 13044, 13036, 4293, 15930, 5862, 10320, 8094, 9165, 12630, 697,
    15345, 10926, 15756, 3971, 13820, 2542, 6737, 9297, 10004, 402, 3690,
    493, 6142, 7588, 5969, 13913, 10217, 4946, 8022, 12591, 4715, 12833,
    10484, 1485, 11084, 8777, 15279, 3241, 6228, 15353, 313, 11718, 6073,
    5297, 11891, 24, 6014, 4024, 9279, 3744, 3684, 10382, 13477, 4759,
    5076, 3293, 1973, 10028, 4125, 8191, 14363, 513, 8753, 8048, 10750,
    6555, 1242, 2099, 7238, 12597, 15805, 2738, 1499, 16253, 14280, 6300,
    10118, 13367, 9714, 3491, 501, 1061, 6963, 7232, 14583, 11438, 1737,
    8174, 5819, 11058, 5314, 5031, 7626, 551, 13989, 1867, 8379, 11354,
    7204, 9243, 12645, 1925, 1316, 16031, 8904, 11092, 8591, 4955, 7158,
    6999, 7012, 12414, 11773, 10997, 3061, 4203, 13134, 8440, 2488,
    14747, 14275, 6367, 3344, 6245, 7618, 7365, 6041, 4811, 10109, 11504,
    10952, 327, 2512, 14073, 8296, 8680, 11749, 7635, 76, 14569, 516,
    729, 4947, 15439, 11120, 4743, 13873, 1843, 6557, 2288, 360, 15235,
    15572, 1453, 10170, 1984, 2969, 12361, 7747, 7106, 6532, 11287, 4561,
    2829, 10181, 5402, 6924, 9794, 4780, 3957, 8493, 4165, 14835, 15768,
    1463, 14727, 14948, 11000, 43, 2959, 1532, 10505, 5560, 7341, 6083,
    12935, 6765, 9426, 13783, 13444, 13780, 12761, 2351, 1143, 6336,
    11076, 10490, 1313, 15627, 2074, 14780, 14620, 7739, 14600, 11021,
    7874, 5358, 10660, 4761, 1556, 3381, 14305, 4000, 11297, 11048,
    10338, 285, 3942, 15113, 15561, 11182, 15158, 15894, 2695, 7076,
    9575, 12926, 3309, 9936, 16339, 13815, 9841, 5609, 96, 12811, 2997,
    13568, 9218, 893, 3460, 10808, 4747, 2101, 5383, 4247, 13466, 13460,
    10544, 15119, 4036, 7901, 10273, 11174, 8733, 1524, 223, 16038, 3633,
    14990, 2285, 12296, 986, 7249, 8855, 1785, 5204, 3782, 9502, 15719,
    2819, 1397, 301, 14052, 5400, 11576, 2617, 3976, 6307, 10410, 12053,
    14444, 6702, 5045, 790, 2450, 12927, 13830, 13062, 1321, 1089, 7192,
    1741, 355, 6355, 1402, 8407, 9947, 4224, 6145, 13070, 14100, 2628,
    9746, 11421, 16187, 415, 7418, 13111, 2389, 1515, 7831, 6345, 1549,
    13345, 14284, 6025, 13491, 14285, 2521, 568, 7742, 9077, 9381, 14499,
    15196, 2110, 14026, 14691, 15256, 10607, 10497, 12749, 8618, 14686,
    1604, 7718, 2921, 12931, 6442, 15357, 9022, 4559, 5415, 15761, 12008,
    6150, 4408, 9554, 788, 7948, 12744, 2669, 1793, 2375, 10745, 4194,
    14247, 7162, 15001, 14488, 10806, 5356, 14262, 2989, 13971, 11483,
    6268, 829, 16013, 15943, 15074, 5774, 7976, 912, 5437, 3709, 4091,
    11314, 12913, 9489, 3128, 11085, 11497, 9158, 7027, 16162, 6559,
    4378, 11767, 22, 6298, 15951, 187, 8147, 13096, 8890, 4681, 6913,
    4861, 10800, 8046, 1015, 10425, 9333, 11470, 6375, 5172, 8444, 8992,
    10232, 11214, 11519, 2685, 12763, 15149, 6883, 16354, 13376, 12047,
    11184, 12705, 12964, 12235, 12275, 8909, 5081, 463, 5653, 11301,
    5651, 4303, 9088, 6022, 14174, 9134, 8438, 1466, 6036, 12423, 4990,
    7169, 1488, 3789, 14046, 3576, 4712, 9787, 3528, 4513, 11777, 11264,
    10036, 2219, 9217, 7872, 1635, 6371, 5373, 8416, 9045, 6361, 3670,
    1614, 1727, 1375, 9012, 6538, 14595, 4799, 6072, 13061, 15422, 1228,
    5639, 6144, 9271, 12999, 10350, 3042, 4912, 2283, 3730, 13992, 8468,
    10758, 13285, 16188, 11561, 10472, 1302, 14556, 4892, 3268, 5466,
    8369, 2379, 15259, 4821, 2037, 10710, 14965, 7333, 3727, 2030, 7028,
    13580, 9937, 330, 7530, 13100, 15981, 6581, 5288, 13189, 58, 13249,
    9672, 3118, 11560, 12405, 8423, 8813, 2570, 9999, 8281, 11986, 6013,
    15146, 12291, 9614, 2503, 12268, 13035, 10870, 12267, 8983, 12360,
    9542, 6031, 16257, 3449, 9327, 1174, 14726, 8355, 11836, 3001, 4734,
    803, 6416, 5503, 233, 4494, 5294, 14047, 12518, 6074, 15406, 5196,
    2179, 15792, 7010, 8708, 3271, 9434, 10337, 5627, 14456, 2032, 840,
    11496, 14512, 3556, 6897, 6399, 4160, 16376, 846, 4798, 6349, 4478,
    15874, 6715, 5133, 10423, 5938, 11664, 2506, 4582, 3477, 3823, 7353,
    9984, 6189, 15959, 13638, 271, 12433, 15858, 6876, 3689, 6353, 6264,
    2500, 3267, 15090, 15598, 2891, 9622, 14603, 10150, 3138, 8737, 4503,
    5670, 14085, 14560, 7864, 47, 4852, 2577, 5875, 7164, 209, 15702,
    3181, 1065, 14867, 4026, 12144, 3354, 20, 9940, 460, 14800, 14068,
    10450, 6522, 700, 9541, 6210, 11606, 7658, 1726, 15852, 629, 10494,
    2453, 10474, 5845, 9830, 5879, 7401, 10741, 9679, 13406, 11205,
    10521, 7436, 3888, 15523, 3997, 8667, 14135, 69, 9387, 1240, 259,
    9901, 10615, 5473, 14001, 6589, 7264, 9161, 2343, 8103, 16019, 6843,
    10072, 1002, 5649, 8859, 15039, 1594, 13304, 10650, 8550, 1208,
    13079, 727, 9449, 6766, 14019, 10088, 3013, 345, 16055, 6803, 8045,
    11956, 3907, 4391, 7069, 9534, 8216, 16246, 14035, 3953, 7134, 107,
    13682, 196, 9331, 12695, 13877, 9122, 692, 8701, 8306, 3714, 4189,
    5277, 13231, 2047, 14906, 7463, 1722, 12985, 10662, 2476, 1421,
    13640, 14393, 6131, 2475, 7233, 13010, 1607, 5510, 4243, 9084, 2406,
    14606, 11589, 4070, 12827, 13239, 5393, 9753, 182, 8759, 141, 10249,
    9268, 12234, 3806, 6023, 10877, 10034, 14750, 5441, 15856, 9379,
    14687, 5262, 2249, 4425, 14529, 5843, 11132, 16111, 12288, 2523,
    3583, 4758, 3552, 5676, 8190, 14837, 1930, 6938, 5898, 3598, 13116,
    4837, 14705, 997, 1472, 9106, 4411, 13194, 13700, 1747, 3643, 1807,
    15427, 8968, 14880, 16170, 10595, 2806, 14440, 9956, 1842, 7381,
    2967, 4827, 5342, 6783, 11701, 12568, 7358, 430, 11127, 8092, 914,
    7002, 11515, 1175, 7882, 11546, 9916, 5665, 15363, 11401, 3034, 2615,
    3329, 2186, 2872, 724, 3362, 6045, 10896, 4945, 4769, 37, 7955, 5289,
    5237, 7706, 2911, 7194, 15274, 6007, 16099, 7092, 2333, 2026, 9430,
    7789, 4854, 7569, 1685, 2490, 9898, 4918, 8751, 12522, 8257, 13752,
    13943, 10349, 9632, 9223, 6661, 15697, 16249, 12531, 5922, 440, 7970,
    9002, 5399, 10975, 12289, 524, 5409, 8382, 9579, 9558, 6276, 11523,
    11307, 15661, 9520, 11880, 4389, 652, 3993, 4245, 6295, 12800, 8673,
    1834, 13300, 14843, 14746, 16010, 3820, 14614, 7116, 11529, 13987,
    16021, 11996, 2401, 4933, 1082, 1265, 5557, 6267, 6516, 2058, 1127,
    8575, 15994, 7990, 10652, 11505, 6895, 1822, 8655, 13137, 6865, 8232,
    3394, 8130, 5153, 13759, 9416, 5678, 5579, 1763, 13174, 2772, 12820,
    7421, 12449, 11556, 457, 5507, 12031, 16159, 3084, 1641, 2318, 12729,
    7481, 2310, 15813, 13254, 10683, 15637, 5913, 6085, 12185, 15486,
    478, 3057, 5963, 4762, 15609, 8412, 12026, 2120, 11463, 12313, 9154,
    10383, 1801, 12745, 9935, 14084, 13981, 6374, 694, 10656, 16000,
    2548, 16236, 14344, 5883, 9343, 11805, 6389, 5915, 6591, 12128, 4802,
    4750, 2976, 4473, 139, 3962, 2511, 12719, 4565, 15505, 15845, 13150,
    10071, 303, 11694, 9435, 8320, 15452, 13468, 13922, 2946, 8422,
    13089, 14059, 8681, 14414, 10818, 2005, 6857, 903, 11656, 4094, 6049,
    11395, 4696, 6858, 10476, 12540, 3423, 13316, 16313, 2425, 2291,
    11081, 15231, 4294, 6010, 759, 15649, 5205, 3568, 13537, 876, 8501,
    8679, 11735, 8610, 15187, 13959, 16123, 11389, 1038, 14582, 8585,
    1571, 8903, 11462, 4563, 14066, 12949, 3167, 12010, 4211, 13082,
    3015, 15292, 14664, 1608, 3250, 1036, 255, 7253, 11798, 6973, 13940,
    1377, 11999, 10654, 16373, 6508, 4122, 15770, 15311, 2784, 10158,
    3189, 11436, 6160, 10805, 12331, 9773, 9628, 14764, 5048, 690, 14899,
    12339, 15470, 8218, 5999, 4039, 12838, 8625, 6113, 7639, 6819, 10203,
    8068, 249, 12747, 15629, 1169, 11722, 15773, 5509, 14017, 11266,
    15778, 11520, 172, 10673, 15399, 8944, 1890, 8541, 11702, 5099, 74,
    2962, 15783, 3734, 12081, 2027, 10734, 13162, 13097, 9824, 3918,
    10317, 1042, 13076, 32, 1765, 15487, 15270, 7487, 16262, 12121, 7144,
    15989, 10925, 15482, 4606, 2203, 4129, 348, 11416, 204, 3081, 10527,
    6562, 592, 16184, 10131, 11683, 11974, 8555, 479, 7120, 5057, 7735,
    1166, 12369, 3625, 7950, 5941, 10136, 8037, 10668, 9006, 11619, 2588,
    6594, 13999, 8265, 3644, 6568, 15385, 12845, 14759, 7217, 1702,
    12860, 3207, 6923, 4932, 11854, 5582, 14133, 3273, 10235, 14473,
    7115, 10639, 8435, 5011, 13824, 1432, 9928, 11610, 3117, 1464, 3098,
    12951, 12855, 12983, 4994, 9438, 935, 10331, 8313, 8544, 2611, 3530,
    9382, 712, 1232, 15782, 1771, 9087, 8530, 13828, 15867, 6527, 2357,
    392, 2172, 4838, 10863, 11122, 6997, 628, 1495, 12283, 7846, 10696,
    11856, 8481, 5382, 3007, 6798, 14728, 8814, 3933, 4176, 15762, 4068,
    226, 11396, 937, 12307, 2147, 9861, 6005, 2146, 11992, 3499, 9705,
    14838, 1850, 8410, 693, 14160, 3123, 2190, 3660, 12090, 11238, 3996,
    15322, 5041, 15916, 3468, 1856, 12901, 9755, 8797, 6012, 15351,
    15953, 9914, 13978, 2185, 13773, 11160, 9046, 13309, 10793, 3567,
    7231, 1816, 15663, 3339, 3728, 2304, 13687, 3294, 15612, 5720, 12687,
    15563, 12072, 3009, 11688, 8234, 11260, 9031, 5433, 5272, 8117, 6552,
    126, 10115, 15111, 5455, 11736, 15771, 16214, 278, 5751, 2568, 12191,
    7490, 818, 11255, 5111, 7505, 7797, 12358, 3127, 4346, 9097, 14829,
    5632, 5036, 15195, 12017, 1451, 13230, 8978, 7852, 9126, 2622, 1288,
    4537, 7482, 4292, 455, 6120, 5086, 9400, 9587, 6079, 15489, 10854,
    7725, 2773, 5261, 11674, 9917, 3602, 3234, 9378, 6690, 14535, 8973,
    9535, 8915, 10911, 7638, 11259, 1342, 2341, 5529, 1929, 8236, 10265,
    12273, 6091, 1105, 15501, 1435, 5706, 10501, 13611, 2702, 13405,
    11356, 2983, 6528, 3284, 5407, 15590, 2474, 13029, 10283, 13413,
    5858, 15623, 7779, 8129, 1566, 13124, 7575, 1997, 12085, 3045, 9172,
    11951, 14534, 10135, 6427, 7674, 4428, 4583, 12821, 2013, 838, 8075,
    13897, 4357, 8765, 4312, 11927, 1994, 15340, 2224, 5483, 11365, 4709,
    1768, 10857, 6620, 11419, 4847, 10670, 6449, 14626, 2021, 12322,
    6334, 3411, 15177, 7802, 1293, 1804, 11446, 12616, 2466, 14379, 4265,
    7284, 16362, 15245, 7716, 13170, 8370, 14770, 923, 10153, 2109, 7650,
    8405, 11918, 10897, 13957, 13328, 7734, 5828, 2428, 5760, 7016, 8266,
    6757, 15065, 14096, 662, 13526, 8722, 14022, 15740, 1060, 12766,
    16027, 1799, 3324, 13266, 1888, 14986, 4605, 16169, 9035, 13970,
    11851, 6130, 7494, 10229, 2593, 5641, 11989, 7371, 12865, 13607,
    6567, 12770, 2302, 13522, 11476, 9693, 8072, 13055, 6224, 8945, 71,
    2535, 11944, 15008, 9967, 9983, 13480, 1210, 1872, 9600, 14447,
    16085, 5091, 5846, 12933, 2515, 3175, 13059, 9780, 1390, 2244, 3343,
    2814, 8613, 8515, 1981, 2497, 6917, 2305, 5388, 11770, 4617, 3201,
    7630, 16016, 208, 11517, 15472, 11010, 4549, 2895, 9398, 10766,
    10499, 6279, 1939, 2920, 2267, 7612, 11440, 714, 3436, 12302, 7524,
    3119, 14245, 8465, 5687, 14165, 11990, 12590, 11117, 14972, 439,
    7045, 3817, 12618, 14400, 7216, 6535, 12472, 9185, 15975, 15815,
    9934, 4682, 5384, 13456, 9776, 10104, 15023, 2442, 5376, 7058, 14794,
    582, 4505, 15024, 1180, 12822, 2078, 14786, 419, 12758, 6453, 6662,
    1073, 34, 11789, 6794, 11388, 2231, 13287, 5040, 15299, 16090, 14130,
    11377, 621, 4200, 7851, 10680, 2839, 6668, 6771, 3673, 6782, 8969,
    14409, 10576, 12492, 9671, 12853, 12783, 14134, 15294, 16006, 11730,
    15006, 60, 10689, 10404, 14677, 1847, 9864, 670, 13503, 8953, 2947,
    11257, 6269, 6488, 660, 8561, 11005, 5497, 9319, 2866, 9019, 16173,
    9507, 4539, 4072, 4052, 15170, 5042, 9943, 7514, 12567, 12126, 342,
    21, 5186, 10873, 12576, 4607, 7814, 9428, 11186, 6655, 306, 904,
    13914, 1396, 12290, 1202, 8185, 11492, 15992, 7354, 7778, 14882,
    2239, 7833, 15808, 12863, 6086, 8956, 14365, 4726, 11801, 6684, 1568,
    641, 11046, 12851, 2330, 5596, 13323, 7957, 6474, 1269, 11033, 2835,
    10962, 11328, 2739, 531, 274, 10778, 10082, 16097, 12074, 12900,
    14565, 7148, 2374, 2802, 3783, 7213, 688, 14166, 12986, 12746, 4199,
    15476, 9813, 2195, 1802, 2417, 7571, 14840, 6302, 15342, 7929, 11288,
    8249, 14484, 9109, 11054, 12514, 5851, 16346, 1632, 12866, 6611,
    5442, 16225, 4676, 3224, 506, 9300, 3456, 9025, 10117, 1971, 15014,
    15547, 14286, 14360, 12338, 2576, 7854, 10909, 16067, 11549, 12595,
    6284, 6060, 6741, 6844, 177, 1484, 1428, 7596, 13165, 15454, 1582,
    3760, 14775, 1250, 1598, 8622, 15100, 12510, 15961, 8595, 5434,
    11113, 5829, 5213, 2526, 335, 6443, 9697, 12384, 13054, 15753, 4106,
    9478, 13009, 6839, 12829, 2260, 231, 5320, 14745, 5210, 13157, 6768,
    16250, 9639, 2339, 390, 2879, 12115, 11635, 1826, 2295, 11966, 10893,
    13371, 5800, 13749, 70, 15050, 11075, 1846, 12980, 1076, 10114,
    10930, 1481, 5993, 10083, 192, 5552, 1160, 5722, 1225, 1278, 16259,
    16106, 4906, 710, 12293, 16213, 1829, 12341, 1541, 12679, 3173, 6871,
    2342, 11706, 8538, 12158, 3033, 5597, 2651, 7741, 12841, 10697,
    15687, 6926, 4123, 5194, 8076, 13208, 4805, 4065, 11217, 5464, 13721,
    14641, 11643, 4950, 14031, 3151, 14390, 4276, 9667, 9305, 5712,
    11278, 15473, 13756, 7740, 8184, 5489, 14333, 2749, 12915, 5412,
    11834, 11442, 1254, 14863, 12587, 4937, 12460, 6038, 1366, 3261,
    5865, 13712, 14106, 7628, 9771, 3858, 12686, 10459, 8528, 5939,
    11604, 2742, 2791, 11045, 3812, 751, 13732, 1917, 15517, 11309, 394,
    4366, 2308, 10132, 2501, 9177, 7894, 13334, 3384, 11968, 6677, 10885,
    6608, 10260, 4286, 7268, 4102, 8717, 13976, 14817, 4370, 2972, 7456,
    14004, 962, 2313, 5315, 14024, 6436, 14027, 6974, 10208, 6244, 4185,
    5159, 11018, 8485, 8090, 12012, 11828, 7151, 2097, 747, 12138, 11827,
    9821, 13241, 1588, 13669, 13593, 10395, 10370, 7221, 4804, 726, 144,
    3297, 9143, 8540, 7043, 14684, 10049, 5814, 8581, 7413, 16095, 1627,
    15153, 16063, 11151, 4029, 9682, 6061, 9863, 7617, 15724, 13776,
    1814, 3230, 135, 11638, 8335, 11079, 14111, 11785, 3135, 7455, 8805,
    13560, 9465, 72, 3897, 4647, 3616, 4872, 14593, 4702, 8053, 8231,
    1761, 5378, 8089, 799, 9440, 13967, 9699, 13401, 7319, 5427, 3256,
    2033, 11485, 12145, 16178, 9754, 1738, 1711, 4886, 10794, 9219,
    12238, 11011, 12897, 11648, 1713, 11585, 5389, 7109, 12579, 16065,
    13715, 11355, 14397, 2551, 7078, 1137, 10704, 1975, 11965, 12797,
    16261, 11063, 3890, 6900, 1380, 8950, 2549, 12287, 5644, 12312, 9855,
    3925, 9797, 10119, 1215, 12875, 5882, 2493, 11268, 12706, 10900,
    9491, 10027, 16200, 15301, 5278, 7572, 293, 14734, 8570, 15899,
    10938, 630, 1335, 601, 5534, 16359, 2764, 8310, 4058, 7485, 9823,
    9623, 13679, 7347, 5968, 11521, 6287, 2727, 16168, 9194, 4532, 5553,
    600, 13080, 1716, 14208, 2975, 15374, 12738, 10588, 3486, 2443, 7152,
    15289, 3930, 11458, 10046, 11447, 5364, 15954, 6673, 3711, 7666,
    8503, 4588, 15591, 687, 10707, 10121, 4875, 13757, 8258, 7841, 7128,
    159, 2961, 405, 612, 911, 14563, 11577, 12736, 6347, 9282, 9960,
    15909, 4433, 7473, 10009, 11407, 10862, 5093, 3966, 3363, 13356,
    5520, 2833, 14249, 10712, 9580, 2530, 1873, 364, 1436, 2982, 6211,
    7066, 4236, 15999, 6133, 12352, 8261, 13868, 13848, 16083, 13758,
    7448, 11241, 14498, 9492, 5336, 425, 14938, 15646, 6356, 15570,
    16370, 6173, 1131, 10296, 10297, 10033, 14511, 9085, 14415, 16272,
    16001, 15225, 8156, 1305, 6257, 5118, 11135, 6861, 10836, 7297, 2316,
    4409, 10575, 974, 2261, 1086, 581, 10825, 13800, 16369, 3062, 4453,
    10736, 9910, 8168, 1875, 10002, 12475, 8202, 1907, 3408, 9145, 4077,
    5108, 2540, 2336, 12952, 12323, 9881, 3609, 15785, 10514, 2176,
    10074, 9011, 13745, 4009, 7933, 9399, 5370, 1959, 7653, 2170, 3459,
    1841, 11941, 5919, 12508, 13083, 1393, 13629, 8779, 3615, 5405, 3543,
    2054, 6743, 8686, 6148, 1239, 11270, 10713, 15397, 13765, 3041,
    10511, 8362, 389, 1552, 6421, 5110, 7499, 14423, 7243, 6444, 6845,
    3800, 15295, 3827, 5656, 16375, 931, 13388, 5652, 15424, 9909, 9878,
    15919, 5847, 15512, 6290, 11587, 4284, 5137, 8518, 3708, 12867,
    14476, 8748, 9307, 11215, 5490, 2444, 5491, 12954, 11936, 4398,
    10089, 4842, 3587, 15731, 8254, 11414, 3192, 9805, 13509, 10687,
    8189, 11261, 6779, 5038, 12636, 10096, 5352, 12301, 8148, 10491,
    11778, 9437, 6096, 15143, 12941, 10384, 1251, 473, 899, 10477, 10086,
    14054, 8110, 14139, 12539, 6437, 13221, 11404, 888, 2081, 1778,
    15804, 6032, 3219, 6595, 1433, 13299, 2780, 2464, 11306, 5660, 3063,
    7817, 8126, 5746, 5808, 8535, 10851, 13558, 11652, 3048, 1008, 8166,
    3850, 1415, 7602, 14798, 12974, 12078, 7450, 14755, 6770, 2345, 9118,
    6294, 3721, 7898, 12852, 4451, 4643, 12953, 16303, 797, 5683, 11677,
    2217, 13023, 7245, 3493, 13881, 15070, 14253, 4299, 11626, 5375,
    7622, 4936, 14918, 8714, 6851, 3682, 2354, 12276, 14521, 14483, 821,
    5304, 8866, 130, 14394, 14974, 6408, 1053, 2990, 12006, 3276, 5555,
    2385, 7219, 12569, 16115, 16061, 41, 3427, 294, 13224, 13154, 2112,
    7890, 6402, 9908, 1275, 2914, 859, 932, 8982, 3235, 9725, 5078, 8987,
    8177, 9424, 3313, 5470, 13178, 10609, 9790, 11360, 9616, 3630, 11157,
    6654, 8, 7924, 1514, 9080, 1152, 15491, 6446, 1310, 6849, 14320,
    2765, 15577, 227, 14943, 12558, 2160, 10177, 6134, 12206, 8482,
    13398, 8712, 11524, 8085, 16139, 4426, 7785, 10571, 10172, 15620,
    4049, 4435, 4157, 16245, 3989, 7910, 2986, 9630, 12877, 6787, 178,
    15169, 14936, 9802, 6009, 3357, 1884, 12661, 2518, 388, 5318, 3500,
    2431, 15117, 2848, 10587, 9955, 2241, 15421, 10597, 7147, 5536,
    12664, 1893, 3159, 719, 4958, 3592, 8886, 3, 5705, 11230, 10703,
    1448, 7922, 4230, 991, 4334, 772, 7679, 1197, 6985, 1130, 11469,
    3995, 16182, 331, 3397, 13390, 6048, 9029, 8323, 3707, 122, 1813,
    12731, 8734, 1259, 7067, 5075, 11310, 183, 6393, 5053, 5715, 3544,
    11847, 1133, 15896, 8786, 1030, 9793, 3815, 9902, 15860, 7672, 11661,
    14826, 6218, 527, 2646, 2696, 2868, 10786, 488, 12157, 9375, 2070,
    2926, 6815, 7614, 2556, 15751, 2076, 8934, 12657, 10140, 16274,
    15862, 5102, 15202, 6299, 5171, 7676, 75, 15705, 13380, 4942, 14797,
    339, 12699, 12242, 5909, 4740, 8220, 13980, 8995, 4869, 12041, 11615,
    7475, 164, 14332, 8055, 9124, 11902, 2151, 9460, 15247, 3738, 5027,
    7799, 3341, 3457, 11593, 16015, 5083, 2113, 4054, 1227, 12530, 11810,
    5436, 5411, 12040, 10389, 9144, 9357, 1057, 6146, 7380, 14609, 14330,
    13571, 6304, 537, 8400, 1609, 12526, 8788, 14929, 10220, 15242, 9150,
    4984, 6090, 10190, 1643, 2717, 1249, 13661, 14633, 10247, 3538,
    10486, 12326, 12654, 10343, 4437, 2974, 3180, 1774, 4828, 1772, 9636,
    11274, 5244, 9737, 4092, 13108, 7040, 766, 1658, 13354, 15494, 15970,
    12002, 5246, 3488, 11991, 9670, 4456, 14857, 8582, 8432, 9395, 12479,
    13785, 10971, 6738, 10969, 15210, 2950, 14190, 15458, 5134, 7681,
    6980, 13218, 9604, 12211, 13656, 14453, 961, 7104, 6206, 5426, 13855,
    8064, 14063, 4516, 1742, 5890, 6398, 6166, 6512, 67, 10590, 7579,
    1171, 15759, 9561, 9944, 4375, 6772, 11067, 12212, 11291, 13551,
    9247, 10346, 8730, 176, 5750, 10548, 2735, 3392, 11318, 1051, 8289,
    13436, 8906, 15174, 5326, 124, 16143, 4046, 5428, 7433, 9836, 889,
    7642, 12993, 5634, 11538, 1412, 410, 4788, 6683, 3280, 2779, 8318,
    2083, 13347, 264, 14901, 13996, 9517, 4205, 14708, 8732, 1306, 3717,
    1767, 5485, 14126, 7632, 12673, 13238, 3570, 12712, 13120, 8834,
    9764, 10168, 13461, 1980, 8134, 3225, 7064, 12716, 11461, 2039, 7611,
    4765, 9455, 10663, 8067, 5051, 5959, 5419, 1895, 5716, 2934, 6813,
    8466, 5421, 13548, 12111, 2813, 9201, 11216, 1531, 4464, 4015, 10303,
    11129, 372, 8408, 1023, 14318, 7215, 8917, 6674, 1193, 15378, 13156,
    15304, 7810, 12362, 12193, 7055, 2434, 1998, 1167, 1479, 15688, 4634,
    8030, 2220, 6904, 3239, 13329, 12038, 11252, 15854, 2842, 2139, 6237,
    10939, 3580, 9653, 2498, 2068, 9888, 5184, 8694, 6866, 11886, 14374,
    6143, 6565, 10636, 8348, 1364, 10144, 1149, 10540, 1282, 7248, 10085,
    6952, 12884, 2703, 1346, 5923, 7394, 7715, 10549, 1723, 9929, 13447,
    4492, 16032, 169, 1538, 13188, 8215, 15521, 4600, 7425, 7068, 13340,
    14707, 699, 15007, 5580, 6942, 2152, 12854, 9569, 8066, 12489, 3892,
    52, 14501, 118, 6853, 4874, 4619, 1636, 1591, 15766, 12469, 13479,
    9175, 15839, 15387, 6778, 11931, 2663, 3332, 13052, 9828, 13021,
    3188, 10809, 5950, 8645, 6545, 9515, 8097, 1011, 1268, 7756, 14072,
    10252, 16381, 13748, 880, 8976, 2861, 1141, 14842, 11876, 11057,
    15326, 247, 15035, 4239, 14328, 15203, 1159, 10640, 6289, 4931, 7080,
    16366, 2129, 4703, 1039, 8132, 9615, 2269, 648, 13, 1991, 13860,
    13521, 4693, 9973, 2088, 7705, 11155, 2909, 6534, 14957, 4295, 5717,
    1354, 6615, 3617, 16, 1447, 15671, 4970, 13205, 10978, 10341, 6016,
    15741, 8342, 11400, 7647, 6105, 7909, 2173, 15689, 15527, 4657, 1659,
    847, 4257, 14546, 8676, 8361, 7099, 13136, 15438, 6714, 14665, 10832,
    4087, 6873, 12102, 3295, 1054, 6263, 4992, 1699, 14122, 4252, 11457,
    5207, 8098, 4774, 8620, 2257, 892, 5805, 4388, 10967, 8049, 4078,
    4981, 15513, 9882, 6990, 13680, 2178, 6409, 10743, 11842, 3828, 8524,
    4876, 3769, 14146, 3706, 1730, 5855, 11213, 15138, 13190, 1783,
    16350, 5245, 7843, 9508, 11611, 4002, 10236, 5406, 14129, 14585,
    4118, 7024, 16120, 11995, 13549, 9383, 15305, 15651, 11316, 15041,
    6705, 2426, 15569, 6860, 12775, 15102, 5379, 7107, 4326, 9456, 9717,
    6495, 1794, 2069, 11258, 11423, 8650, 9155, 15376, 6099, 10772,
    16084, 1273, 12051, 7755, 14576, 5410, 8988, 7222, 7035, 3027, 3210,
    4603, 9351, 574, 16224, 4301, 10079, 3374, 1701, 15830, 4111, 16204,
    14172, 8406, 8428, 10890, 15793, 15201, 5540, 5825, 6939, 7591,
    15993, 7713, 14678, 11406, 9125, 633, 13550, 5607, 3359, 13114, 5807,
    11788, 8165, 8376, 5948, 5044, 14412, 14858, 1247, 11771, 10206,
    7786, 15068, 15097, 5869, 4754, 16368, 6604, 15173, 2649, 2980, 8020,
    15412, 856, 1729, 12241, 2604, 8112, 4624, 14668, 9233, 2243, 12215,
    10228, 13125, 14074, 13339, 5795, 9162, 3105, 1046, 14763, 14848,
    9009, 9975, 6524, 10578, 10972, 7053, 1564, 9720, 5772, 2271, 11359,
    14358, 13610, 2460, 4501, 9570, 2700, 10164, 3588, 9739, 4733, 13425,
    586, 5701, 12019, 1367, 8086, 5542, 3754, 3632, 10029, 7603, 16148,
    442, 4567, 2360, 12814, 9136, 16160, 14807, 3894, 12372, 3947, 3507,
    10463, 3264, 1922, 23, 13949, 2596, 10462, 15217, 6587, 3862, 7431,
    6328, 12552, 10803, 9553, 3869, 4742, 8277, 14177, 1507, 4889, 10637,
    3382, 265, 9560, 2708, 5132, 6685, 8214, 13528, 1245, 8640, 12095,
    11017, 13269, 5514, 547, 13014, 13805, 8922, 13207, 7710, 2328,
    10819, 197, 2031, 11775, 15625, 6906, 10538, 8985, 6282, 11030,
    11156, 8127, 2863, 15531, 5394, 14176, 15976, 8105, 1333, 3732, 2202,
    12885, 5140, 11060, 2437, 14465, 7093, 11791, 10336, 3076, 12070,
    3884, 39, 4731, 8876, 9591, 11233, 308, 1819, 1044, 3824, 3638, 485,
    5197, 7935, 1770, 11130, 11444, 10351, 13338, 14608, 7678, 3068,
    12996, 5201, 9137, 8965, 9447, 5150, 15307, 8882, 13372, 503, 2639,
    15996, 1176, 3262, 9949, 6678, 14654, 15757, 11069, 14281, 1694,
    13941, 5284, 8520, 12709, 3778, 5496, 1304, 7271, 5049, 4668, 14703,
    10030, 15015, 6441, 10378, 10646, 2754, 2445, 12847, 7987, 14053,
    2274, 15280, 9642, 7202, 2250, 5292, 941, 3818, 11168, 13500, 7331,
    14246, 14852, 9380, 6986, 12042, 12892, 10973, 14062, 3518, 12011,
    15497, 13490, 15264, 9877, 8902, 15873, 5929, 15656, 8304, 7317,
    4104, 4465, 12396, 7963, 10641, 6169, 328, 3320, 5073, 11703, 6886,
    3050, 6170, 16301, 8431, 14592, 11818, 15447, 14195, 15600, 3403,
    2044, 15870, 10679, 6817, 6646, 11987, 5417, 15204, 6946, 12272,
    11501, 5877, 14545, 8846, 4267, 12407, 14988, 12984, 2080, 8981,
    5300, 9386, 2451, 48, 4713, 11203, 9166, 13213, 2854, 1339, 9972,
    8590, 10929, 9422, 5348, 12965, 7386, 13649, 13516, 9715, 3248, 9371,
    12859, 11541, 1953, 3713, 12035, 8829, 10875, 2917, 666, 6786, 12412,
    9751, 14594, 14540, 375, 189, 14086, 9795, 12462, 14216, 3215, 15386,
    12723, 6629, 5222, 13397, 14289, 10411, 593, 5797, 4699, 6672, 3577,
    14490, 8255, 1163, 5074, 14338, 11662, 246, 9895, 16073, 14191, 2299,
    3414, 14631, 444, 6332, 9990, 5748, 619, 9655, 8023, 12308, 13677,
    11327, 8418, 15150, 7644, 8923, 3921, 1045, 287, 14944, 15308, 10312,
    11748, 14997, 10722, 9819, 1869, 13626, 10677, 14020, 15979, 998,
    9493, 14366, 1852, 7114, 11821, 15630, 14161, 11763, 1931, 6501,
    14341, 8439, 8626, 4206, 15089, 15748, 9840, 6574, 12594, 1525,
    11106, 16011, 10532, 2264, 1937, 2484, 6736, 10675, 12938, 6125,
    16091, 3981, 9545, 559, 15987, 6151, 4281, 12282, 15800, 1234, 4502,
    15911, 10295, 4001, 2381, 13140, 9815, 10946, 14815, 6978, 5784,
    8268, 7762, 10352, 8377, 13590, 2095, 895, 3458, 10765, 12432, 11165,
    921, 3903, 15348, 6833, 10309, 10248, 317, 2349, 14999, 10902, 8709,
    13870, 12303, 3816, 952, 211, 13092, 4704, 12298, 5513, 8224, 5258,
    9865, 11906, 8219, 11679, 15043, 9867, 3504, 15457, 14624, 14928,
    16130, 4667, 11616, 13692, 3891, 7577, 11119, 7509, 7943, 5973, 9564,
    8744, 4832, 2338, 6780, 14692, 9645, 14735, 4810, 12116, 14306,
    15774, 8162, 9476, 14625, 1533, 7269, 9532, 14813, 2482, 8354, 12348,
    13229, 6365, 12806, 10047, 1748, 15826, 5590, 12823, 5390, 2276,
    3549, 8773, 657, 12424, 1403, 5107, 1315, 10515, 6616, 7753, 2761,
    3134, 15781, 1672, 11405, 16137, 5964, 515, 8754, 9778, 14751, 7813,
    2189, 16232, 7398, 15601, 15578, 8858, 1216, 5123, 15668, 1486, 2796,
    3291, 14033, 10442, 14806, 9518, 580, 9322, 8549, 8695, 4314, 4581,
    14704, 9229, 2539, 12279, 1773, 13975, 15698, 11480, 9273, 1123, 461,
    13306, 12261, 13244, 3943, 10711, 13320, 4279, 15440, 2347, 9123,
    5747, 8394, 717, 16207, 12500, 489, 6035, 13290, 14818, 2757, 12114,
    13091, 832, 4610, 12486, 12496, 569, 14384, 10375, 4880, 9779, 2992,
    10811, 5870, 16092, 8187, 10107, 2055, 315, 1114, 13392, 3238, 2816,
    9747, 14312, 7385, 6888, 9350, 2122, 5567, 8532, 10362, 7247, 5185,
    12515, 15923, 14579, 3143, 11321, 4093, 16336, 13582, 5063, 16050,
    7511, 9306, 2422, 3209, 10791, 1537, 1055, 2877, 9510, 12659, 13446,
    10542, 347, 15419, 14509, 15955, 8415, 14998, 14730, 12076, 11029,
    2758, 111, 11739, 4670, 14672, 10628, 6541, 8434, 12995, 9477, 5420,
    3626, 649, 8980, 5612, 10147, 6308, 4055, 15913, 14239, 15904, 5707,
    2480, 16117, 11027, 8430, 12321, 12589, 2793, 12737, 12014, 15166,
    1291, 11056, 12120, 7979, 3490, 13918, 1868, 1717, 6761, 8381, 9457,
    9817, 2020, 11245, 2405, 13821, 5430, 8881, 206, 4510, 769, 9607,
    9335, 4341, 14577, 1581, 6158, 15541, 12910, 1219, 15183, 10221, 970,
    1653, 13823, 13910, 5347, 10760, 9082, 5622, 148, 16206, 9750, 5924,
    7014, 16329, 11714, 215, 15506, 8972, 15614, 15186, 8225, 1449,
    12573, 3079, 152, 13040, 3008, 11173, 6575, 11983, 7698, 5766, 9252,
    15632, 2093, 11138, 2131, 8080, 13453, 2240, 2282, 5429, 6383, 14659,
    6601, 12066, 13972, 14438, 10545, 15883, 14629, 14904, 10886, 8883,
    1805, 4082, 15382, 9816, 9971, 9777, 16227, 12043, 4691, 4035, 9818,
    7568, 3286, 11499, 13591, 11900, 14472, 9469, 1320, 11051, 13617,
    5486, 10828, 10205, 2098, 7840, 522, 7805, 3395, 5911, 1993, 3618,
    1049, 10844, 7489, 1229, 3634, 9837, 15344, 8324, 14405, 4898, 8810,
    5458, 811, 5267, 8897, 15537, 3909, 15108, 13539, 1617, 13902, 11425,
    2979, 14098, 9495, 15622, 5966, 9769, 3156, 11083, 10583, 7729, 1762,
    11815, 5733, 6649, 6324, 14099, 12092, 9707, 13250, 13077, 5619,
    11830, 6069, 12620, 4343, 8921, 13256, 16308, 7061, 11746, 5279,
    7749, 3389, 11300, 5595, 9634, 696, 16049, 15675, 14048, 11208, 7913,
    9647, 129, 15028, 6820, 10648, 4815, 15162, 4500, 5095, 1109, 15780,
    15509, 7142, 2012, 755, 7796, 10355, 9135, 9028, 4022, 3187, 10921,
    706, 8739, 5680, 12629, 8718, 9183, 4672, 1431, 6178, 5623, 16284,
    7397, 13589, 2369, 10773, 12259, 5144, 13336, 7259, 1493, 853, 2335,
    11039, 8966, 8571, 5392, 8411, 1452, 8151, 1919, 11727, 10775, 4784,
    332, 7302, 3750, 5626, 6774, 760, 10888, 2121, 9216, 11977, 2510,
    12786, 573, 15589, 2855, 13478, 2000, 3578, 1647, 356, 4766, 10193,
    14714, 3400, 9626, 6663, 16379, 5629, 8401, 1915, 10212, 3474, 13639,
    5834, 2675, 5357, 9259, 10452, 395, 11934, 4195, 10167, 1977, 5584,
    9059, 7306, 16194, 663, 8940, 13915, 10820, 5211, 15335, 15679,
    11008, 12422, 1916, 10264, 13798, 4447, 4576, 11716, 14739, 6708,
    10035, 8775, 438, 9368, 12007, 8347, 10461, 13741, 13391, 1500,
    12717, 14175, 12250, 13135, 10665, 16154, 10258, 8931, 6919, 3066,
    11872, 15462, 15208, 14348, 8716, 2208, 2660, 7808, 3052, 15535,
    10723, 6109, 13671, 83, 10050, 7531, 12881, 8099, 5864, 3608, 7690,
    5781, 11800, 10604, 14149, 484, 3203, 2017, 13225, 5730, 4555, 1914,
    5806, 4483, 9743, 15240, 14791, 1883, 10612, 4528, 2569, 1864, 10730,
    2396, 672, 15227, 2214, 2608, 7746, 7050, 9961, 13784, 7453, 6123,
    2938, 4237, 15787, 3023, 10562, 9017, 616, 12134, 10994, 1222, 7574,
    15246, 5928, 9073, 16371, 6747, 1795, 10561, 8631, 9321, 5709, 12386,
    1459, 10077, 14107, 11628, 6848, 3401, 6192, 4690, 8657, 6550, 13312,
    5193, 4771, 7000, 8713, 2544, 3758, 13871, 6647, 4450, 9396, 11432,
    4609, 8548, 6621, 376, 7608, 3803, 9121, 12248, 11776, 16018, 8877,
    4727, 15750, 2867, 12809, 4253, 10834, 14790, 13557, 99, 7244, 735,
    11725, 4384, 5947, 9868, 7567, 16304, 14632, 7563, 2358, 13917, 5386,
    3124, 5191, 31, 5498, 10020, 10166, 4471, 10043, 8153, 5533, 6451,
    8059, 13505, 11819, 14244, 7835, 15634, 16056, 13153, 14784, 7298,
    3770, 5019, 5700, 4491, 14455, 9811, 16252, 12101, 15222, 14209,
    15712, 5167, 10614, 14364, 13008, 12355, 7751, 638, 54, 1154, 6490,
    8725, 5962, 10923, 10709, 14345, 14206, 2155, 14799, 8079, 11244,
    7449, 11170, 3142, 11459, 2394, 2724, 14295, 11571, 10658, 5033,
    6832, 1651, 15640, 10871, 8574, 1405, 270, 2899, 5517, 13813, 6943,
    14881, 13644, 5365, 14884, 2929, 6592, 13476, 11326, 16251, 2830,
    844, 1404, 5385, 11639, 8633, 7325, 16193, 7996, 6558, 12713, 6767,
    8417, 6932, 15641, 1161, 4755, 3026, 1423, 4311, 1866, 10439, 13716,
    4579, 4594, 1262, 9527, 9744, 3323, 5550, 3586, 4142, 12925, 11472,
    10753, 8364, 6312, 12419, 143, 4881, 15403, 13544, 2029, 10326, 5892,
    5908, 10211, 981, 5610, 9113, 3206, 14809, 14479, 353, 10420, 6749,
    8822, 6052, 4290, 1329, 6386, 9562, 8314, 2620, 13839, 5502, 14991,
    6498, 12946, 10225, 14127, 14434, 12651, 7875, 5546, 13799, 11506,
    1156, 15134, 6168, 3668, 7332, 15389, 11627, 13701, 13330, 5983,
    5980, 6987, 2245, 5655, 9208, 1800, 13622, 7942, 9876, 8913, 13421,
    13107, 2010, 10161, 11277, 8180, 3217, 15120, 13259, 377, 13740,
    15053, 15520, 3417, 15160, 2254, 5228, 4671, 16254, 8233, 15130,
    4648, 15599, 10560, 10802, 14876, 6238, 2937, 13898, 9120, 13923,
    4806, 2845, 514, 8867, 3051, 14321, 14748, 2725, 15277, 15013, 1398,
    4415, 6481, 9385, 12715, 14871, 12921, 5438, 5852, 12634, 3956,
    14227, 12477, 5697, 8470, 15432, 1522, 804, 3353, 3676, 7360, 916,
    10051, 14607, 2180, 7939, 15619, 2154, 12602, 4196, 16201, 7417,
    11053, 288, 4716, 2430, 4525, 1325, 8558, 2520, 10982, 1483, 6949,
    11570, 5591, 5413, 14041, 7087, 10359, 13246, 3519, 9187, 934, 80,
    10128, 13766, 1626, 995, 12164, 5735, 15098, 4512, 14310, 9658,
    10145, 6838, 833, 6914, 8121, 12861, 14781, 16223, 12808, 14097,
    2287, 13265, 875, 9974, 6635, 13327, 11597, 9348, 15216, 10559,
    12395, 15369, 9005, 10856, 131, 2182, 8566, 8458, 1231, 6384, 10055,
    11678, 4214, 5325, 1512, 4695, 8784, 10757, 12254, 14076, 15261,
    10728, 5088, 6630, 3496, 5839, 3379, 16216, 703, 3545, 7980, 771,
    565, 10403, 7836, 3338, 7304, 928, 12382, 11871, 13293, 5130, 4541,
    2265, 7294, 7337, 9367, 11358, 14373, 15519, 4096, 10201, 15576,
    2247, 14680, 2670, 7041, 8474, 4770, 10580, 14757, 15144, 11455,
    4887, 1172, 78, 5920, 8477, 15306, 640, 12945, 2100, 12050, 14722,
    3621, 1473, 362, 9186, 10557, 6730, 9511, 4497, 3287, 16231, 6911,
    7521, 2713, 12106, 15109, 8933, 15405, 5280, 4482, 3501, 1142, 15355,
    4114, 9709, 11028, 11729, 4020, 10113, 14148, 8078, 13889, 4993,
    11235, 2423, 2904, 739, 3419, 13532, 138, 15190, 6205, 8449, 4554,
    14167, 6902, 12233, 1851, 5151, 10216, 14828, 9040, 4085, 4551, 9538,
    6693, 3780, 8711, 1624, 12696, 720, 8392, 2391, 7720, 14427, 6217,
    12329, 9090, 1203, 12920, 4652, 1307, 4530, 3450, 10467, 16292, 9210,
    14670, 10222, 7656, 9662, 3445, 8146, 13001, 207, 4485, 12142, 2679,
    16035, 9692, 4063, 3471, 12147, 7832, 8227, 6186, 10390, 10914, 2124,
    1050, 9890, 7036, 9702, 7594, 13410, 7096, 11792, 5758, 622, 8632,
    9723, 12295, 1281, 9239, 11382, 4680, 459, 16189, 8794, 8363, 7483,
    10751, 13073, 1746, 9548, 2714, 7263, 9791, 11922, 13063, 15821,
    2082, 11040, 14954, 11080, 11908, 11608, 1120, 2626, 11813, 13038,
    4902, 8467, 14870, 2028, 15980, 16241, 11037, 15205, 3088, 10163,
    7290, 3091, 12793, 6153, 15914, 6671, 2268, 897, 13237, 12929, 15087,
    4914, 13809, 7307, 9980, 9870, 4263, 9074, 13119, 7914, 10910, 7379,
    432, 6996, 12221, 13411, 7961, 8602, 4083, 3938, 3200, 15230, 10369,
    14557, 10284, 12044, 11055, 10100, 1345, 12188, 10269, 5380, 9597,
    262, 4208, 11993, 3012, 9987, 3779, 8827, 16029, 5251, 14648, 15045,
    13706, 2016, 6580, 2293, 2048, 15255, 11509, 4452, 10045, 11402,
    6039, 14955, 3695, 2461, 8120, 5482, 13789, 6055, 15692, 4259, 15254,
    11642, 5745, 9280, 15714, 4241, 13982, 7959, 3176, 2046, 7984, 1562,
    9317, 16153, 14861, 2797, 6062, 12803, 7214, 9188, 7518, 5508, 5397,
    1117, 7605, 10040, 12442, 10957, 14537, 7529, 836, 10829, 9056, 7892,
    12063, 8446, 12697, 14531, 4638, 11794, 1387, 10651, 4318, 13067,
    11724, 3564, 6505, 13585, 10496, 15479, 6890, 15367, 14801, 552,
    8741, 12633, 10198, 13101, 9894, 16060, 6892, 1000, 1646, 14470,
    5350, 11649, 5789, 13693, 2008, 8039, 12592, 8058, 4080, 4877, 9058,
    5974, 12200, 10262, 13109, 2747, 15171, 10430, 8246, 5448, 1962,
    7824, 14069, 9043, 3240, 6066, 7157, 9013, 2836, 8975, 14311, 5985,
    13829, 4669, 2116, 680, 7149, 9377, 12581, 13834, 5208, 3304, 11710,
    8517, 5249, 13068, 9168, 14202, 56, 13827, 2857, 9516, 9954, 14224,
    10731, 15621, 6100, 15318, 15946, 9431, 417, 11474, 15859, 11787,
    13022, 13606, 5794, 2652, 5894, 7949, 4524, 2634, 4650, 10148, 7327,
    9241, 9573, 9696, 9911, 3228, 15338, 8615, 11460, 12249, 240, 8104,
    2322, 4636, 12375, 7146, 4662, 15838, 13342, 10638, 2494, 5885,
    13483, 845, 1079, 15693, 695, 9294, 413, 309, 13027, 6219, 3223,
    8551, 13485, 15492, 7899, 15165, 4633, 14793, 5690, 14932, 13514,
    4300, 6338, 6523, 3832, 10764, 1450, 6777, 5476, 10965, 5331, 6556,
    305, 10572, 12436, 1289, 4376, 7528, 13051, 12347, 11933, 3430, 975,
    13531, 11289, 3922, 1881, 4943, 8366, 12780, 14820, 10243, 10325,
    4291, 4413, 2002, 14259, 11062, 15244, 11969, 1465, 7013, 4935,
    15566, 8441, 1849, 3380, 16119, 2507, 12601, 9734, 12605, 2755,
    13222, 8123, 15096, 10622, 10936, 7129, 8248, 7969, 13794, 6110,
    13767, 6221, 11666, 15395, 6773, 14049, 11159, 579, 2907, 6021, 4321,
    13753, 13417, 4159, 555, 5114, 9392, 15485, 16264, 7912, 13567, 2900,
    4954, 13072, 15939, 3641, 7042, 7001, 534, 13876, 533, 12231, 16017,
    13734, 13845, 55, 13474, 12924, 12336, 9366, 5987, 1009, 6686, 2783,
    5125, 9007, 8040, 16242, 11427, 9021, 2309, 2273, 15735, 13612, 427,
    7235, 16121, 1766, 6814, 7866, 14258, 8014, 14846, 11542, 6067, 2487,
    15788, 4753, 1687, 12439, 12057, 3179, 5719, 14279, 7559, 14682,
    12349, 8303, 14744, 2427, 2787, 11467, 13415, 4678, 13690, 15436,
    3290, 14646, 381, 9213, 14173, 8793, 1087, 10726, 2586, 13724, 4612,
    14919, 7995, 2159, 9413, 13929, 12468, 3554, 3065, 5721, 6563, 4729,
    9467, 14090, 15178, 7165, 5286, 10586, 1016, 13204, 9488, 7326, 8949,
    11153, 13037, 3948, 4547, 10433, 8660, 9660, 10214, 11826, 15639,
    2805, 6320, 7978, 13875, 8771, 13463, 12021, 462, 12533, 14168, 2471,
    16190, 3881, 9061, 13825, 3399, 8892, 3516, 4427, 12125, 8238, 10826,
    12172, 14422, 7150, 9192, 6382, 12903, 10406, 2664, 7037, 6372,
    11894, 8137, 10363, 6137, 4897, 15323, 9875, 8453, 12198, 3619,
    12219, 6080, 14478, 81, 283, 11513, 5790, 3195, 10434, 7925, 8727,
    7207, 860, 4818, 5187, 11882, 5056, 1084, 5200, 13936, 4067, 15866,
    1664, 12018, 15453, 606, 1071, 8221, 8054, 5767, 4495, 10981, 8871,
    13835, 6339, 2901, 16270, 4146, 14725, 15093, 1475, 9783, 8297, 9402,
    11025, 15218, 1106, 3152, 10782, 9563, 15337, 14163, 1384, 12907,
    9758, 6311, 8804, 3726, 15837, 14578, 10159, 7936, 6475, 5796, 4578,
    2383, 7607, 6239, 9339, 14339, 5231, 15275, 831, 15962, 906, 14920,
    4232, 6097, 1022, 1426, 6582, 10671, 15157, 13394, 15742, 5874,
    15786, 12668, 1221, 3387, 3141, 12304, 2987, 746, 10556, 7759, 219,
    4372, 7407, 5638, 9825, 2489, 6377, 520, 11852, 2673, 5271, 12870,
    6428, 13046, 1276, 9986, 13462, 689, 10546, 12726, 12513, 4344, 7615,
    14212, 8002, 10543, 3245, 92, 14878, 10631, 852, 13127, 7938, 11110,
    16335, 8710, 13409, 7590, 11431, 10138, 8851, 465, 15677, 2235, 2799,
    2865, 1534, 15850, 2966, 4844, 13930, 4836, 1129, 8299, 14276, 12994,
    10998, 14083, 8375, 10947, 14350, 5866, 583, 4507, 14267, 380, 7968,
    12143, 8437, 7153, 14466, 10333, 13720, 645, 3161, 4862, 13502, 5635,
    94, 8455, 7782, 10669, 4598, 2760, 11579, 882, 5937, 13642, 14823,
    7006, 14128, 8003, 3305, 10287, 8173, 13905, 16282, 1110, 2263, 2988,
    13074, 979, 2198, 12089, 4987, 15455, 10288, 5713, 14980, 1118,
    10239, 2691, 15784, 1213, 3663, 3795, 5308, 7536, 15706, 6764, 4584,
    404, 6322, 77, 12740, 10905, 4934, 3270, 12879, 15515, 4632, 3327,
    7340, 745, 16356, 1952, 1323, 8018, 3402, 613, 10813, 7240, 6225,
    16384, 9759, 11874, 334, 6735, 9365, 1072, 15251, 1211, 6721, 2637,
    11924, 8997, 15056, 12918, 9266, 4663, 1328, 16108, 3067, 6811, 6129,
    9727, 15272, 14461, 15040, 2384, 7237, 9316, 5688, 7363, 6082, 10816,
    12614, 12371, 1358, 4637, 6578, 6752, 2775, 15425, 5229, 5401, 340,
    5174, 15188, 13360, 1503, 9832, 14616, 5094, 15064, 8841, 1583,
    10749, 2470, 7404, 13167, 12381, 2036, 9050, 9138, 10954, 6315, 4249,
    9176, 6020, 10081, 4858, 11612, 4907, 5899, 15583, 2955, 12108, 8336,
    7206, 10912, 14094, 1899, 644, 12615, 14939, 9678, 2922, 5711, 16337,
    11741, 3972, 5824, 11478, 3351, 5504, 2353, 2597, 12470, 1855, 9032,
    9415, 2317, 7065, 12109, 1932, 944, 14263, 868, 5009, 9551, 3071,
    15055, 4116, 14220, 11488, 4381, 6804, 14532, 10579, 8990, 10116,
    1477, 3715, 9458, 10525, 7015, 5776, 2378, 13142, 1052, 16157, 13958,
    14983, 3106, 4708, 9654, 9033, 4995, 7862, 1528, 16280, 7372, 2885,
    664, 11817, 132, 14539, 7732, 13808, 8662, 1559, 15885, 12548, 6348,
    8914, 11617, 3906, 15018, 42, 1662, 15383, 2601, 4504, 15466, 13937,
    5299, 9995, 5066, 9453, 3917, 11994, 16334, 15129, 9522, 10285,
    14442, 15545, 10842, 1496, 584, 3155, 5117, 15122, 3647, 6515, 8290,
    9015, 8791, 12165, 7812, 9772, 1283, 8577, 15051, 5006, 3246, 12448,
    6463, 14504, 14194, 11645, 2075, 9896, 9873, 13802, 11580, 5821,
    6841, 15643, 12848, 10536, 11380, 1629, 10933, 5154, 4545, 8060,
    1575, 5998, 6223, 7562, 12816, 1196, 6056, 3988, 959, 8062, 1936,
    9265, 14634, 12181, 4079, 1037, 2455, 14489, 13314, 10999, 12689,
    6071, 229, 5192, 15658, 10949, 12022, 12253, 13708, 2366, 11832,
    5927, 13149, 800, 2398, 13643, 10784, 1420, 15799, 13378, 3087, 2364,
    8873, 4187, 5182, 2350, 1298, 13931, 10616, 2607, 13608, 64, 91,
    16172, 3292, 1311, 7497, 15137, 11863, 3967, 28, 4162, 4961, 8239,
    2579, 12151, 9835, 6989, 8864, 8629, 10574, 1492, 2331, 15723, 10503,
    14979, 13718, 2923, 12373, 6606, 14064, 9526, 9829, 5802, 7474, 3424,
    5136, 7145, 8358, 11319, 14650, 806, 10310, 5147, 14952, 10207, 2609,
    2230, 6791, 4117, 16341, 5844, 12828, 12367, 5408, 13493, 8742,
    14242, 6470, 3199, 2942, 8288, 8109, 4059, 1224, 7209, 11511, 9879,
    4944, 1070, 5316, 2001, 10175, 8451, 9903, 3031, 4066, 10393, 14006,
    3404, 15703, 4147, 16105, 11584, 673, 9207, 1700, 11569, 12968, 8492,
    7316, 15026, 10080, 12521, 1338, 8817, 4358, 1784, 15176, 14391,
    14637, 191, 12512, 5605, 11734, 5160, 3905, 15871, 2655, 10884, 9249,
    10801, 9479, 12409, 7527, 12393, 1074, 9976, 6155, 1460, 15197, 2834,
    5818, 1190, 3603, 281, 1277, 9756, 15921, 1092, 2056, 11756, 1943,
    6089, 9847, 5161, 13434, 1081, 5636, 14443, 9174, 12251, 13034,
    12049, 14702, 4395, 1511, 3108, 3746, 2025, 2161, 2752, 7694, 6469,
    12626, 1652, 8252, 14223, 14356, 10054, 13024, 5692, 7070, 12759,
    3126, 10659, 15037, 1689, 7389, 12, 11315, 7083, 7659, 10011, 651,
    9142, 14802, 15897, 14402, 2439, 5900, 9051, 3737, 1675, 11121,
    10067, 4480, 8693, 1455, 12873, 10518, 9114, 11855, 2991, 15880,
    5544, 12685, 4172, 5647, 10447, 12263, 4272, 5209, 5554, 12923,
    10101, 7465, 15112, 7435, 15467, 6856, 8878, 2818};
    size_t array_keys_sz = sizeof(array_keys) / sizeof(array_keys[0]);
    std::vector<int64_t> keys;
    keys.reserve(array_keys_sz);
    for(size_t i = 0; i < array_keys_sz; i++){
        keys.push_back(array_keys[i]);
    }
    rewiring_check(keys, false, 64);
}

TEST_CASE("sum"){
    pma::initialise();
    using Implementation = APMA_BH07_v2;
    shared_ptr<Implementation> implementation{ new Implementation{/* index block size */ 8, /* segment size */ 32, /* extent size */ 8} };

//    size_t sz = 64;
//    for(size_t i = 1; i <= sz; i++){
//        implementation->insert(i, i * 10);
//    }

    // a permutation of the numbers between 1 and 1033
    int64_t sample[] = {543, 805, 74, 79, 250, 685, 580, 447, 86, 116, 299, 122, 1028, 769,
            976, 702, 126, 353, 381, 888, 374, 822, 77, 139, 991, 986, 407, 259,
            905, 183, 98, 286, 15, 360, 242, 924, 331, 919, 175, 33, 3, 435, 506,
            372, 516, 815, 594, 748, 852, 860, 659, 990, 310, 1004, 497, 345,
            614, 303, 526, 632, 394, 401, 972, 964, 671, 49, 933, 9, 679, 903,
            662, 863, 899, 209, 645, 365, 975, 755, 841, 366, 747, 461, 923, 699,
            980, 796, 438, 1019, 636, 112, 697, 655, 240, 158, 935, 878, 994,
            408, 1030, 517, 129, 724, 551, 498, 600, 673, 604, 456, 695, 224,
            376, 17, 648, 323, 823, 713, 117, 450, 589, 23, 694, 913, 134, 267,
            609, 762, 814, 12, 11, 227, 618, 81, 16, 235, 615, 654, 95, 1023,
            579, 606, 334, 807, 458, 828, 352, 206, 371, 111, 775, 464, 746, 165,
            586, 857, 812, 793, 94, 43, 889, 170, 71, 383, 1015, 477, 448, 953,
            308, 395, 593, 318, 432, 29, 239, 205, 123, 521, 522, 55, 154, 361,
            612, 959, 504, 880, 869, 625, 251, 667, 216, 797, 798, 476, 453, 825,
            624, 405, 851, 128, 194, 375, 133, 813, 722, 977, 399, 363, 145, 682,
            119, 473, 930, 562, 764, 967, 234, 678, 338, 605, 215, 868, 367, 786,
            90, 38, 162, 136, 558, 496, 248, 84, 463, 581, 651, 75, 290, 411,
            354, 417, 602, 737, 311, 195, 966, 391, 518, 767, 93, 57, 564, 416,
            356, 350, 220, 811, 948, 4, 916, 835, 849, 243, 177, 288, 474, 954,
            277, 268, 6, 35, 137, 1003, 125, 293, 779, 816, 565, 629, 337, 887,
            494, 182, 124, 788, 283, 621, 834, 444, 479, 539, 54, 931, 818, 327,
            21, 771, 336, 428, 58, 40, 475, 409, 776, 355, 932, 709, 845, 89,
            359, 893, 885, 507, 595, 1020, 120, 820, 657, 821, 870, 388, 683,
            908, 140, 324, 985, 901, 840, 696, 396, 961, 672, 965, 530, 951, 442,
            50, 937, 853, 1, 457, 426, 304, 871, 263, 343, 576, 731, 315, 1021,
            873, 368, 941, 511, 617, 791, 262, 78, 377, 664, 829, 830, 460, 649,
            751, 768, 468, 691, 92, 386, 992, 258, 317, 616, 537, 484, 877, 152,
            45, 270, 236, 275, 431, 47, 499, 859, 803, 726, 445, 525, 218, 725,
            599, 100, 141, 989, 106, 918, 715, 533, 400, 563, 710, 910, 443, 690,
            217, 341, 228, 712, 890, 626, 592, 495, 25, 1001, 446, 906, 166, 393,
            650, 244, 720, 349, 153, 552, 1002, 392, 513, 64, 862, 781, 684, 716,
            284, 281, 601, 385, 173, 635, 997, 900, 210, 634, 200, 437, 429, 570,
            414, 280, 316, 757, 264, 883, 1018, 707, 157, 717, 557, 515, 766,
            742, 603, 692, 1009, 677, 178, 266, 760, 864, 466, 109, 455, 652,
            898, 981, 736, 837, 936, 85, 572, 993, 127, 911, 333, 184, 675, 528,
            674, 307, 510, 362, 826, 824, 150, 151, 488, 598, 465, 289, 608, 643,
            312, 1005, 167, 232, 896, 199, 172, 330, 642, 1031, 514, 665, 87,
            246, 817, 238, 97, 378, 640, 568, 193, 204, 138, 744, 535, 287, 469,
            656, 291, 357, 915, 7, 756, 783, 66, 879, 960, 348, 255, 529, 31,
            221, 547, 189, 44, 384, 571, 962, 810, 459, 963, 83, 110, 14, 329,
            1006, 418, 790, 597, 619, 1007, 279, 800, 186, 104, 256, 5, 53, 269,
            56, 647, 872, 855, 774, 523, 897, 895, 440, 838, 831, 987, 508, 926,
            984, 27, 582, 276, 26, 765, 114, 633, 542, 519, 588, 861, 301, 858,
            390, 761, 847, 943, 978, 403, 2, 76, 135, 1013, 24, 82, 561, 693,
            921, 721, 425, 728, 653, 548, 912, 503, 105, 427, 321, 502, 758, 549,
            666, 196, 88, 52, 819, 41, 143, 292, 983, 934, 836, 480, 688, 223,
            265, 101, 389, 198, 213, 591, 844, 118, 947, 300, 611, 806, 638, 566,
            550, 708, 839, 380, 260, 909, 369, 146, 569, 532, 644, 161, 925, 340,
            107, 231, 754, 785, 956, 646, 792, 433, 103, 322, 610, 387, 18, 866,
            65, 10, 876, 802, 491, 1032, 296, 854, 434, 735, 843, 833, 531, 113,
            740, 749, 714, 658, 698, 147, 623, 59, 99, 168, 319, 1024, 174, 298,
            160, 573, 902, 988, 917, 554, 534, 320, 778, 946, 422, 130, 730, 48,
            1014, 732, 939, 622, 982, 734, 470, 998, 211, 607, 430, 711, 254,
            784, 449, 185, 285, 28, 505, 574, 197, 297, 567, 342, 22, 544, 187,
            132, 865, 486, 979, 920, 1026, 108, 809, 230, 436, 782, 439, 326,
            344, 192, 536, 1017, 306, 750, 102, 538, 875, 493, 703, 886, 180,
            928, 927, 670, 804, 729, 957, 904, 585, 745, 358, 272, 179, 527, 949,
            524, 273, 481, 958, 639, 164, 867, 881, 313, 181, 364, 63, 462, 1011,
            892, 191, 1012, 471, 950, 91, 328, 441, 67, 739, 247, 973, 596, 669,
            613, 741, 641, 73, 482, 995, 19, 970, 590, 555, 808, 346, 660, 148,
            294, 397, 155, 706, 668, 794, 752, 188, 974, 131, 1033, 229, 556,
            339, 631, 249, 62, 546, 219, 309, 34, 1025, 509, 208, 176, 743, 545,
            225, 676, 424, 121, 489, 347, 413, 332, 237, 302, 780, 795, 938, 472,
            850, 575, 553, 305, 214, 421, 907, 1027, 914, 929, 689, 630, 60, 351,
            1008, 945, 370, 222, 500, 955, 540, 20, 763, 190, 520, 212, 8, 1010,
            490, 587, 884, 325, 13, 252, 382, 874, 39, 968, 687, 163, 492, 856,
            373, 202, 637, 80, 952, 415, 680, 801, 169, 1029, 753, 583, 940, 46,
            922, 423, 70, 770, 335, 30, 999, 282, 541, 245, 1016, 142, 487, 257,
            419, 261, 404, 36, 68, 37, 944, 271, 274, 559, 759, 894, 467, 772,
            584, 96, 777, 485, 560, 512, 233, 406, 149, 718, 483, 799, 115, 686,
            705, 451, 842, 882, 156, 1000, 848, 846, 454, 207, 295, 51, 478, 32,
            663, 891, 628, 420, 72, 789, 701, 203, 727, 996, 241, 410, 971, 620,
            69, 452, 501, 661, 226, 827, 719, 201, 773, 159, 704, 942, 171, 738,
            398, 577, 42, 61, 723, 379, 700, 402, 253, 278, 832, 412, 578, 314,
            681, 969, 144, 1022, 787, 627, 733};
    int64_t sz = sizeof(sample) / sizeof(sample[0]); // 1033
    for(size_t i = 0; i < sz; i++){
        implementation->insert(sample[i], sample[i] * 10);
    }

    implementation->build();
    REQUIRE(implementation->size() == sz);

//    implementation->dump();

    for(size_t i = 0; i <= sz + 1; i++){
        for(size_t j = i; j <= sz + 2; j++){
            auto sum = implementation->sum(i, j);
//            cout << "RANGE [" << i << ", " << j << "] result: " << sum << endl;

            if(j <= 0 || i > sz){
                REQUIRE(sum.m_num_elements == 0);
                REQUIRE(sum.m_sum_keys == 0);
                REQUIRE(sum.m_sum_values == 0);
            } else {
                int64_t vmin = std::max<int64_t>(1, i);
                int64_t vmax = std::min<int64_t>(sz, j);

                REQUIRE(sum.m_first_key == vmin);
                REQUIRE(sum.m_last_key == vmax);
                REQUIRE(sum.m_num_elements == (vmax - vmin +1));
                auto expected_sum = /* sum of the first vmax numbers */ (vmax * (vmax +1) /2) - /* sum of the first vmin -1 numbers */ ((vmin -1) * vmin /2);
                REQUIRE(sum.m_sum_keys == expected_sum);
                REQUIRE(sum.m_sum_values == expected_sum * 10);
            }
        }
    }
}

