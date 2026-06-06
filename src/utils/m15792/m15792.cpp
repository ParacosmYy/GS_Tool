#include "m15792/m15792.h"
QVector<double> m15792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
