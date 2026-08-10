#pragma once

#include <QString>

namespace Readability {

QString articleFromHtml(const QString &html);
QString commentFromHtml(const QString &html);
QString plainText(const QString &html);

}
