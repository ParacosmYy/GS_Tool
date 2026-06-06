#include "a9100/m9100.h"
QVector<double> m9100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
