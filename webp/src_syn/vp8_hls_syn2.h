/**********
Copyright (c) 2017, Xilinx, Inc.
All rights reserved.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********/

#ifndef _VP8_HLS_SYN2_H_
#define _VP8_HLS_SYN2_H_

#include "vp8_hls_syn.h"
#include <hls_stream.h>
#include <ap_int.h>
#include <stdio.h>
#include <string.h>

enum { B_DC_PRED = 0,   // 4x4 modes
       B_TM_PRED = 1,
       B_VE_PRED = 2,
       B_HE_PRED = 3,
       B_RD_PRED = 4,
       B_VR_PRED = 5,
       B_LD_PRED = 6,
       B_VL_PRED = 7,
       B_HD_PRED = 8,
       B_HU_PRED = 9,
       NUM_BMODES = B_HU_PRED + 1 - B_DC_PRED,  // = 10

       // Luma16 or UV modes
       DC_PRED = B_DC_PRED, V_PRED = B_VE_PRED,
       H_PRED = B_HE_PRED, TM_PRED = B_TM_PRED,
       B_PRED = NUM_BMODES,   // refined I4x4 mode
       NUM_PRED_MODES = 4,

       // special modes
       B_DC_PRED_NOTOP = 4,
       B_DC_PRED_NOLEFT = 5,
       B_DC_PRED_NOTOPLEFT = 6,
       NUM_B_DC_MODES = 7 };

const ap_uint<12> my_VP8FixedCostsI4[NUM_BMODES][NUM_BMODES][NUM_BMODES] = {
  { {   40, 1151, 1723, 1874, 2103, 2019, 1628, 1777, 2226, 2137 },
    {  192,  469, 1296, 1308, 1849, 1794, 1781, 1703, 1713, 1522 },
    {  142,  910,  762, 1684, 1849, 1576, 1460, 1305, 1801, 1657 },
    {  559,  641, 1370,  421, 1182, 1569, 1612, 1725,  863, 1007 },
    {  299, 1059, 1256, 1108,  636, 1068, 1581, 1883,  869, 1142 },
    {  277, 1111,  707, 1362, 1089,  672, 1603, 1541, 1545, 1291 },
    {  214,  781, 1609, 1303, 1632, 2229,  726, 1560, 1713,  918 },
    {  152, 1037, 1046, 1759, 1983, 2174, 1358,  742, 1740, 1390 },
    {  512, 1046, 1420,  753,  752, 1297, 1486, 1613,  460, 1207 },
    {  424,  827, 1362,  719, 1462, 1202, 1199, 1476, 1199,  538 } },
  { {  240,  402, 1134, 1491, 1659, 1505, 1517, 1555, 1979, 2099 },
    {  467,  242,  960, 1232, 1714, 1620, 1834, 1570, 1676, 1391 },
    {  500,  455,  463, 1507, 1699, 1282, 1564,  982, 2114, 2114 },
    {  672,  643, 1372,  331, 1589, 1667, 1453, 1938,  996,  876 },
    {  458,  783, 1037,  911,  738,  968, 1165, 1518,  859, 1033 },
    {  504,  815,  504, 1139, 1219,  719, 1506, 1085, 1268, 1268 },
    {  333,  630, 1445, 1239, 1883, 3672,  799, 1548, 1865,  598 },
    {  399,  644,  746, 1342, 1856, 1350, 1493,  613, 1855, 1015 },
    {  622,  749, 1205,  608, 1066, 1408, 1290, 1406,  546,  971 },
    {  500,  753, 1041,  668, 1230, 1617, 1297, 1425, 1383,  523 } },
  { {  394,  553,  523, 1502, 1536,  981, 1608, 1142, 1666, 2181 },
    {  655,  430,  375, 1411, 1861, 1220, 1677, 1135, 1978, 1553 },
    {  690,  640,  245, 1954, 2070, 1194, 1528,  982, 1972, 2232 },
    {  559,  834,  741,  867, 1131,  980, 1225,  852, 1092,  784 },
    {  690,  875,  516,  959,  673,  894, 1056, 1190, 1528, 1126 },
    {  740,  951,  384, 1277, 1177,  492, 1579, 1155, 1846, 1513 },
    {  323,  775, 1062, 1776, 3062, 1274,  813, 1188, 1372,  655 },
    {  488,  971,  484, 1767, 1515, 1775, 1115,  503, 1539, 1461 },
    {  740, 1006,  998,  709,  851, 1230, 1337,  788,  741,  721 },
    {  522, 1073,  573, 1045, 1346,  887, 1046, 1146, 1203,  697 } },
  { {  105,  864, 1442, 1009, 1934, 1840, 1519, 1920, 1673, 1579 },
    {  534,  305, 1193,  683, 1388, 2164, 1802, 1894, 1264, 1170 },
    {  305,  518,  877, 1108, 1426, 3215, 1425, 1064, 1320, 1242 },
    {  683,  732, 1927,  257, 1493, 2048, 1858, 1552, 1055,  947 },
    {  394,  814, 1024,  660,  959, 1556, 1282, 1289,  893, 1047 },
    {  528,  615,  996,  940, 1201,  635, 1094, 2515,  803, 1358 },
    {  347,  614, 1609, 1187, 3133, 1345, 1007, 1339, 1017,  667 },
    {  218,  740,  878, 1605, 3650, 3650, 1345,  758, 1357, 1617 },
    {  672,  750, 1541,  558, 1257, 1599, 1870, 2135,  402, 1087 },
    {  592,  684, 1161,  430, 1092, 1497, 1475, 1489, 1095,  822 } },
  { {  228, 1056, 1059, 1368,  752,  982, 1512, 1518,  987, 1782 },
    {  494,  514,  818,  942,  965,  892, 1610, 1356, 1048, 1363 },
    {  512,  648,  591, 1042,  761,  991, 1196, 1454, 1309, 1463 },
    {  683,  749, 1043,  676,  841, 1396, 1133, 1138,  654,  939 },
    {  622, 1101, 1126,  994,  361, 1077, 1203, 1318,  877, 1219 },
    {  631, 1068,  857, 1650,  651,  477, 1650, 1419,  828, 1170 },
    {  555,  727, 1068, 1335, 3127, 1339,  820, 1331, 1077,  429 },
    {  504,  879,  624, 1398,  889,  889, 1392,  808,  891, 1406 },
    {  683, 1602, 1289,  977,  578,  983, 1280, 1708,  406, 1122 },
    {  399,  865, 1433, 1070, 1072,  764,  968, 1477, 1223,  678 } },
  { {  333,  760,  935, 1638, 1010,  529, 1646, 1410, 1472, 2219 },
    {  512,  494,  750, 1160, 1215,  610, 1870, 1868, 1628, 1169 },
    {  572,  646,  492, 1934, 1208,  603, 1580, 1099, 1398, 1995 },
    {  786,  789,  942,  581, 1018,  951, 1599, 1207,  731,  768 },
    {  690, 1015,  672, 1078,  582,  504, 1693, 1438, 1108, 2897 },
    {  768, 1267,  571, 2005, 1243,  244, 2881, 1380, 1786, 1453 },
    {  452,  899, 1293,  903, 1311, 3100,  465, 1311, 1319,  813 },
    {  394,  927,  942, 1103, 1358, 1104,  946,  593, 1363, 1109 },
    {  559, 1005, 1007, 1016,  658, 1173, 1021, 1164,  623, 1028 },
    {  564,  796,  632, 1005, 1014,  863, 2316, 1268,  938,  764 } },
  { {  266,  606, 1098, 1228, 1497, 1243,  948, 1030, 1734, 1461 },
    {  366,  585,  901, 1060, 1407, 1247,  876, 1134, 1620, 1054 },
    {  452,  565,  542, 1729, 1479, 1479, 1016,  886, 2938, 1150 },
    {  555, 1088, 1533,  950, 1354,  895,  834, 1019, 1021,  496 },
    {  704,  815, 1193,  971,  973,  640, 1217, 2214,  832,  578 },
    {  672, 1245,  579,  871,  875,  774,  872, 1273, 1027,  949 },
    {  296, 1134, 2050, 1784, 1636, 3425,  442, 1550, 2076,  722 },
    {  342,  982, 1259, 1846, 1848, 1848,  622,  568, 1847, 1052 },
    {  555, 1064, 1304,  828,  746, 1343, 1075, 1329, 1078,  494 },
    {  288, 1167, 1285, 1174, 1639, 1639,  833, 2254, 1304,  509 } },
  { {  342,  719,  767, 1866, 1757, 1270, 1246,  550, 1746, 2151 },
    {  483,  653,  694, 1509, 1459, 1410, 1218,  507, 1914, 1266 },
    {  488,  757,  447, 2979, 1813, 1268, 1654,  539, 1849, 2109 },
    {  522, 1097, 1085,  851, 1365, 1111,  851,  901,  961,  605 },
    {  709,  716,  841,  728,  736,  945,  941,  862, 2845, 1057 },
    {  512, 1323,  500, 1336, 1083,  681, 1342,  717, 1604, 1350 },
    {  452, 1155, 1372, 1900, 1501, 3290,  311,  944, 1919,  922 },
    {  403, 1520,  977, 2132, 1733, 3522, 1076,  276, 3335, 1547 },
    {  559, 1374, 1101,  615,  673, 2462,  974,  795,  984,  984 },
    {  547, 1122, 1062,  812, 1410,  951, 1140,  622, 1268,  651 } },
  { {  165,  982, 1235,  938, 1334, 1366, 1659, 1578,  964, 1612 },
    {  592,  422,  925,  847, 1139, 1112, 1387, 2036,  861, 1041 },
    {  403,  837,  732,  770,  941, 1658, 1250,  809, 1407, 1407 },
    {  896,  874, 1071,  381, 1568, 1722, 1437, 2192,  480, 1035 },
    {  640, 1098, 1012, 1032,  684, 1382, 1581, 2106,  416,  865 },
    {  559, 1005,  819,  914,  710,  770, 1418,  920,  838, 1435 },
    {  415, 1258, 1245,  870, 1278, 3067,  770, 1021, 1287,  522 },
    {  406,  990,  601, 1009, 1265, 1265, 1267,  759, 1017, 1277 },
    {  968, 1182, 1329,  788, 1032, 1292, 1705, 1714,  203, 1403 },
    {  732,  877, 1279,  471,  901, 1161, 1545, 1294,  755,  755 } },
  { {  111,  931, 1378, 1185, 1933, 1648, 1148, 1714, 1873, 1307 },
    {  406,  414, 1030, 1023, 1910, 1404, 1313, 1647, 1509,  793 },
    {  342,  640,  575, 1088, 1241, 1349, 1161, 1350, 1756, 1502 },
    {  559,  766, 1185,  357, 1682, 1428, 1329, 1897, 1219,  802 },
    {  473,  909, 1164,  771,  719, 2508, 1427, 1432,  722,  782 },
    {  342,  892,  785, 1145, 1150,  794, 1296, 1550,  973, 1057 },
    {  208, 1036, 1326, 1343, 1606, 3395,  815, 1455, 1618,  712 },
    {  228,  928,  890, 1046, 3499, 1711,  994,  829, 1720, 1318 },
    {  768,  724, 1058,  636,  991, 1075, 1319, 1324,  616,  825 },
    {  305, 1167, 1358,  899, 1587, 1587,  987, 1988, 1332,  501 } }
};

