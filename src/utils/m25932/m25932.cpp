#include "m25932/m25932.h"
QVector<double> m25932::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
