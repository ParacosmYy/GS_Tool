#include "m32592/m32592.h"
QVector<double> m32592::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
