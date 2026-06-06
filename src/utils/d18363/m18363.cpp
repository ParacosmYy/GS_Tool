#include "d18363/m18363.h"
QVector<double> m18363::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
