#include "l14711/m14711.h"
QVector<double> m14711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
