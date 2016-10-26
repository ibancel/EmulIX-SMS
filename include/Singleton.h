#ifndef _H_EMULATOR_SINGLETON
#define _H_EMULATOR_SINGLETON

template<typename T>
class Singleton
{

public:

	static T* instance()
	{
		if(_instance == nullptr)
			_instance = new T();

	//	cout << "SINGLETON" << endl;

		return _instance;
	}

public:

	static T* _instance;
};


template<typename T>
T* Singleton<T>::_instance = nullptr;

#endif
