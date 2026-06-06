#include "k27030/m27030.h"
QVector<double> m27030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
