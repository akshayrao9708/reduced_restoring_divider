module divider(Q, R_n1, R_0, D);
  wire _0_;
  wire _1_;
  wire _2_;
  wire _3_;
  wire _4_;
  wire _5_;
  wire _6_;
  wire _7_;
  wire _8_;
  wire _9_;
  wire _10_;
  wire _11_;
  wire _12_;
  wire _13_;
  wire _14_;
  wire _15_;
  wire _16_;
  wire _17_;
  wire _18_;
  wire _19_;
  wire _20_;
  wire _21_;
  wire _22_;
  wire _23_;
  wire _24_;
  wire _25_;
  wire _26_;
  wire _27_;
  wire _28_;
  wire _29_;
  wire _30_;
  wire _31_;
  wire _32_;
  wire _33_;
  wire _34_;
  wire _35_;
  wire _36_;
  wire _37_;
  wire _38_;
  wire _39_;
  wire _40_;
  wire _41_;
  wire _42_;
  wire _43_;
  wire _44_;
  wire _45_;
  wire _46_;
  wire _47_;
  wire _48_;
  wire _49_;
  wire _50_;
  wire _51_;
  wire _52_;
  wire _53_;
  wire _54_;
  wire _55_;
  wire _56_;
  wire _57_;
  wire _58_;
  wire _59_;
  wire _60_;
  wire _61_;
  wire _62_;
  wire _63_;
  wire _64_;
  wire _65_;
  wire _66_;
  wire _67_;
  wire _68_;
  wire _69_;
  wire _70_;
  wire _71_;
  wire _72_;
  wire _73_;
  wire _74_;
  wire _75_;
  wire _76_;
  wire _77_;
  wire _78_;
  wire _79_;
  wire _80_;
  wire _81_;
  wire _82_;
  wire _83_;
  wire _84_;
  wire _85_;
  wire _86_;
  wire _87_;
  wire _88_;
  wire _89_;
  wire _90_;
  wire _91_;
  wire _92_;
  wire _93_;
  wire _94_;
  wire _95_;
  wire _96_;
  wire _97_;
  wire _98_;
  wire _99_;
  wire _100_;
  wire _101_;
  wire _102_;
  wire _103_;
  wire _104_;
  wire _105_;
  wire _106_;
  wire _107_;
  wire _108_;
  wire _109_;
  wire _110_;
  wire _111_;
  wire [6:0] r_0;
  wire [6:0] r_1;
  wire [3:0] m_1;
  wire [3:0] sum_1;
  wire [3:0] inv_1;
  wire [5:0] r_2;
  wire [3:0] m_2;
  wire [3:0] sum_2;
  wire [3:0] inv_2;
  wire [4:0] r_3;
  wire [3:0] m_3;
  wire [3:0] sum_3;
  wire [3:0] inv_3;
  wire [3:0] r_4;
  wire [3:0] m_4;
  wire [3:0] sum_4;
  wire [3:0] inv_4;
  wire zeroWire;
  wire oneWire;
  input [5:0] R_0;
  input [2:0] D;
  output [3:0] Q;
  output [3:0] R_n1;
  assign zeroWire = 1'b0 /*0*/;
  assign oneWire = 1'b1 /*0*/;
  assign r_0[0] = R_0[0] /*267*/;
  assign r_0[1] = R_0[1] /*266*/;
  assign r_0[2] = R_0[2] /*265*/;
  assign r_0[3] = R_0[3] /*264*/;
  assign r_0[4] = R_0[4] /*263*/;
  assign r_0[5] = R_0[5] /*262*/;
  assign r_0[6] = zeroWire /*261*/;
  assign inv_1[0] = ~D[0] /*260*/;
  assign inv_1[1] = ~D[1] /*259*/;
  assign inv_1[2] = ~D[2] /*258*/;
  assign inv_1[3] = oneWire /*257*/;
  assign _0_ = inv_1[0] ^ r_0[3] /*255*/;
  assign sum_1[0] = _0_ ^ oneWire /*254*/;
  assign _1_ = _0_ & oneWire /*253*/;
  assign _2_ = inv_1[0] & r_0[3] /*252*/;
  assign _3_ = _1_ | _2_ /*251*/;
  assign _4_ = inv_1[1] ^ r_0[4] /*249*/;
  assign sum_1[1] = _4_ ^ _3_ /*248*/;
  assign _5_ = _4_ & _3_ /*247*/;
  assign _6_ = inv_1[1] & r_0[4] /*246*/;
  assign _7_ = _5_ | _6_ /*245*/;
  assign _8_ = inv_1[2] ^ r_0[5] /*244*/;
  assign sum_1[2] = _8_ ^ _7_ /*243*/;
  assign _9_ = _8_ & _7_ /*242*/;
  assign _10_ = inv_1[2] & r_0[5] /*241*/;
  assign _11_ = _9_ | _10_ /*240*/;
  assign _12_ = inv_1[3] ^ r_0[6] /*239*/;
  assign sum_1[3] = _12_ ^ _11_ /*238*/;
  assign _13_ = _12_ & _11_ /*237*/;
  assign _14_ = inv_1[3] & r_0[6] /*236*/;
  assign _15_ = _13_ | _14_ /*235*/;
  assign Q[3] = ~sum_1[3] /*233*/;
  assign _16_ = ~Q[3] /*232*/;
  assign _17_ = sum_1[0] & Q[3] /*231*/;
  assign _18_ = m_1[0] & _16_ /*230*/;
  assign r_1[3] = _18_ | _17_ /*229*/;
  assign m_1[0] = r_0[3] /*228*/;
  assign _19_ = ~Q[3] /*227*/;
  assign _20_ = sum_1[1] & Q[3] /*226*/;
  assign _21_ = m_1[1] & _19_ /*225*/;
  assign r_1[4] = _21_ | _20_ /*224*/;
  assign m_1[1] = r_0[4] /*223*/;
  assign _22_ = ~Q[3] /*222*/;
  assign _23_ = sum_1[2] & Q[3] /*221*/;
  assign _24_ = m_1[2] & _22_ /*220*/;
  assign r_1[5] = _24_ | _23_ /*219*/;
  assign m_1[2] = r_0[5] /*218*/;
  assign _25_ = ~Q[3] /*217*/;
  assign _26_ = sum_1[3] & Q[3] /*216*/;
  assign _27_ = m_1[3] & _25_ /*215*/;
  assign r_1[6] = _27_ | _26_ /*214*/;
