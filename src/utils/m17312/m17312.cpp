#include "m17312/m17312.h"
QVector<double> m17312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
