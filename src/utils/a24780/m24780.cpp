#include "a24780/m24780.h"
QVector<double> m24780::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