/////hls_VP8CoeffsUpdateProba//////////////////
static const uint8_t hls_VP8CoeffsUpdateProba[4][8][3][11] = {//4, 8, 4(3), 8(11)}=1024 *
  { { { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 176, 246, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 223, 241, 252, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 249, 253, 253, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 244, 252, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 234, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 246, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 239, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 254, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 248, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 251, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 251, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 254, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 254, 253, 255, 254, 255, 255, 255, 255, 255, 255 },
      { 250, 255, 254, 255, 254, 255, 255, 255, 255, 255, 255 },
      { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    }
  },/////////////////////////////////////////////////////////////////////
  { { { 217, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 225, 252, 241, 253, 255, 255, 254, 255, 255, 255, 255 },
      { 234, 250, 241, 250, 253, 255, 253, 254, 255, 255, 255 }
    },
    { { 255, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 223, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 238, 253, 254, 254, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 248, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 249, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 253, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 247, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 252, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 254, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 250, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    }
  },/////////////////////////////////////////////////////////////
  { { { 186, 251, 250, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 234, 251, 244, 254, 255, 255, 255, 255, 255, 255, 255 },
      { 251, 251, 243, 253, 254, 255, 254, 255, 255, 255, 255 }
    },
    { { 255, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 236, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 251, 253, 253, 254, 254, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    }
  },/////////////////////////////////////////////////////////////
  { { { 248, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 250, 254, 252, 254, 255, 255, 255, 255, 255, 255, 255 },
      { 248, 254, 249, 253, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 253, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 246, 253, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 252, 254, 251, 254, 254, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 254, 252, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 248, 254, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 253, 255, 254, 254, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 251, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 245, 251, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 253, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 251, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 252, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 252, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 249, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 255, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 250, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    },
    { { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
    }
  }
};

///////////hls_VP8EntropyCost//////////////
static const ap_uint<11> hls_VP8EntropyCost[256] = {
1792, 1792, 1792, 1536, 1536, 1408, 1366, 1280, 1280, 1216,
1178, 1152, 1110, 1076, 1061, 1024, 1024,  992,  968,  951,
 939,  911,  896,  878,  871,  854,  838,  820,  811,  794,
 786,  768,  768,  752,  740,  732,  720,  709,  704,  690,
 683,  672,  666,  655,  647,  640,  631,  622,  615,  607,
 598,  592,  586,  576,  572,  564,  559,  555,  547,  541,
 534,  528,  522,  512,  512,  504,  500,  494,  488,  483,
 477,  473,  467,  461,  458,  452,  448,  443,  438,  434,
 427,  424,  419,  415,  410,  406,  403,  399,  394,  390,
 384,  384,  377,  374,  370,  366,  362,  359,  355,  351,
 347,  342,  342,  336,  333,  330,  326,  323,  320,  316,
 312,  308,  305,  302,  299,  296,  293,  288,  287,  283,
 280,  277,  274,  272,  268,  266,  262,  256,  256,  256,
 251,  248,  245,  242,  240,  237,  234,  232,  228,  226,
 223,  221,  218,  216,  214,  211,  208,  205,  203,  201,
 198,  196,  192,  191,  188,  187,  183,  181,  179,  176,
 175,  171,  171,  168,  165,  163,  160,  159,  156,  154,
 152,  150,  148,  146,  144,  142,  139,  138,  135,  133,
 131,  128,  128,  125,  123,  121,  119,  117,  115,  113,
 111,  110,  107,  105,  103,  102,  100,   98,   96,   94,
  92,   91,   89,   86,   86,   83,   82,   80,   77,   76,
  74,   73,   71,   69,   67,   66,   64,   63,   61,   59,
  57,   55,   54,   52,   51,   49,   47,   46,   44,   43,
  41,   40,   38,   36,   35,   33,   32,   30,   29,   27,
  25,   24,   22,   21,   19,   18,   16,   15,   13,   12,
  10,    9,    7,    6,    4,    3
};

static const ap_uint<9> VP8LevelCodes_hls[67][2] = { { 0x001, 0x000 },
		{ 0x007, 0x001 }, { 0x00f, 0x005 }, { 0x00f, 0x00d }, { 0x033, 0x003 },
		{ 0x033, 0x003 }, { 0x033, 0x023 }, { 0x033, 0x023 }, { 0x033, 0x023 },
		{ 0x033, 0x023 }, { 0x0d3, 0x013 }, { 0x0d3, 0x013 }, { 0x0d3, 0x013 },
		{ 0x0d3, 0x013 }, { 0x0d3, 0x013 }, { 0x0d3, 0x013 }, { 0x0d3, 0x013 },
		{ 0x0d3, 0x013 }, { 0x0d3, 0x093 }, { 0x0d3, 0x093 }, { 0x0d3, 0x093 },
		{ 0x0d3, 0x093 }, { 0x0d3, 0x093 }, { 0x0d3, 0x093 }, { 0x0d3, 0x093 },
		{ 0x0d3, 0x093 }, { 0x0d3, 0x093 }, { 0x0d3, 0x093 }, { 0x0d3, 0x093 },
		{ 0x0d3, 0x093 }, { 0x0d3, 0x093 }, { 0x0d3, 0x093 }, { 0x0d3, 0x093 },
		{ 0x0d3, 0x093 }, { 0x153, 0x053 }, { 0x153, 0x053 }, { 0x153, 0x053 },
		{ 0x153, 0x053 }, { 0x153, 0x053 }, { 0x153, 0x053 }, { 0x153, 0x053 },
		{ 0x153, 0x053 }, { 0x153, 0x053 }, { 0x153, 0x053 }, { 0x153, 0x053 },
		{ 0x153, 0x053 }, { 0x153, 0x053 }, { 0x153, 0x053 }, { 0x153, 0x053 },
		{ 0x153, 0x053 }, { 0x153, 0x053 }, { 0x153, 0x053 }, { 0x153, 0x053 },
		{ 0x153, 0x053 }, { 0x153, 0x053 }, { 0x153, 0x053 }, { 0x153, 0x053 },
		{ 0x153, 0x053 }, { 0x153, 0x053 }, { 0x153, 0x053 }, { 0x153, 0x053 },
		{ 0x153, 0x053 }, { 0x153, 0x053 }, { 0x153, 0x053 }, { 0x153, 0x053 },
		{ 0x153, 0x053 }, { 0x153, 0x153 } };
////hls_VP8CoeffsProba0//////////////////////////
const uint8_t hls_VP8CoeffsProba0[4][8][3][11] = {
  { { { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
      { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
      { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 }
    },
    { { 253, 136, 254, 255, 228, 219, 128, 128, 128, 128, 128 },
      { 189, 129, 242, 255, 227, 213, 255, 219, 128, 128, 128 },
      { 106, 126, 227, 252, 214, 209, 255, 255, 128, 128, 128 }
    },
    { { 1, 98, 248, 255, 236, 226, 255, 255, 128, 128, 128 },
      { 181, 133, 238, 254, 221, 234, 255, 154, 128, 128, 128 },
      { 78, 134, 202, 247, 198, 180, 255, 219, 128, 128, 128 },
    },
    { { 1, 185, 249, 255, 243, 255, 128, 128, 128, 128, 128 },
      { 184, 150, 247, 255, 236, 224, 128, 128, 128, 128, 128 },
      { 77, 110, 216, 255, 236, 230, 128, 128, 128, 128, 128 },
    },
    { { 1, 101, 251, 255, 241, 255, 128, 128, 128, 128, 128 },
      { 170, 139, 241, 252, 236, 209, 255, 255, 128, 128, 128 },
      { 37, 116, 196, 243, 228, 255, 255, 255, 128, 128, 128 }
    },
    { { 1, 204, 254, 255, 245, 255, 128, 128, 128, 128, 128 },
      { 207, 160, 250, 255, 238, 128, 128, 128, 128, 128, 128 },
      { 102, 103, 231, 255, 211, 171, 128, 128, 128, 128, 128 }
    },
    { { 1, 152, 252, 255, 240, 255, 128, 128, 128, 128, 128 },
      { 177, 135, 243, 255, 234, 225, 128, 128, 128, 128, 128 },
      { 80, 129, 211, 255, 194, 224, 128, 128, 128, 128, 128 }
    },
    { { 1, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
      { 246, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
      { 255, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 }
    }
  },
  { { { 198, 35, 237, 223, 193, 187, 162, 160, 145, 155, 62 },
      { 131, 45, 198, 221, 172, 176, 220, 157, 252, 221, 1 },
      { 68, 47, 146, 208, 149, 167, 221, 162, 255, 223, 128 }
    },
    { { 1, 149, 241, 255, 221, 224, 255, 255, 128, 128, 128 },
      { 184, 141, 234, 253, 222, 220, 255, 199, 128, 128, 128 },
      { 81, 99, 181, 242, 176, 190, 249, 202, 255, 255, 128 }
    },
    { { 1, 129, 232, 253, 214, 197, 242, 196, 255, 255, 128 },
      { 99, 121, 210, 250, 201, 198, 255, 202, 128, 128, 128 },
      { 23, 91, 163, 242, 170, 187, 247, 210, 255, 255, 128 }
    },
    { { 1, 200, 246, 255, 234, 255, 128, 128, 128, 128, 128 },
      { 109, 178, 241, 255, 231, 245, 255, 255, 128, 128, 128 },
      { 44, 130, 201, 253, 205, 192, 255, 255, 128, 128, 128 }
    },
    { { 1, 132, 239, 251, 219, 209, 255, 165, 128, 128, 128 },
      { 94, 136, 225, 251, 218, 190, 255, 255, 128, 128, 128 },
      { 22, 100, 174, 245, 186, 161, 255, 199, 128, 128, 128 }
    },
    { { 1, 182, 249, 255, 232, 235, 128, 128, 128, 128, 128 },
      { 124, 143, 241, 255, 227, 234, 128, 128, 128, 128, 128 },
      { 35, 77, 181, 251, 193, 211, 255, 205, 128, 128, 128 }
    },
    { { 1, 157, 247, 255, 236, 231, 255, 255, 128, 128, 128 },
      { 121, 141, 235, 255, 225, 227, 255, 255, 128, 128, 128 },
      { 45, 99, 188, 251, 195, 217, 255, 224, 128, 128, 128 }
    },
    { { 1, 1, 251, 255, 213, 255, 128, 128, 128, 128, 128 },
      { 203, 1, 248, 255, 255, 128, 128, 128, 128, 128, 128 },
      { 137, 1, 177, 255, 224, 255, 128, 128, 128, 128, 128 }
    }
  },
  { { { 253, 9, 248, 251, 207, 208, 255, 192, 128, 128, 128 },
      { 175, 13, 224, 243, 193, 185, 249, 198, 255, 255, 128 },
      { 73, 17, 171, 221, 161, 179, 236, 167, 255, 234, 128 }
    },
    { { 1, 95, 247, 253, 212, 183, 255, 255, 128, 128, 128 },
      { 239, 90, 244, 250, 211, 209, 255, 255, 128, 128, 128 },
      { 155, 77, 195, 248, 188, 195, 255, 255, 128, 128, 128 }
    },
    { { 1, 24, 239, 251, 218, 219, 255, 205, 128, 128, 128 },
      { 201, 51, 219, 255, 196, 186, 128, 128, 128, 128, 128 },
      { 69, 46, 190, 239, 201, 218, 255, 228, 128, 128, 128 }
    },
    { { 1, 191, 251, 255, 255, 128, 128, 128, 128, 128, 128 },
      { 223, 165, 249, 255, 213, 255, 128, 128, 128, 128, 128 },
      { 141, 124, 248, 255, 255, 128, 128, 128, 128, 128, 128 }
    },
    { { 1, 16, 248, 255, 255, 128, 128, 128, 128, 128, 128 },
      { 190, 36, 230, 255, 236, 255, 128, 128, 128, 128, 128 },
      { 149, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128 }
    },
    { { 1, 226, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
      { 247, 192, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
      { 240, 128, 255, 128, 128, 128, 128, 128, 128, 128, 128 }
    },
    { { 1, 134, 252, 255, 255, 128, 128, 128, 128, 128, 128 },
      { 213, 62, 250, 255, 255, 128, 128, 128, 128, 128, 128 },
      { 55, 93, 255, 128, 128, 128, 128, 128, 128, 128, 128 }
    },
    { { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
      { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
      { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 }
    }
  },
  { { { 202, 24, 213, 235, 186, 191, 220, 160, 240, 175, 255 },
      { 126, 38, 182, 232, 169, 184, 228, 174, 255, 187, 128 },
      { 61, 46, 138, 219, 151, 178, 240, 170, 255, 216, 128 }
    },
    { { 1, 112, 230, 250, 199, 191, 247, 159, 255, 255, 128 },
      { 166, 109, 228, 252, 211, 215, 255, 174, 128, 128, 128 },
      { 39, 77, 162, 232, 172, 180, 245, 178, 255, 255, 128 }
    },
    { { 1, 52, 220, 246, 198, 199, 249, 220, 255, 255, 128 },
      { 124, 74, 191, 243, 183, 193, 250, 221, 255, 255, 128 },
      { 24, 71, 130, 219, 154, 170, 243, 182, 255, 255, 128 }
    },
    { { 1, 182, 225, 249, 219, 240, 255, 224, 128, 128, 128 },
      { 149, 150, 226, 252, 216, 205, 255, 171, 128, 128, 128 },
      { 28, 108, 170, 242, 183, 194, 254, 223, 255, 255, 128 }
    },
    { { 1, 81, 230, 252, 204, 203, 255, 192, 128, 128, 128 },
      { 123, 102, 209, 247, 188, 196, 255, 233, 128, 128, 128 },
      { 20, 95, 153, 243, 164, 173, 255, 203, 128, 128, 128 }
    },
    { { 1, 222, 248, 255, 216, 213, 128, 128, 128, 128, 128 },
      { 168, 175, 246, 252, 235, 205, 255, 255, 128, 128, 128 },
      { 47, 116, 215, 255, 211, 212, 255, 255, 128, 128, 128 }
    },
    { { 1, 121, 236, 253, 212, 214, 255, 255, 128, 128, 128 },
      { 141, 84, 213, 252, 201, 202, 255, 219, 128, 128, 128 },
      { 42, 80, 160, 240, 162, 185, 255, 205, 128, 128, 128 }
    },
    { { 1, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
      { 244, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
      { 238, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128 }
    }
  }
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//==========================      kernel_IntraPredLoop2_NoOut             ==============================//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//kernel_IntraPredLoop2_NoOut
//|-memcpy
//|-TopVp8_top_dataflow_32bit_k1NoStruct_cnt_DeepFIFO
//  |-TopVp8_read_2_32bit_NoStruct
//  | |-TopVp8_read__dataflow_32bit...
//  |-TopVp8_compute...
//  |-TopVp8_RecordCoeff_hls_cnt
//  | |-FindLast
//  | |-VP8RecordCoeffs_hls_str_w_cnt
//  |   |-Record_str
//  |   |-VP8EncBands_hls
//  |-TopVp8_RecordProb_hls_cnt
//  | |-RecordPorb_ReadCoeff_dataflow2_cnt
//  |   |-RecordPorb_ReadCoeff_dataflow_dc_cnt
//  |     |-RecordPorb_ReadCoeff_dataflow_ac_cnt
//  |     | |-VP8RecordCoeffs_hls_str_r_cnt
//  |     |-RecordPorb_ReadCoeff_dataflow_uv_cnt...
//  |     |-RecordPorb_ReadCoeff_dataflow2_cnt...
//  |-TopVp8_send_32bit
void TopVp8_top_dataflow_32bit_k1NoStruct_cnt_DeepFIFO(
		ap_uint<32> id_pic,
		ap_uint<32> mb_line,
		ap_uint<LG2_MAX_W_PIX> y_stride,
		ap_uint<LG2_MAX_W_PIX> uv_stride,
		ap_uint<LG2_MAX_W_PIX> width,
		ap_uint<LG2_MAX_W_PIX> height,
		ap_uint<LG2_MAX_NUM_MB_W> mb_w,
		ap_uint<LG2_MAX_NUM_MB_H> mb_h,
		ap_uint<WD_LMD> lambda_p16,
		ap_uint<WD_LMD> lambda_p44,
		ap_uint<WD_LMD> tlambda,
		ap_uint<WD_LMD> lambda_uv,
		ap_uint<WD_LMD> tlambda_m,
		hls_QMatrix hls_qm1,
		hls_QMatrix hls_qm2,
		hls_QMatrix hls_qm_uv,
		ap_int<WD_sharpen * 16> ap_sharpen,
		ap_int<WD_sharpen * 16> ap_sharpen_uv,
		uint32_t* ysrc,
		uint32_t* usrc,
		uint32_t* vsrc,
		int32_t* pout_level,
		uint8_t* pout_prob,
		int* dirty);
void TopVp8_top_dataflow_32bit_k1NoStruct_cnt_DeepFIFO_HideDirty(
		ap_uint<32> id_pic,
		ap_uint<32> mb_line,
		ap_uint<LG2_MAX_W_PIX> y_stride,
		ap_uint<LG2_MAX_W_PIX> uv_stride,
		ap_uint<LG2_MAX_W_PIX> width,
		ap_uint<LG2_MAX_W_PIX> height,
		ap_uint<LG2_MAX_NUM_MB_W> mb_w,
		ap_uint<LG2_MAX_NUM_MB_H> mb_h,
		ap_uint<WD_LMD> lambda_p16,
		ap_uint<WD_LMD> lambda_p44,
		ap_uint<WD_LMD> tlambda,
		ap_uint<WD_LMD> lambda_uv,
		ap_uint<WD_LMD> tlambda_m,
		hls_QMatrix hls_qm1,
		hls_QMatrix hls_qm2,
		hls_QMatrix hls_qm_uv,
		ap_int<WD_sharpen * 16> ap_sharpen,
		ap_int<WD_sharpen * 16> ap_sharpen_uv,
		uint32_t* ysrc,
		uint32_t* usrc,
		uint32_t* vsrc,
		int32_t* pout_level,
		uint8_t* pout_prob);

//////////======================================================================/////////////////////////////
//////////====================  TopVp8_read_2_32bit_NoStruct  =================/////////////////////////////
//////////======================================================================/////////////////////////////
void TopVp8_read_2_32bit_NoStruct(
		//input
		uint32_t* ysrc,
		uint32_t* usrc,
		uint32_t* vsrc,
		ap_uint<LG2_MAX_W_PIX> y_stride,
		ap_uint<LG2_MAX_W_PIX> uv_stride,
		ap_uint<LG2_MAX_W_PIX> width,
		ap_uint<LG2_MAX_W_PIX> height,
		ap_uint<LG2_MAX_NUM_MB_W> mb_w,
		ap_uint<LG2_MAX_NUM_MB_H> mb_h,
		//output
		hls::stream<ap_uint<WD_PIX * 16> >* str_din_y,
		hls::stream<ap_uint<WD_PIX * 16> >* str_din_uv);

void TopVp8_read__dataflow_32bit(
		//input
		ap_uint<LG2_MAX_W_PIX> y_stride,//
	    ap_uint<LG2_MAX_W_PIX> uv_stride,//
	    ap_uint<LG2_MAX_W_PIX> width,//
	    ap_uint<LG2_MAX_W_PIX> height,//
	    ap_uint<LG2_MAX_NUM_MB_W> mb_w,//
	    ap_uint<LG2_MAX_NUM_MB_H> mb_h,//
		uint32_t ysrc[MAX_W_PIX*MAX_H_PIX/4],
		uint32_t usrc[MAX_W_PIX*MAX_H_PIX/4/4],
		uint32_t vsrc[MAX_W_PIX*MAX_H_PIX/4/4],
		//output
		hls::stream< ap_uint<WD_PIX*16> >* str_din_y,
		hls::stream< ap_uint<WD_PIX*16> >* str_din_uv);

void hls_ReadMBLine_32bit_const(
    uint32_t ysrc[MAX_W_PIX*MAX_H_PIX/4],
    uint32_t usrc[MAX_W_PIX*MAX_H_PIX/4/4],
    uint32_t vsrc[MAX_W_PIX*MAX_H_PIX/4/4],
	int     y_mb,
    int     y_stride,
	int     uv_stride,
	//output
    uint32_t  buff_line_mb_y[MAX_W_PIX*16/4],//32bb
    uint32_t  buff_line_mb_u[MAX_W_PIX*4/4],//32bb
    uint32_t  buff_line_mb_v[MAX_W_PIX*4/4] //32bb
);

void hls_CopyMBLine_y_32bit_const(
	uint32_t ydes[MAX_W_PIX*16/4],
	uint32_t ysrc[MAX_W_PIX*MAX_H_PIX/4],
    int num_read
);

void hls_CopyMBLine_uv_32bit_const(
    uint32_t uvdes[MAX_W_PIX/2*8/4],
	uint32_t uvsrc[MAX_W_PIX*MAX_H_PIX/4/4],
    int num_read
	);

void TopVp8_read_MB_32bit_const(
	    ap_uint<LG2_MAX_W_PIX> width,//      = p_info[4];  // = pic->width
	    ap_uint<LG2_MAX_W_PIX> height,//      = p_info[5];  // = pic->height
	    ap_uint<LG2_MAX_NUM_MB_W> mb_w,// = p_info[2+2+2];///;
	    ap_uint<LG2_MAX_NUM_MB_H> mb_h,// = p_info[3+2+2];//;
		int y_mb,
	    uint32_t  buff_line_mb_y[MAX_W_PIX*16/4],
	    uint32_t  buff_line_mb_u[MAX_W_PIX*4/4],
	    uint32_t  buff_line_mb_v[MAX_W_PIX*4/4],
	    //uint32_t  buff_line_mb_y2[MAX_W_PIX*16/4],
	    //uint32_t  buff_line_mb_u2[MAX_W_PIX*4/4],
	    //uint32_t  buff_line_mb_v2[MAX_W_PIX*4/4],
		int stride_y,
		int stride_uv,
		//output
		hls::stream< ap_uint<WD_PIX*16> >* str_din_y,
		hls::stream< ap_uint<WD_PIX*16> >* str_din_uv
	);

void hls_GetMB_parallel_32bit_const(
		uint32_t ysrc_MBline[MAX_W_PIX*16/4],
		uint32_t usrc_MBline[MAX_W_PIX*4/4],
		uint32_t vsrc_MBline[MAX_W_PIX*4/4],
		int x_mb,
		int y_mb,
		int width,
		int height,
		int stride_y,
		int stride_uv,
		ap_uint<WD_PIX*16>  ap_y_in_[16],
		ap_uint<WD_PIX*16>  ap_u_in_[4],
		ap_uint<WD_PIX*16>  ap_v_in_[4]
		);

void hls_GetMB_y_32bit_const(
		uint32_t src[MAX_W_PIX*16/4],
		int x_mb,
		int y_mb,
		int width,
		int height,
		int stride,
		ap_uint<WD_PIX*16>  ap_y_in_[16]
		);

ap_uint<32> GetEdgeImage(ap_uint<32> org, int off, bool isAllIn, bool isAllOut );

void hls_GetMB_uv_32bit_const(
		uint32_t src[MAX_W_PIX*4/4],
		int x_mb,
		int y_mb,
		int width,
		int height,
		int stride,
		ap_uint<WD_PIX*16>  ap_uv_in_[4]
		);

ap_uint<32> get32bits_2_const(ap_uint<3> n_rem, ap_uint<32> rem, ap_uint<32> crt);
//////////======================================================================/////////////////////////////
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
//////////======================================================================/////////////////////////////
//TopVp8_compute_NoOut===========================================================================/
//(Note, following names of functions may has already changed but not updated)
//-Intraprediction_mb_syn_str2
//--hls_LoadPre_out
//--hls_LoadPre_mode
//--Pickup_dataflow3
//---Pickup_Y44
//----hls_p4_test
//----hls_GetCost
//----hls_channel_p44
//-----hls_FTransform
//-----hls_QuantizeBlock
//-----hls_ITransformOne
//-----hls_SSE4X4
//-----hls_Disto4x4
//-----hls_fast_cost
//---Pickup_Y16
//----hls_channel_p16
//-----hls_p16_test
//-----hls_FTransform
//-----hls_FTransformWHT
//-----hls_QuantizeBlockWHT
//-----hls_IFTransformWHT
//-----hls_QuantizeBlock
//-----hls_ITransformOne
//-----hls_SSE4X4
//-----hls_Disto4x4
//-----hls_fast_cost
//-----hls_ca_score
//---Pickup_UV
//----hls_p8_test
//----hls_channel_uv_8
//-----hls_p8_test
//-----hls_FTransform
//-----hls_QuantizeBlock
//-----hls_ITransformOne
//-----hls_fast_cost
//-----hls_ca_score
//--hls_SetBestAs4_mode

void TopVp8_compute (
	ap_uint<LG2_MAX_NUM_MB_W> mb_w,
    ap_uint<LG2_MAX_NUM_MB_H> mb_h,
    hls::stream <ap_uint<WD_PIX*16> >  *str_din_y,
	hls::stream <ap_uint<WD_PIX*16> >  *str_din_uv,
    ap_uint<WD_LMD> lambda_p16,
    ap_uint<WD_LMD> lambda_p44,
    ap_uint<WD_LMD> tlambda,
    ap_uint<WD_LMD> lambda_uv,
    ap_uint<WD_LMD> tlambda_m,
    hls_QMatrix hls_qm1,
	hls_QMatrix hls_qm2,
	hls_QMatrix hls_qm_uv,
    ap_int<WD_sharpen*16>   ap_sharpen,
	ap_int<WD_sharpen*16>   ap_sharpen_uv,
	hls::stream< ap_uint<WD_PIX*16> >* str_out,
	hls::stream< ap_int<WD_LEVEL*16> >* str_level_dc,
	hls::stream< ap_int<WD_LEVEL*16> >* str_level_y,
	hls::stream< ap_int<WD_LEVEL*16> >* str_level_uv,
	hls::stream< ap_int<64> >* str_pred,
	hls::stream< ap_int<6> >* str_ret);
void TopVp8_compute_NoOut (
	ap_uint<LG2_MAX_NUM_MB_W> mb_w,
    ap_uint<LG2_MAX_NUM_MB_H> mb_h,
    hls::stream <ap_uint<WD_PIX*16> >  *str_din_y,
	hls::stream <ap_uint<WD_PIX*16> >  *str_din_uv,
    ap_uint<WD_LMD> lambda_p16,
    ap_uint<WD_LMD> lambda_p44,
    ap_uint<WD_LMD> tlambda,
    ap_uint<WD_LMD> lambda_uv,
    ap_uint<WD_LMD> tlambda_m,
    hls_QMatrix hls_qm1,
	hls_QMatrix hls_qm2,
	hls_QMatrix hls_qm_uv,
    ap_int<WD_sharpen*16>   ap_sharpen,
	ap_int<WD_sharpen*16>   ap_sharpen_uv,
	hls::stream< ap_int<WD_LEVEL*16> >* str_level_dc,
	hls::stream< ap_int<WD_LEVEL*16> >* str_level_y,
	hls::stream< ap_int<WD_LEVEL*16> >* str_level_uv,
	hls::stream< ap_int<64> >* str_pred,
	hls::stream< ap_int<6> >* str_ret);

//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
void Intraprediction_mb_syn_str2_widen (
    ap_uint<LG2_MAX_NUM_MB_W> x_mb,
    ap_uint<LG2_MAX_NUM_MB_W> y_mb,
	ap_uint<LG2_MAX_NUM_MB_W> mb_w,
    hls::stream <ap_uint<WD_PIX*16> >  *str_ap_yuv_in_y,
	hls::stream <ap_uint<WD_PIX*16> >  *str_ap_yuv_in_uv,
    ap_uint<WD_LMD> lambda_p16,
    ap_uint<WD_LMD> lambda_p44,
    ap_uint<WD_LMD> tlambda,
    ap_uint<WD_LMD> lambda_uv,
    ap_uint<WD_LMD> tlambda_m,
    hls_QMatrix hls_qm1,
	hls_QMatrix hls_qm2,
	hls_QMatrix hls_qm_uv,
    ap_int<WD_sharpen*16>   ap_sharpen,
	ap_int<WD_sharpen*16>   ap_sharpen_uv,
	hls::stream<ap_uint<WD_PIX * 16> >* str_out,
	hls::stream<ap_int<WD_LEVEL * 16> >* str_level_dc,
	hls::stream<ap_int<WD_LEVEL * 16> >* str_level_y,
	hls::stream<ap_int<WD_LEVEL * 16> >* str_level_uv,
	hls::stream< ap_int<64> >* str_pred,
	hls::stream< ap_int<6> >* str_ret
	);

void Intraprediction_mb_syn_str2_widen_NoOut (
    ap_uint<LG2_MAX_NUM_MB_W> x_mb,
    ap_uint<LG2_MAX_NUM_MB_W> y_mb,
	ap_uint<LG2_MAX_NUM_MB_W> mb_w,
    hls::stream <ap_uint<WD_PIX*16> >  *str_ap_yuv_in_y,
	hls::stream <ap_uint<WD_PIX*16> >  *str_ap_yuv_in_uv,
    ap_uint<WD_LMD> lambda_p16,
    ap_uint<WD_LMD> lambda_p44,
    ap_uint<WD_LMD> tlambda,
    ap_uint<WD_LMD> lambda_uv,
    ap_uint<WD_LMD> tlambda_m,
    hls_QMatrix hls_qm1,
	hls_QMatrix hls_qm2,
	hls_QMatrix hls_qm_uv,
    ap_int<WD_sharpen*16>   ap_sharpen,
	ap_int<WD_sharpen*16>   ap_sharpen_uv,
	//NoOut: hls::stream<ap_uint<WD_PIX * 16> >* str_out,
	hls::stream<ap_int<WD_LEVEL * 16> >* str_level_dc,
	hls::stream<ap_int<WD_LEVEL * 16> >* str_level_y,
	hls::stream<ap_int<WD_LEVEL * 16> >* str_level_uv,
	hls::stream< ap_int<64> >* str_pred,
	hls::stream< ap_int<6> >* str_ret
	);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
void hls_LoadPre_out_widen(
			ap_uint<WD_PIX>     *ap_y_m,
			ap_uint<WD_PIX>     *ap_u_m,
			ap_uint<WD_PIX>     *ap_v_m,
			ap_uint<WD_PIX*4>   ap_y_top_c[4],
			ap_uint<WD_PIX*4>   ap_y4_top_c[4],
			ap_uint<WD_PIX*4>   ap_uv_top_c[4],
			ap_uint<WD_PIX*4>   *ap_y4_topright_c,
			ap_uint<WD_PIX*4>   ap_y_left_c[4],
			ap_uint<WD_PIX*4>   ap_y4_left_c[4],
			ap_uint<WD_PIX*4>   ap_uv_left_c[4],
		    ap_uint<WD_PIX*4>   ap_y_left_[4],
		    ap_uint<WD_PIX*4>   ap_uv_left_[4],
			ap_uint<WD_PIX*4>   ap_y_top_[MAX_NUM_MB_W*4],
			ap_uint<WD_PIX*4>   ap_uv_top_[MAX_NUM_MB_W*4],
    		ap_uint<LG2_MAX_NUM_MB_W> x_mb,
    		ap_uint<LG2_MAX_NUM_MB_W> y_mb,
    		ap_uint<LG2_MAX_NUM_MB_W> mb_w);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
void hls_LoadPre_mode_widen(
		ap_uint<WD_MODE>    ap_y_top_mode[MAX_NUM_MB_W*4],
		ap_uint<WD_MODE>    ap_y_left_mode[4],
		ap_uint<WD_MODE>      ap_y_top_c_mode[4],
		ap_uint<WD_MODE>      ap_y4_top_c_mode[16],
		ap_uint<WD_MODE>      ap_y_left_c_mode[4],
		ap_uint<WD_MODE>      *ap_y_m_mode,
		ap_uint<LG2_MAX_NUM_MB_W> x_mb,
		ap_uint<LG2_MAX_NUM_MB_W> y_mb,
		ap_uint<LG2_MAX_NUM_MB_W> mb_w);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
ap_uint<12> hls_GetCost_widen(
		ap_uint<4> n_sb,
		ap_uint<4> mode,
	    ap_uint<WD_MODE>    ap_y_top_c_mode[4],// at beginning, default is DC
	    ap_uint<WD_MODE>    ap_y_left_c_mode[4],
	    ap_uint<WD_MODE>    local_mod);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
void Pickup_dataflow3_widen(
	// Parameters unParameters changed for one picture/segment
	ap_uint<WD_LMD>         I__tlambda,//              :
	ap_uint<WD_LMD>         I__tlambda_m,//
	ap_uint<WD_LMD> 	    I__lambda_p44,//
	ap_uint<WD_LMD>         I__lambda_p16,//
	ap_uint<WD_LMD>         I__lambda_uv,//
	hls_QMatrix             I__hls_qm1,//y44,y16
	hls_QMatrix             I__hls_qm2,//y16
	hls_QMatrix             I__hls_qm_uv,//
	ap_int<WD_sharpen*16>   I__ap_sharpen,//
	ap_int<WD_sharpen*16>   I__ap_sharpen_uv,//
	//Parameters changed for each MB
	ap_uint<WD_PIX*16>      I__ap_yuv_in_y44[16],//
	ap_uint<WD_PIX*16>      I__ap_yuv_in_y16[16],//
	ap_uint<WD_PIX*16>      I__ap_uv_in_[8],//
    ap_uint<1> 		        I__istop,//
    ap_uint<1> 		        I__isleft,//
    ap_uint<1> 		        I__isright,//
	// image context
	ap_uint<WD_PIX*4>       I__ap_y_top_c_y44[4],//
	ap_uint<WD_PIX*4>       I__ap_y_top_c_y16[4],//
	ap_uint<WD_PIX*4>       I__ap_y_left_c_y44[4],//
	ap_uint<WD_PIX*4>       I__ap_y_left_c_y16[4],//
	ap_uint<WD_PIX*4>       I__ap_uv_top_c[4],//
	ap_uint<WD_PIX*4>       I__ap_uv_left_c[4],//
	ap_uint<WD_PIX>         I__ap_y_m,//
	ap_uint<WD_PIX>         I__ap_u_m,//
	ap_uint<WD_PIX>         I__ap_v_m,//
	ap_uint<WD_PIX*4>       I__ap_y4_topright_c,//
	// mode context
	ap_uint<WD_MODE>        I__ap_y_top_c_mode[4],//
	ap_uint<WD_MODE>        I__ap_y_left_c_mode[4],//
	//OUTPUT
	ap_uint<WD_PIX*16>      O__ap_y4_out_cb[16],//
	ap_uint<WD_PIX*16>      O__ap_y_out_cb[2][17],//
	ap_uint<WD_PIX*16>      O__ap_uv_out_cb[2][17],//
	ap_int<WD_LEVEL*16>     O__ap_y4_level_cb[17],//
	ap_int<WD_LEVEL*16>     O__ap_y_level_cb[2][17],//
	ap_int<WD_LEVEL*16>     O__ap_y16dc_level_cb[2],//
	ap_int<WD_LEVEL*16>     O__ap_uv_level_cb[2][16],//
	//str_rd_i4*              OP_rd_y4_acc,//
	ap_uint<WD_RD_SCORE+4>* O__score_acc,
	ap_uint<25>*            O__nz_mb,
	str_rd                  O__rd_y16_cb[2],//
	str_rd                  O__rd_uv_cb[2],//
	ap_uint<WD_MODE>        O_ap_y4_top_c_mode[16],//
	ap_uint<WD_MODE>*       OP_ap_y16_mode_c,//
	ap_uint<WD_MODE>*       OP_ap_uv_mode_c,//
	ap_uint<1>*             OP_b_uv,//
	ap_uint<2>*             OP_b_y//
);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
void Pickup_Y44_widen(
		ap_uint<1>              istop,
		ap_uint<1>              isleft,
		ap_uint<WD_PIX*4>       ap_y_top_c[4],
		ap_uint<WD_PIX*4>       ap_y_left_c[4],
		ap_uint<WD_MODE>        ap_y_top_c_mode[4],// at beginning, default is DC
		ap_uint<WD_MODE>        ap_y_left_c_mode[4],
		//ap_uint<WD_MODE>        ap_y4_top_c_mode[16],
		ap_uint<WD_PIX*4>       ap_y4_topright_c,
		ap_uint<WD_PIX*4>       ap_y_m,
		//ap_uint<4>              MACRO_n_sb,
		ap_uint<WD_PIX*16>      ap_yuv_in[16],
		hls_QMatrix             hls_qm1,
		ap_int<WD_sharpen*16>   ap_sharpen,
	    ap_uint<WD_LMD>         lambda_p44,
	    ap_uint<WD_LMD>         tlambda,
	    ap_uint<WD_LMD>         tlambda_m,
		ap_uint<WD_PIX*16>		ap_y4_out_mb[16],
		ap_int<WD_LEVEL*16>		ap_y4_level_mb[16],
		ap_uint<WD_RD_SCORE+4>  *score_acc,
		ap_uint<25>             *nz_mb,
		ap_uint<WD_MODE>		O__modes_mb[16]);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
void hls_LoadPreds4_ins(
			ap_uint<WD_PIX*4> ap_y4_top_c[16],
			ap_uint<WD_PIX*4> ap_y4_left_c[16],
			ap_uint<WD_PIX*4> ap_y4_topright_c,
			ap_uint<WD_PIX*4> ap_y_m,
            ap_uint<WD_PIX*4>*   abcd,
    	    ap_uint<WD_PIX*4>*   efgh,
    	    ap_uint<WD_PIX*4>*   ijkl,
            ap_uint<WD_PIX>*     x44,
            ap_uint<1>           isleft,
            ap_uint<1>           istop,
    	    ap_uint<4>           n_sb);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
ap_uint<WD_PIX*16> hls_p4_test(	ap_uint<WD_PIX*4> abcd,
								ap_uint<WD_PIX*4> efgh,
								ap_uint<WD_PIX*4> ijkl,
								ap_uint<WD_PIX> 	x44,
								ap_uint<4> mode);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
void hls_channel_p44(
		ap_uint<4>              mode_in,
		ap_uint<WD_PIX*16>      ap_yuv_in_sb,
		ap_uint<WD_PIX*16>      ap_ref_p44,
		hls_QMatrix             hls_qm1,
		ap_int<WD_sharpen*16>   ap_sharpen,
	    ap_uint<WD_LMD>         lambda_p44,
	    ap_uint<WD_LMD>         tlambda,
	    ap_uint<WD_LMD>         tlambda_m,
		ap_uint<12>		        pre_dis_h,
		ap_uint<WD_PIX*16>		*ap_y4_out_cb_n_sb2,
		ap_int<WD_LEVEL*16>		*ap_y4_level_cb_n_sb2,
		ap_uint<WD_RD_SCORE+4>  *score_sb,
		ap_uint<25>             *nz_sb,
		ap_uint<4>				*mode_out);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
/* FOR SD CACULATION */
ap_uint<WD_DISTO>  hls_Disto4x4(ap_uint<WD_PIX*16> a,ap_uint<WD_PIX*16> b) ;
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
/* FOR R CACULATION */
ap_uint<WD_LEVEL+4> hls_LV0(ap_uint <WD_LEVEL-1> lv) ;

ap_uint<WD_LEVEL> hls_LV1(ap_uint <WD_LEVEL-1> lv) ;

ap_uint<WD_LEVEL> hls_LV2(ap_uint <WD_LEVEL-1> lv) ;

ap_uint<WD_LEVEL> hls_LVn(ap_uint <WD_LEVEL-1> lv) ;

ap_uint<WD_FAST> hls_fast_cost(ap_int<WD_LEVEL*16>  vlevel, ap_uint<2> type);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
/* FOR S CACULATION */
ap_uint<WD_SSE4> hls_SSE4X4(ap_uint<WD_PIX*16> src, ap_uint<WD_PIX*16> rec );
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
/* NORMAL TRANSFORM */
ap_int<WD_DCT*16> hls_FTransform(ap_uint<WD_PIX*16> src_ap, ap_uint<WD_PIX*16> ref_ap) ;
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
ap_uint<5> hls_QuantizeBlock( ap_int<WD_DCT*16>     in,
                            ap_int<WD_LEVEL*16>     *out,
                            ap_int<WD_DCT*16>       *out2,
                            hls_QMatrix*            pQM ,// frequency boosters for slight sharpening
                            ap_uint<WD_sharpen*16>  sharpen_,
                            ap_uint<1>              is16);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////

/* QUANTITION FOR NORMAL AND DC */
ap_uint<5> hls_QuantizeBlock_old( ap_int<WD_DCT*16>       in,
                            ap_int<WD_LEVEL*16>        *out,
                            ap_int<WD_DCT*16>       *out2,
                            ap_uint<WD_q>           q_0,        // quantizer steps
                            ap_uint<WD_q>           q_n,
                            ap_uint<WD_iq>          iq_0,       // reciprocals, fixed point.
                            ap_uint<WD_iq>          iq_n,
                            ap_uint<WD_bias>        bias_0,     // rounding bias
                            ap_uint<WD_bias>        bias_n,
                            ap_uint<WD_sharpen*16>  sharpen_,
                            ap_uint<1>              is16);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
/*Invers  Transforms */
ap_uint<WD_PIX*16> hls_ITransformOne(ap_uint<WD_PIX*16> ap_ref,
                                    ap_int<WD_IQT*16> ap_in);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
void Pickup_Y16(
		ap_uint<WD_LMD>         I__tlambda,//              :
		ap_uint<WD_LMD>         I__tlambda_m,//
		ap_uint<WD_LMD>         I__lambda_p16,//
		hls_QMatrix             I__hls_qm1,//y44,y16
		hls_QMatrix             I__hls_qm2,//y16
		ap_int<WD_sharpen*16>   I__ap_sharpen,//
		//Parameters changed for each MB
		ap_uint<WD_PIX*16>      I__ap_y_in_[16],//
	    ap_uint<1> 		        I__istop,//
	    ap_uint<1> 		        I__isleft,//
	    ap_uint<1> 		        I__isright,//
		// image context
		ap_uint<WD_PIX*4>       I__ap_y_top_c[4],//
		ap_uint<WD_PIX*4>       I__ap_y_left_c[4],//
		ap_uint<WD_PIX>         I__ap_y_m,//
		//OUTPUT
		ap_uint<WD_PIX*16>      O__ap_y_out_cb[2][17],//
		ap_int<WD_LEVEL*16>     O__ap_y_level_cb[2][17],//
		ap_int<WD_LEVEL*16>     O__ap_y16dc_level_cb[2],//
		str_rd                  O__rd_y16_cb[2],//
		ap_uint<WD_MODE>*       OP_ap_y16_mode_c,//
		ap_uint<2>*             OP_b_y//
    );
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
ap_uint<WD_RD_SCORE+4>  hls_channel_p16(
		ap_uint<4>              mode_p16,
		ap_uint<1>              istop,
		ap_uint<1>              isleft,
		ap_uint<WD_PIX*4>       ap_y_top_c[4],
		ap_uint<WD_PIX*4>       ap_y_left_c[4],
		ap_uint<WD_PIX>         ap_y_m,
		ap_uint<WD_PIX*16>      ap_yuv_in_[24],
		hls_QMatrix             hls_qm1,
		hls_QMatrix             hls_qm2,
		ap_int<WD_sharpen*16>   ap_sharpen,
	    ap_uint<WD_LMD>         tlambda,//     = dqm->tlambda_;
	    ap_uint<WD_LMD>         tlambda_m,//   = dqm->lambda_mode_;
		ap_int<WD_LEVEL*16>     ap_y16_level_c[17],
		ap_int<WD_LEVEL*16>*    ap_y16dc_level_c,
		ap_uint<WD_PIX*16>      ap_y16_out_c[17],
		ap_uint<25>*            nz
		);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
/*Invers  Transforms */
ap_int<WD_IWHT*16> hls_ITransformWHT(ap_int<WD_WHT*16> in);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
/* QUANTITION FOR DC */
ap_uint<1> hls_QuantizeBlockWHT_old( ap_int<WD_WHT*16>       in,
                            ap_int<WD_LEVEL*16>        *out,
                            ap_int<WD_WHT*16>       *out2,
                            ap_uint<WD_q>           q_0,        // quantizer steps
                            ap_uint<WD_q>           q_n,
                            ap_uint<WD_iq>          iq_0,       // reciprocals, fixed point.
                            ap_uint<WD_iq>          iq_n,
                            ap_uint<WD_bias>        bias_0,     // rounding bias
                            ap_uint<WD_bias>        bias_n) ;

ap_uint<1> hls_QuantizeBlockWHT( ap_int<WD_WHT*16>  in,
                            ap_int<WD_LEVEL*16>     *out,
                            ap_int<WD_WHT*16>       *out2,
                            hls_QMatrix*            pQM);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
ap_uint<WD_PIX*16> hls_p16_test(
		ap_uint<2> mode,
		ap_uint<4> n,
		ap_uint<1> istop,
		ap_uint<1> isleft,
	    ap_uint<WD_PIX*4>   ap_y_top_c[4],
	    ap_uint<WD_PIX*4>   ap_y_left_c[4],
		ap_uint<WD_PIX>     ap_y_m);

ap_uint<WD_RD_SCORE+4> hls_ca_score(ap_uint<WD_LMD> lmbda, str_dis* dis ,ap_uint<4>m);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
void Pickup_UV(
		// Parameters unParameters changed for one picture/segment
			ap_uint<WD_LMD>         I__tlambda,//              :
			ap_uint<WD_LMD>         I__tlambda_m,//
			ap_uint<WD_LMD>         I__lambda_uv,//
			hls_QMatrix             I__hls_qm_uv,//
			ap_int<WD_sharpen*16>   I__ap_sharpen_uv,//
			//Parameters changed for each MB
			ap_uint<WD_PIX*16>      I__ap_uv_in_[8],//
		    ap_uint<1> 		        I__istop,//
		    ap_uint<1> 		        I__isleft,//
		    ap_uint<1> 		        I__isright,//
			// image context
			ap_uint<WD_PIX*4>       I__ap_uv_top_c[4],//
			ap_uint<WD_PIX*4>       I__ap_uv_left_c[4],//
			ap_uint<WD_PIX>         I__ap_u_m,//
			ap_uint<WD_PIX>         I__ap_v_m,//
			ap_uint<WD_PIX*16>      O__ap_uv_out_cb[2][17],//
			ap_int<WD_LEVEL*16>     O__ap_uv_level_cb[2][16],//
			str_rd                  O__rd_uv_cb[2],//
			ap_uint<WD_MODE>*       OP_ap_uv_mode_c,//
			ap_uint<1>*             OP_b_uv//
		);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
ap_uint<WD_RD_SCORE+4>  hls_channel_uv_8(
				ap_uint<4>              mode_uv,
				ap_uint<1>              istop,
				ap_uint<1>              isleft,
				ap_uint<WD_PIX*4>       ap_uv_top_c[4],
				ap_uint<WD_PIX*4>       ap_uv_left_c[4],
				ap_uint<WD_PIX>         ap_u_m,
				ap_uint<WD_PIX>         ap_v_m,
				ap_uint<WD_PIX*16>      ap_uv_in_[8],
				hls_QMatrix             hls_qm_uv,
				ap_int<WD_sharpen*16>   ap_sharpen_uv,
				ap_uint<WD_LMD>         lambda_uv,//     = dqm->tlambda_;
				ap_int<WD_LEVEL*16>     ap_uv_level_c[8],
				ap_uint<WD_PIX*16>      ap_uv_out_c[8],
				ap_uint<25>*            nz);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
void hls_SetBestAs4_mode_widen(
		ap_uint<WD_MODE>    ap_y_top_mode[MAX_NUM_MB_W*4],
		ap_uint<WD_MODE>    ap_y_left_mode[4],
		ap_uint<WD_MODE>      ap_y4_top_c_mode[16],
		ap_uint<WD_MODE>      ap_y_left_c_mode[4],
		ap_uint<WD_MODE*16>   *ap_y_mode_b,
		ap_uint<LG2_MAX_NUM_MB_W+2> x_sb_w
		);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
void hls_SetBestAs16_mode_widen(
		ap_uint<WD_MODE>    ap_y_top_mode[MAX_NUM_MB_W*4],
		ap_uint<WD_MODE>    ap_y_left_mode[4],
		ap_uint<WD_MODE>      ap_y16_mode_c,
		ap_uint<WD_MODE*16>   *ap_y_mode_b,
		ap_uint<LG2_MAX_NUM_MB_W+2> x_sb_w);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
void hls_StoreTopLeft_uv(
	    ap_uint<WD_PIX*4>   ap_uv_top_[MAX_NUM_MB_W*4],
	    ap_uint<WD_PIX*4>   ap_uv_left_[4],
		ap_uint<WD_PIX*16>   ap_uv_out_cb[8],
		ap_uint<LG2_MAX_NUM_MB_W> x_mb );
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
void hls_StoreTopLeft_y(
	    ap_uint<WD_PIX*4>   ap_y_top_[MAX_NUM_MB_W*4],
		ap_uint<WD_PIX*4>   ap_y_left_[4],
		ap_uint<WD_PIX*16>   ap_y_out_cb[16],
		ap_uint<LG2_MAX_NUM_MB_W> x_mb );
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
ap_uint<12> hls_GetCost(
		ap_uint<4> n_sb,
		ap_uint<4> mode,
	    ap_uint<WD_MODE>    ap_y_top_c_mode[4],// at beginning, default is DC
	    ap_uint<WD_MODE>    ap_y_left_c_mode[4],
	    ap_uint<WD_MODE>    ap_y4_top_c_mode[16]);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
ap_uint<WD_PIX*16> hls_p8_test(
		ap_uint<2> mode,
		ap_uint<3> n,
		ap_uint<1> istop,
		ap_uint<1> isleft,
	    ap_uint<WD_PIX*4>   ap_uv_top_c[4],
	    ap_uint<WD_PIX*4>   ap_uv_left_c[4],
		ap_uint<WD_PIX>     ap_u_m,
		ap_uint<WD_PIX>     ap_v_m);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
ap_uint<WD_PIX*16> hls_DC16_4_uv_old( //ref: lut:56, 3.19+1.25
        ap_uint<WD_PIX*4> top[2],
        ap_uint<WD_PIX*4> left[2],
        ap_uint<1> istop,
        ap_uint<1> isleft
    );
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
ap_uint<WD_PIX*16> hls_TM16_4( //ref: lut:56, 3.19+1.25
        ap_uint<WD_PIX*4> abcd,
        ap_uint<WD_PIX*4> ijkl,
        ap_uint<WD_PIX>    X44,
        ap_uint<1> istop,
        ap_uint<1> isleft
    );
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
ap_uint<WD_PIX*16> hls_VE16_4( //ref: lut:56, 3.19+1.25//lut 452vs997, 2.72+1.25ns,
        ap_uint<WD_PIX*4> abcd,
        ap_uint<1> istop
        );
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
ap_uint<WD_PIX*16> hls_HE16_4( //ref: lut:56, 3.19+1.25
        ap_uint<WD_PIX*4> ijkl,
        ap_uint<1> isleft
    );
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
ap_int<WD_WHT*16> hls_FTransformWHT(ap_int<WD_DCT*16> in);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
ap_int<WD_IWHT*16> hls_ITransformWHT(ap_int<WD_WHT*16> in);
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
ap_uint<WD_TTR> hls_TTransform(ap_uint<WD_PIX*16> in );
//////////=====================   TopVp8_compute_NoOut==========================/////////////////////////////
ap_uint<WD_PIX*16> hls_DC16_4_y( //ref: lut:56, 3.19+1.25
        ap_uint<WD_PIX*4> top0,
		ap_uint<WD_PIX*4> top1,
		ap_uint<WD_PIX*4> top2,
		ap_uint<WD_PIX*4> top3,
        ap_uint<WD_PIX*4> left0,
		ap_uint<WD_PIX*4> left1,
		ap_uint<WD_PIX*4> left2,
		ap_uint<WD_PIX*4> left3,
        ap_uint<1> istop,
        ap_uint<1> isleft
    );
//////////======================================================================/////////////////////////////
//////////====================  TopVp8_send_32bit  =================/////////////////////////////
//////////======================================================================/////////////////////////////
void TopVp8_send_32bit(
		ap_uint<LG2_MAX_NUM_MB_W> mb_w,
	    ap_uint<LG2_MAX_NUM_MB_H> mb_h,
		hls::stream< ap_int<WD_LEVEL*16> >* str_level_dc,
		hls::stream< ap_int<WD_LEVEL*16> >* str_level_y,
		hls::stream< ap_int<WD_LEVEL*16> >* str_level_uv,
		hls::stream< ap_int<64> >* str_pred,
		hls::stream< ap_int<6> >* str_ret,
		//output
		int32_t* pout_level
		);
//////////====================  TopVp8_send_32bit  =================/////////////////////////////
void TopVp8_send__strs_to_array(
		short int* pout,
		hls::stream< ap_int<WD_LEVEL*16> >* str_level_dc,
		hls::stream< ap_int<WD_LEVEL*16> >* str_level_y,
		hls::stream< ap_int<WD_LEVEL*16> >* str_level_uv,
		hls::stream< ap_int<64> >* str_pred,
		hls::stream< ap_int<6> >* str_ret
		);
//////////======================================================================/////////////////////////////
//////////=====================   TopVp8_RecordCoeff_hls_cnt          ===========/////////////////////////////
//////////======================================================================/////////////////////////////
void TopVp8_RecordCoeff_hls_cnt(
		ap_uint<LG2_MAX_NUM_MB_W> mb_w,
		ap_uint<LG2_MAX_NUM_MB_H> mb_h,
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_dc,
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_y,
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_uv,
		hls::stream<ap_int<64> >* str_pred, hls::stream<ap_int<6> >* str_ret,
		//output
		hls::stream<ap_uint<1> > &str_mb_type,
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_dc2,
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_y2,
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_uv2,
		hls::stream<ap_int<64> >* str_pred2, hls::stream<ap_int<6> >* str_ret2,
		hls::stream<ap_uint<1 + 1 + 3 + 2 + 4> > &str_rec_dc,
		hls::stream<ap_uint<1 + 1 + 3 + 2 + 4> > &str_rec_ac,
		hls::stream<ap_uint<1 + 1 + 3 + 2 + 4> > &str_rec_uv,
		hls::stream<ap_uint<8> > &str_cnt_dc,
		hls::stream<ap_uint<8> > &str_cnt_ac,
		hls::stream<ap_uint<8> > &str_cnt_uv
		);

//////////=====================   TopVp8_RecordCoeff_hls_cnt          ===========/////////////////////////////
ap_uint<9>  RecordCoeff_dataflow(
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_dc,
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_y,
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_uv,
		hls::stream<ap_int<64> >* str_pred, hls::stream<ap_int<6> >* str_ret,
		//output
		hls::stream<ap_uint<1> > &str_mb_type,
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_dc2,
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_y2,
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_uv2,
		hls::stream<ap_int<64> >* str_pred2, hls::stream<ap_int<6> >* str_ret2,
		hls::stream<ap_uint<1 + 1 + 3 + 2 + 4> > &str_rec_dc,
		hls::stream<ap_uint<1 + 1 + 3 + 2 + 4> > &str_rec_ac,
		hls::stream<ap_uint<1 + 1 + 3 + 2 + 4> > &str_rec_uv,
		hls::stream<ap_uint<8> > &str_cnt_dc,
		hls::stream<ap_uint<8> > &str_cnt_ac,
		hls::stream<ap_uint<8> > &str_cnt_uv,
		ap_uint<9> &top_nz_dc,//
		ap_uint<9> left_nz_dc,// = ap_left_nz;
        ap_uint<9> &top_nz_y,// = ap_top_nz;
		ap_uint<9> &left_nz_y,// = ap_left_nz;
		ap_uint<9> &top_nz_uv,// = ap_top_nz;
		ap_uint<9> &left_nz_uv// = ap_left_nz;
		);
//////////=====================   TopVp8_RecordCoeff_hls_cnt          ===========/////////////////////////////
ap_uint<9> RecordCoeff_dataflow_dc(
		ap_uint<1> mb_type,
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_dc,
		//output
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_dc2,
		hls::stream<ap_uint<1 + 1 + 3 + 2 + 4> > &str_rec_dc,
		hls::stream<ap_uint<8> > &str_cnt_dc,
		ap_uint<9> &top_nz_dc,//
		ap_uint<9> left_nz_dc// = ap_left_nz;
		);

//////////=====================   TopVp8_RecordCoeff_hls_cnt          ===========/////////////////////////////

void RecordCoeff_dataflow_y(
		ap_uint<1> mb_type,
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_y,
		//output
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_y2,
		hls::stream<ap_uint<1 + 1 + 3 + 2 + 4> > &str_rec_ac,
		hls::stream<ap_uint<8> > &str_cnt_ac,
        ap_uint<9> &top_nz_y,// = ap_top_nz;
		ap_uint<9> &left_nz_y// = ap_left_nz;
		);

//////////=====================   TopVp8_RecordCoeff_hls_cnt          ===========/////////////////////////////

void RecordCoeff_dataflow_uv(
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_uv,
		//output
		hls::stream<ap_int<WD_LEVEL * 16> >* str_level_uv2,
		hls::stream<ap_uint<1 + 1 + 3 + 2 + 4> > &str_rec_uv,
		hls::stream<ap_uint<8> > &str_cnt_uv,
		ap_uint<9> &top_nz_uv,// = ap_top_nz;
		ap_uint<9> &left_nz_uv// = ap_left_nz;
		);

//////////=====================   TopVp8_RecordCoeff_hls_cnt          ===========/////////////////////////////
ap_int<5> FindLast(ap_int<WD_LEVEL * 16> level);

//////////=====================   TopVp8_RecordCoeff_hls_cnt          ===========/////////////////////////////
int VP8RecordCoeffs_hls_str_w_cnt(
        ap_uint<2> ctx,
	    ap_int<WD_LEVEL * 16> coeffs,
		ap_uint<1> first,
		ap_int<5> last,
		hls::stream<ap_uint<11> > &str_rec,
		hls::stream<ap_uint<8> > &str_cnt
		);

//////////=====================   TopVp8_RecordCoeff_hls_cnt          ===========/////////////////////////////
void Record_str(
		hls::stream<ap_uint<11> > &str_rec,
		ap_uint<1> isEnd,
		ap_uint<1> bit,
		ap_uint<3> band,
		ap_uint<2> ctx,
		ap_uint<4> off);

//////////=====================   TopVp8_RecordCoeff_hls_cnt          ===========/////////////////////////////
ap_uint<3> VP8EncBands_hls(ap_uint<5> n);

//////////======================================================================/////////////////////////////
//////////============  TopVp8_RecordProb_hls_cnt                    ===========/////////////////////////////
//////////======================================================================/////////////////////////////
int TopVp8_RecordProb_hls_cnt(
		ap_uint<LG2_MAX_NUM_MB_W> mb_w,
		ap_uint<LG2_MAX_NUM_MB_H> mb_h,
		hls::stream<ap_uint<1> > &str_mb_type,
		hls::stream<ap_uint<1 + 1 + 3 + 2 + 4> > &str_rec_dc,
		hls::stream<ap_uint<1 + 1 + 3 + 2 + 4> > &str_rec_ac,
		hls::stream<ap_uint<1 + 1 + 3 + 2 + 4> > &str_rec_uv,
		hls::stream<ap_uint<8> > &str_cnt_dc,
		hls::stream<ap_uint<8> > &str_cnt_ac,
		hls::stream<ap_uint<8> > &str_cnt_uv,
		uint8_t* pout_prob //4, 8, 3,11
		);
void TopVp8_RecordProb_hls_cnt_HideDirty(
		ap_uint<LG2_MAX_NUM_MB_W> mb_w,
		ap_uint<LG2_MAX_NUM_MB_H> mb_h,
		hls::stream<ap_uint<1> > &str_mb_type,
		hls::stream<ap_uint<1 + 1 + 3 + 2 + 4> > &str_rec_dc,
		hls::stream<ap_uint<1 + 1 + 3 + 2 + 4> > &str_rec_ac,
		hls::stream<ap_uint<1 + 1 + 3 + 2 + 4> > &str_rec_uv,
		hls::stream<ap_uint<8> > &str_cnt_dc,
		hls::stream<ap_uint<8> > &str_cnt_ac,
		hls::stream<ap_uint<8> > &str_cnt_uv,
		uint8_t* pout_prob //4, 8, 3,11
		);

//////////============  TopVp8_RecordProb_hls_cnt_HideDirty          ===========/////////////////////////////
void RecordPorb_ReadCoeff_dataflow2_cnt(ap_uint<1> mb_type,
		hls::stream<ap_uint<11> > &str_rec_dc,
		hls::stream<ap_uint<11> > &str_rec_ac,
		hls::stream<ap_uint<11> > &str_rec_uv,
		hls::stream<ap_uint<8> > &str_cnt_dc,
		hls::stream<ap_uint<8> > &str_cnt_ac,
		hls::stream<ap_uint<8> > &str_cnt_uv,
		uint32_t stats_dc[8][3][11],
		uint32_t stats_ac0_dc[8][3][11],
		uint32_t stats_ac3[8][3][11],
		uint32_t stats_uv[8][3][11]);

//////////============  TopVp8_RecordProb_hls_cnt_HideDirty          ===========/////////////////////////////
////RecordPorb_ReadCoeff_dataflow_dc_cnt//////////////////////////
void RecordPorb_ReadCoeff_dataflow_dc_cnt(
		ap_uint<1> mb_type,
		hls::stream<ap_uint<11> > &str_rec_dc,
		hls::stream<ap_uint<8> > &str_cnt,
		uint32_t stats_dc[8][3][11]);

////RecordPorb_ReadCoeff_dataflow_ac_cnt//////////////////////////
void RecordPorb_ReadCoeff_dataflow_ac_cnt(
		ap_uint<1> mb_type,
		hls::stream<ap_uint<11> > &str_rec_ac,
		hls::stream<ap_uint<8> > &str_cnt,
		uint32_t stats_ac0_dc[8][3][11],
		uint32_t stats_ac3[8][3][11]);

////RecordPorb_ReadCoeff_dataflow_uv_cnt//////////////////////////
void RecordPorb_ReadCoeff_dataflow_uv_cnt(
		hls::stream<ap_uint<11> > &str_rec_uv,
		hls::stream<ap_uint<8> > &str_cnt,
		uint32_t stats_uv[8][3][11]);

//////////============  TopVp8_RecordProb_hls_cnt_HideDirty          ===========/////////////////////////////

//////VP8RecordCoeffs_hls_str_r_cnt//////////////////
void VP8RecordCoeffs_hls_str_r_cnt(
		hls::stream<ap_uint<11> > &str_rec,
		hls::stream<ap_uint<8> > &str_cnt,
		uint32_t stats[8][3][11]);

void VP8RecordCoeffs_hls_str_r_cnt_old(
		hls::stream<ap_uint<11> > &str_rec,
		hls::stream<ap_uint<8> > &str_cnt,
		uint32_t stats[8][3][11]);

//////////============  TopVp8_RecordProb_hls_cnt_HideDirty          ===========/////////////////////////////
ap_uint<32> Record_hls(ap_uint<1> bit, ap_uint<32> p);

//////////============  TopVp8_RecordProb_hls_cnt_HideDirty          ===========/////////////////////////////
int FinalizeTokenProbas_hls(
		uint32_t p_stats[4][8][3][11],
		uint8_t p_coeffs_[4][8][3][11],
		int* dirty);
/* 1 */
ap_uint<WD_PIX*16> hls_DC4( //ref:581  lut 56, 4.46+1.25 ,
        ap_uint<WD_PIX*4> abcd,
		ap_uint<WD_PIX*4> ijkl);

/* 2 */
ap_uint<WD_PIX*16> hls_VE4( //ref: lut:56, 3.19+1.25//lut 452vs997, 2.72+1.25ns,
        ap_uint<WD_PIX*4> abcd,
        ap_uint<WD_PIX*4> efgh,
        ap_uint<WD_PIX>     X44);

/* 3 */
ap_uint<WD_PIX*16> hls_HE4( //ref: lut:56, 3.19+1.25
        ap_uint<WD_PIX*4> ijkl,
        ap_uint<WD_PIX>     X44);

/* 4 */
ap_uint<WD_PIX*16> hls_RD4( //ref: lut:98  3.19+1.25, ,
        ap_uint<WD_PIX*4> abcd,
        ap_uint<WD_PIX*4> ijkl,
        ap_uint<WD_PIX>     X44);

/* 5 */
ap_uint<WD_PIX*16> hls_LD4( //ref: lut:98  3.19+1.25  , ,
        ap_uint<WD_PIX*4> abcd,
        ap_uint<WD_PIX*4> efgh);

/* 6 */
ap_uint<WD_PIX*16> hls_VR4( //ref: lut: 100  3.19+1.25 , ,
        ap_uint<WD_PIX*4> abcd,
        ap_uint<WD_PIX*4> ijkl,
        ap_uint<WD_PIX>     X44);

/* 7 */
ap_uint<WD_PIX*16> hls_VL4( //ref: lut: 100 3.19+1.25 , ,
        ap_uint<WD_PIX*4> abcd,
        ap_uint<WD_PIX*4> efgh);

/* 8 */
ap_uint<WD_PIX*16> hls_HU4( //ref: lut 54 3.19+1.25 , ,
        ap_uint<WD_PIX*4> ijkl);

/* 9 */
ap_uint<WD_PIX*16> hls_HD4( //ref:544 vs lut:100 ,3.19+1.25 ,
        ap_uint<WD_PIX*4> abcd,
        ap_uint<WD_PIX*4> ijkl,
        ap_uint<WD_PIX>     X44);

/* 10 */
ap_uint<WD_PIX*16> hls_TM4(
        ap_uint<WD_PIX*4> abcd,
        ap_uint<WD_PIX*4> ijkl,
        ap_uint<WD_PIX>     X44
        );
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//==================================kernel_2_ArithmeticCoding===========================================//
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//kernel_2_ArithmeticCoding
//|-memcpy
//|-Kernel2_top_read
//|-kernel_2_RecordTokens_pre
//|-kernel_2_CreateTokens_with_isFinal
//|-VP8EmitTokens_str_hls_4stages
//|-PackStr2Mem32_t_NoLast
//|-PackWideStr2Mem32_t_NoLast
//==================================kernel_2_ArithmeticCoding===========================================//
void Kernel2_top_read(
		uint32_t pin_level[SIZE32_MEM_LEVEL],
		//output
		hls::stream<ap_int<WD_LEVEL * 16> > &str_level_dc,
		hls::stream<ap_int<WD_LEVEL * 16> > &str_level_ac,
		hls::stream<ap_int<WD_LEVEL * 16> > &str_level_uv,
		hls::stream<ap_uint<64> > &str_pred,
		hls::stream<ap_uint<6> > &str_ret,
		hls::stream<ap_uint<1> > &str_type_mb,
		hls::stream<uint16_t> &str_mb_h,
		hls::stream<uint16_t> &str_mb_w);
//==================================kernel_2_ArithmeticCoding===========================================//
void Kernel2_read__array_to_str(
		uint32_t pin[256],
		hls::stream<ap_int<WD_LEVEL * 16> > &str_level_dc,
		hls::stream<ap_int<WD_LEVEL * 16> > &str_level_ac,
		hls::stream<ap_int<WD_LEVEL * 16> > &str_level_uv,
		hls::stream<ap_uint<64> > &str_pred,
		hls::stream<ap_uint<6> > &str_ret,
		hls::stream<ap_uint<1> > &str_type_mb);
//==================================kernel_2_ArithmeticCoding===========================================//
ap_int<WD_LEVEL * 16> SetVectFrom32bit(uint32_t* pin);
//==================================kernel_2_ArithmeticCoding===========================================//
ap_uint<4 * 16> SetVect64From32bit(uint32_t* pin);
//==================================kernel_2_ArithmeticCoding===========================================//
void kernel_2_RecordTokens_pre(
		hls::stream<uint16_t> &str_mb_h,
		hls::stream<uint16_t> &str_mb_w,
		hls::stream<ap_uint<1> > &str_type_mb,
		hls::stream<ap_int<WD_LEVEL * 16> > &str_level_dc,
		hls::stream<ap_int<WD_LEVEL * 16> > &str_level_ac,
		hls::stream<ap_int<WD_LEVEL * 16> > &str_level_uv,
		hls::stream<ap_uint<64> > &str_0_dc,
		hls::stream<ap_uint<64> > &str_1_dc,
		hls::stream<ap_uint<64> > &str_2_dc,
		hls::stream<ap_uint<64> > &str_3_dc,
		hls::stream<ap_uint<64> > &str_0_ac,
		hls::stream<ap_uint<64> > &str_1_ac,
		hls::stream<ap_uint<64> > &str_2_ac,
		hls::stream<ap_uint<64> > &str_3_ac,
		hls::stream<ap_uint<64> > &str_0_uv,
		hls::stream<ap_uint<64> > &str_1_uv,
		hls::stream<ap_uint<64> > &str_2_uv,
		hls::stream<ap_uint<64> > &str_3_uv,
		hls::stream<uint16_t> &str_mb_h_out,
		hls::stream<uint16_t> &str_mb_w_out,
		hls::stream<ap_uint<1> > &str_type_mb_out);

//==================================kernel_2_ArithmeticCoding===========================================//
/////RecordTokens_nrd2_mb_w////////////////////////////////
void RecordTokens_nrd2_mb_w(
		ap_NoneZero *ap_nz,
		int x_, int y_,
		hls::stream<ap_uint<1> > &str_type_mb,
		hls::stream<ap_int<WD_LEVEL * 16> > &str_level_dc,
		hls::stream<ap_int<WD_LEVEL * 16> > &str_level_ac,
		hls::stream<ap_int<WD_LEVEL * 16> > &str_level_uv,
		hls::stream<ap_uint<64> > &str_0_dc,
		hls::stream<ap_uint<64> > &str_1_dc,
		hls::stream<ap_uint<64> > &str_2_dc,
		hls::stream<ap_uint<64> > &str_3_dc,
		hls::stream<ap_uint<64> > &str_0_ac,
		hls::stream<ap_uint<64> > &str_1_ac,
		hls::stream<ap_uint<64> > &str_2_ac,
		hls::stream<ap_uint<64> > &str_3_ac,
		hls::stream<ap_uint<64> > &str_0_uv,
		hls::stream<ap_uint<64> > &str_1_uv,
		hls::stream<ap_uint<64> > &str_2_uv,
		hls::stream<ap_uint<64> > &str_3_uv,
		hls::stream<ap_uint<1> > &str_type_mb_out);

//==================================kernel_2_ArithmeticCoding===========================================//
int VP8RecordCoeffTokens_hls_w(
		ap_uint<2> ctx,
		ap_uint<2> coeff_type,
		ap_int<5> last,
		ap_int<WD_LEVEL * 16> coeffs,
		hls::stream<ap_uint<64> > &str_0,
		hls::stream<ap_uint<64> > &str_1,
		hls::stream<ap_uint<64> > &str_2,
		hls::stream<ap_uint<64> > &str_3);

//==================================kernel_2_ArithmeticCoding===========================================//
void PackToken_hls(
		ap_uint<64> &w,
		ap_uint<2> be,
		uint32_t bit,
		uint32_t proba_idx);
void PackConstantToken_hls(
		ap_uint<64> &w,
		ap_uint<2> be,
		uint32_t bit,
		uint32_t proba_idx);
/////TokensStr0_hls////////////////////////
void TokensStr0_hls(
		ap_uint<2> ctx,
		ap_uint<2> coeff_type,
		ap_int<5> last,
		hls::stream<ap_uint<64> > &str_0);

/////TokensStr1_hls//////////////////////////////////////

int TokensStr1_hls(
		ap_uint<1> isV_N0,	 // = v!=0,
		ap_uint<1> isV_B1,	 // = v>1
		ap_uint<1> sign,
		ap_uint<1> isLastBEi,	 // = i<last,
		ap_uint<11> base_id,
		ap_uint<11> base_id_next,
		ap_uint<11> v,
		hls::stream<ap_uint<64> > &str_1);

/////TokensStr2_hls/////////////////////////////////////////
int TokensStr2_hls(
		ap_uint<1> isV_B4,	 // = v>4;
		ap_uint<1> isV_N2,	 // = v!=2;
		ap_uint<1> isV_4,	 // = v==4;
		ap_uint<1> isV_B10,	 // = v>10;
		ap_uint<11> base_id,
		hls::stream<ap_uint<64> > &str_2);

/////TokensStr3_hls//////////////////////////////////////
int TokensStr3_hls(
		ap_uint<1> isV_B6,	 // = v>6;
		ap_uint<1> isV_6,	 // = v==6;
		ap_uint<1> isV_BE9,	 // = v>=9;
		ap_uint<1> isV_even,	 // = 1-v&1;//!(v & 1)
		ap_uint<11> base_id,
		hls::stream<ap_uint<64> > &str_3);

//==================================kernel_2_ArithmeticCoding===========================================//
void kernel_2_CreateTokens_with_isFinal(
		hls::stream<uint16_t> &str_mb_h,
		hls::stream<uint16_t> &str_mb_w,
		hls::stream<ap_uint<1> > &str_type_mb,
		hls::stream<ap_uint<64> > &str_0_dc,
		hls::stream<ap_uint<64> > &str_1_dc,
		hls::stream<ap_uint<64> > &str_2_dc,
		hls::stream<ap_uint<64> > &str_3_dc,
		hls::stream<ap_uint<64> > &str_0_ac,
		hls::stream<ap_uint<64> > &str_1_ac,
		hls::stream<ap_uint<64> > &str_2_ac,
		hls::stream<ap_uint<64> > &str_3_ac,
		hls::stream<ap_uint<64> > &str_0_uv,
		hls::stream<ap_uint<64> > &str_1_uv,
		hls::stream<ap_uint<64> > &str_2_uv,
		hls::stream<ap_uint<64> > &str_3_uv,
		hls::stream<uint16_t> &str_mb_h_out,
		hls::stream<uint16_t> &str_mb_w_out,
		hls::stream<ap_uint<16> > &str_tokens_final);

//==================================kernel_2_ArithmeticCoding===========================================//
void RecordTokens_nrd2_mb_r_str_AddFinal(
		hls::stream<ap_uint<1> > &str_type_mb,
		hls::stream<ap_uint<64> > &str_0_dc,
		hls::stream<ap_uint<64> > &str_1_dc,
		hls::stream<ap_uint<64> > &str_2_dc,
		hls::stream<ap_uint<64> > &str_3_dc,
		hls::stream<ap_uint<64> > &str_0_ac,
		hls::stream<ap_uint<64> > &str_1_ac,
		hls::stream<ap_uint<64> > &str_2_ac,
		hls::stream<ap_uint<64> > &str_3_ac,
		hls::stream<ap_uint<64> > &str_0_uv,
		hls::stream<ap_uint<64> > &str_1_uv,
		hls::stream<ap_uint<64> > &str_2_uv,
		hls::stream<ap_uint<64> > &str_3_uv,
		hls::stream<ap_uint<16> > &tokens,
		bool isFinal);

//==================================kernel_2_ArithmeticCoding===========================================//
int VP8RecordCoeffTokens_hls_r_str_AddFanel(
		hls::stream<ap_uint<64> > &str_0,
		hls::stream<ap_uint<64> > &str_1,
		hls::stream<ap_uint<64> > &str_2,
		hls::stream<ap_uint<64> > &str_3,
		hls::stream<ap_uint<16> > &tokens,
		bool isFinal);

//==================================kernel_2_ArithmeticCoding===========================================//
ap_uint<1> AddConstantToken_hls_AddFanel(
		hls::stream<ap_uint<16> > &str_tokens,
		ap_uint<1> bit,
		ap_uint<11> proba_idx,
		ap_uint<1> isFinal);
//==================================kernel_2_ArithmeticCoding===========================================//
ap_uint<1> AddToken_hls_AddFanel(
		hls::stream<ap_uint<16> > &str_tokens,
		ap_uint<1> bit,
		ap_uint<11> proba_idx,
		ap_uint<1> isFinal);
//==================================kernel_2_ArithmeticCoding===========================================//
//==================================kernel_2_ArithmeticCoding===========================================//
void VP8EmitTokens_allstr_hls_dataflow_4stages(
		uint32_t pout_bw[SIZE32_MEM_BW],
		hls::stream<ap_uint<16> > &str_token,
		uint8_t probas[4 * 8 * 3 * 11],
		ap_uint<8> &bw_range,   // = 254;      // range-1
		ap_uint<24> &bw_value,   //= 0;
		ap_int<4> &bw_nb_bits,   // = -8;
		ap_uint<32> &bw_pos,   //= 0
		ap_uint<16> &bw_run);
//==================================kernel_2_ArithmeticCoding===========================================//
void VP8EmitTokens_str_hls_4stages(
		uint32_t pout_bw[SIZE32_MEM_BW],
		hls::stream<ap_uint<16> > &str_token,
		uint8_t probas[4 * 8 * 3 * 11]);
//==================================kernel_2_ArithmeticCoding===========================================//
ap_uint<8> hls_AC_range_str(
    hls::stream<ap_uint<16> > &str_token,
    uint8_t probas[4 * 8 * 3 * 11],
    hls::stream<ap_uint<2 + 3 + 8> > &str_fnl_bit_shift_split_1);
//==================================kernel_2_ArithmeticCoding===========================================//
ap_uint<4 + 24> hls_AC_value_str(
		hls::stream<ap_uint<2 + 3 + 8> > &str_fnl_bit_shift_split_1,
		hls::stream<ap_uint<18> > &str_fnl_isBit_Bits) ;
//==================================kernel_2_ArithmeticCoding===========================================//
ap_uint<16> VP8PutBit_hls_BytePackage_str_run(
		hls::stream<ap_uint<18> > &str_Last_isBit_Bits,
		hls::stream<ap_uint<26> > &str_isFinal_run_cy_pre);
//==================================kernel_2_ArithmeticCoding===========================================//
ap_uint<32> VP8PutBit_hls_BytePackage_str_pos(
		hls::stream<ap_uint<26> > &str_isFinal_run_cy_pre,
		hls::stream<ap_uint<9> > &str_Last_byte) ;

/////template PackStr2Mem_t////////////////////////
template < int W_STR,int B_LAST, int N_BURST>
int PackStr2Mem_t(
		uint32_t* pdes,
		hls::stream<ap_uint<W_STR> > &str_s)
{
	const int N_BYTE = ((W_STR-1+7)/8);
	const int N_PACK = (4/N_BYTE);
	uint32_t* ptmp = pdes;
	int num_w = 0;
	ap_uint<1> isLast = 0;
	uint32_t buff[512];
#ifndef __SYNTHESIS__
	assert(N_BURST<=512);
#endif
	ap_uint<N_BYTE * 8 * N_PACK> tmp;
	//hls::stream<ap_uint<1>> str_last_buff;
	do {
		for (int i = 0; i < N_BURST * N_PACK; i++) {
#pragma HLS PIPELINE II=1
			ap_uint<2> bs = i % N_PACK;
			if (isLast == 0) {
				ap_uint<W_STR> w = str_s.read();
				isLast = w(W_STR-1, W_STR-1);
				tmp( bs*N_BYTE*8+W_STR-2 , bs*N_BYTE*8) = w(W_STR-2, 0);
				num_w++;
			} else
				tmp( bs*N_BYTE*8+W_STR-2 , bs*N_BYTE*8 ) = 0;
			if (bs == N_PACK - 1) {
				buff[i / N_PACK] = tmp;
				//str_last_buff.write(isLast);
			}
		}
		//memcpy((void*)ptmp, (void*)buff, N_BURST*N_PACK);
		for (int j = 0; j < N_BURST; j++)
#pragma HLS PIPELINE II=1
			ptmp[j] = buff[j];
		ptmp += N_BURST;
	} while (isLast == 0);
	return num_w;
}
//////template PackStr2Mem32_t_NoLast////////////////////////
template < int W_STR, int N_BURST>
int PackStr2Mem32_t_NoLast(
		uint32_t* pdes,
		hls::stream<ap_uint<W_STR> > &str_s,
		int num_str)
{
	const int N_BYTE = ((W_STR-1+7)/8);
	const int N_PACK = (4/N_BYTE);
	uint32_t* ptmp = pdes;
	int num_w = 0;
	ap_uint<1> isLast = 0;
	uint32_t buff[512];
#ifndef __SYNTHESIS__
	assert(N_BURST<=512);
#endif
	ap_uint<N_BYTE * 8 * N_PACK> tmp;
	//hls::stream<ap_uint<1>> str_last_buff;
	do {
		for (int i = 0; i < N_BURST * N_PACK; i++) {
#pragma HLS PIPELINE II=1
			ap_uint<2> bs = i % N_PACK;
			if (isLast == 0) {
				ap_uint<W_STR> w = str_s.read();
				if(num_w == num_str-1)
					isLast=1;
				tmp( bs*N_BYTE*8+W_STR-1 , bs*N_BYTE*8) = w(W_STR-1, 0);
				num_w++;
			} else
				tmp( bs*N_BYTE*8+W_STR-1 , bs*N_BYTE*8 ) = 0;
			if (bs == N_PACK - 1) {
				buff[i / N_PACK] = tmp;
			}
		}
		for (int j = 0; j < N_BURST; j++)
#pragma HLS PIPELINE II=1
			ptmp[j] = buff[j];
		ptmp += N_BURST;
	} while (isLast == 0);
	return num_w;
}
//////template PackWideStr2Mem32_t_NoLast////////////////////////
template < int W_STR, int W_BURST>
int PackWideStr2Mem32_t_NoLast(
		uint32_t* pdes,
		hls::stream<ap_uint<64> > &str_w,
		int num_str)
{
	uint32_t buff[512];
	const int NUM_B32 = (W_STR+31)>>5; // 8*4/8 = 4;
#ifndef __SYNTHESIS__
	assert(W_BURST*NUM_B32<=512);
#endif

	for(int num_w=0;  num_w< num_str; ){
		uint16_t offset = 0;
		for(int n_wb = 0; n_wb < W_BURST; n_wb++)
		{
#pragma HLS PIPELINE II=1
			ap_uint<W_STR> w;
			if(num_w< num_str){
				w= str_w.read();
				num_w++;
			}else
				w = 0;

			for( int n_b32 = 0; n_b32< NUM_B32; n_b32++)
#pragma HLS PIPELINE II=1
				buff[offset++] = w((n_b32<<5)+31, n_b32<<5);
		}//n_wb
		memcpy(pdes, buff, offset*WD_BUS_BYTE);
		pdes+=offset;
	}//num_w
}

#endif
