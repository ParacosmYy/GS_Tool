#include "s16898/m16898.h"
QVector<double> m16898::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
