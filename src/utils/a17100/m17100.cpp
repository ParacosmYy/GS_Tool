#include "a17100/m17100.h"
QVector<double> m17100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
