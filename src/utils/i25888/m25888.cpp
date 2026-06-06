#include "i25888/m25888.h"
QVector<double> m25888::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
