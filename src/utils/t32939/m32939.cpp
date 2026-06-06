#include "t32939/m32939.h"
QVector<double> m32939::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
