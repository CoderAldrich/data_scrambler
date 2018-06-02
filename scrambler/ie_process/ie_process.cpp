void EnumIE(void)
{
    LRESULT ret = ERROR_SUCCESS;
    CString sURL;	//sURL为要注册的网址，如http://www.Ice.com
    BOOLEAN RegOrComment = 0;

    do
    {
        if (!RegOrComment)	sURL = TEXT("file://D:/冰网会员注册.htm");
        else				sURL = TEXT("file://D:/冰网门户.htm");
        /*本地的网址有两种方式
        1】file://D:/冰网会员注册.htm
        2】http://localhost:1909/Default.aspx			因为我做的是本地测试，所有用这个，这个端口是由VS2008调试时的网址得到的
        调试结果是哪个网址就填哪个网址，	网上找到的那些配置IIS的，都默认是80端口，也就是以http://localhost:80/Default.aspx为网址
        很明显我这里就打不开
        */


        CComQIPtr<IWebBrowser2>spBrowser;
        HRESULT	hr = CoCreateInstance(CLSID_InternetExplorer, NULL, CLSCTX_LOCAL_SERVER, IID_IWebBrowser2, (LPVOID *)&spBrowser);	//打开浏览器
        if (hr != S_OK)		break;

        COleVariant vaURL(sURL);
        COleVariant null;

        VARIANT_BOOL pBool = TRUE;
        spBrowser->put_Visible(pBool);//TRUE显示浏览器，FALSE隐藏浏览器
        spBrowser->Navigate2(vaURL, null, null, null, null); //Open the URL page,打开网页


        Sleep(2000);//等待网页加载完毕
        if (!RegOrComment)	sURL = TEXT("file:///D:/冰网会员注册.htm");	//注意：这里的file后面有三个斜杠。	如果是http开头的，则不需要这个if语句
        else				sURL = TEXT("file:///D:/冰网门户.htm");

        CComPtr<IShellWindows> spShellWin;
        hr = spShellWin.CoCreateInstance(CLSID_ShellWindows);
        if (FAILED(hr))   		break;

        long nCount = 0;    //取得浏览器实例个数(Explorer和IExplorer)   
        spShellWin->get_Count(&nCount);
        if (0 == nCount)   		break;

        for (int i = 0; i<nCount; i++)
        {
            CComPtr<IDispatch> spDispIE;
            hr = spShellWin->Item(CComVariant((long)i), &spDispIE);
            if (FAILED(hr)) continue;

            spBrowser = spDispIE;
            if (!spBrowser) continue;

            IDispatch*spDispDoc;
            hr = spBrowser->get_Document(&spDispDoc);
            if (FAILED(hr)) continue;

            CComQIPtr<IHTMLDocument2>spDocument2 = spDispDoc;
            if (!spDocument2) continue;

            CString cIEUrl_Filter;  //设置URL(必须是此URL的网站才有效);
            cIEUrl_Filter = sURL;

            CComBSTR IEUrl;
            spBrowser->get_LocationURL(&IEUrl);
            CString cIEUrl_Get;     //从机器上取得的HTTP的完整的URL;
            cIEUrl_Get = IEUrl;
            cIEUrl_Get = cIEUrl_Get.Left(cIEUrl_Filter.GetLength()); //截取前面N位

                                                                     //MessageBox(cIEUrl_Get+TEXT("2")+cIEUrl_Filter); 

            if (cIEUrl_Get == cIEUrl_Filter)
            {
                // 程序运行到此，已经找到了IHTMLDocument2的接口指针   

                //	CComPtr<IHTMLFormElement> pForm;
                //	if (SUCCEEDED(spBrowser->get_Document( &spDispDoc)))	spDocument2 = spDispDoc;
                //	if(SUCCEEDED(spDispDoc->QueryInterface(IID_IHTMLFormElement,(void**)&pForm)))
                //	if(SUCCEEDED(pFormElement->item(id,index, &spDispDoc)))
                //	spDispDoc->QueryInterface(IID_IHTMLInputTextElement,(void**)&pInputElement);

                EnumForm(spDocument2, spDispDoc); //枚举所有的表单 
            }
        }

    } while (FALSE);

    return;
}