class HONDA:
  CIVIC = "HONDA CIVIC 2016 TOURING"
  ACURA_ILX = "ACURA ILX 2016 ACURAWATCH PLUS"
  CRV = "HONDA CR-V 2016 TOURING"
  ODYSSEY = "HONDA ODYSSEY 2018 EX-L"
  ACURA_RDX = "ACURA RDX 2018 ACURAWATCH PLUS"
  PILOT = "HONDA PILOT 2017 TOURING"
  RIDGELINE = "HONDA RIDGELINE 2017 BLACK EDITION"


class TOYOTA:
  PRIUS = "TOYOTA PRIUS 2017"
  RAV4H = "TOYOTA RAV4 2017 HYBRID"
  RAV4 = "TOYOTA RAV4 2017"
  COROLLA = "TOYOTA COROLLA 2017"
  LEXUS_RXH = "LEXUS RX HYBRID 2017"
  AVALON = "TOYOTA AVALON 2017"

class GM:
  VOLT = "CHEVROLET VOLT PREMIER 2017"

_DEBUG_ADDRESS = {1880: 8}   # reserved for debug purposes

_FINGERPRINTS = {
  HONDA.ACURA_ILX: [{
    1024L: 5, 513L: 6, 1027L: 5, 1029L: 8, 929L: 4, 1057L: 5, 777L: 8, 1034L: 5, 1036L: 8, 398L: 3, 399L: 7, 145L: 8, 660L: 8, 985L: 3, 923L: 2, 542L: 7, 773L: 7, 800L: 8, 432L: 7, 419L: 8, 420L: 8, 1030L: 5, 422L: 8, 808L: 8, 428L: 8, 304L: 8, 819L: 7, 821L: 5, 57L: 3, 316L: 8, 545L: 4, 464L: 8, 1108L: 8, 597L: 8, 342L: 6, 983L: 8, 344L: 8, 804L: 8, 1039L: 8, 476L: 4, 892L: 8, 490L: 8, 1064L: 7, 882L: 2, 884L: 7, 887L: 8, 888L: 8, 380L: 8, 1365L: 5,
    # sent messages
    0xe4: 5, 0x1fa: 8, 0x200: 6, 0x30c: 8, 0x33d: 5,
  }],
  HONDA.ACURA_RDX: [{
    57L: 3, 145L: 8, 229L: 4, 308L: 5, 316L: 8, 342L: 6, 344L: 8, 380L: 8, 392L: 6, 398L: 3, 399L: 6, 404L: 4, 420L: 8, 422L: 8, 426L: 8, 432L: 7, 464L: 8, 474L: 5, 476L: 4, 487L: 4, 490L: 8, 506L: 8, 542L: 7, 545L: 4, 597L: 8, 660L: 8, 773L: 7, 777L: 8, 780L: 8, 800L: 8, 804L: 8, 808L: 8, 819L: 7, 821L: 5, 829L: 5, 882L: 2, 884L: 7, 887L: 8, 888L: 8, 892L: 8, 923L: 2, 929L: 4, 963L: 8, 965L: 8, 966L: 8, 967L: 8, 983L: 8, 985L: 3, 1024L: 5, 1027L: 5, 1029L: 8, 1033L: 5, 1034L: 5, 1036L: 8, 1039L: 8, 1057L: 5, 1064L: 7, 1108L: 8, 1365L: 5, 1424L: 5, 1729L: 1
  }],
  HONDA.CIVIC: [{
    1024L: 5, 513L: 6, 1027L: 5, 1029L: 8, 777L: 8, 1036L: 8, 1039L: 8, 1424L: 5, 401L: 8, 148L: 8, 662L: 4, 985L: 3, 795L: 8, 773L: 7, 800L: 8, 545L: 6, 420L: 8, 806L: 8, 808L: 8, 1322L: 5, 427L: 3, 428L: 8, 304L: 8, 432L: 7, 57L: 3, 450L: 8, 929L: 8, 330L: 8, 1302L: 8, 464L: 8, 1361L: 5, 1108L: 8, 597L: 8, 470L: 2, 344L: 8, 804L: 8, 399L: 7, 476L: 7, 1633L: 8, 487L: 4, 892L: 8, 490L: 8, 493L: 5, 884L: 8, 891L: 8, 380L: 8, 1365L: 5,
    # sent messages
    0xe4: 5, 0x1fa: 8, 0x200: 6, 0x30c: 8, 0x33d: 5, 0x35e: 8, 0x39f: 8,
  }],
  HONDA.CRV: [{
    57L: 3, 145L: 8, 316L: 8, 340L: 8, 342L: 6, 344L: 8, 380L: 8, 398L: 3, 399L: 6, 401L: 8, 420L: 8, 422L: 8, 426L: 8, 432L: 7, 464L: 8, 474L: 5, 476L: 4, 487L: 4, 490L: 8, 493L: 3, 507L: 1, 542L: 7, 545L: 4, 597L: 8, 660L: 8, 661L: 4, 773L: 7, 777L: 8, 800L: 8, 804L: 8, 808L: 8, 882L: 2, 884L: 7, 888L: 8, 891L: 8, 892L: 8, 923L: 2, 929L: 8, 983L: 8, 985L: 3, 1024L: 5, 1027L: 5, 1029L: 8, 1033L: 5, 1036L: 8, 1039L: 8, 1057L: 5, 1064L: 7, 1108L: 8, 1125L: 8, 1296L: 8, 1365L: 5, 1424L: 5, 1600L: 5, 1601L: 8,
    # sent messages
    0x194: 4, 0x1fa: 8, 0x30c: 8, 0x33d: 5,
  }],
  HONDA.ODYSSEY: [{
    57L: 3, 148L: 8, 228L: 5, 229L: 4, 316L: 8, 342L: 6, 344L: 8, 380L: 8, 399L: 7, 411L: 5, 419L: 8, 420L: 8, 427L: 3, 432L: 7, 450L: 8, 463L: 8, 464L: 8, 476L: 4, 490L: 8, 506L: 8, 542L: 7, 545L: 6, 597L: 8, 662L: 4, 773L: 7, 777L: 8, 780L: 8, 795L: 8, 800L: 8, 804L: 8, 806L: 8, 808L: 8, 817L: 4, 819L: 7, 821L: 5, 825L: 4, 829L: 5, 837L: 5, 856L: 7, 862L: 8, 871L: 8, 881L: 8, 882L: 4, 884L: 8, 891L: 8, 892L: 8, 905L: 8, 923L: 2, 927L: 8, 929L: 8, 963L: 8, 965L: 8, 966L: 8, 967L: 8, 983L: 8, 985L: 3, 1029L: 8, 1036L: 8, 1052L: 8, 1064L: 7, 1088L: 8, 1089L: 8, 1092L: 1, 1108L: 8, 1110L: 8, 1125L: 8, 1296L: 8, 1302L: 8, 1600L: 5, 1601L: 8, 1612L: 5, 1613L: 5, 1614L: 5, 1615L: 8, 1616L: 5, 1619L: 5, 1623L: 5, 1668L: 5
  },
  # Odyssey Elite
  {
    57L: 3, 148L: 8, 228L: 5, 229L: 4, 304L: 8, 342L: 6, 344L: 8, 380L: 8, 399L: 7, 411L: 5, 419L: 8, 420L: 8, 427L: 3, 432L: 7, 440L: 8, 450L: 8, 463L: 8, 464L: 8, 476L: 4, 490L: 8, 506L: 8, 507L: 1, 542L: 7, 545L: 6, 597L: 8, 662L: 4, 773L: 7, 777L: 8, 780L: 8, 795L: 8, 800L: 8, 804L: 8, 806L: 8, 808L: 8, 817L: 4, 819L: 7, 821L: 5, 825L: 4, 829L: 5, 837L: 5, 856L: 7, 862L: 8, 871L: 8, 881L: 8, 882L: 4, 884L: 8, 891L: 8, 892L: 8, 905L: 8, 923L: 2, 927L: 8, 929L: 8, 963L: 8, 965L: 8, 966L: 8, 967L: 8, 983L: 8, 985L: 3, 1029L: 8, 1036L: 8, 1052L: 8, 1064L: 7, 1088L: 8, 1089L: 8, 1092L: 1, 1108L: 8, 1110L: 8, 1125L: 8, 1296L: 8, 1302L: 8, 1600L: 5, 1601L: 8, 1612L: 5, 1613L: 5, 1614L: 5, 1616L: 5, 1619L: 5, 1623L: 5, 1668L: 5
  }],
  HONDA.PILOT: [{
    1600L: 5, 1027L: 5, 1668L: 5, 1029L: 8, 1601L: 8, 777L: 8, 891L: 8, 1036L: 8, 399L: 7, 1424L: 5, 145L: 8, 660L: 8, 985L: 3, 1616L: 5, 538L: 3, 795L: 8, 542L: 7, 773L: 7, 800L: 8, 545L: 5, 546L: 3, 419L: 8, 420L: 8, 422L: 8, 1064L: 7, 425L: 8, 426L: 8, 427L: 3, 432L: 7, 819L: 7, 308L: 5, 821L: 5, 57L: 3, 965L: 8, 316L: 8, 829L: 5, 1088L: 8, 1089L: 8, 963L: 8, 837L: 5, 966L: 8, 929L: 8, 780L: 8, 923L: 2, 1613L: 5, 334L: 8, 463L: 8, 464L: 8, 1618L: 5, 1108L: 8, 597L: 8, 342L: 6, 983L: 8, 856L: 7, 804L: 8, 1612L: 5, 476L: 4, 1125L: 8, 344L: 8, 1296L: 8, 379L: 8, 228L: 5, 229L: 4, 871L: 8, 892L: 8, 490L: 8, 808L: 8, 882L: 2, 884L: 7, 967L: 8, 506L: 8, 507L: 1, 380L: 8,
  }],
  HONDA.RIDGELINE: [{
    57L: 3, 145L: 8, 228L: 5, 229L: 4, 308L: 5, 316L: 8, 339L: 7, 342L: 6, 344L: 8, 380L: 8, 392L: 6, 399L: 7, 419L: 8, 420L: 8, 422L: 8, 425L: 8, 426L: 8, 427L: 3, 432L: 7, 464L: 8, 471L: 3, 476L: 4, 490L: 8, 506L: 8, 545L: 5, 546L: 3, 597L: 8, 660L: 8, 773L: 7, 777L: 8, 780L: 8, 795L: 8, 800L: 8, 804L: 8, 808L: 8, 819L: 7, 821L: 5, 829L: 5, 871L: 8, 882L: 2, 884L: 7, 892L: 8, 923L: 2, 927L: 8, 929L: 8, 963L: 8, 965L: 8, 966L: 8, 967L: 8, 983L: 8, 985L: 3, 1027L: 5, 1029L: 8, 1036L: 8, 1039L: 8, 1064L: 7, 1088L: 8, 1089L: 8, 1108L: 8, 1125L: 8, 1296L: 8, 1365L: 5, 1424L: 5, 1600L: 5, 1601L: 8, 1613L: 5, 1616L: 5, 1618L: 5, 1668L: 5, 2015L: 3
  }],
  TOYOTA.RAV4: [{
    36L: 8, 37L: 8, 170L: 8, 180L: 8, 186L: 4, 426L: 6, 452L: 8, 464L: 8, 466L: 8, 467L: 8, 547L: 8, 548L: 8, 552L: 4, 562L: 4, 608L: 8, 610L: 5, 643L: 7, 705L: 8, 725L: 2, 740L: 5, 800L: 8, 835L: 8, 836L: 8, 849L: 4, 869L: 7, 870L: 7, 871L: 2, 896L: 8, 897L: 8, 900L: 6, 902L: 6, 905L: 8, 911L: 8, 916L: 3, 918L: 7, 921L: 8, 933L: 8, 944L: 8, 945L: 8, 951L: 8, 955L: 4, 956L: 8, 979L: 2, 998L: 5, 999L: 7, 1000L: 8, 1001L: 8, 1005L: 2, 1008L: 2, 1014L: 8, 1017L: 8, 1041L: 8, 1042L: 8, 1043L: 8, 1044L: 8, 1056L: 8, 1059L: 1, 1114L: 8, 1161L: 8, 1162L: 8, 1163L: 8, 1176L: 8, 1177L: 8, 1178L: 8, 1179L: 8, 1180L: 8, 1181L: 8, 1190L: 8, 1191L: 8, 1192L: 8, 1196L: 8, 1227L: 8, 1228L: 8, 1235L: 8, 1237L: 8, 1263L: 8, 1264L: 8, 1279L: 8, 1408L: 8, 1409L: 8, 1410L: 8, 1552L: 8, 1553L: 8, 1554L: 8, 1555L: 8, 1556L: 8, 1557L: 8, 1561L: 8, 1562L: 8, 1568L: 8, 1569L: 8, 1570L: 8, 1571L: 8, 1572L: 8, 1584L: 8, 1589L: 8, 1592L: 8, 1593L: 8, 1595L: 8, 1596L: 8, 1597L: 8, 1600L: 8, 1656L: 8, 1664L: 8, 1728L: 8, 1745L: 8, 1779L: 8, 1904L: 8, 1912L: 8, 1990L: 8, 1998L: 8
  }],
  TOYOTA.RAV4H: [{
    36L: 8, 37L: 8, 170L: 8, 180L: 8, 186L: 4, 296L: 8, 426L: 6, 452L: 8, 464L: 8, 466L: 8, 467L: 8, 547L: 8, 548L: 8, 550L: 8, 552L: 4, 560L: 7, 562L: 4, 581L: 5, 608L: 8, 610L: 5, 643L: 7, 705L: 8, 713L: 8, 725L: 2, 740L: 5, 800L: 8, 835L: 8, 836L: 8, 849L: 4, 869L: 7, 870L: 7, 871L: 2, 896L: 8, 897L: 8, 900L: 6, 902L: 6, 905L: 8, 911L: 8, 916L: 3, 918L: 7, 921L: 8, 933L: 8, 944L: 8, 945L: 8, 950L: 8, 951L: 8, 953L: 3, 955L: 8, 956L: 8, 979L: 2, 998L: 5, 999L: 7, 1000L: 8, 1001L: 8, 1005L: 2, 1008L: 2, 1014L: 8, 1017L: 8, 1041L: 8, 1042L: 8, 1043L: 8, 1044L: 8, 1056L: 8, 1059L: 1, 1114L: 8, 1161L: 8, 1162L: 8, 1163L: 8, 1176L: 8, 1177L: 8, 1178L: 8, 1179L: 8, 1180L: 8, 1181L: 8, 1184L: 8, 1185L: 8, 1186L: 8, 1190L: 8, 1191L: 8, 1192L: 8, 1196L: 8, 1197L: 8, 1198L: 8, 1199L: 8, 1212L: 8, 1227L: 8, 1228L: 8, 1232L: 8, 1235L: 8, 1237L: 8, 1263L: 8, 1264L: 8, 1279L: 8, 1408L: 8, 1409L: 8, 1410L: 8, 1552L: 8, 1553L: 8, 1554L: 8, 1555L: 8, 1556L: 8, 1557L: 8, 1561L: 8, 1562L: 8, 1568L: 8, 1569L: 8, 1570L: 8, 1571L: 8, 1572L: 8, 1584L: 8, 1589L: 8, 1592L: 8, 1593L: 8, 1595L: 8, 1596L: 8, 1597L: 8, 1600L: 8, 1656L: 8, 1664L: 8, 1728L: 8, 1745L: 8, 1779L: 8, 1904L: 8, 1912L: 8, 1990L: 8, 1998L: 8
 }],
  TOYOTA.PRIUS: [{
    36L: 8, 37L: 8, 166L: 8, 170L: 8, 180L: 8, 295L: 8, 296L: 8, 426L: 6, 452L: 8, 466L: 8, 467L: 8, 550L: 8, 552L: 4, 560L: 7, 562L: 6, 581L: 5, 608L: 8, 610L: 8, 614L: 8, 643L: 7, 658L: 8, 713L: 8, 740L: 5, 742L: 8, 743L: 8, 800L: 8, 810L: 2, 814L: 8, 829L: 2, 830L: 7, 835L: 8, 836L: 8, 863L: 8, 869L: 7, 870L: 7, 871L: 2, 898L: 8, 900L: 6, 902L: 6, 905L: 8, 918L: 8, 921L: 8, 933L: 8, 944L: 8, 945L: 8, 950L: 8, 951L: 8, 953L: 8, 955L: 8, 956L: 8, 971L: 7, 975L: 5, 993L: 8, 998L: 5, 999L: 7, 1000L: 8, 1001L: 8, 1014L: 8, 1017L: 8, 1020L: 8, 1041L: 8, 1042L: 8, 1044L: 8, 1056L: 8, 1057L: 8, 1059L: 1, 1071L: 8, 1077L: 8, 1082L: 8, 1083L: 8, 1084L: 8, 1085L: 8, 1086L: 8, 1114L: 8, 1132L: 8, 1161L: 8, 1162L: 8, 1163L: 8, 1175L: 8, 1227L: 8, 1228L: 8, 1235L: 8, 1237L: 8, 1279L: 8, 1552L: 8, 1553L: 8, 1556L: 8, 1557L: 8, 1568L: 8, 1570L: 8, 1571L: 8, 1572L: 8, 1595L: 8, 1777L: 8, 1779L: 8, 1904L: 8, 1912L: 8, 1990L: 8, 1998L: 8
  },
  # Prius Prime
  {
    36L: 8, 37L: 8, 166L: 8, 170L: 8, 180L: 8, 295L: 8, 296L: 8, 426L: 6, 452L: 8, 466L: 8, 467L: 8, 550L: 8, 552L: 4, 560L: 7, 562L: 6, 581L: 5, 608L: 8, 610L: 8, 614L: 8, 643L: 7, 658L: 8, 713L: 8, 740L: 5, 742L: 8, 743L: 8, 800L: 8, 810L: 2, 814L: 8, 824L: 2, 829L: 2, 830L: 7, 835L: 8, 836L: 8, 863L: 8, 869L: 7, 870L: 7, 871L: 2,898L: 8, 900L: 6, 902L: 6, 905L: 8, 913L: 8, 918L: 8, 921L: 8, 933L: 8, 944L: 8, 945L: 8, 950L: 8, 951L: 8, 953L: 8, 955L: 8, 956L: 8, 971L: 7, 974L: 8, 975L: 5, 993L: 8, 998L: 5, 999L: 7, 1000L: 8, 1001L: 8, 1014L: 8, 1017L: 8, 1020L: 8, 1041L: 8, 1042L: 8, 1044L: 8, 1056L: 8, 1057L: 8, 1059L: 1, 1071L: 8, 1076L: 8, 1077L: 8, 1082L: 8, 1083L: 8, 1084L: 8, 1085L: 8, 1086L: 8, 1114L: 8, 1132L: 8, 1161L: 8, 1162L: 8, 1163L: 8, 1164L: 8, 1165L: 8, 1166L: 8, 1167L: 8, 1175L: 8, 1227L: 8, 1228L: 8, 1235L: 8, 1237L: 8, 1279L: 8, 1552L: 8, 1553L: 8, 1556L: 8, 1557L: 8, 1568L: 8, 1570L: 8, 1571L: 8, 1572L: 8, 1595L: 8, 1777L: 8, 1779L: 8, 1904L: 8, 1912L: 8, 1990L: 8, 1998L: 8
  },
  # Taiwanese Prius Prime
  {
    36L: 8, 37L: 8, 166L: 8, 170L: 8, 180L: 8, 295L: 8, 296L: 8, 426L: 6, 452L: 8, 466L: 8, 467L: 8, 550L: 8, 552L: 4, 560L: 7, 562L: 6, 581L: 5, 608L: 8, 610L: 8, 614L: 8, 643L: 7, 658L: 8, 713L: 8, 740L: 5, 742L: 8, 743L: 8, 800L: 8, 810L: 2, 814L: 8, 824L: 2, 829L: 2, 830L: 7, 835L: 8, 836L: 8, 845L: 5, 863L: 8, 869L: 7, 870L: 7, 871L: 2,898L: 8, 900L: 6, 902L: 6, 905L: 8, 913L: 8, 918L: 8, 921L: 8, 933L: 8, 944L: 8, 945L: 8, 950L: 8, 951L: 8, 953L: 8, 955L: 8, 956L: 8, 971L: 7, 974L: 8, 975L: 5, 993L: 8, 998L: 5, 999L: 7, 1000L: 8, 1001L: 8, 1005L: 2, 1014L: 8, 1017L: 8, 1020L: 8, 1041L: 8, 1042L: 8, 1044L: 8, 1056L: 8, 1057L: 8, 1059L: 1, 1071L: 8, 1076L: 8, 1077L: 8, 1082L: 8, 1083L: 8, 1084L: 8, 1085L: 8, 1086L: 8, 1114L: 8, 1132L: 8, 1161L: 8, 1162L: 8, 1163L: 8, 1164L: 8, 1165L: 8, 1166L: 8, 1167L: 8, 1175L: 8, 1227L: 8, 1228L: 8, 1235L: 8, 1237L: 8, 1264L: 8, 1279L: 8, 1552L: 8, 1553L: 8, 1556L: 8, 1557L: 8, 1568L: 8, 1570L: 8, 1571L: 8, 1572L: 8, 1595L: 8, 1777L: 8, 1779L: 8, 1904L: 8, 1912L: 8, 1990L: 8, 1998L: 8
  }],
  TOYOTA.COROLLA: [{
    36: 8, 37: 8, 170: 8, 180: 8, 186: 4, 426: 6, 452: 8, 464: 8, 466: 8, 467: 8, 547: 8, 548: 8, 552: 4, 608: 8, 610: 5, 643: 7, 705: 8, 740: 5, 800: 8, 835: 8, 836: 8, 849: 4, 869: 7, 870: 7, 871: 2, 896: 8, 897: 8, 900: 6, 902: 6, 905: 8, 911: 8, 916: 2, 921: 8, 933: 8, 944: 8, 945: 8, 951: 8, 955: 4, 956: 8, 979: 2, 992: 8, 998: 5, 999: 7, 1000: 8, 1001: 8, 1017: 8, 1041: 8, 1042: 8, 1043: 8, 1044: 8, 1056: 8, 1059: 1, 1114: 8, 1161: 8, 1162: 8, 1163: 8, 1196: 8, 1227: 8, 1235: 8, 1279: 8, 1552: 8, 1553: 8, 1556: 8, 1557: 8, 1561: 8, 1562: 8, 1568: 8, 1569: 8, 1570: 8, 1571: 8, 1572: 8, 1584: 8, 1589: 8, 1592: 8, 1596: 8, 1597: 8, 1600: 8, 1664: 8, 1728: 8, 1779: 8, 1904: 8, 1912: 8, 1990: 8, 1998: 8
  },
  # Corolla LE 2017
  {
    36: 8, 37: 8, 170: 8, 180: 8, 186: 4, 426: 6, 452: 8, 464: 8, 466: 8, 467: 8, 547: 8, 548: 8, 552: 4, 608: 8, 610: 5, 643: 7, 705: 8, 740: 5, 800: 8, 835: 8, 836: 8, 849: 4, 869: 7, 870: 7, 871: 2, 896: 8, 897: 8, 900: 6, 902: 6, 905: 8, 911: 8, 916: 2, 921: 8, 933: 8, 944: 8, 945: 8, 951: 8, 955: 4, 956: 8, 979: 2, 998: 5, 999: 7, 1000: 8, 1001: 8, 1017: 8, 1041: 8, 1042: 8, 1043: 8, 1044: 8, 1056: 8, 1059: 1, 1114: 8, 1161: 8, 1162: 8, 1163: 8, 1196: 8, 1227: 8, 1235: 8, 1279: 8, 1552: 8, 1553: 8, 1556: 8, 1557: 8, 1561: 8, 1562: 8, 1568: 8, 1569: 8, 1570: 8, 1571: 8, 1572: 8, 1592: 8, 1596: 8, 1597: 8, 1600: 8, 1664: 8, 1779: 8, 1904: 8, 1912: 8, 1990: 8, 1998: 8, 2016: 8, 2017: 8, 2018: 8, 2019: 8, 2020: 8, 2021: 8, 2022: 8, 2023: 8, 2024: 8
  }],
  TOYOTA.LEXUS_RXH: [{
    36: 8, 37: 8, 166: 8, 170: 8, 180: 8, 295: 8, 296: 8, 426: 6, 452: 8, 466: 8, 467: 8, 550: 8, 552: 4, 560: 7, 562: 6, 581: 5, 608: 8, 610: 5, 643: 7, 658: 8, 713: 8, 740: 5, 742: 8, 743: 8, 800: 8, 810: 2, 812: 3, 814: 8, 830: 7, 835: 8, 836: 8, 845: 5, 863: 8, 869: 7, 870: 7, 871: 2, 898: 8, 900: 6, 902: 6, 905: 8, 913: 8, 918: 8, 921: 8, 933: 8, 944: 8, 945: 8, 950: 8, 951: 8, 953: 8, 955: 8, 956: 8, 971: 7, 975: 6, 993: 8, 998: 5, 999: 7, 1000: 8, 1001: 8, 1005: 2, 1014: 8, 1017: 8, 1020: 8, 1041: 8, 1042: 8, 1044: 8, 1056: 8, 1059: 1, 1063: 8, 1071: 8, 1077: 8, 1082: 8, 1114: 8, 1161: 8, 1162: 8, 1163: 8, 1164: 8, 1165: 8, 1166: 8, 1167: 8, 1227: 8, 1228: 8, 1235: 8, 1237: 8, 1264: 8, 1279: 8, 1552: 8, 1553: 8, 1556: 8, 1557: 8, 1568: 8, 1570: 8, 1571: 8, 1572: 8, 1575: 8, 1595: 8, 1777: 8, 1779: 8, 1808: 8, 1810: 8, 1816: 8, 1818: 8, 1840: 8, 1848: 8, 1904: 8, 1912: 8, 1940: 8, 1941: 8, 1948: 8, 1949: 8, 1952: 8, 1956: 8, 1960: 8, 1964: 8, 1986: 8, 1990: 8, 1994: 8, 1998: 8, 2004: 8, 2012: 8
  }],
  TOYOTA.AVALON: [{
   57: 3, 145: 8, 228: 5, 229: 4, 308: 5, 316: 8, 339: 7, 342: 6, 344: 8, 380: 8, 392: 6, 399: 7, 419: 8, 420: 8, 422: 8, 425: 8, 426: 8, 427: 3, 432: 7, 464: 8, 476: 4, 490: 8, 506: 8, 545: 5, 546: 3, 597: 8, 660: 8, 773: 7, 777: 8, 780: 8, 795: 8, 800: 8, 804: 8, 808: 8, 819: 7, 821: 5, 829: 5, 871: 8, 882: 2, 884: 7, 892: 8, 923: 2, 929: 8, 963: 8, 965: 8, 966: 8, 967: 8, 983: 8, 985: 3, 1027: 5, 1029: 8, 1036: 8, 1039: 8, 1064: 7, 1088: 8, 1089: 8, 1108: 8, 1125: 8, 1296: 8, 1365: 5, 1424: 5, 1613: 5, 1616: 5, 1618: 5, 1668: 5
  }],
TOYOTA.AVALON: {
    36: 8, 37: 8, 170: 8, 180: 8, 426: 6, 452: 8, 464: 8, 466: 8, 467: 8, 547: 8, 550: 8, 552: 4, 562: 6, 608: 8, 610: 5, 643: 7, 705: 8, 740: 5, 800: 8, 835: 8, 836: 8, 849: 4, 869: 7, 870: 7, 871: 2, 896: 8, 897: 8, 911: 1, 916: 2, 921: 8, 944: 8, 945: 8, 951: 8, 955: 8, 956: 8, 979: 2, 1017: 8, 1041: 8, 1042: 8, 1044: 8, 1056: 8, 1059: 1, 1114: 8, 1161: 8, 1162: 8, 1163: 8, 1176: 8, 1177: 8, 1178: 8, 1179: 8, 1180: 8, 1181: 8, 1190: 8, 1191: 8, 1192: 8, 1196: 8, 1200: 8, 1201: 8, 1202: 8, 1203: 8, 1206: 8, 1227: 8, 1235: 8, 1279: 8, 1552: 8, 1553: 8, 1554: 8, 1555: 8, 1556: 8, 1557: 8, 1558: 8, 1561: 8, 1562: 8, 1568: 8, 1569: 8, 1570: 8, 1571: 8, 1572: 8, 1584: 8, 1589: 8, 1592: 8, 1593: 8, 1595: 8, 1596: 8, 1597: 8, 1664: 8, 1728: 8, 1779: 8, 1904: 8, 1912: 8, 1990: 8, 1998: 8
 },

  GM.VOLT: [{
    170: 8, 171: 8, 189: 7, 190: 6, 193: 8, 197: 8, 199: 4, 201: 8, 209: 7, 211: 2, 241: 6, 288: 5, 289: 8, 298: 8, 304: 1, 308: 4, 309: 8, 311: 8, 313: 8, 320: 3, 328: 1, 352: 5, 381: 6, 386: 8, 388: 8, 389: 2, 390: 7, 417: 7, 419: 1, 426: 7, 451: 8, 452: 8, 453: 6, 454: 8, 456: 8, 479: 3, 481: 7, 485: 8, 489: 8, 493: 8, 495: 4, 497: 8, 499: 3, 500: 6, 501: 8, 508: 8, 528: 4, 532: 6, 546: 7, 550: 8, 554: 3, 558: 8, 560: 8, 562: 8, 563: 5, 564: 5, 565: 5, 566: 5, 567: 3, 568: 1, 573: 1, 577: 8, 647: 3, 707: 8, 711: 6, 761: 7, 810: 8, 840: 5, 842: 5, 844: 8, 866: 4, 961: 8, 969: 8, 977: 8, 979: 7, 988: 6, 989: 8, 995: 7, 1001: 8, 1005: 6, 1009: 8, 1017: 8, 1019: 2, 1020: 8, 1105: 6, 1187: 4, 1217: 8, 1221: 5, 1223: 3, 1225: 7, 1227: 4, 1233: 8, 1249: 8, 1257: 6, 1265: 8, 1267: 1, 1273: 3, 1275: 3, 1280: 4, 1300: 8, 1322: 6, 1323: 4, 1328: 4, 1417: 8, 1601: 8, 1905: 7, 1906: 7, 1907: 7, 1910: 7, 1912: 7, 1922: 7, 1927: 7, 1928: 7, 2016: 8, 2020: 8, 2024: 8, 2028: 8
  }],
}

# support additional internal only fingerprints
try:
  from common.fingerprints_internal import add_additional_fingerprints
  add_additional_fingerprints(_FINGERPRINTS)
except ImportError:
  pass


def is_valid_for_fingerprint(msg, car_fingerprint):
  adr = msg.address
  return msg.src != 0 or (adr in car_fingerprint and car_fingerprint[adr] == len(msg.dat))


