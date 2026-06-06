#include "b18341/m18341.h"
QVector<double> m18341::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
