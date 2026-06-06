#include "k14030/m14030.h"
QVector<double> m14030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
