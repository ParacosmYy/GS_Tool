#include "k29670/m29670.h"
QVector<double> m29670::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
