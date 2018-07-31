> kson

```
kson是对json进行简化的数据传输格式，说不定会用于我的各种项目中，谁知道呢？

kson支持格式：
object, array, string, number, bool, null，其中bool类型值为true(TRUE)或false(FALSE)。

object: 
以 '{' 开始，以 '}' 结束，内部是多个 key/value 对。

key/value:
key不用引号包括例如：a : "string", b : 123, c : { ... }, d : [...]。最后不能由逗号 ','。
key的命名规则同C语言变量，即允许字母/数字/下划线，但是首字符不能是数字

array:
以 '[' 开始，以 ']' 结束，内部是多个 value。
如: ["abc", 123, true, {...}, [...]]

string:
以 '"' 开始，以 '"' 结束，内部是 ASCII 码为 0~127 的（键盘上有的）可见字符、空格、少量转义字符（\n, \t，这个可以随时加）。

number:
数字支持十进制或十六进制整数，浮点数（必须带小数点，且小数点后必须有数字，都以double类型进行存储），都可以使用科学计数法表示。

bool:
支持 true（TRUE）和 false（FALSE）。

null:
就是符号集合 null（NULL），表示空，实际存储为空指针 (void*)(nullptr)。
```

> kson 格式定义 (不规范的 EBNF 文法，意会)

```
// object 由多个 pair 组成
<object>   ==> '{' {<pair>}+ '}'

// pair 是个键值对
<pair>     ==> <key> ':' <value>

// array 由多个元素值
<array>    ==> '[' {<value>}+ ']'

// 值有 6 种类型（true/false 是 bool 类型）
<value>    ==> <string>
             | <number>
             | <object>
             | <array>
             | 'true'
             | 'false'
             | 'TRUE'
             | 'FALSE'
             | 'null'
             | 'NULL'

// 字符串由多个字符组成
<string>   ==> '"' {<char>}+ '"'

// 合法的字符包括（键盘上）可见字符，空格，转义字符（'\n'，'\t'，'\''，'\"'）
<char>     ==> visible-charactors
            | '\' 'n'
            | '\' 't'
            | '\' '''
            | '\' '"'
            | ' '
// 数字包括：十进制、十六进制的整数，和浮点数，都可以使用正负号和科学计数法
<number>   ==> <int> <double>
```

> kson格式解析器: kson

```
kson也是kson格式解析器的名字，厉害吧！
```
