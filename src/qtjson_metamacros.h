#ifndef QTJSON_METAMACROS_H
#define QTJSON_METAMACROS_H

#include <type_traits>

#include <QtCore/qobjectdefs.h>

#define QTJSON_SERIALIZABLE_KEY "__qtjson_serializable"
#define QTJSON_SERIALIZABLE Q_CLASSINFO(QTJSON_SERIALIZABLE_KEY, "true")

#define QTJSON_PROPERTY_OFFSET_KEY "__qtjson_offset"
#define QTJSON_PROPERTY_OFFSET(offset) Q_CLASSINFO(QTJSON_PROPERTY_OFFSET_KEY, #offset)

#define QTJSON_SERIALIZABLE_PROP_KEY(name) ("__qtjson_propinfo_" #name)
#define QTJSON_SERIALIZABLE_PROP(name, ...) \
    Q_INVOKABLE static inline constexpr bool __qtjson_propinfo_ ## name() { \
        return std::is_base_of_v<QtJson::ISerializable, __VA_ARGS__>; \
    }

#define QTJSON_PROP(name, ...) \
    Q_PROPERTY(__VA_ARGS__ name MEMBER name STORED true) \
    QTJSON_SERIALIZABLE_PROP(name, __VA_ARGS__)

#endif // QTJSON_METAMACROS_H