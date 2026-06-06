#include "f9725/m9725.h"
QVector<double> m9725::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
