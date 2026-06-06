#include "m36412/m36412.h"
QVector<double> m36412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
