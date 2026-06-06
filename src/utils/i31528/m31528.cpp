#include "i31528/m31528.h"
QVector<double> m31528::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
