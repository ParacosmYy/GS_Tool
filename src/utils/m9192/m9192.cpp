#include "m9192/m9192.h"
QVector<double> m9192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
