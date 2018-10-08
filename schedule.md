# Hailde Boilerplate

## 構文基本

```cs
Func output;                 // 関数名の宣言
Var c, x, y;                 // 次元を指定する変数（最大4次元）
Expr e = x + y;              // 式の宣言

Buffer<int> src(hoge, channnels, width, height); // int*からの変換
output(c, x, y) = src(c, x, y) + e;              // ロジックの記述
// src(c, x, y) + (x + y); と等価     

output.realize(channnels, width, height);
```

## Halide用関数

- select(a, b, c) : 条件分岐 a ? b : c と等価
- sum : 加算、+= で記載もできる
- clamped(a, b, c) :
- max(a, b) : 大きいほうの数値を返す
- min(a, b) : 小さいほうの数値を返す
- cast<T> : キャスト
- saturating_cast : クランプ付きのキャスト
- fast_exp

etc...

キャストはちゃんと行いましょう

## 境界条件

```cs
Func filter(Func input) {

  Func f = BoundaryConditions::constant_exterior(input, 0); // 境界条件の指定

  Func output;
  Var x, y;
  RDom rx(-1, 3); // リダクションドメイン, -1, 0, 1に変化するループ変数
  RDom ry(-1, 3); // リダクションドメイン

  output(x, y) = sum(f(x + rx, y + ry));

  // RDom r(-1, 3, -1, 3);
  // output(x, y) = sum(input(x + r.x, y + r.y));

}
```

境界条件の種類

- repeat_edge : 画面端のコピー
- mirror_image : 画面端で折り返し（最端もコピー）
- mirror_interior : 画面端で折り返し（最端はコピーしないで二画素目からコピー）
- repeat_image : 反対端をコピー（タイル状に画像が並んでいるイメージ）
- constant_exterior : 固定値

## スケジューリング/最適化

```cs
Func filter(Func input) {
  Func blur_x, blur_y;
  Var x, y;
  blur_x(x, y) = (input(x - 1, y) + input(x, y) + input(x + 1, y))/3;
  blur_y(x, y) = (blur_x(x, y - 1) + blur_x(x, y) + blur_x(x, y + 1))/3;

  if(target.has_gpu_feature()){

    //GPU用のスケジュール
    Var tx, ty;
    blur_y.gpu_tile(x, y, tx, ty, 32, 8);

  } else {

    //CPU用のスケジュール
    blur_y.tile(x, y, xi, yi, 256, 32).vectorize(xi, 8).parallel(y);
    blur_x.compute_at(blur_y, x).store_at(blur_y, x).vectorize(x, 8);  

  }
  return blury;
}
```

### 各命令

#### 計算タイミング

- compute_inline : (デフォルト)
- compute_root : メモリ確保（計算量削減）
- compute_at : 必要領域のみ計算結果メモリ確保

> blur_x.compute_root() : blur_y評価前に全領域計算される  
> blur_x.compute_at(blur_y, x) :

#### 並列化

- parallel(x) : 横に二分割
- parallel(y) : 縦に二分割

#### ベクトル化

SIMD命令の使用

```cs

f(x) = g(x) + h(x);
f.vectorize(x, 4); //四要素同時計算

 ```

#### ループ展開

分岐命令、レジスタブロッキングの削減

```cs
f(x, y) = x + y;
f.unroll(x, 2);

//展開後
for(int y=0;y<h;y++)
 for(int x=0;x<w;x+=1){
   f[y][x] = x + y;
   f[y][x+1] = x+1 + y;
}
```

#### タイル化

データの再利用（キャッシュ）性の向上

- tile : box分割したループ
- split : ループの分割
- reorder : ループ順の並び替え
- fuse : ループのマージ

```cs
f(x, y) = in(x, y);
h(x, y) = in(x, y);

f.tile(x, y, xi, yi, 16, 16); // 16x16のwindow毎に処理

// f.split(x, x_outer, x_inner, 16);
// f.split(y, y_outer, y_inner, 16);
// f.reorder(x_inner, y_inner, x_outer, y_outer);
// と
// f..tile(x, y, x_outer, y_outer, x_inner, y_inner, 16, 16);
// は等価

h.split(y,yo,yi,4);
 // yのループを最外 yo > yi最内ループに分割
 // 最内ループが4回ループ
h.reorder(yi,x,yo);　//最外 yo > x > yi 最内ループとなる
h.unroll(yi); //yiループを展開
h.parallel(yo);
```

#### エリア指定

```cs
f(x, y) = in(x, y);
Buffer<int> shifted(5, 7);
shifted.set_min(100, 50);
```

### 方針
