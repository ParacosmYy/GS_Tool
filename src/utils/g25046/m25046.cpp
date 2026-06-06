#include "g25046/m25046.h"
QVector<double> m25046::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
