## Pycer: Free Of Retract

一种缩进**不敏感**的基于Python的编程语言。

### 背景

你是否习惯了书写C、C++、Java代码，在书写Python代码时感到种种不便？

你是否厌倦了无止境的缩进地狱，是否厌倦了在非代码块中复制代码后的格式紊乱？

你需要**Pycer**语言：用C-like的方式书写Python代码。

Pycer的名字来源于Python和parser，这意味着它是一种关于Python的语法解析器。同时，它将parser中的字母**s**改为字母**c**，意味着它遵循C-like的语法风格。

### 下载

请将仓库克隆到本地即可。

```bash
git clone https://github.com/MicRev/Pycer.git
```

### 使用方法

#### 语法规范

Pycer的语法像是Python与C的结合体：它的关键字是Python的，而语法结构更像是C的。Pycer保留了Python的大多数优点和语法，但关于缩进的部分被改为了大括号的结构。更详细地来说，这意味着以下几点：

- 语句后应该加**分号**。多数情况下这在语法层面并不严格，但我们推荐您这么做。另外，如果语句含有以下Python关键字，一定要使用分号作为结尾;

```
assert
lambda
return
yield
```

- 所有需要使用冒号换行缩进的场合，均被替换为不需要语法层面缩进的**大括号**。如

```
class A {
def __init__(self) {
pass;
}
}
```

当然，出于可读性考虑，您也可以这样写：

```
class A {
    def __init__(self) {
        pass;
    }
}
```

**前大括号不能换行**。即，以下形式是不合法的：

```
class A
{
    def __init__(self)
    {
        pass;
    }
}
```

另外，与C语言不同，类声明的后大括号之后不需要加分号;

- 以下Python关键字后，需要用**小括号**将相关条件包裹：

```
if
elif
while
for
except
```

如

```
for (i in range(10)) {
    if (i > 5) {
        pass;
    }
    elif (i > 3) {
        pass;
    }
}
```

但是，这并不会限制**三元表达式**和**列表推导式**的使用。也就是说，以下语句依然是合法的

```
array = [i if i % 2 == 0 else -1 for i in range(10)];
```

然而，三元表达式和列表推导式中将不能显式地使用集合和字典。也就是说，三元表达式和列表推导式中不能包含语法的大括号。

遵循以上语法规范，然后像写`Python`那样书写`Pycer`。您需要将代码保存在`.pyp`文件中。

#### 运行方法

请确保您的C++编译器和Python环境可以正常运行。

首先编译语法解析器Pycer。Pycer使用C++语言构建，您可以在本地用gcc等编译器编译。

gcc/g++编译器:
```bash
gcc ./Pycer/src/Pycer.cpp -o Pycer
```

其他编译器也可类似编译。

接着将`.pyp`文件解析为Python代码。如，若要解析您的`helloworld.pyp`文件，

```bash
Pycer helloworld.pyp
```

这会在您的当前文件夹下生成一个`helloworld.py`文件，它可以直接被您本地的Python解释器运行。

```bash
python helloworld.py
```