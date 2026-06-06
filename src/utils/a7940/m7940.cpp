#include "a7940/m7940.h"
QVector<double> m7940::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
