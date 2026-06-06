#include "k31330/m31330.h"
QVector<double> m31330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