assign m_1[3] = r_0[6] /*213*/;
assign r_1[0]= r_0[0] /*212*/;
assign r_1[1]= r_0[1] /*211*/;
assign r_1[2]= r_0[2] /*210*/;
  assign inv_2[0] = ~D[0] /*204*/;
  assign inv_2[1] = ~D[1] /*203*/;
  assign inv_2[2] = ~D[2] /*202*/;
assign inv_2[3] = oneWire /*201*/;
  assign _28_ = inv_2[0] ^ r_1[2] /*199*/;
  assign sum_2[0] = _28_ ^ oneWire /*198*/;
  assign _29_ = _28_ & oneWire /*197*/;
  assign _30_ = inv_2[0] & r_1[2] /*196*/;
  assign _31_ = _29_ | _30_ /*195*/;
  assign _32_ = inv_2[1] ^ r_1[3] /*193*/;
  assign sum_2[1] = _32_ ^ _31_ /*192*/;
  assign _33_ = _32_ & _31_ /*191*/;
  assign _34_ = inv_2[1] & r_1[3] /*190*/;
  assign _35_ = _33_ | _34_ /*189*/;
  assign _36_ = inv_2[2] ^ r_1[4] /*188*/;
  assign sum_2[2] = _36_ ^ _35_ /*187*/;
  assign _37_ = _36_ & _35_ /*186*/;
  assign _38_ = inv_2[2] & r_1[4] /*185*/;
  assign _39_ = _37_ | _38_ /*184*/;
  assign _40_ = inv_2[3] ^ r_1[5] /*183*/;
  assign sum_2[3] = _40_ ^ _39_ /*182*/;
  assign _41_ = _40_ & _39_ /*181*/;
  assign _42_ = inv_2[3] & r_1[5] /*180*/;
  assign _43_ = _41_ | _42_ /*179*/;
  assign Q[2] = ~sum_2[3] /*177*/;
  assign _44_ = ~Q[2] /*176*/;
  assign _45_ = sum_2[0] & Q[2] /*175*/;
  assign _46_ = m_2[0] & _44_ /*174*/;
  assign r_2[2] = _46_ | _45_ /*173*/;
