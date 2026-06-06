#include "n20833/m20833.h"
QVector<double> m20833::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
