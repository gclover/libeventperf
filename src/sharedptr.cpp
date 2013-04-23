
#include <Poco/SharedPtr.h>

#include <string>
#include <iostream>

using Poco::SharedPtr;

int main(int argc, char* argv[])
{
	SharedPtr<std::string> p1(new std::string("hello world!"));
	SharedPtr<std::string> p2(p1);

	p2 = 0;
	p2 = p1;

	std::string::size_type len = p1->length();
	std::cout << *p1 << std::endl;
	return 0;
}
