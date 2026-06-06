#include "i9528/m9528.h"
QVector<double> m9528::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
