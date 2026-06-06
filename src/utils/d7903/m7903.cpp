#include "d7903/m7903.h"
QVector<double> m7903::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
