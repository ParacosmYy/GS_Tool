#include "a32000/m32000.h"
QVector<double> m32000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