assign m_2[0] = r_1[2] /*172*/;
  assign _47_ = ~Q[2] /*171*/;
  assign _48_ = sum_2[1] & Q[2] /*170*/;
  assign _49_ = m_2[1] & _47_ /*169*/;
  assign r_2[3] = _49_ | _48_ /*168*/;
assign m_2[1] = r_1[3] /*167*/;
  assign _50_ = ~Q[2] /*166*/;
  assign _51_ = sum_2[2] & Q[2] /*165*/;
  assign _52_ = m_2[2] & _50_ /*164*/;
  assign r_2[4] = _52_ | _51_ /*163*/;
assign m_2[2] = r_1[4] /*162*/;
  assign _53_ = ~Q[2] /*161*/;
  assign _54_ = sum_2[3] & Q[2] /*160*/;
  assign _55_ = m_2[3] & _53_ /*159*/;
  assign r_2[5] = _55_ | _54_ /*158*/;
assign m_2[3] = r_1[5] /*157*/;
assign r_2[0]= r_1[0] /*156*/;
assign r_2[1]= r_1[1] /*155*/;
  assign inv_3[0] = ~D[0] /*148*/;
  assign inv_3[1] = ~D[1] /*147*/;
  assign inv_3[2] = ~D[2] /*146*/;
assign inv_3[3] = oneWire /*145*/;
  assign _56_ = inv_3[0] ^ r_2[1] /*143*/;
  assign sum_3[0] = _56_ ^ oneWire /*142*/;
  assign _57_ = _56_ & oneWire /*141*/;
  assign _58_ = inv_3[0] & r_2[1] /*140*/;
  assign _59_ = _57_ | _58_ /*139*/;
  assign _60_ = inv_3[1] ^ r_2[2] /*137*/;
  assign sum_3[1] = _60_ ^ _59_ /*136*/;
  assign _61_ = _60_ & _59_ /*135*/;
  assign _62_ = inv_3[1] & r_2[2] /*134*/;
  assign _63_ = _61_ | _62_ /*133*/;
  assign _64_ = inv_3[2] ^ r_2[3] /*132*/;
  assign sum_3[2] = _64_ ^ _63_ /*131*/;
  assign _65_ = _64_ & _63_ /*130*/;
  assign _66_ = inv_3[2] & r_2[3] /*129*/;
  assign _67_ = _65_ | _66_ /*128*/;
  assign _68_ = inv_3[3] ^ r_2[4] /*127*/;
  assign sum_3[3] = _68_ ^ _67_ /*126*/;
  assign _69_ = _68_ & _67_ /*125*/;
  assign _70_ = inv_3[3] & r_2[4] /*124*/;
  assign _71_ = _69_ | _70_ /*123*/;
  assign Q[1] = ~sum_3[3] /*121*/;
  assign _72_ = ~Q[1] /*120*/;
  assign _73_ = sum_3[0] & Q[1] /*119*/;
  assign _74_ = m_3[0] & _72_ /*118*/;
  assign r_3[1] = _74_ | _73_ /*117*/;
assign m_3[0] = r_2[1] /*116*/;
  assign _75_ = ~Q[1] /*115*/;
  assign _76_ = sum_3[1] & Q[1] /*114*/;
  assign _77_ = m_3[1] & _75_ /*113*/;
  assign r_3[2] = _77_ | _76_ /*112*/;
