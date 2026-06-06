#include "e8704/m8704.h"
QVector<double> m8704::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
