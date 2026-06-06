#include "p7835/m7835.h"
QVector<double> m7835::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
