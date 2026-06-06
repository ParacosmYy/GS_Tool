#include "o16494/m16494.h"
QVector<double> m16494::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
