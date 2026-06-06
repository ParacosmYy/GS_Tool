#include "f8085/m8085.h"
QVector<double> m8085::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
