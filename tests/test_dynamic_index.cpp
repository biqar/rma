/*
 * test_dynamic_index.cpp
 *
 *  Created on: Oct 8, 2018
 *      Author: dleo@cwi.nl
 */

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#define CATCH_CONFIG_MAIN
#include "third-party/catch/catch.hpp"

#include "pma/generic/dynamic_index.hpp"

using namespace pma;
using namespace std;

TEST_CASE("only_root"){
    DynamicIndex<int64_t, int64_t, 4, 4> abtree;
    REQUIRE( abtree.size() == 0 );
    REQUIRE( abtree.empty() );

    abtree.insert(0, 10);
    abtree.insert(1, 20);
    abtree.insert(2, 30);
    REQUIRE( abtree.size() == 3 );
    REQUIRE( !abtree.empty() );

    int64_t value = -1;
    REQUIRE( abtree.find_any(-1, &value) == false );
    REQUIRE( abtree.find_any(0, &value) == true );
    REQUIRE( value == 10 );
    REQUIRE( abtree.find_any(1, &value) == true );
    REQUIRE( value == 20 );
    REQUIRE( abtree.find_any(2, &value) == true );
    REQUIRE( value == 30 );
    REQUIRE( abtree.find_any(3, &value) == false );

    REQUIRE( abtree.remove_any(-1, &value) == false );
    REQUIRE( abtree.remove_any(3, &value) == false );
    REQUIRE( abtree.remove_any(1, &value) == true );
    REQUIRE( value == 20 );
    REQUIRE( abtree.remove_any(1, &value) == false ); // already removed
    REQUIRE( abtree.size() == 2 );
    REQUIRE( abtree.remove_any(0, &value) == true );
    REQUIRE( value == 10 );
    REQUIRE( abtree.remove_any(2, &value) == true );
    REQUIRE( value == 30 );
    REQUIRE( abtree.remove_any(2, &value) == false ); // already removed

    REQUIRE( abtree.size() == 0 );
    REQUIRE( abtree.empty() );
}


TEST_CASE("height2"){
    DynamicIndex<int64_t, int64_t, 4, 4> abtree;
    for(int i = 10; i <= 70; i+=10){ abtree.insert(i, i * 10); } // 10, 20, 30, 40, 50, 60, 70

    REQUIRE( abtree.size() == 7 );

    // check
    for(int i = 10; i <= 7; i+= 10) {
        int64_t value = 0;
        REQUIRE( abtree.find_any(i, &value) == true );
        REQUIRE( value == (i * 10) );
    }
    for(int64_t key = 5; key <= 75; key+= 10){
        int64_t value = 0;
        REQUIRE( abtree.find_any(key, &value) == false );
    }

    // find_leq
    int64_t value;
    REQUIRE(abtree.find_leq(5, nullptr, &value) == false);
    for(int key = 10; key <= 75; key+=5){
        int64_t value = 0;
        bool result = abtree.find_leq(key, nullptr, &value);
        REQUIRE(result == true);
        REQUIRE(value == (key /10) * 100);
    }

    // remove
    for(int key = 70; key >= 10; key -= 10){
        int64_t value = 0;
        REQUIRE( abtree.remove_any(key, &value) == true );
        REQUIRE( value == key * 10 );
        REQUIRE( abtree.remove_any(key, &value) == false ); // already removed
    }

    REQUIRE( abtree.size() == 0 );
    REQUIRE( abtree.empty() );
}

TEST_CASE("full_tree_sequential"){
    constexpr size_t num_keys = 512;

    DynamicIndex<int64_t, int64_t, 4, 4> abtree{};
    for(int i = 0; i < num_keys; i++){
        abtree.insert(i, (i+1) * 10);
        for(int j = 0; j <= i; j++) {
            int64_t value = 0;
            REQUIRE(abtree.find_any(j, &value) == true);
            REQUIRE(value == (j+1) * 10);
        }
    } // 10, 20, 30, 40, 50, 60, 70, etc.


    for(int i = num_keys -1; i >= 0; i--){
        int64_t value = 0;
        abtree.remove_any(i, &value);

        for(int j = 0; j < i; j++){
            int64_t value = 0;
            REQUIRE(abtree.find_any(j, &value) == true);
            REQUIRE(value == (j+1) * 10);
        }
        for(int j = i; j < num_keys; j++){
            int64_t value = 0;
            REQUIRE(abtree.find_any(j, &value) == false);
        }
    }

    REQUIRE(abtree.empty());
}

TEST_CASE("uniform"){
    DynamicIndex<int64_t, int64_t, 64, 64> abtree{};

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
        abtree.insert(sample[i], sample[i] * 10);
    }

    REQUIRE(abtree.size() == sz);

    int64_t value = -1;
    REQUIRE(abtree.find_any(0, &value) == false);
    REQUIRE(abtree.find_any(sz +1, &value) == false);

    for(size_t i = 1; i <= sz; i++){
        value = -1;
        REQUIRE(abtree.find_any(i,&value) == true);
        REQUIRE(value == i *10);
    }

    for(size_t i = 0; i < sz; i++){
        value = -1;
        bool removed = abtree.remove_any(sample[i], &value);
        REQUIRE( removed == true );
        REQUIRE( value == sample[i] * 10 );
        REQUIRE( abtree.find_any(sample[i],&value) == false );
    }

    REQUIRE(abtree.size() == 0);
    REQUIRE(abtree.empty());
}

TEST_CASE("duplicates"){
    DynamicIndex<int64_t, int64_t, 64, 64> abtree{};

    auto do_insert = [&abtree](int i, int duplicates){
        int key = i * 100;
        for(int i = 1; i <= duplicates; i++){
            abtree.insert(key, key * 10 + i);
        }
    };

    do_insert(3, 18);
    do_insert(1, 5);
    do_insert(4, 83);
    do_insert(5, 7);
    do_insert(2, 42);

    int expected_size = 18 + 5 + 7 + 42 + 83;

    REQUIRE(abtree.size() == expected_size);

    abtree.remove(200);
    REQUIRE(abtree.size() == expected_size - 42);
    abtree.remove(300);
    REQUIRE(abtree.size() == expected_size - 42 - 18);
    abtree.remove(500);
    REQUIRE(abtree.size() == expected_size - 42 - 18 - 7);
    abtree.remove(100);
    REQUIRE(abtree.size() == 83);
    abtree.remove(400);
    REQUIRE(abtree.size() == 0);

}

TEST_CASE("type_mismatch"){
    using payload = std::vector<int64_t>*;
    // check it compiles
    DynamicIndex<double, payload, 64, 64> abtree{};
    abtree.insert(0.1, (payload) 0x1);
    payload value;
    abtree.find_any(0.1, &value);
    abtree.remove_any(0.1, &value);
    abtree.remove(0.1);}
