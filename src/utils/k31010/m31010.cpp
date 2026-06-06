#include "k31010/m31010.h"
QVector<double> m31010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
