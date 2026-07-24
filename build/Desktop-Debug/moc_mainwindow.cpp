/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../gui/mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[19];
    char stringdata0[210];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 14), // "onLabelClicked"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 3), // "pos"
QT_MOC_LITERAL(4, 31, 13), // "onVideoStatus"
QT_MOC_LITERAL(5, 45, 3), // "txt"
QT_MOC_LITERAL(6, 49, 22), // "onCapturePointReceived"
QT_MOC_LITERAL(7, 72, 1), // "x"
QT_MOC_LITERAL(8, 74, 1), // "y"
QT_MOC_LITERAL(9, 76, 32), // "onCapturePointNormalizedReceived"
QT_MOC_LITERAL(10, 109, 2), // "nx"
QT_MOC_LITERAL(11, 112, 2), // "ny"
QT_MOC_LITERAL(12, 115, 22), // "onStopTrackingReceived"
QT_MOC_LITERAL(13, 138, 19), // "onCameraFovReceived"
QT_MOC_LITERAL(14, 158, 4), // "hDeg"
QT_MOC_LITERAL(15, 163, 4), // "vDeg"
QT_MOC_LITERAL(16, 168, 24), // "onTrackingParamsReceived"
QT_MOC_LITERAL(17, 193, 8), // "uint16_t"
QT_MOC_LITERAL(18, 202, 7) // "roiSize"

    },
    "MainWindow\0onLabelClicked\0\0pos\0"
    "onVideoStatus\0txt\0onCapturePointReceived\0"
    "x\0y\0onCapturePointNormalizedReceived\0"
    "nx\0ny\0onStopTrackingReceived\0"
    "onCameraFovReceived\0hDeg\0vDeg\0"
    "onTrackingParamsReceived\0uint16_t\0"
    "roiSize"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x08 /* Private */,
       4,    1,   52,    2, 0x08 /* Private */,
       6,    2,   55,    2, 0x08 /* Private */,
       9,    2,   60,    2, 0x08 /* Private */,
      12,    0,   65,    2, 0x08 /* Private */,
      13,    2,   66,    2, 0x08 /* Private */,
      16,    1,   71,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QPoint,    3,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::UShort, QMetaType::UShort,    7,    8,
    QMetaType::Void, QMetaType::Float, QMetaType::Float,   10,   11,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Float, QMetaType::Float,   14,   15,
    QMetaType::Void, 0x80000000 | 17,   18,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onLabelClicked((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 1: _t->onVideoStatus((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->onCapturePointReceived((*reinterpret_cast< quint16(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        case 3: _t->onCapturePointNormalizedReceived((*reinterpret_cast< float(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 4: _t->onStopTrackingReceived(); break;
        case 5: _t->onCameraFovReceived((*reinterpret_cast< float(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 6: _t->onTrackingParamsReceived((*reinterpret_cast< uint16_t(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
