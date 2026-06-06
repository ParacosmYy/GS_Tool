#include "k16030/m16030.h"
QVector<double> m16030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
