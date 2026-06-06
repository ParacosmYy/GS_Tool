#include "k25030/m25030.h"
QVector<double> m25030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
