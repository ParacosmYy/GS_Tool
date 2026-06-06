#include "p8675/m8675.h"
QVector<double> m8675::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
