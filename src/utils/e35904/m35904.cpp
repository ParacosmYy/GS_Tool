#include "e35904/m35904.h"
QVector<double> m35904::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
