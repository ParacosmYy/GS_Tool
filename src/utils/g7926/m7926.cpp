#include "g7926/m7926.h"
QVector<double> m7926::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
