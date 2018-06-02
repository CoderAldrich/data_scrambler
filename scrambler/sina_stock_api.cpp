

/*
=5 then  
stockdata=gethttp("http://hq.sinajs.cn/list=sh"&code&"")  
if not len(stockdata)=0 then stockdata=split(stockdata,chr(34))(1)  
end if  
if len(stockdata)=0 then  
stockdata="0,0,0,0,0,0,0,0,0,0,0,0"  
else  
stockdatasplit=split(stockdata,",") stockdata=""&exstock.checkstr(stockdatasplit(0))&","&stockdatasplit(1)&","&stockdatasplit(2)&","&stockdatasplit(3)&","&stockdatasplit(4)&","&stockdatasplit(5)&","&formatdatetime(""&stockdatasplit(30)&" "&stockdatasplit(31)&"",0)&""  
end if  
‘0=股票名称,1=开盘价格,2=昨收盘价格,3=当前价格,4=最高价,5=最低价,6=更新时间  
getstockdata=stockdata  
end function  
function getstockimg(code)  
dim rndnum,addnum,checkcode,imgsource  
if len(code)=5 then  
getstockimg="http://image.sinajs.cn/newchart/daily/n/sh"&code&".gif"  
end if  
imgsource="http://finance.sina.com.cn"  
case 3  
getstockimg="http://hq.gazxfe.com/stockchart/realline.chart?"&code&"&1003&SZ 500 330"  
imgsource="http://hq.gazxfe.com"  
case 4  
getstockimg="http://chartse.stockstar.com/chartserver?code="&code&""  
imgsource="http://www.stockstar.com/"  
end select  
getstockimg=split(""&getstockimg&"||"&imgsource&"","||")  
end function  
function getastockimg()  
dim rndnum,addnum,checkcode  
dim getastockimgb,imgsource  
addnum=6  
randomize:rndnum=cint(rnd*addnum)  
select case rndnum  
case 0  
getastockimg="http://202.109.106.1/gifchartse/gif/000001.gif"  
getastockimgb="http://202.109.106.1/gifchartse/gif/399001.gif"  
imgsource="http://www.stockstar.com/"  
case 1  
getastockimg="http://money.163.com/special/100.gif?C39"  
getastockimgb="http://money.163.com/special/101.gif?HrS"  
imgsource="http://www.163.com"  
case 2  
getastockimg="http://www.10jqka.com.cn/curve/realtime/index2.php?code=1a0001&w=180&h=140"  
getastockimgb="http://www.10jqka.com.cn/curve/realtime/index2.php?code=399001&w=180&h=140"  
imgsource="http://www.10jqka.com.cn"  
case 3  
getastockimg="http://chart.cnlist.com/stockchart/realline.chart?1a0001&1002&SZ 180 140"  
getastockimgb="http://chart.cnlist.com/stockchart/realline.chart?399001&1002&SZ 180 140"  
imgsource="http://chart.cnlist.com/"  
case 4  
getastockimg="http://image.sinajs.cn/newchart/small/ish000001.gif?1189176558328"  
getastockimgb="http://image.sinajs.cn/newchart/small/isz399001.gif?1189176558328"  
imgsource="http://www.sinajs.cn"  
case 5  
getastockimg="http://218.1.72.66/cgi/pic/sh/realtime/JA000001164143.png"  
getastockimgb="http://218.1.72.66/cgi/pic/sz/realtime/JA399001164143.png"  
imgsource="http://www.cnstock.com/"  
case 6  
getastockimg="http://222.73.29.85/img/000001.png"  
getastockimgb="http://222.73.29.85/img/399001.png"  
imgsource="http://www.eastmoney.com/"  
end select  
getastockimg=split(""&getastockimg&"||"&getastockimgb&"||"&imgsource&"","||")  
end function  
%>  
*/