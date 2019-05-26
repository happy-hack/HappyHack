#include "../../Hooks.h"
class key_values
{
public:
	char _pad[0x20];
};
template<typename T>
class singleton {
protected:
	singleton() { }
	~singleton() { }

	singleton(const singleton&) = delete;
	singleton& operator=(const singleton&) = delete;

	singleton(singleton&&) = delete;
	singleton& operator=(singleton&&) = delete;
public:
	static T& get() {
		static T inst{};
		return inst;
	}
};
class chams : public singleton<chams>
{
public:
	IMaterial * create_material(bool ignorez, bool lit, bool wireframe);
};