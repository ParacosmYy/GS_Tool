#include "f25725/m25725.h"
QVector<double> m25725::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
