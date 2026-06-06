#include "i15008/m15008.h"
QVector<double> m15008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
