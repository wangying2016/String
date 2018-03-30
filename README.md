# 一、引言
我们在面试 C++ 相关岗位的时候，总会遇到这样的笔试面试题：
> 请你实现一个 String 类

这道题，说简单也简单，说难也难，是一个考察 C++ 基础的非常好的题目。正好在今天，我萌生了一个想法，那就是一步一步，一点一点，从构造析构，到拷贝赋值，再到 C++11 的移动，我们都给这个 String 类加进去，让我们看看，这将是一个怎样的类 ：）

让我们先写下这么一段代码：
```C++
class String {
	char *m_data;
};
```

这也就是我们的最基础最基础的一个实现（好吧，这也叫做实现 T_T）。当我们写出了上述代码，我们也就默认了编译器给我们生成其默认的构造、拷贝构造、拷贝赋值、移动构造、移动赋值、析构函数了。但是，在这种情况 下的默认生成函数，必然是错误的！为什么呢？因为其内部有一个 m_data 的动态内存需要去处理，这可不能简简单单使用默认的函数去按值拷贝就能处理好的。

既然我们明白了我们为什么要扩展这个 String 类了，那么现在就开始吧！

ps：考虑到网上的实现，要么没有移动操作，要么过于简单，不适用新手阅读，此篇博客力图从每个细节入手，希望能够给广大网友一些帮助。由于鄙人水平有限，纰漏之处在所难免，有问题还请见谅：）

# 二、构造：默认构造与传参构造的结合体

**1.** 函数声明

我们都知道，当我们声明了一个类，却不声明构造函数，编译器就会生成一个默认的构造函数，其对类中的成员进行默认值初始化，并且不接受任何参数。

我们的 String 类除了默认构造函数之外，肯定还需要一个传入字符串参数的构造函数。

此时，我们综合以上两个需求，可以书写一个函数就完成两个函数的功能，也就是将该字符串参数定义为默认为空（也就是默认构造函数的功能）。

那么，此时，我们就可以书写以下代码了：
```C++
String(const char *str = nullptr);
```
注意了，这里为什么要使用 const char *str 呢？
> 这是因为，如果你使用了 char *str，而不是 const char *str，则你只能向这个构造函数传递非 const 的 char* 参数；当你定义为了 const char *str 之后，你既可以传递 const char * 的字符串，也可以传递 char * 的字符串。
> 为什么呢？这是因为非 const 向 const 的转化是可以的，const 向非 const 的转化却是有风险的！

**2.** 函数定义

我们已经完成了该函数的声明。让我们想想，在这个构造函数中，我们究竟要完成什么操作呢？

处理传递进来的字符串参数，用它来初始化 String 类中的 m_data 成员变量。又由于我们的 m_data 是一个需要动态管理的内存成员，因此我们需要一些分配空间的操作。

另外，因为我们的参数字符串可能为空，我们还需要进行分情况处理。

那么，我们也就可以写出以下的代码：
```C++
String::String(const char *str)
{
	if (str == nullptr) {
		m_data = new char[1];
		*m_data = '\0';
		cout << "Default constructor" << endl;
	}
	else {
		int length = strlen(str);
		m_data = new char[length + 1];
		strcpy(m_data, str);
		cout << "Pass argument constructor" << endl;
	}
}
```
让我们看看，这段代码里面有什么好玩的地方：

当检测到传入参数字符串为空的时候，我们为 m_data 分配了一个字节的空间。为什么是一个字节呢？因为 `\0` 啊，因为即使为空，还是有一个字节的结束标志符的空间需要分配，这点非常非常重要！

当检测到传入参数字符串不为空，我们获取传入字符串的长度，按照此长度 + 1，进行内存空间的分配。为什么要 + 1 其实很简单，还是那个 `\0` 的原因。最后，将 str 拷贝到 m_data 中去。

**3.** 总结

其实这个构造函数的实现思路是非常清晰的：

就是要处理好传入参数是否为空，并且在分配内存空间得时候，充分考虑到最后的 `\0` 结尾标志符的空间分配。

# 三、析构：我挥一挥衣袖，真的不带走一片内存空间

说完了构造，必然要谈析构。

析构函数的语义其实非常简单，就是为了清理好类对象的一些使用的资源。比如动态内存的分配啦、数据库连接啦之类之类的。

良好的类的设计，就是在它离开的时候，就像它未曾来过一样那么干净清爽。

那么，在这个 String 类中，我们要处理的也就只有 m_data 这个动态分配内存的成员变量而已。

