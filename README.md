# QtHooker
Simple dll to hook [QNetworkAccessManager::post](https://doc.qt.io/qt-5/qnetworkaccessmanager.html#post) and [QNetworkAccessManager::get](https://doc.qt.io/qt-5/qnetworkaccessmanager.html#get).<br>
Once this library is injected into a process using QT version 5.12.5, it will open a command print and print all post and get requests.<br>
Only for x86 windows systems.


## Compilation
Be sure to have the QT library in "C:\Qt\Qt5.12.5\5.12.5\msvc2017", select Release and just hit compile. To change the path, just edit the "Additional include directories" and "Additional library directories"

## Dependencies
QT - 5.12.5 <br>
[MSDetours](https://github.com/microsoft/Detours)

## Remarks
The functions are found by checking "Qt5Network.dll" and checking the exporting table by ordinal, which means that for other versions this ordinals miight need ot be changed. <br>
To use with other versions of QT just change the include and library directories. 
