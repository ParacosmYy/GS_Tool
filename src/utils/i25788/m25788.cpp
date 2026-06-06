#include "i25788/m25788.h"
QVector<double> m25788::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
