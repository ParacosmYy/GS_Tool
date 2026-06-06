#include "k13030/m13030.h"
QVector<double> m13030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
