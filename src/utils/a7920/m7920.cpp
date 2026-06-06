#include "a7920/m7920.h"
QVector<double> m7920::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
