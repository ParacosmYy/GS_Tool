#include "m37412/m37412.h"
QVector<double> m37412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
