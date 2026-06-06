#include "b25801/m25801.h"
QVector<double> m25801::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
