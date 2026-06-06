#include "d15683/m15683.h"
QVector<double> m15683::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
