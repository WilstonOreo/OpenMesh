/****************************************************************************
** Meta object code from reading C++ file 'MeshViewerWidget.hh'
**
** Created: Mon Jul 2 13:13:45 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../QtViewer/MeshViewerWidget.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MeshViewerWidget.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MeshViewerWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x0a,
      41,   17,   17,   17, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MeshViewerWidget[] = {
    "MeshViewerWidget\0\0query_open_mesh_file()\0"
    "query_open_texture_file()\0"
};

void MeshViewerWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MeshViewerWidget *_t = static_cast<MeshViewerWidget *>(_o);
        switch (_id) {
        case 0: _t->query_open_mesh_file(); break;
        case 1: _t->query_open_texture_file(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData MeshViewerWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MeshViewerWidget::staticMetaObject = {
    { &MeshViewerWidgetT<MyMesh>::staticMetaObject, qt_meta_stringdata_MeshViewerWidget,
      qt_meta_data_MeshViewerWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MeshViewerWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MeshViewerWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MeshViewerWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MeshViewerWidget))
        return static_cast<void*>(const_cast< MeshViewerWidget*>(this));
    return MeshViewerWidgetT<MyMesh>::qt_metacast(_clname);
}

int MeshViewerWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = MeshViewerWidgetT<MyMesh>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
