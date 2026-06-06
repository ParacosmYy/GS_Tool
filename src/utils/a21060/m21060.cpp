#include "a21060/m21060.h"
QVector<double> m21060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
