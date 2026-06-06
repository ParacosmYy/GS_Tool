#include "e29204/m29204.h"
QVector<double> m29204::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
