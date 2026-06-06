#include "e32704/m32704.h"
QVector<double> m32704::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
