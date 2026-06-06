#include "a35100/m35100.h"
QVector<double> m35100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
