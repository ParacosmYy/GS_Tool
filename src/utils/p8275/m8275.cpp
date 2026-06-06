#include "p8275/m8275.h"
QVector<double> m8275::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
