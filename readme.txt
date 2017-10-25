Parameter.txt

0.0039 0.5 0.2		//老化公式 0.0039*(0.5*t^0.2)
thershold 0.80		//邊的R^2限制
edge error 0.01		//邊的老化差限制
PLUS fixed 7		//原始的lifetime設定 加fixed代表固定的年 否則是n+此值
TIGHT 1.000001		//clock period的一個小乘數 不用改
FINAL 10		//最後refine的次數
MONTE YES		//蒙地卡羅是否開啟


AgingRate.txt
由上而下為1,2,3,4,5,10年老化
左到右為20%, 40%, 80%, 50%的老化

0.0463 0.0762 0.1073 0.0797
0.0562 0.0875 0.1217 0.0915
0.0626 0.0949 0.1312 0.0993
0.0675 0.1005 0.1383 0.1051
0.0715 0.1051 0.1442 0.1099
0.0851 0.1208 0.1641 0.1263

輸入指令格式:

./research [netlist檔名] [timing report檔名] [老化關聯性檔名] [希望lifetime] [找多少次shortlist] [每次shortlist做多少次refinement] [n+error的error值(可不輸入 預設為0.1n)]
