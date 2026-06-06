#include "k13550/m13550.h"
QVector<double> m13550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
