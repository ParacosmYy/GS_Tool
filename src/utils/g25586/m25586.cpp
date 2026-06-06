#include "g25586/m25586.h"
QVector<double> m25586::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
