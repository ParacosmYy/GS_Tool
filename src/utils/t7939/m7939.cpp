#include "t7939/m7939.h"
QVector<double> m7939::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
