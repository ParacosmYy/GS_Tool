#include "k30030/m30030.h"
QVector<double> m30030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
