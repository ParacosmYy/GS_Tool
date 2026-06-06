#include "i36528/m36528.h"
QVector<double> m36528::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
