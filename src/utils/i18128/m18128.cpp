#include "i18128/m18128.h"
QVector<double> m18128::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
