# logger使用说明
## 使用方式
> \#include "logger.hpp"


## API说明

* `LOGGER_INIT(fileName,level)`<br>
初始化logger，全局唯一。`fileName`为文件名或输出流，`level`为日志等级。
>`fileName`可以是带路径的文件名，例如`"/tmp/temp.log"`。<br>
`fileName`可以是输出流，例如`stdout`或`stderr`。<br>
`level`为日志记录等级：<br>
0 - ERROR<br>
1 - WARNING<br>
2 - INFO<br> 
3 - DEBUG<br>
示例：`LOGGER_INIT(stderr,3);`表示将日志输出到`stderr`，日志记录等级为所有.

* `LOGGER_SET(level,fileName)`<br>
设置单个级别的日志输出位置，全局属性。`fileName`和`level`的定义同上

* `LOGGER_ERROR(format, ...)`
* `LOGGER_WARNING(format, ...)`
* `LOGGER_INFO(format, ...)`
* `LOGGER_DEBUG(format, ...)`<br>
打印日志，打印4个不同的等级的日志，使用方式同C语言标准输出`printf()`.

>note: 如果单个文件内需要屏蔽某个等级的日志，可以使用宏覆盖的方式，例如:

```cpp
#include "logger.hpp"

//重新定义LOGGER_DEBUG宏可以在单个文件内屏蔽某个等级的日志
#define LOGGER_DEBUG(...)

int main()
{
    LOGGER_DEBUG("app start");
}
```

>note: 日志文件采用追加的方式，使用相同文件不会覆盖掉之前的日志。

>note: 在linux内核的系统下，`stdout`和`stderr`可以分别重定向，例如：(更详细的用法参考linux shell)

```shell
console >> ./sample 1>stdout.log 2>stderr.log
```

## 完整demo

```cpp
#include "logger.hpp"

int main()
{
    //初始化载入日志等级为3,输出到stderr
    LOGGER_INIT(stderr,3);
    //设置DEBUG级别的日志单独输出到stdout
    LOGGER_SET(3,stdout);
    //输出一条ERROR级别的日志
    LOGGER_ERROR("error code:%d",4);
    //输出一条WARNING级别的日志
    LOGGER_WARNING("warning:%s","a warning sample");

    return 0;
}
```