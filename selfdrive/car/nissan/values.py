from selfdrive.car import dbc_dict

STEER_THRESHOLD = 1.0

class CAR:
  XTRAIL = "NISSAN X-TRAIL 2017"
  LEAF = "NISSAN LEAF 2018"
  ROGUE = "NISSAN ROGUE 2019"


FINGERPRINTS = {
  CAR.XTRAIL: [
    {
      2: 5, 42: 6, 346: 6, 347: 5, 348: 8, 349: 7, 361: 8, 386: 8, 389: 8, 397: 8, 398: 8, 403: 8, 520: 2, 523: 6, 548: 8, 645: 8, 658: 8, 665: 8, 666: 8, 674: 2, 682: 8, 683: 8, 689: 8, 723: 8, 758: 3, 768: 2, 783: 3, 851: 8, 855: 8, 1041: 8, 1055: 2, 1104: 4, 1105: 6, 1107: 4, 1108: 8, 1111: 4, 1227: 8, 1228: 8, 1247: 4, 1266: 8, 1273: 7, 1342: 1, 1376: 6, 1401: 8, 1474: 2, 1497: 3, 1821: 8, 1823: 8, 1837:8, 2015: 8, 2016: 8, 2024: 8
    },
    {
      2: 5, 42: 6, 346: 6, 347: 5, 348: 8, 349: 7, 361: 8, 386: 8, 389: 8, 397: 8, 398: 8, 403: 8, 520: 2, 523: 6, 527: 1, 548: 8, 637: 4, 645: 8, 658: 8, 665: 8, 666: 8, 674: 2, 682: 8, 683: 8, 689: 8, 723: 8, 758: 3, 768: 6, 783: 3, 851: 8, 855: 8, 1041: 8, 1055: 2, 1104: 4, 1105: 6, 1107: 4, 1108: 8, 1111: 4, 1227: 8, 1228: 8, 1247: 4, 1266: 8, 1273: 7, 1342: 1, 1376: 6, 1401: 8, 1474: 8, 1497: 3,1534: 6, 1792: 8, 1821: 8, 1823: 8, 1837: 8, 1872: 8, 1937: 8, 1953: 8, 1968: 8, 2015: 8, 2016: 8, 2024: 8
    },
  ],
  CAR.LEAF: [
    {
      2: 5, 42: 6, 264: 3, 361: 8, 372: 8, 384: 8, 389: 8, 403: 8, 459: 7, 460: 4, 470: 8, 520: 1, 569: 8, 581: 8, 634: 7, 640: 8, 644: 8, 645: 8, 646: 5, 658: 8, 682: 8, 683: 8, 689: 8, 724: 6, 758: 3, 761: 2, 783: 3, 852: 8, 853: 8, 856: 8, 861: 8, 944: 1, 976: 6, 1008: 7, 1011: 7, 1057: 3, 1227: 8, 1228: 8, 1261: 5, 1342: 1, 1354: 8, 1361: 8, 1459: 8, 1477: 8, 1497: 3, 1549: 8, 1573: 6, 1821: 8, 1837: 8, 1856: 8, 1859: 8, 1861: 8, 1864: 8, 1874: 8, 1888: 8, 1891: 8, 1893: 8, 1906: 8, 1947: 8, 1949: 8, 1979: 8, 1981: 8, 2016: 8, 2017: 8, 2021: 8
    },
    {
      2: 5, 42: 6, 264: 3, 361: 8, 372: 8, 384: 8, 389: 8, 403: 8, 459: 7, 460: 4, 470: 8, 520: 1, 569: 8, 581: 8, 634: 7, 640: 8, 643: 5, 644: 8, 645: 8, 646: 5, 658: 8, 682: 8, 683: 8, 689: 8, 724: 6, 758: 3, 761: 2, 783: 3, 852: 8, 853: 8, 856: 8, 861: 8, 944: 1, 976: 6, 1008: 7, 1011: 7, 1057: 3, 1227: 8, 1228: 8, 1261: 5, 1342: 1, 1354: 8, 1361: 8, 1459: 8, 1477: 8, 1497: 3, 1549: 8, 1573: 6, 1792: 8, 1821: 8, 1837: 8, 1872: 8, 1937: 8, 1953: 8, 1968: 8, 1988: 8, 2000: 8, 2001: 8, 2004: 8, 2005: 8, 2015: 8, 2016: 8, 2017: 8
    }
  ],
  CAR.ROGUE: [
    {
      2: 5, 42: 6, 346: 6, 347: 5, 348: 8, 361: 8, 386: 8, 389: 8, 397: 8, 398: 8, 403: 8, 520: 2, 523: 6, 548: 8, 634: 7, 643: 5, 645: 8, 658: 8, 665: 8, 666: 8, 674: 2, 682: 8, 683: 8, 689: 8, 723: 8, 758: 3, 772: 8, 773: 6, 774: 7, 775: 8, 776: 6, 777: 7, 778: 6, 783: 3, 851: 8, 855: 8, 1041: 8, 1042: 8, 1055: 2, 1104: 4, 1105: 6, 1107: 4, 1108: 8, 1110: 7, 1111: 7, 1227: 8, 1228: 8, 1247: 4, 1266: 8, 1273: 7, 1342: 1, 1376: 6, 1401: 8, 1474: 2, 1497: 3, 1534: 7, 1821: 8, 1823: 8, 1837: 8, 1839: 8
    },
  ]
}

DBC = {
  CAR.XTRAIL: dbc_dict('nissan_x_trail_2017', None),
  CAR.LEAF: dbc_dict('nissan_leaf_2018', None),
  CAR.ROGUE: dbc_dict('nissan_x_trail_2017', None),
}
