#include "m15412/m15412.h"
QVector<double> m15412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
