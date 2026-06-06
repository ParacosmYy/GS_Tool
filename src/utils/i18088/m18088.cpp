#include "i18088/m18088.h"
QVector<double> m18088::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