这个函数还是非常简单的：
```C++
String::~String()
{
	delete[] m_data;
	cout << "Destructor" << endl;
}
```
也没什么好解释的，这里使用 delete[] 还是 delete 都是可以的，就我个人来说，更加喜欢清晰的语义化，使用了 delete[] 来释放 m_data 的内存空间。

# 四、拷贝：构造、赋值，这个工作并不简单

到现在，我们终于要讨论 String 类实现中，最最需要注意的地方了。

拷贝操作，顾名思义，就是通过一个已经存在的类的对象，去构造或者赋值另一个对象。

在这个操作中，我们要考虑很多方面，比如说这两个对象是不是同一个对象啦（自赋值问题），原来的对象的痕迹有没有被清除干净啦（先析构）。

让我们先来谈谈拷贝构造。

**1.** 拷贝构造函数

拷贝构造函数，就是传入参数为该类 const 引用对象的构造函数：
> **为什么要是 const 类型**：
> 因为只有 const 类型，才能接收 const 对象和非 const 对象参数。
> **为什么要是引用类型**：
> 因为只有是引用类型，才能够规避递归使用拷贝构造的死循环问题：
> String(const String other);
> 这个函数传递参数就会发生另一个参数传入，默认实参匹配调用拷贝构造，然后又调用这个函数，...，直到死循环。
> 使用引用直接传对象实体进来，就不会在实参匹配时调用拷贝构造函数了。

作为拷贝构造函数，只需要处理好本对象的动态内存空间的分配，以及另一个对象的数据的拷贝即可：
```C++
String::String(const String &other)
{
	int length = strlen(other.m_data);
	m_data = new char[length + 1];
	strcpy(m_data, other.m_data);
	cout << "Copy constructor" << endl;
}
```
构造构造，必然要对本对象进行一些处理，这里就是将另一个对象的数据拿来初始化了本对象的数据。还是比较简单的。

**2.** 拷贝赋值运算符

难的就是拷贝赋值运算符了。

我们可能觉得，我们只需要将另一个对象赋值给本对象即可。还需要考虑其他什么吗：
> **我们是不是要析构本对象的数据**？
> 是的，我们必须要析构我们本对象的数据，不然赋值过来，原来的动态内存空间就是野空间了，这就是内存泄露！
> **如果是本对象赋值给本对象呢**？
> 你也许会说，卧槽，还有这种操作！真的有哦。怎么办呢？我们要先进行本对象的甄别操作。否则的话，我们析构了本对象的数据，再拿另一个对象的数据进行拷贝时，会发现，卧槽，数据已经在刚才被我析构掉了 T_T 

其实只要考虑好了以上两点，拷贝赋值运算符也就不那么难写了：
```C++
String &String::operator=(const String &other)
{
	if (this != &other) {
		if (!m_data) delete[] m_data;
		int length = strlen(other.m_data);
		m_data = new char[length + 1];
		strcpy(m_data, other.m_data);
	}
	cout << "Copy assignment" << endl;
	return *this;
}
```
我们首先进行了自赋值的检查，this 是本对象的地址，因此比较的时候，使用的是 other 的地址进行比较。

然后，我们进行了本对象 m_data 的释放操作，这是为了避免内存泄露。

最后，我们分配 m_data 的空间，将 other 的数据拷贝到 m_data 中去，最后返回本对象即可（this 是本对象的地址，因此返回实体就是 *this）。

**3.** 总结
总的来说，拷贝操作，要注意好内存空间分配前，自赋值的检查，本对象的内存空间的释放，注意好这两点，基本没有什么问题了。

# 五、移动：构造、赋值，我不是归人，只是一个过客

说到移动操作，其实也只是概念难理解，写起来真的是非常轻松的。

移动移动，说白了就是一个对象直接接管一块临时对象的数据而已。

这有什么好处呢？我们可以直接移动对象数据，而不需要进行（有时候多余的）拷贝操作。

从概念上也就决定了，移动操作的代码，必然要比拷贝操作的代码，少一些内存分配的工作，为什么呢？因为移动操作直接移动内存数据，根本不需要重新分配内存空间。

**1.** 移动构造

移动构造的书写是非常非常简单的，我们甚至都不需要分配内存空间！

但是，根据 C++ Primer 第五版第 13 章作者的建议，我们在使用了移动构造传入对象之后，需要使得该对象不能再访问已经移动的内存区域，所以该对象的 m_data 应该置为空：
```C++
String::String(String &&other)
{
	m_data = other.m_data;
	other.m_data = nullptr;
	cout << "Move constructor" << endl;
}

```
传入 String && 代表着右值对象。我们直接接管了 other.m_data 数据，在移动过来了之后，我们将 other.m_data 置为了空，以免出现问题。

