#include "n9033/m9033.h"
QVector<double> m9033::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
