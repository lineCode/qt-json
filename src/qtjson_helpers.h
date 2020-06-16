#pragma once

#include "qtjson_global.h"

#include <QtCore/QCborValue>

namespace QtJson::__private {

QTJSON_EXPORT QCborValue extract(const QCborValue &value, QCborTag *innerTag = nullptr, bool skipInner = false);

}
