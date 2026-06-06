#include "m35512/m35512.h"
QVector<double> m35512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
