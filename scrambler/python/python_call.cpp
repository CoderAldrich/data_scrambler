#include <iostream> 
#include <stdio.h>
#include <stdlib.h>
#include <Python.h>
#include <string.h>
using namespace std;

int main()
{
    Py_Initialize(); // 初始化，这是必须的，用来初始化python所需的环境
    if (!Py_IsInitialized())
        return -1;
    // 导入模块
    PyObject* pModule = NULL;
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");
    pModule = PyImport_ImportModule("myclass");
    if (!pModule) {
        printf("Cant open python file!/n");
        return -1;
    }
    // 模块的字典列表
    PyObject* pDict = PyModule_GetDict(pModule);
    if (!pDict) {
        printf("Cant find dictionary./n");
        return -1;

        // 函数调用
        cout << "calling python program..." << endl;
        PyObject* pFunHi = PyDict_GetItemString(pDict, "hi_function");
        // 利用python函数返回hi_class类（hi_function函数返回hi_class类，看我python程序）
        PyObject* phi_class = PyObject_CallFunction(pFunHi, NULL, NULL);
        if (!phi_class) {
            printf("Cant find phi_class class./n");
            return -1;
        }
        //构造hi_class的实例
        PyObject* pInstance_hi_class = PyInstance_New(phi_class, NULL, NULL);
        if (!pInstance_hi_class) {
            printf("Cant create instance./n");
            return -1;
        }
        //调用hi_class类实例pInstance_hi_class里面的方法
        PyObject_CallMethod(phi_class, "sayHi", "O", pInstance_hi_class);
        //释放
        Py_DECREF(phi_class);
        Py_DECREF(pInstance_hi_class);
        Py_DECREF(pFunHi);
        Py_DECREF(pModule);
        Py_Finalize(); // 与初始化对应
        system("pause");
        return 0;
    }

#!/usr/bin/env python
# -*- coding:utf8 -*-

    class hi_class :
        def sayHi(self) :
        print 'Hi! I am a python class!'

        def hi_function() :
        py_class = hi_class()
        return py_class



#include <iostream> 
#include <stdio.h>
#include <stdlib.h>
#include <Python.h>
#include <string.h>
        using namespace std;

    int main()
    {
        Py_Initialize(); // 初始化，这是必须的，用来初始化python所需的环境
        if (!Py_IsInitialized())
            return -1;
        // 导入模块
        PyObject* pModule = NULL;
        PyRun_SimpleString("import sys");
        PyRun_SimpleString("sys.path.append('./')");
        pModule = PyImport_ImportModule("myclass");
        if (!pModule) {
            printf("Cant open python file!/n");
            return -1;
        }
        // 模块的字典列表
        PyObject* pDict = PyModule_GetDict(pModule);
        if (!pDict) {
            printf("Cant find dictionary./n");
            return -1;

            // 演示函数调用
            cout << "calling python function..." << endl;
            PyObject* pFunHi = PyDict_GetItemString(pDict, "hi_function");
            PyObject_CallFunction(pFunHi, NULL, NULL);
            Py_DECREF(pFunHi);

            //演示类调用
            cout << "calling python class..." << endl;
            // 演示构造一个Python对象，并调用Class的方法
            // 获取hi_class类
            PyObject* phi_class = PyDict_GetItemString(pDict, "hi_class");
            if (!phi_class) {
                printf("Cant find phi_class class./n");
                return -1;
            }
            //构造hi_class的实例
            PyObject* pInstance_hi_class = PyInstance_New(phi_class, NULL, NULL);
            //如果python类中有初始化参数，那么创建实例的时候需要将初始化参数以元组形式传递进去（亲测可用）PyObject* pInstance_hi_class = PyInstance_New(phi_class , PyObject*类型的元组（需要在C++里创建python元组）, NULL);
            //C++中创建并初始化python元组示例如下两行：
            //PyObject *pArgs3 = PyTuple_New(1);
            //PyTuple_SetItem(pArgs3, 0, Py_BuildValue("i", 3));
            if (!pInstance_hi_class) {
                printf("Cant create instance./n");
                return -1;
            }
            //调用hi_class类实例pInstance_hi_class里面的方法
            PyObject_CallMethod(phi_class, "sayHi", "O", pInstance_hi_class);
            //释放
            Py_DECREF(phi_class);
            Py_DECREF(pInstance_hi_class);
            Py_DECREF(pModule);
            Py_Finalize(); // 与初始化对应
            system("pause");
            return 0;
        }


#!/usr/bin/env python
# -*- coding:utf8 -*-

        class hi_class :
            def sayHi(self) :
            print 'Hi! I am a python class!'

            def hi_function() :
            print 'Hi! I am a python function!'

