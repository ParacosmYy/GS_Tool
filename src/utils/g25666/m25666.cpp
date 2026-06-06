#include "g25666/m25666.h"
QVector<double> m25666::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
