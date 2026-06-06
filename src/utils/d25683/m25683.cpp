#include "d25683/m25683.h"
QVector<double> m25683::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
