#include "k15330/m15330.h"
QVector<double> m15330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
