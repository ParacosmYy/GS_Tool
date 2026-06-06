#include "m12412/m12412.h"
QVector<double> m12412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
