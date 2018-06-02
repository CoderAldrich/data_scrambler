[Visual Basic]
' A New 'HttpWebRequest' objetc is created.
Dim myHttpWebRequest As HttpWebRequest = WebRequest.Create("http://www.contoso.com")
myHttpWebRequest.AddRange(50, 150)
' The response object of 'HttpWebRequest' is assigned to a 'HttpWebResponse' variable.
Dim myHttpWebResponse As HttpWebResponse = CType(myHttpWebRequest.GetResponse(), HttpWebResponse)
' Displaying the contents of the page to the console
Dim streamResponse As Stream = myHttpWebResponse.GetResponseStream()
Dim streamRead As New StreamReader(streamResponse)
Dim readBuffer(256) As[Char]
Dim count As Integer = streamRead.Read(readBuffer, 0, 256)
Console.WriteLine(ControlChars.Cr + "The HTML contents of the page from 50th to 150 charaters are :" + ControlChars.Cr + "  ")
While count > 0
Dim outputData As New[String](readBuffer, 0, count)
Console.WriteLine(outputData)
count = streamRead.Read(readBuffer, 0, 256)
End While
' Release the response object resources.
streamRead.Close()
streamResponse.Close()
myHttpWebResponse.Close()

[C#]
// Create a New 'HttpWebRequest' object .
HttpWebRequest myHttpWebRequest = (HttpWebRequest)WebRequest.Create("http://www.contoso.com");
myHttpWebRequest.AddRange(50, 150);
// Assign the response object of 'HttpWebRequest' to a 'HttpWebResponse' variable.
HttpWebResponse myHttpWebResponse = (HttpWebResponse)myHttpWebRequest.GetResponse();
// Display the contents of the page to the console.
Stream streamResponse = myHttpWebResponse.GetResponseStream();
StreamReader streamRead = new StreamReader(streamResponse);
Char[] readBuffer = new Char[256];
int count = streamRead.Read(readBuffer, 0, 256);
Console.WriteLine("\nThe HTML contents of the page from 50th to 150 charaters are :\n  ");
while (count > 0)
{
    String outputData = new String(readBuffer, 0, count);
    Console.WriteLine(outputData);
    count = streamRead.Read(readBuffer, 0, 256);
}
// Release the response object resources.
streamRead.Close();
streamResponse.Close();
myHttpWebResponse.Close();

[C++]
// Create a New 'HttpWebRequest' object.
HttpWebRequest* myHttpWebRequest =
dynamic_cast<HttpWebRequest*>(WebRequest::Create(S"http://www.contoso.com"));
myHttpWebRequest->AddRange(50, 150);
// Assign the response object of 'HttpWebRequest' to a 'HttpWebResponse' variable.
HttpWebResponse* myHttpWebResponse =
dynamic_cast<HttpWebResponse*>(myHttpWebRequest->GetResponse());
// Display the contents of the page to the console.
Stream* streamResponse = myHttpWebResponse->GetResponseStream();
StreamReader* streamRead = new StreamReader(streamResponse);
Char readBuffer[] = new Char[256];
int count = streamRead->Read(readBuffer, 0, 256);
Console::WriteLine(S"\nThe HTML contents of the page from 50th to 150 charaters are :\n  ");
while (count > 0) {
    String* outputData = new String(readBuffer, 0, count);
    Console::WriteLine(outputData);
    count = streamRead->Read(readBuffer, 0, 256);
}
// Release the response object resources.
streamRead->Close();
streamResponse->Close();
myHttpWebResponse->Close();