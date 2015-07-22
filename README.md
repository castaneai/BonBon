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
BonBon <channel> [<bon_dll_path>]
```

地上波のチャンネル `<channel>` を 標準出力に出力します．
`<bon_dll_path>` はBonDriverのDLLファイルのパスを指定します．省略した場合は`BonDriver_PT3-T.dll`となります．
