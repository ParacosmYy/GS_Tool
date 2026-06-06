#include "e12864/m12864.h"
QVector<double> m12864::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
