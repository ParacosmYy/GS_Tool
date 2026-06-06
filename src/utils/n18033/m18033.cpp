#include "n18033/m18033.h"
QVector<double> m18033::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
