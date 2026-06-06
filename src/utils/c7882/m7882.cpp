#include "c7882/m7882.h"
QVector<double> m7882::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
