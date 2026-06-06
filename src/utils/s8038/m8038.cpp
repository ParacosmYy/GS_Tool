#include "s8038/m8038.h"
QVector<double> m8038::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
