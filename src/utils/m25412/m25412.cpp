#include "m25412/m25412.h"
QVector<double> m25412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
