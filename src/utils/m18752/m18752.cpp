#include "m18752/m18752.h"
QVector<double> m18752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
