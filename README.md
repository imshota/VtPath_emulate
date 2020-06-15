# VtPath_emulate
VtPathの再現実装に関するレポジトリ

PinやTritonを使ってVtPathの（一部を）再現実装をします。
- 論文は[Anomaly detection using call stack information](https://ieeexplore.ieee.org/document/1199328)  

再現実装は以下の通り
- 仮想パスやVPテーブルは別の形（型）で作る。
- DLLは考慮するが、signal、setjmp()/longjmp()、スレッドは実装しない。
