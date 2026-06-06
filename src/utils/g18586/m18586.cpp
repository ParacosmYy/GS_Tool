#include "g18586/m18586.h"
QVector<double> m18586::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
