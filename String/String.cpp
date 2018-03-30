#define _CRT_SECURE_NO_WARNINGS

#include <cstdlib>
#include <cstring>
#include <iostream>

using std::cout;
using std::endl;

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
