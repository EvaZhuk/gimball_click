/****************************************************************************
** Meta object code from reading C++ file 'canparserworker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../can/parser/canparserworker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'canparserworker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CANParserWorker_t {
    QByteArrayData data[11];
    char stringdata0[111];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CANParserWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CANParserWorker_t qt_meta_stringdata_CANParserWorker = {
    {
QT_MOC_LITERAL(0, 0, 15), // "CANParserWorker"
QT_MOC_LITERAL(1, 16, 13), // "messageParsed"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 10), // "parseError"
QT_MOC_LITERAL(4, 42, 5), // "error"
QT_MOC_LITERAL(5, 48, 20), // "capturePointReceived"
QT_MOC_LITERAL(6, 69, 8), // "uint16_t"
QT_MOC_LITERAL(7, 78, 1), // "x"
QT_MOC_LITERAL(8, 80, 1), // "y"
QT_MOC_LITERAL(9, 82, 20), // "stopTrackingReceived"
QT_MOC_LITERAL(10, 103, 7) // "process"

    },
    "CANParserWorker\0messageParsed\0\0"
    "parseError\0error\0capturePointReceived\0"
    "uint16_t\0x\0y\0stopTrackingReceived\0"
    "process"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CANParserWorker[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x06 /* Public */,
       3,    1,   40,    2, 0x06 /* Public */,
       5,    2,   43,    2, 0x06 /* Public */,
       9,    0,   48,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    0,   49,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 6,    7,    8,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void CANParserWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CANParserWorker *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->messageParsed(); break;
        case 1: _t->parseError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->capturePointReceived((*reinterpret_cast< uint16_t(*)>(_a[1])),(*reinterpret_cast< uint16_t(*)>(_a[2]))); break;
        case 3: _t->stopTrackingReceived(); break;
        case 4: _t->process(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CANParserWorker::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CANParserWorker::messageParsed)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CANParserWorker::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CANParserWorker::parseError)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CANParserWorker::*)(uint16_t , uint16_t );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CANParserWorker::capturePointReceived)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (CANParserWorker::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CANParserWorker::stopTrackingReceived)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CANParserWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CANParserWorker.data,
    qt_meta_data_CANParserWorker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CANParserWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CANParserWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CANParserWorker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CANParserWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void CANParserWorker::messageParsed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void CANParserWorker::parseError(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void CANParserWorker::capturePointReceived(uint16_t _t1, uint16_t _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void CANParserWorker::stopTrackingReceived()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
