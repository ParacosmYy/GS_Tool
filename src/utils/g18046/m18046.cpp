#include "g18046/m18046.h"
QVector<double> m18046::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
