#include "i25448/m25448.h"
QVector<double> m25448::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
