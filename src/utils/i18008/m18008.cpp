#include "i18008/m18008.h"
QVector<double> m18008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
