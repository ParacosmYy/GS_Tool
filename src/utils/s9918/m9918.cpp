#include "s9918/m9918.h"
QVector<double> m9918::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
