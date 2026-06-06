#include "d18683/m18683.h"
QVector<double> m18683::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
