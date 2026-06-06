#include "k17030/m17030.h"
QVector<double> m17030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
