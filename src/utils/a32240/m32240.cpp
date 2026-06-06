#include "a32240/m32240.h"
QVector<double> m32240::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
