# QtHooker
Simple dll to hook [QNetworkAccessManager::post](https://doc.qt.io/qt-5/qnetworkaccessmanager.html#post) and [QNetworkAccessManager::get](https://doc.qt.io/qt-5/qnetworkaccessmanager.html#get).<br>
Once this library is injected into a process using QT version 5.12.5, it will open a command print and print all post and get requests.<br>
Only for x86 windows systems.


## Dependencies
QT - 5.12.5 <br>
[MSDetours](https://github.com/microsoft/Detours)

## Remarks
The functions are found by checking "Qt5Network.dll" and checking the exporting table by ordinal, which means that for other versions this ordinals might need  to be changed. <br>
To use with other versions of QT just change the include and library directories. 
