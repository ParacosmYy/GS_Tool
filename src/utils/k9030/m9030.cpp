#include "k9030/m9030.h"
QVector<double> m9030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
