#include "d9683/m9683.h"
QVector<double> m9683::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
