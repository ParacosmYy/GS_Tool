#include "m35812/m35812.h"
QVector<double> m35812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
