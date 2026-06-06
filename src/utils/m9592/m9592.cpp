#include "m9592/m9592.h"
QVector<double> m9592::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
