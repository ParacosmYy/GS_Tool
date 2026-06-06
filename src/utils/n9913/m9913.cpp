#include "n9913/m9913.h"
QVector<double> m9913::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
