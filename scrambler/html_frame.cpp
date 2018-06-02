function TfrmSetTask.GetActiveElmPos(Doc: IHTMLDocument2;ELm:ihtmlelement): tpoint;
var
Element: IHTMLElement;
Frame: IHTMLFrameElement;
WB: IWebBrowser2;
HtmlDoc2: IHTMLDocument2;
begin
ELm.scrollIntoView(0);
(ELm as IHTMLElement3).setActive;

HtmlDoc2:=Doc;
Element := HtmlDoc2.activeElement;

Result:=Point(0,0);

while not ((Element.tagName=ELm.tagName) and (Element.sourceIndex=ELm.sourceIndex)) do
begin

Element := HtmlDoc2.activeElement;

Result.X:=Result.X+(Element as ihtmlelement2).getBoundingClientRect.left;
Result.y:=Result.y+(Element as ihtmlelement2).getBoundingClientRect.top;

if not Assigned(Element) then Break;
if Element.QueryInterface(IHTMLFrameElement, Frame) <> S_OK then Break;
if Frame.QueryInterface(IID_IWebBrowser2, WB) <> S_OK then  Break;
if not Assigned(WB.Document) then  Break;
if WB.Document.QueryInterface(IID_IHTMLDocument2, HtmlDoc2) <> S_OK then Break;

end;
Result.X:=Result.X+(Elm as ihtmlelement2).getBoundingClientRect.left;
Result.y:=Result.y+(Elm as ihtmlelement2).getBoundingClientRect.top;
end;


Browser          -     IWebBrowser2
Document       -     IHTMLDocument2
Frame/IFrame-     IHTMLWindow2
Element          -     IHTMLElement
可以通过下面方法互相获取 
browser      -> document        IWebBrowser2::get_Document
document     -> frame           IHTMLDocument2::get_parentWindow
frame        -> document        IHTMLWindow2::get_document
frame        -> parent frame    IHTMLWindow2::get_parent
frame        -> children frames IHTMLWindow2::get_frames
element     -> Frame              IHTMLElement->QI(IHTMLFrameBase2) -> IHTMLFrameBase2->get_contentWindow -> IHTMLWindow2 ->