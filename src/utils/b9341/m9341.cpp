#include "b9341/m9341.h"
QVector<double> m9341::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