**2.** 移动赋值

移动赋值，相比拷贝赋值来说，少了内存空间的分配操作，多了传入右值对象的 m_data 成员的置空考虑。

自赋值都是两者需要考虑的（想想，怎么可能有自己移动到自己的说法 T_T）：
```C++
String &String::operator=(String &&other)
{
	if (this != &other) {
		delete[] m_data;
		m_data = other.m_data;
		other.m_data = nullptr;
	}
	cout << "Move assignment" << endl;
	return *this;
}
```


**3.** 总结

总的来说，移动操作的实现是要比拷贝操作的实现简单的，因为少了内存空间的分配工作。

但是，我们需要处理好移动后传入对象对于该内存区域的访问情况，最好置为空，以免出现问题。

# 六、测试：让我们看看自己的 String 类的使用吧

至此，我们的 String 类的完整实现如下：
```C++
class String {
public:
	// 构造：默认（传参）、拷贝构造、移动构造
	String(const char *str = nullptr);
	String(const String &other);
	String(String &&other);

	// 析构
	~String();

	// 赋值：拷贝赋值、移动赋值
	String &operator=(const String &other);	
	String &operator=(String &&other);

private:
	char *m_data;
};

String::String(const char *str)
{
	if (str == nullptr) {
		m_data = new char[1];
		*m_data = '\0';
		cout << "Default constructor" << endl;
	}
	else {
		int length = strlen(str);
		m_data = new char[length + 1];
		strcpy(m_data, str);
		cout << "Pass argument constructor" << endl;
	}
}

String::String(const String &other)
{
	int length = strlen(other.m_data);
	m_data = new char[length + 1];
	strcpy(m_data, other.m_data);
	cout << "Copy constructor" << endl;
}


String::String(String &&other)
{
	m_data = other.m_data;
	other.m_data = nullptr;
	cout << "Move constructor" << endl;
}

String::~String()
{
	delete[] m_data;
	cout << "Destructor" << endl;
}

String &String::operator=(const String &other)
{
	if (this != &other) {
		if (!m_data) delete[] m_data;
		int length = strlen(other.m_data);
		m_data = new char[length + 1];
		strcpy(m_data, other.m_data);
	}
	cout << "Copy assignment" << endl;
	return *this;
}

String &String::operator=(String &&other)
{
	if (this != &other) {
		delete[] m_data;
		m_data = other.m_data;
		other.m_data = nullptr;
	}
	cout << "Move assignment" << endl;
	return *this;
}
```

最后，我在 VS2017 的开发环境下，进行了 String 类的测试，编写测试代码如下：
```C++
int main()
{
	{
		// 测试：默认构造
		String s1;

		// 测试：传参构造
		String s2("hello world");

		// 测试：拷贝构造
		String s3(s1);

		// 测试：移动构造
		String s4(std::move(s3));

		// 测试：拷贝赋值
		String s5;
		s5 = s4;

		// 测试：移动赋值
		String s6;
		s6 = std::move(s5);

		// 测试：自动析构

	}
	system("pause");
	return 0;
}
```

中间加了一个 `{}` 是为了体现析构函数的作用。

std::move 是为了使用移动操作，将其转化为右值进行调用。

在 VS2017 环境下，可能会出现提示 strcpy 不是安全的字符串函数的问题，我们只需要在所有的代码之前（#include 之前），加上这行定义即可：
```C++
#define _CRT_SECURE_NO_WARNINGS
```
禁用掉安全提示。

最后测试结果如下：
![Result](https://img-blog.csdn.net/20180330103922995?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3UwMTI4MTQ4NTY=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

至此，完结，撒花 ^_^

# 七、总结

实现一个 String 类，这种考察，其实是非常非常有效的。能够完整写出这个类的实现，并且能够说出其每个点的原因和目的，可以说 C++ 水平已经到了一定的入门水平了。

String 类的实现还是非常有趣的，我还想往里面加入更多有趣的函数，比如操作符重载啦之类的，再加点模板，感觉 C++ 大部分的知识点就都涵盖了，哈哈~~~

C++ 学习之路还要继续
To be Strong：）

ps: 本篇博客的完整 String 定义以及测试代码，都放在了本人的 GitHub 上，有兴趣的同学可以访问查看
[wangying2016/String](https://github.com/wangying2016/String)
