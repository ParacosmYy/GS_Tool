#include "b16341/m16341.h"
QVector<double> m16341::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
