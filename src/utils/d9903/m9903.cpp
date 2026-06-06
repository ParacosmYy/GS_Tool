#include "d9903/m9903.h"
QVector<double> m9903::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
