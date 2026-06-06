#include "a26880/m26880.h"
QVector<double> m26880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
