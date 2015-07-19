BonBon
========
WindowsにおけるBonDriver_PT3-STを利用した録画テストプログラムです．
現在は地上波のみでテストを行っています．

必要なもの
-------------
- BonDriver_PT3-ST の地上波関連一式
  - BonDriver_PT3-T.dll
  - BonDriver_PT3-ST.ini
  - BonDriver_PT3-T.ChSet.txt
  - PT3Ctrl.exe
- B25Decoder.dll (スクランブル解除用, 要B-CAS card + カードリーダー)

使い方
-----------

```
BonBon <channel> <output_file>
```

地上波のチャンネル `<channel>` を `<output_file>` に出力します．
`<output_file>`が指定されていない場合は標準出力(`stdout`)が使用されます．
