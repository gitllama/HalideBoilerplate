# Hailde Boilerplate

## Description

まだまだ日本語の情報が少ないHailde（ハライド）ですがBoilerplate作ってみる。

C#でGUIを作るので、DLLにします。  

## Background

ぶっちゃけ一般的な画像処理ではOpenCVでだいたい事が足り、C#erな私はOpenCVSharpを愛用させていただいています。しかし、こと特殊センサなどを使用する場合では（どんなセンサかは訊かないでください）カユイところに手が届かない事態に陥ります。

例えば、```cvtColor(src, dst, cv::COLOR_BayerBG2BGR);```なんてやろうにも、```CV_8U/CV_16U```（要は8bit/16bitの整数型）にしか対応してませんと言われてしまうと、うぬぬぬとなってしまいます。(デモザイクなんて一番面倒臭いところなのに)。

ということで、ロジックを自分で組む羽目になるのですが、C#erだとちょっと速度的にまだ無理があるかなというのと、面倒臭いC++使うにしてもちょっとでも楽をしませう、ということでチャレンジしてみた次第です。

## Usage

- HalideGenerator : DLLを生成するexeファイルを作成します
- HalideGenerated : 生成されたDLLをC#から参照できるようにラッピングします
- ConsoleApp : C#コンソール(.Net)での呼び出しテスト用

一番素直そうな、生成されたlib群を一度C++でラップしてから呼び出す方法を取っています。


### 1. HalideGenerator Setting

HalideのライブラリへのPathの設定

※プロジェクトのrootに[halide_x64]として展開した場合

```
プロパティ > C/C++ > 全般 > 追加のインクルードディレクトリ
  $(SolutionDir)halide_x64\include

プロパティ > リンカー > 全般 > 追加のインクルードディレクトリ
  $(SolutionDir)halide_x64\$(Configuration)

プロパティ > リンカー > 入力 > 追加の依存ファイル
  Halide.lib

プロパティ > ビルドイベント > ビルド前のイベント
  copy "$(SolutionDir)halide_x64\$(Configuration)\Halide.dll" "$(OutDir)\"
```

### 2. HalideGenerator Code

Logic.cpp / Logic.hにロジックを記述します。

特に意味はないのですが、個人的な趣味でロジックの組み合わせをメソッドチェーンで記述できるようにしています。

プロジェクト実行後、```main()```内の```Logic::Compile("hoge", { input }, "hoge");```記述に応じて、ライブラリが生成されます(hoge.h, hoge.lib)

### 3. HalideGenerated Setting

HalideGeneratorと同様の設定に加えて、HalideGeneratorで生成された.libのリンカー参照と.hのプロジェクト追加する。

```
プロパティ > C/C++ > 全般 > 追加のインクルードディレクトリ
  $(SolutionDir)halide_x64\include
  $(SolutionDir)x64\lib (HalideGeneratorでのlibの出力先)

プロパティ > リンカー > 全般 > 追加のインクルードディレクトリ
  $(SolutionDir)halide_x64\$(Configuration)
  $(SolutionDir)x64\lib\

プロパティ > リンカー > 入力 > 追加の依存ファイル
  Halide.lib
  HalideGenerated.lib

プロパティ > ビルドイベント > ビルド前のイベント
  lib /OUT:$(SolutionDir)x64\lib\HalideGenerated.lib $(SolutionDir)x64\lib\HalideGenerated_*.lib
```

特に考慮無しに```compile_to_static_library```、```compile_to_file```行うと付属の関数群が.lib/.objに同梱されます。そのままでは、リンク時に関数名の衝突が発生し、リンカーで落ちますので```Target```で```Target::NoRuntime```を行うこと。

リンカの設定が面倒だったのでビルドイベント前に一度libをマージしてマージ後のファイルを参照するようにしています（ワイルドカードって指定できないのでしょうか？）。また、それ以外のライブラリ依存解決も面倒だったので、プロジェクト作成時にコンソールアプリとしてして作成しDLLに変更してます。ビルド設定やスクリプトなどでもっとスマートな設定がありそうですが。

### 4. HalideGenerated Code

```cpp
#include "hogehoge.h"

DllExport void Hoge(int* src, int* dst, int width, int height, int offset)
{
	Halide::Runtime::Buffer<int> input(src, width, height, 1);
	Halide::Runtime::Buffer<int> output(dst, width, height, 1);

	int result = hogehoge(input, offset, output);
}
```

### 5. ConsoleApp Code

```cs
static class HaildeGenerated
{
    [DllImport("HalideGenerated.dll")]
    public unsafe static extern void Hoge(IntPtr src, IntPtr dst, int width, int height, int offset);
}

class Program
{
    static void Main(string[] args)
    {
        int w = 4;
        int h = 4;
        var src = new int[w * h];
        var dst = new int[w * h];

        unsafe
        {
            fixed (int* i = src)
            fixed (int* j = dst)
            {
                HaildeGenerated.Hoge(new IntPtr(i), new IntPtr(j), w, h, 5);
            }
        }
    }
}
```
