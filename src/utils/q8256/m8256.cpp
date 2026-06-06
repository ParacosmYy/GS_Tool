#include "q8256/m8256.h"
QVector<double> m8256::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
