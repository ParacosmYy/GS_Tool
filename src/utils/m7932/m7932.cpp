#include "m7932/m7932.h"
QVector<double> m7932::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