assign m_3[1] = r_2[2] /*111*/;
  assign _78_ = ~Q[1] /*110*/;
  assign _79_ = sum_3[2] & Q[1] /*109*/;
  assign _80_ = m_3[2] & _78_ /*108*/;
  assign r_3[3] = _80_ | _79_ /*107*/;
assign m_3[2] = r_2[3] /*106*/;
  assign _81_ = ~Q[1] /*105*/;
  assign _82_ = sum_3[3] & Q[1] /*104*/;
  assign _83_ = m_3[3] & _81_ /*103*/;
  assign r_3[4] = _83_ | _82_ /*102*/;
assign m_3[3] = r_2[4] /*101*/;
assign r_3[0]= r_2[0] /*100*/;
  assign inv_4[0] = ~D[0] /*92*/;
  assign inv_4[1] = ~D[1] /*91*/;
  assign inv_4[2] = ~D[2] /*90*/;
assign inv_4[3] = oneWire /*89*/;
  assign _84_ = inv_4[0] ^ r_3[0] /*87*/;
  assign sum_4[0] = _84_ ^ oneWire /*86*/;
  assign _85_ = _84_ & oneWire /*85*/;
  assign _86_ = inv_4[0] & r_3[0] /*84*/;
  assign _87_ = _85_ | _86_ /*83*/;
  assign _88_ = inv_4[1] ^ r_3[1] /*81*/;
  assign sum_4[1] = _88_ ^ _87_ /*80*/;
  assign _89_ = _88_ & _87_ /*79*/;
  assign _90_ = inv_4[1] & r_3[1] /*78*/;
  assign _91_ = _89_ | _90_ /*77*/;
  assign _92_ = inv_4[2] ^ r_3[2] /*76*/;
  assign sum_4[2] = _92_ ^ _91_ /*75*/;
  assign _93_ = _92_ & _91_ /*74*/;
  assign _94_ = inv_4[2] & r_3[2] /*73*/;
  assign _95_ = _93_ | _94_ /*72*/;
  assign _96_ = inv_4[3] ^ r_3[3] /*71*/;
  assign sum_4[3] = _96_ ^ _95_ /*70*/;
  assign _97_ = _96_ & _95_ /*69*/;
  assign _98_ = inv_4[3] & r_3[3] /*68*/;
  assign _99_ = _97_ | _98_ /*67*/;
  assign Q[0] = ~sum_4[3] /*65*/;
  assign _100_ = ~Q[0] /*64*/;
  assign _101_ = sum_4[0] & Q[0] /*63*/;
  assign _102_ = m_4[0] & _100_ /*62*/;
  assign r_4[0] = _102_ | _101_ /*61*/;
assign m_4[0] = r_3[0] /*60*/;
  assign _103_ = ~Q[0] /*59*/;
  assign _104_ = sum_4[1] & Q[0] /*58*/;
  assign _105_ = m_4[1] & _103_ /*57*/;
  assign r_4[1] = _105_ | _104_ /*56*/;
assign m_4[1] = r_3[1] /*55*/;
  assign _106_ = ~Q[0] /*54*/;
  assign _107_ = sum_4[2] & Q[0] /*53*/;
  assign _108_ = m_4[2] & _106_ /*52*/;
  assign r_4[2] = _108_ | _107_ /*51*/;
assign m_4[2] = r_3[2] /*50*/;
  assign _109_ = ~Q[0] /*49*/;
  assign _110_ = sum_4[3] & Q[0] /*48*/;
  assign _111_ = m_4[3] & _109_ /*47*/;
  assign r_4[3] = _111_ | _110_ /*46*/;
assign m_4[3] = r_3[3] /*45*/;
  assign R_n1[0] = r_4[0] /*0*/;
  assign R_n1[1] = r_4[1] /*1*/;
  assign R_n1[2] = r_4[2] /*2*/;
  assign R_n1[3] = r_4[3] /*3*/;
endmodule