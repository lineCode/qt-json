#include "qtjson.h"
#include <QtCore/QSet>
#include <QtCore/QMetaProperty>
#include <QtCore/QSharedPointer>
#include <QtCore/QPointer>
#include <QtCore/QSequentialIterable>
#include <QtCore/QAssociativeIterable>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QCborMap>
#include <QtCore/QCborArray>
#include <optional>
using namespace QtJson;

namespace {

const QSet<int> MetaExceptions {
    QMetaType::QKeySequence,
    QMetaType::QFont,
    QMetaType::QLocale,
};

// TODO use DataValueInfo instead
template <typename TType>
struct Serializer;

template <typename TType>
typename Serializer<TType>::Value serialize(const QVariant &value, const Configuration &configuration);

template <>
struct Serializer<QJsonValue> {
    using Value = QJsonValue;
    using Map = QJsonObject;
    using List = QJsonArray;

    static constexpr auto Undefined = QJsonValue::Undefined;
    static constexpr auto Null = QJsonValue::Null;

    static inline QString key(const QVariant &value, const Configuration &) {
        return value.toString();
    }
};

template <>
struct Serializer<QCborValue> {
    using Value = QCborValue;
    using Map = QCborMap;
    using List = QCborArray;

    static constexpr auto Undefined = QCborValue::Undefined;
    static constexpr auto Null = QCborValue::Null;

    static inline QCborValue key(const QVariant &value, const Configuration &configuration) {
        return serialize<QCborValue>(value, configuration);
    }
};

template <typename TType>
typename Serializer<TType>::Map mapGadget(const QMetaObject *mo, const void *gadget, const Configuration &configuration)
{
    typename Serializer<TType>::Map map;
    for (auto i = 0; i < mo->propertyCount(); ++i) {
        const auto property = mo->property(i);
        if (!property.isStored())
            continue;
        map.insert(QString::fromUtf8(property.name()),
                   serialize<TType>(property.readOnGadget(gadget), configuration));
    }
    return map;
}

template <typename TType>
typename Serializer<TType>::Map mapObject(QObject *object, const Configuration &configuration)
{
    typename Serializer<TType>::Map map;
    const auto mo = object->metaObject();
    for (auto i = configuration.keepObjectName ? 0 : 1; i < mo->propertyCount(); ++i) {
        const auto property = mo->property(i);
        if (!property.isStored())
            continue;
        map.insert(QString::fromUtf8(property.name()),
                   serialize<TType>(property.read(object), configuration));
    }
    return map;
}

template <typename TType>
typename Serializer<TType>::Value serialize(const QVariant &value, const Configuration &configuration)
{
    if (!value.isValid())
        return Serializer<TType>::Undefined;
    if (value.isNull())
        return Serializer<TType>::Null;

    // check for objects/gadgets
    const auto type = value.userType();
    const auto mo = QMetaType::metaObjectForType(type);
    if (mo && !MetaExceptions.contains(type)) {
        const auto flags = QMetaType::typeFlags(type);
        if (flags.testFlag(QMetaType::PointerToGadget)) {
            Q_ASSERT_X(value.constData(), Q_FUNC_INFO, "value.constData() should not be null");
            const auto gadgetPtr = *reinterpret_cast<const void* const *>(value.constData());
            if (gadgetPtr)
                return mapGadget<TType>(mo, gadgetPtr, configuration);
            else
                return Serializer<TType>::Null;
        } else if (flags.testFlag(QMetaType::IsGadget)) {
            return mapGadget<TType>(mo, value.constData(), configuration);
        } else if (flags.testFlag(QMetaType::SharedPointerToQObject)) {
            const auto objPtr = value.value<QSharedPointer<QObject>>();
            if (objPtr)
                return mapObject<TType>(objPtr.get(), configuration);
            else
                return Serializer<TType>::Null;
        } else if (flags.testFlag(QMetaType::WeakPointerToQObject)) {
            const auto weakPtr = value.value<QWeakPointer<QObject>>();
            if (const auto objPtr = weakPtr.toStrongRef(); objPtr)
                return mapObject<TType>(objPtr.get(), configuration);
            else
                return Serializer<TType>::Null;
        } else if (flags.testFlag(QMetaType::TrackingPointerToQObject)) {
            const auto objPtr = value.value<QPointer<QObject>>();
            if (objPtr)
                return mapObject<TType>(objPtr, configuration);
            else
                return Serializer<TType>::Null;
        } else if (flags.testFlag(QMetaType::PointerToQObject)) {
            const auto objPtr = value.value<QObject*>();
            if (objPtr)
                return mapObject<TType>(objPtr, configuration);
            else
                return Serializer<TType>::Null;
        } else if (flags.testFlag(QMetaType::IsEnumeration)) {
            if (configuration.enumAsString) {
                const auto enumName = QString::fromUtf8(QMetaType::typeName(type))
                                          .split(QStringLiteral("::"))
                                          .last()
                                          .toUtf8();
                auto meIndex = mo->indexOfEnumerator(enumName.data());
                if (meIndex >= 0) {
                    const auto me = mo->enumerator(meIndex);
                    if (me.isFlag())
                        return QString::fromUtf8(me.valueToKeys(value.toInt()));
                    else
                        return QString::fromUtf8(me.valueToKey(value.toInt()));
                }
            } else
                return value.toInt();
        }
    }

    // check for lists
    if (value.canConvert(QMetaType::QVariantList)) {
        typename Serializer<TType>::List list;
        for (const auto element : value.value<QSequentialIterable>())
            list.append(serialize<TType>(element, configuration));
        return list;
    }

    // check for maps
    if (value.canConvert(QMetaType::QVariantMap) ||
        value.canConvert(QMetaType::QVariantHash)) {
        typename Serializer<TType>::Map map;
        const auto iterator = value.value<QAssociativeIterable>();
        for (auto it = iterator.begin(), end = iterator.end(); it != end; ++it)
            map.insert(Serializer<TType>::key(it.key(), configuration),
                       serialize<TType>(it.value(), configuration));
        return map;
    }

    // all other cases: default convert
    return Serializer<TType>::Value::fromVariant(value);
}

}

QJsonValue QtJson::stringify(const QVariant &value, const Configuration &configuration)
{
    return serialize<QJsonValue>(value, configuration);
}

QCborValue QtJson::binarify(const QVariant &value, const Configuration &configuration)
{
    return serialize<QCborValue>(value, configuration);
}