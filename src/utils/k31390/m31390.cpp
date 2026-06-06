#include "k31390/m31390.h"
QVector<double> m31390::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
