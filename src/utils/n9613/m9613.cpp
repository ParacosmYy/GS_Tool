#include "n9613/m9613.h"
QVector<double> m9613::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
