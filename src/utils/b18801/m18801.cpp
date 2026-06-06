#include "b18801/m18801.h"
QVector<double> m18801::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
