#include "i36128/m36128.h"
QVector<double> m36128::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
